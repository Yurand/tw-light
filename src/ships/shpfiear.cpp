/* $Id: shpfiear.cpp,v 1.17 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"
#include "../frame.h"
#include "../melee/mview.h"
REGISTER_FILE

class FierasShotSpark : public SpaceLine
{
	public:
		int lifetime, lifetime_max, r, g, b;

	public:

		FierasShotSpark (SpaceLocation *creator, double oangle, double ov,
			int olifetime, int blah_or, int og, int ob);
		virtual void calculate();
		virtual void animate(Frame *space);
};

class FierasShot : public Missile
{
	public:
		int spark_counter;
		bool track_directly;

	public:
		double turning;
		double rotational_intertia;
		int tracking;
		FierasShot(double ox, double oy, double oangle, double ov, int odamage,
			int oarmour, double mass, double turning, double range, Ship *oship, SpaceSprite *osprite,
			int ofcount, int ofsize, bool otd=false);

		virtual void calculate();
};

class FierasArbiter : public Ship
{
	public:
	public:
		double       weaponTurning;
		double       weaponVelocity;
		double       weaponMass;
		int          weaponDamage;
		int          weaponArmour;
		int          weaponFired;
		double       weaponRange;
		FierasShot   *weaponObject;
		bool         weaponTrackDirectly;

		double      specialRange, specialStartRange;
		double      specialVelocity;
		int         specialNumber;
		double      specialSpeedLimitFactor;
		double      specialBounceFactor, specialPlanetBounceFactor, specialMassFactor;
		bool        specialChangeOwner;

		double      repulsor_radius;

	public:
		FierasArbiter(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual SpaceLocation * get_focus();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void animate(Frame *frame);
};

SpaceLocation *FierasArbiter::get_focus()
{
	STACKTRACE;
	if (weaponObject) return weaponObject;
	else return this;
}


void FierasArbiter::animate(Frame *frame)
{
	STACKTRACE;
	Ship::animate(frame);

	//////////////////
	int circle_r;
	Vector2 p0 = corner(pos,0);
	int circle_x0 = iround(p0.x);
	int circle_y0 = iround(p0.y);

	int i;

	if (repulsor_radius > 0) {
		double rc = repulsor_radius/specialRange;
		rc *= rc*rc;
		double r2 = sqrt(space_zoom);
		if (r2 > 1) r2 = 1;
		tw_drawing_mode(TW_DRAW_MODE_TRANS, NULL, 0, 0);

		for (i=-specialNumber+1;i<=0;i++) {
			tw_set_trans_blender(0,0,0, iround(r2*(1-rc)*(specialNumber+i)*255.0/(specialNumber)));
			circle_r = iround((repulsor_radius+4*i)*space_zoom);
			tw_circle(frame->surface,circle_x0,circle_y0, circle_r, tw_makecol(100,100,255));
			frame->add_circle(circle_x0, circle_y0, circle_r, 0);
		}
		//		space->add_box(circle_x0-circle_r-1, circle_y0-circle_r-1, 2*circle_r+2, 2*circle_r+2);

		tw_drawing_mode(TW_DRAW_MODE_SOLID, NULL, 0, 0);
	}

}


FierasArbiter::FierasArbiter(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 1);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 1);
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 100));
	weaponTurning  = scale_turning(tw_get_config_float("Weapon", "TurnRate", 2));
	weaponMass     = tw_get_config_float("Weapon", "Mass", 0);
	weaponRange    = scale_range(tw_get_config_int("Weapon", "Range", 15));
	weaponFired    = FALSE;
	weaponObject   = NULL;

	weaponTrackDirectly = (tw_get_config_int("Weapon", "TrackDirectly", 0) > 0);

	specialRange        = scale_range(tw_get_config_float("Special", "Range", 0));
	specialStartRange   = scale_range(tw_get_config_float("Special", "StartRange", 0));
	specialVelocity     = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialNumber       = tw_get_config_int("Special", "Number", 0);
	specialSpeedLimitFactor = tw_get_config_float("Special", "SpeedLimitFactor", 1.0);
	specialChangeOwner  = (tw_get_config_int("Special", "ChangeOwner", 0) != 0);
	specialBounceFactor = tw_get_config_float("Special", "BounceFactor", 1.0);
	specialPlanetBounceFactor = tw_get_config_float("Special", "PlanetBounceFactor", 1.0);
	specialMassFactor   = tw_get_config_float("Special", "MassFactor", 1.0);

	repulsor_radius = -1;

}


int FierasArbiter::activate_weapon()
{
	STACKTRACE;
	if (weaponFired)
		return(FALSE);
	weaponObject = new FierasShot(0.0, (size.y / 2.0), angle,
		weaponVelocity, weaponDamage, weaponArmour,
		weaponMass, weaponTurning, weaponRange, this,
		data->spriteWeapon, 64, 50, weaponTrackDirectly);
	game->add(weaponObject);
	weaponFired = TRUE;
	return(TRUE);
}


int FierasArbiter::activate_special()
{
	STACKTRACE;
	repulsor_radius = specialStartRange;
	return(TRUE);
}


void FierasArbiter::calculate()
{
	STACKTRACE;
	if ((weaponObject != NULL) && (!weaponObject->exists()))
		weaponObject = NULL;
	if (weaponFired && (!fire_weapon))
		weaponFired = FALSE;

	if ((weaponObject != NULL) && (!fire_weapon)) {
		weaponObject->tracking = false;
		weaponObject = NULL;
	}

	if (repulsor_radius > 0) {

		double nv, k, r, ta;
		SpaceObject *o;

		Vector2 ov, tv;

		Query q;
		for (q.begin(this, OBJECT_LAYERS, repulsor_radius); q.currento; q.next()) {
			if (!q.current->isObject())
				continue;
			o = q.currento;
			if (!o->sameShip(this)) {
				ta = trajectory_angle(o);
				r = distance(o);

				// fix GEO.
				// narool poison has the same location as the ship, resulting in 0 distance
				// note: new fix in the unit-vector routine ...
				if (pos == o->pos)
					continue;

				tv = unit_vector(min_delta(o->pos, pos));
				ov = o->get_vel();
				nv = dot_product(ov, tv);
				double rc = repulsor_radius/specialRange;
				rc *= rc*rc;
				rc = 1-rc;
				if (o->isPlanet()) {
					this->translate(tv*(r-repulsor_radius));
					nv -= specialVelocity*specialPlanetBounceFactor*rc;
					if (nv < 0)
						this->accelerate(this, 2*tv*nv, MAX_SPEED*specialSpeedLimitFactor);
				} else {
					o->translate(tv*(repulsor_radius-r));
					nv -= specialVelocity*specialBounceFactor*rc;

					if (nv < 0) {
						if (o->mass > 0 && mass > 0 && specialMassFactor > 0)
							k = o->mass / (mass*specialMassFactor);
						else
							k = 0;
						o->accelerate(this, -2*tv*nv/(k+1), MAX_SPEED*specialSpeedLimitFactor);
						this->accelerate(this, 2*tv*nv*k/(k+1), MAX_SPEED*specialSpeedLimitFactor);

						if (o->isShot()) {
							ov += vel - 2*nv*tv;
							((Shot*)o)->changeDirection(atan(ov));
							nv = dot_product(o->get_vel(), tv);
							nv -= specialVelocity*specialBounceFactor*rc;
							if (nv < 0)
								((Shot*)o)->changeDirection(ta);
							if (specialChangeOwner)
								((Shot*)o)->change_owner(this);
						}
					}
				}
			}
		}
		q.end();

		repulsor_radius += specialVelocity * frame_time;
		if (repulsor_radius > specialRange)
			repulsor_radius = -1;
	}

	Ship::calculate();
}


FierasShot::FierasShot(double ox, double oy, double oangle, double ov,
int odamage, int oarmour, double _mass, double _turning, double range, Ship *oship, SpaceSprite *osprite,
int ofcount, int ofsize, bool otd) :
Missile(oship, Vector2(ox,oy), oangle, ov, odamage, range, oarmour, oship,
osprite)
{
	STACKTRACE;
	track_directly = otd;
	rotational_intertia = 0;
	tracking = true;
	turning = _turning;
	mass = _mass;
	collide_flag_sameship = bit(LAYER_SPECIAL);
	//	explosionSprite     = data->spriteWeaponExplosion;
	//	explosionFrameCount = 31;
	//	explosionFrameSize  = 50;
	spark_counter = 0;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void FierasShot::calculate()
{
	STACKTRACE;
	sprite_index = (get_index(angle) + (0*64));
	Missile::calculate();
	sprite_index = (get_index(angle) + (0*64));
	if (tracking) {
		if (track_directly) {
			if (ship)
				changeDirection(ship->angle);
			d = 0;
		} else {
			double da = 0;
			if (ship) {
				if (ship->turn_left) da -= 1;
				if (ship->turn_right) da += 1;
			}
			rotational_intertia += da * frame_time / 20.0;
			rotational_intertia *= 1 - 0.002 * frame_time;
			changeDirection(angle + turning * rotational_intertia);
			d = 0;
		}
	}

	while (spark_counter <= 0) {
		spark_counter += 25;
		game->add(new FierasShotSpark(this, tw_random(PI2),
			scale_velocity(5)*tw_random(1.0),
			iround(4800*(1-0.5*(tw_random(1.0)))),
			160+tw_random(50), 160+tw_random(50), 230+tw_random(25) ));
	}
	spark_counter -= frame_time;
}


/*void FierasShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
  Shot::inflict_damage(other);
	if (other->mass <= 0) return;
  if (other->isShip())
	game->add(new FixedAnimation(this, other,
	  explosionSprite, 0, explosionFrameCount,
			explosionFrameSize, DEPTH_EXPLOSIONS));
  else
	game->add(new Animation(this, x, y,
	  explosionSprite, 0, explosionFrameCount,
			explosionFrameSize, DEPTH_EXPLOSIONS));
}*/

