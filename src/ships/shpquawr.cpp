/* $Id: shpquawr.cpp,v 1.15 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
//#include "../melee/mview.h"

class QuarKathWraith : public Ship
{
	int frame;
	int thrustActive;
	int thrustForward;
	int segments;
	int segment_length, segment_dispersion;
	int rnd_angle, aiming, dispersion;

	double       specialRange;
	double       specialVelocity;
	int          specialDamage;
	int          specialArmour;
	double       specialTurnRate;

	int cloak;
	int cloak_frame;

	int extraChanceToHit;

	int sprite_index2;

	public:

		static int cloak_color[3];

		QuarKathWraith(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		Color crewPanelColor(int k = 0);

		virtual double isInvisible() const;
		virtual void calculate_hotspots();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual int  canCollide(SpaceLocation *other);
		virtual void animate(Frame *space);
};

class QuarKathIllusion  : public HomingMissile
{
	int cloak;
	int cloak_frame;

	public:

		static int cloak_color[3];

		QuarKathIllusion (Vector2 opos, double oangle, double ov,
			int odamage, double orange, int oarmour, double otrate, Ship *oship,
			SpaceSprite *osprite);
		virtual int  canCollide(SpaceLocation *other);
		virtual void calculate();
		virtual void animate(Frame *space);
};

int QuarKathIllusion ::cloak_color[3] = { 15, 10, 2 };

class QuarKathLightning : public Laser
{
	int level;
	SpaceLocation *target;
	Ship *ship;
	double base_length, aiming, dispersion;

	public:
		QuarKathLightning(Ship *lship, SpaceLocation *lroot, SpaceLocation *ltarget,
			int llevel, int b_length, int r_length,
			int r_angle, int oaiming, int odispersion);

		virtual void calculate();
};

int QuarKathWraith::cloak_color[3] = { 15, 10, 2 };

QuarKathWraith::QuarKathWraith(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code),
frame(0),
thrustActive(FALSE),
thrustForward(TRUE)
{
	STACKTRACE;
	cloak = FALSE;
	cloak_frame = 0;

	segment_length            = get_config_int("Weapon", "SegmentLength",0);
	segment_dispersion    = get_config_int("Weapon", "SegmentLengthDispersion",0);
	segments                      = get_config_int("Weapon", "Segments", 0);
	rnd_angle                     = iround(get_config_int("Weapon", "RandomAngle", 0) * ANGLE_RATIO);
	aiming                            = get_config_int("Weapon", "Aiming", 0);
	dispersion                    = get_config_int("Weapon", "Dispersion", 0);

	specialRange                = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity           = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage             = get_config_int("Special", "Damage", 0);
	specialArmour             = get_config_int("Special", "Armour", 0);
	specialTurnRate           = scale_turning(get_config_float("Special", "TurnRate", 0));

	extraChanceToHit    = get_config_int("Extra", "ChanceToHit", 0);
}


Color QuarKathWraith::crewPanelColor(int k)
{
	STACKTRACE;
	Color c = {64,64,64};
	return c;
}


double QuarKathWraith::isInvisible() const
{
	if (cloak_frame >= 300) return(1);
	return 0;
}


void QuarKathWraith::calculate()
{
	STACKTRACE;
	if (batt == batt_max) cloak=true;
	else cloak=false;

	if ((cloak) && (cloak_frame < 300))
		cloak_frame+= frame_time;
	if ((!cloak) && (cloak_frame > 0))
		cloak_frame-= frame_time;

	Ship::calculate();

}


int QuarKathWraith::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	/* GEO: removes partial damage
	int randomNumber = random() % 100;
	if (cloak) {
		if (other->isShot()) {
				// message.print(100,5,"(%6.2d)",randomNumber);
				if (randomNumber <= extraChanceToHit) return true;
					return false;
			} else return false;
		}
	else */
	return Ship::canCollide(other);
}


void QuarKathWraith::animate(Frame *space)
{
	STACKTRACE;
	if ((cloak_frame > 0) && (cloak_frame < 300))
		sprite->animate_character( pos,
			sprite_index, pallete_color[cloak_color[(int)(cloak_frame / 100)]], space);
	else
	if ((cloak_frame >= 300))
		sprite->animate_character( pos,
				sprite_index, pallete_color[0], space);
	else
		Ship::animate(space);
}


void QuarKathWraith::calculate_hotspots()
{
	STACKTRACE;
	if (!cloak)
		Ship::calculate_hotspots();

}


int QuarKathWraith::activate_weapon()
{
	STACKTRACE;

	SpaceLocation *t = NULL;
	double r = 99999;
	int i;
	for (i = 0; i < targets->N; i += 1) {
		if (control->valid_target(targets->item[i]) && (distance(targets->item[i]) < r)) {
			t = targets->item[i];
			r = distance(t);
		}
	}
	add(new QuarKathLightning( this, this, t, segments, segment_length,
		segment_dispersion, rnd_angle, aiming, dispersion));

	return(TRUE);
}