FierasShotSpark::FierasShotSpark (SpaceLocation *creator, double oangle, double ov, int olifetime,
int blah_or, int og, int ob) :
SpaceLine(creator, 0, oangle, 0, 0),
lifetime(olifetime), lifetime_max(olifetime),
r(blah_or), g(og), b(ob)
{
	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;

	color = tw_makecol(r, g, b);
	set_depth(DEPTH_HOTSPOTS);

	double alpha;
	alpha = (creator->get_angle());

	pos = creator->normal_pos();

	vel = ov * unit_vector(angle);

}


void FierasShotSpark::calculate()
{
	STACKTRACE;
	lifetime -= frame_time;
	if (lifetime <= 0)
		die();

	SpaceLine::calculate();
}


void FierasShotSpark::animate(Frame *space)
{
	STACKTRACE;
	tw_drawing_mode(TW_DRAW_MODE_TRANS, NULL, 0, 0);

	double c = lifetime/(double)lifetime_max;

	int x0, y0;

	Vector2 p0 = corner(pos);
	x0 = iround(p0.x);
	y0 = iround(p0.y);

	if (space_zoom <= 1)
		tw_set_trans_blender(0, 0, 0, iround(space_zoom * 255 * c));
	else
		tw_set_trans_blender(0, 0, 0, iround(1 * 255 * c));

	tw_putpixel(space->surface, x0, y0, color);
	space->add_pixel(x0, y0);

	tw_drawing_mode(TW_DRAW_MODE_SOLID, NULL, 0, 0);
}


REGISTER_SHIP(FierasArbiter)