int QuarKathWraith::activate_special()
{
	STACKTRACE;
	if (cloak) {
		add(new QuarKathIllusion(
			Vector2(0.0, get_size().y * 1.0), angle, specialVelocity,
			specialDamage, specialRange,
			specialArmour, specialTurnRate, this, data->spriteSpecial ));
		return(TRUE);
	} return(FALSE);

}


QuarKathIllusion::QuarKathIllusion(Vector2 opos, double oangle,
double ov, int odamage, double orange, int oarmour, double otrate,
Ship *oship, SpaceSprite *osprite) :
HomingMissile(oship, opos, oangle, ov, odamage, orange, oarmour, otrate,
oship, osprite, oship->target)
{
	STACKTRACE;
	cloak = FALSE;
	cloak_frame = 0;

	// it can't collide, therefore set:
	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;
	mass = 0;
}


void QuarKathIllusion::animate(Frame *space)
{
	STACKTRACE;
	if (!cloak)
		sprite->animate_character( pos,
			sprite_index, pallete_color[cloak_color[(int)(cloak_frame / 100)]], space);
	else    sprite->animate_character( pos,
			sprite_index, pallete_color[0], space);
}


void QuarKathIllusion::calculate()
{
	STACKTRACE;
	if ( !(target && target->exists() && ship && ship->exists()) ) {
		state = 0;
		target = 0;
		return;
	}

	if (target && target->exists()) {
		if (distance(target) < range * .05 ) cloak=false;
		else cloak=true;
	}

	if ((cloak) && (cloak_frame < 300))
		cloak_frame+= frame_time;
	if ((!cloak) && (cloak_frame > 0))
		cloak_frame-= frame_time;

	HomingMissile::calculate();

}


// GEO: this is not a perfect test, since sometimes other ships perform the test,
// so you get other->cancollide(this) ; this ship has no control over that with this
// routine.
int QuarKathIllusion::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	return false;
}


QuarKathLightning::QuarKathLightning(Ship *lship, SpaceLocation *lroot,
SpaceLocation *ltarget, int llevel, int b_length, int r_length,
int r_angle, int oaiming, int odispersion) :
Laser(lship, 0, pallete_color[15], 0.0, 1, 300, lroot, Vector2(0.0, 0.0)),
level(llevel), target(ltarget), ship(lship),
aiming(oaiming), dispersion(odispersion)
{
	STACKTRACE;

	base_length = (random()%(2*r_length+1) - r_length) + b_length;
	//	x = pos->normal_x();
	//	y = pos->normal_y();
	pos = lpos->normal_pos();	 // lpos is of class Laser
	if (target) angle = trajectory_angle(target);
	else angle = random(PI2);
	if (ship != lpos) {
		//		x += ((QuarKathLightning *)(pos))->edge_x();
		//		y += ((QuarKathLightning *)(pos))->edge_y();
		pos += ((QuarKathLightning *)(lpos))->edge();
	}
	angle += (random(r_angle*2+1)) - r_angle;
	color = pallete_color[179 - level * 2 - (random(8))];
	if (level) add(new QuarKathLightning(ship, this, target, level-1, b_length,
			r_length, r_angle, iround(aiming), iround(dispersion)));
}


void QuarKathLightning::calculate()
{
	STACKTRACE;
	Laser::calculate();

	if (frame < (frame_count/2)) length = base_length * (frame) / frame_count;
	else length = base_length * (frame_count-frame) / frame_count;
	if (!lpos->exists()) return;
	//	x = pos->normal_x();
	//	y = pos->normal_y();
	pos = lpos->normal_pos();
	if (ship != lpos) {
		//		x += ((QuarKathLightning *)(pos))->edge_x();
		//		y += ((QuarKathLightning *)(pos))->edge_y();
		pos += ((QuarKathLightning *)(lpos))->edge();
	}
	if (!target) return;
	double d_a = normalize(trajectory_angle(target) - angle, PI2);
	if (d_a > PI) d_a -= PI2;
	if (d_a > 0)
		angle += (ANGLE_RATIO * aiming + ANGLE_RATIO * dispersion*(-50+random()%101) / sqrt((double)frame_time)) * frame_time / 100.0;
	else
		angle -= (ANGLE_RATIO * aiming + ANGLE_RATIO * dispersion*(-50+random()%101) / sqrt((double)frame_time)) * frame_time / 100.0;
	if (!target->exists()) target = NULL;
	return;
}


REGISTER_SHIP(QuarKathWraith)
