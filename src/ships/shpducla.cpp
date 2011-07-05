/* $Id: shpducla.cpp,v 1.23 2007/04/16 23:55:32 yurand Exp $ */
/*
Ducly Lanternjaws
Ducly LJ for friends ;)

*/

/*
ANGLE && COORDINATE CONVENTION:

on the screen,

  to the right, angle = 0
  clockwise, angle increases, so
  bottom: angle > 0
  top: angle < 0

  for the vectors:
  (0,0) is top-left of the screen !
  x > 0 points to the right
  y > 0 points _down_ (which is the screen coordinate system)
*/

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class ShipPart;
class LaserArc;
class Lantern;
class ShipPartManager;

class DuclyLanternjaws : public Ship
{
	public:
	public:

		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		double       weaponArmour;

		double       specialRange;
		double       specialVelocity;
		double       specialArmour;
		double       specialDamage;

		double    lanternAccel, lanternIntensity, lanternIntensityIncrease,
			lanternMaxRadius, lanternMaxDensity, lanternLaserSpread;
		double    lanternDamage;
		int       lanternColorR, lanternColorG, lanternColorB, lanternColorRandomAdd;

		int       arm_movement;
		double    arm_angle, arm_time, arm_period, arm_maxangle;
		double    laser_grow_time, arc_corr;

		ShipPart      *ArmLeft, *ArmRight, *JawLeft, *JawRight;
		LaserArc      *ArcLeft, *ArcRight;
		Lantern       *lantern;
		ShipPartManager   *ManageParts;

		DuclyLanternjaws(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
};

class SpaceLineArc : public SpaceLine
{
	public:
		LaserArc    *mother;
	public:
		SpaceLineArc(LaserArc *creator, Vector2 lpos, double langle,
			double llength, int lcolor);

		virtual void calculate();
		void set_props(Vector2 opos, double oangle, double olength);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
		virtual void inflict_damage(SpaceObject *other);
};

// a growing laser arc.
class LaserArc : public SpaceLocation
{
	public:
		DuclyLanternjaws *mother;

	public:
		Vector2     relpos;
		double      angle_min, angle_max, R;
		SpaceLineArc ** laser_o;
		int         Nseg, laser_color;

		LaserArc(DuclyLanternjaws *creator, Vector2 lpos, int lcolor, int N);
		virtual ~LaserArc();

		virtual void calculate();
		//	virtual void inflict_damage(SpaceObject *other);

		void update_lasersegs();
};

class ShipPart : public SpaceObject
{
	public:
		void calc_angle();
								 // calculates it for this angle
		void calc_pos(Vector2 refpos);

		Ship    *mother;

	public:
		int     hascollided;
		int     ship_rotations;
		double  offset_angle;
		Vector2 offset_pos, pivot_point;
		Vector2 change_pos, change_vel;
		double damage_normal;

		ShipPart(Ship *creator, SpaceSprite *osprite,
			double oangle, Vector2 orelpos, Vector2 opivot, double omass);

		virtual void calculate();// should be empty
		virtual void animate(Frame *space);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
		virtual void inflict_damage(SpaceObject *other);
		virtual bool die();

								 // add minor detection part
		virtual void collide(SpaceObject *other);
		int collide_SpaceObject(SpaceObject *other);

		void calculate_manager(Vector2 refpos, Vector2 refvel);
		// should do the real calculations to allow syncing with the mothership
};

class Lantern : public SpaceLocation
{
	public:
		DuclyLanternjaws *mother;

	public:
		double      maxradius, maxdensity;
								 // 0.0 - 1.0.
		double      intensity, int_decrease;
		Vector2     relpos;
		double      relangle, laserspread, weaponAccel;

		Lantern(DuclyLanternjaws *creator, Vector2 lpos, double langle);

		void calculate();
};

// copied from RogueSquadron
class PulseLaser2 : public SpaceLine
{
	public:
	protected:
		double frame;
		double frame_count;

		SpaceLocation *lpos;
		Vector2 rel_pos;

	public:
		PulseLaser2(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage, int lfcount,
			SpaceLocation *opos, Vector2 rpos = Vector2(0,0), double rvelocity = 0.0);

		void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

// uses the ship_part function calculate_manager()
static const int MaxParts = 100;
class ShipPartManager : public Presence
{
	public:
		ShipPart    *partlist[MaxParts];
		int         Nparts;
		Ship        *mother;
	public:
		Vector2     oldpos, oldvel;

		ShipPartManager(Ship *creator);

		void add_part(ShipPart *newpart);

								 // empty
		virtual void calculate() {
		};

		void calculate_manager();// this:
		// should be called immediately following Ship::calculate function of the mother,
		// since it could add additional changes to pos, vel based on "experiences" of
		// the ship parts (read: collisions !).

};

ShipPartManager::ShipPartManager(Ship *creator)
{
	STACKTRACE;
	int i;
	for ( i = 0; i < MaxParts; ++i )
		partlist[i] = 0;
	Nparts = 0;

	mother = creator;
	oldpos = mother->pos;
	oldvel = mother->vel;
}


void ShipPartManager::add_part(ShipPart *newpart)
{
	STACKTRACE;
	partlist[Nparts] = newpart;
	++Nparts;
}


void ShipPartManager::calculate_manager()
{
	STACKTRACE;
	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;

		int i;
		for ( i = 0; i < Nparts; ++i )
			partlist[i]->calculate_manager(oldpos, oldvel);

		return;
	}

	int i;
	for ( i = 0; i < Nparts; ++i ) {
		if (!partlist[i]->exists()) {
			tw_error("A ship part should not die!");
		}

		partlist[i]->calculate_manager(oldpos, oldvel);

								 // important - this should override pos/vel settings
		if ( partlist[i]->hascollided ) {
			mother->pos = oldpos + partlist[i]->change_pos;
			mother->set_vel ( oldvel + partlist[i]->change_vel );
		}

		// reset collision information BUT note, that this is handled separately by the physics...
		// (I suppose that's for synching reasons, so that all objects are stationary while
		// they're being checked). That's why I reset it here, just after it's been used.
		// I guess it's not very neat, but well, it's done by the manager responsible for it :)

		partlist[i]->hascollided = 0;
		partlist[i]->change_pos = 0;
		partlist[i]->change_vel = 0;
	}

	//	oldpos = mother->pos;
	//	oldvel = mother->vel;

}


void ShipPart::calc_angle()
{
	STACKTRACE;
	angle = mother->angle + offset_angle;
	while (angle < 0  ) angle += PI2;
	while (angle > PI2) angle -= PI2;
	// the angle should be between 0 and 2PI
}


void ShipPart::calc_pos(Vector2 refpos)
{
	STACKTRACE;
	pos = refpos + rotate(offset_pos, mother->angle) - rotate(pivot_point, angle);
}


ShipPart::ShipPart(Ship *creator, SpaceSprite *osprite,
double oangle, Vector2 orelpos, Vector2 opivot, double omass)
:
SpaceObject(creator, creator->pos, oangle, osprite),
mother(creator)
{
	STACKTRACE;

	layer = LAYER_SHIPS;

	// angle relative to the mother ship (sprite angle = mother angle + rel. angle)
	offset_angle = oangle;

	// the point relative to center of this sprite, rotation is around this point
	// The pi/2 rotation is needed, since the vectors are declared along y as if angle=0 there,
	// instead angle=0 along x.
	pivot_point = rotate(opivot, PI/2);

	// position of the pivot point relative to mothership central position
	offset_pos = rotate(orelpos, PI/2);

	// just here so that you can override this, if needed.
	ship_rotations = 64;

	// mass, should be non-zero to allow for collisions; the bigger the mass, the less
	// it'll "move" out of position due to collisions
	//	mass = omass;
	mass = ship->mass;			 // might be better ?

	damage_normal = 0;			 // unless you override this externally

	// ok, override earlier settings now
	calc_angle();
	calc_pos(mother->pos);

	collide_flag_anyone = mother->collide_flag_anyone;
	collide_flag_sameteam = mother->collide_flag_sameteam;
	collide_flag_sameship = 0;

	hascollided = 0;
	change_pos = 0;
	change_vel = 0;
}


void ShipPart::calculate()
{
	STACKTRACE;
	// this should be checked here, too, cause the manager can die as well; if the
	// parts aren't dead by then, the check will never be made otherwise !!
	if (!(mother && mother->exists())) {
		mother = 0;
		state = 0;
		return;
	}
}


void ShipPart::calculate_manager(Vector2 refpos, Vector2 refvel)
{
	STACKTRACE;

	if (!(mother && mother->exists())) {
		mother = 0;
		state = 0;
		return;
	}

	calc_angle();				 // update (angle) using mother settings
	calc_pos(refpos);			 // update (pos) using manager settings
	vel = refvel;

	//sprite_index = get_index(angle);

	SpaceObject::calculate();

}


bool ShipPart::die()
{
	STACKTRACE;
	// do nothing
	return false;
}


void ShipPart::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

	damage(other, damage_normal);
}


int ShipPart::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	if (mother && mother->exists()) {
		state = 1;
		return mother->handle_damage(source, normal, direct);
	} else {
		mother = 0;
		state = 0;
		return 0;
	}
}


void ShipPart::animate(Frame *space)
{
	STACKTRACE;

	SpaceObject::animate(space);
}


// an exact copy, with a small modification: it also return a boolean value
int ShipPart::collide_SpaceObject(SpaceObject *other)
{
	STACKTRACE;
	//	double dx, dy;
	//	double dvx, dvy;
	double tmp;

	//	int x1, y1;
	//	int x2, y2;

	if (this == other) {tw_error("SpaceObject::collide - self!");}
	if ((!canCollide(other)) || (!other->canCollide(this))) return 0;
	if (!exists() || !other->exists()) return 0;

	pos = normal_pos();

	Vector2 p1, p2, dp, dv;

	p1 = pos;
	p2 = other->normal_pos();
	dp.x = min_delta(p1.x, p2.x, map_size.x);
	dp.y = min_delta(p1.y, p2.y, map_size.y);
	p2 = p1 - dp - other->size / 2;
	p1 = p1 - size / 2;

	/*	x1 = (int)(normal_x() - (w / 2.0));
		y1 = (int)(normal_y() - (h / 2.0));
		dx = min_delta(normal_x(), other->normal_x(), map_size.x);
		dy = min_delta(normal_y(), other->normal_y(), Y_MAX);
		x2 = (int)(normal_x() - dx - ((other->w) / 2.0));
		y2 = (int)(normal_y() - dy - ((other->h) / 2.0));*/

	if (!sprite->collide(iround(p1.x), iround(p1.y), sprite_index, iround(p2.x), iround(p2.y),
		other->get_sprite_index(), other->get_sprite() ))
		return 0;
	//sprite->collide(x1, y1, sprite_index, x2, y2, other->sprite_index, other->sprite);

	inflict_damage(other);
	other->inflict_damage(this);

	if (!mass || !other->mass) return 0;

	dv = vel - other->vel;

	p1 = pos;
	p2 = other->normal_pos();
	dp.x = min_delta(p1.x, p2.x, map_size.x);
	dp.y = min_delta(p1.y, p2.y, map_size.y);
	p2 = p1 - dp - other->size / 2;
	p1 = p1 - size / 2;

	/*x1 = (int)(normal_x() - (w / 2.0));
	y1 = (int)(normal_y() - (h / 2.0));
	dx = min_delta(normal_x(), other->normal_x(), map_size.x);
	dy = min_delta(normal_y(), other->normal_y(), Y_MAX);
	x2 = (int)(normal_x() - dx - ((other->w) / 2.0));
	y2 = (int)(normal_y() - dy - ((other->h) / 2.0));*/

	while ((dp.x == 0) && (dp.y == 0)) {
		dp.x = (tw_random(5) - 2) / 99.0;
		dp.y = (tw_random(5) - 2) / 99.0;
	}

	Vector2 _dp = unit_vector(dp);
	tmp = dot_product(dv, _dp);
	tmp = ( -2 * tmp );

	if (mass + other->mass > 1)
		tmp = tmp * (mass * other->mass) / (mass + other->mass);

	if (tmp >= 0) {
		if (mass > 1)
			vel += _dp * tmp / mass;
		if (other->mass > 1)
			other->change_vel (- _dp * tmp / other->mass );
	}

	Vector2 nd;
	nd = unit_vector(dp);

	if (mass + other->mass > 1)
		nd /= (mass + other->mass);

	while (sprite->collide(iround(p1.x), iround(p1.y), sprite_index, iround(p2.x), iround(p2.y),
	other->get_sprite_index(), other->get_sprite() )) {
		pos = normalize(pos + nd * other->mass);
		other->pos = normalize(other->pos - nd * mass);

		p1 = pos;
		p2 = other->normal_pos();
		dp.x = min_delta(p1.x, p2.x, map_size.x);
		dp.y = min_delta(p1.y, p2.y, map_size.y);
		p2 = p1 - dp - other->size / 2;
		p1 = p1 - size / 2;
	}

	#ifdef _DEBUG
	SpaceObject *c1 = this;
	SpaceObject *c2 = other;
	if (fabs(c1->vel.x) > 1E6 || fabs(c1->vel.y) > 1E6 || fabs(c2->vel.x) > 1E6 || fabs(c2->vel.y) > 1E6 ) {
		int a1 = c1->canCollide(c2);
		int a2 = c2->canCollide(c1);
		bool b = ((c1->canCollide(c2) & c2->canCollide(c1)) == 0 );
		tw_error("velocity error in collision involving objects [%s] and [%s]", c1->get_identity(), c2->get_identity());
	}
	#endif

	return 1;
}


void ShipPart::collide(SpaceObject *other)
{
	STACKTRACE;
	Vector2 oldpos, oldvel;

	oldpos = pos;
	oldvel = vel;

	// collision changes velocity and position
	int i;

	i = collide_SpaceObject(other);

	if ( i ) {
		if ( (pos - oldpos).magnitude() > change_pos.magnitude() ) {
			hascollided = 1;
			change_pos = pos - oldpos;
			change_vel = vel - oldvel;
		}
	}

}


DuclyLanternjaws::DuclyLanternjaws(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Special", "Damage", 0);
	weaponArmour   = tw_get_config_int("Special", "Armour", 0);

	specialRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	specialDamage   = tw_get_config_float("Weapon", "Damage", 0);
	specialArmour   = tw_get_config_int("Weapon", "Armour", 0);

	lanternAccel              = scale_acceleration(tw_get_config_float("Lantern", "Accel", 0));
	lanternIntensity          = tw_get_config_float("Lantern", "Intensity", 0);
	lanternIntensityIncrease  = tw_get_config_float("Lantern", "IntensityIncrease", 0);
	lanternMaxRadius          = tw_get_config_float("Lantern", "MaxRadius", 0);
	lanternMaxDensity         = tw_get_config_float("Lantern", "MaxDensity", 0);
	lanternLaserSpread        = tw_get_config_float("Lantern", "LaserSpread", 0);
	lanternLaserSpread *= ANGLE_RATIO;
	lanternDamage             = tw_get_config_float("Lantern", "Damage", 0);

	lanternColorR = tw_get_config_int("Lantern", "ColorR", 0);
	lanternColorG = tw_get_config_int("Lantern", "ColorG", 0);
	lanternColorB = tw_get_config_int("Lantern", "ColorB", 0);
	lanternColorRandomAdd = tw_get_config_int("Lantern", "ColorRandomAdd", 0);

	// create arms for this ship:

	arm_angle = 0;
	double m = this->mass;
	// -dy is up, +dy is down !
	ArmLeft  = new ShipPart(this, this->data->more_sprites[0], arm_angle, Vector2(-18,0), Vector2(0,50), m);

	ArmRight = new ShipPart(this, this->data->more_sprites[1], arm_angle, Vector2( 18,0), Vector2(0,50), m);

	add(ArmLeft);
	add(ArmRight);

	arm_movement = 0;
	arm_time = 0;
	//  arm_period = 10.0;
	//  arm_maxangle = 100 * ANGLE_RATIO;
	//  arc_corr = 30.0 * ANGLE_RATIO;
	arm_period = tw_get_config_float("Arms", "ArmPeriod", 0);
	arm_maxangle = tw_get_config_float("Arms", "ArmMaxAngle", 0) * ANGLE_RATIO;
	arc_corr = tw_get_config_float("Arms", "ArmArcCorr", 0) * ANGLE_RATIO;
	// arm_maxangle + arc_corr should be less than 90 degrees !

	//  laser_grow_time = 6.0;	// how long it takes to grow :)
	laser_grow_time = tw_get_config_float("Arms", "ArmLaserGrowTime", 0);

	ArcLeft = 0;
	ArcRight = 0;

	// also, add 2 small jaws in the front:

	JawLeft  = new ShipPart(this, this->data->more_sprites[2], 0.0, Vector2(-10,-40), Vector2(0,5), m);
	JawRight = new ShipPart(this, this->data->more_sprites[3], 0.0, Vector2( 10,-40), Vector2(0,5), m);

	add(JawLeft);
	add(JawRight);

	JawLeft->damage_normal = 1.0;// crunching time :)
	JawRight->damage_normal = 1.0;

	// add a lantern

	lantern = new Lantern(this, Vector2(0,40), 0);
	game->add(lantern);

	ManageParts = new ShipPartManager(this);
	game->add(ManageParts);

	ManageParts->add_part(ArmLeft);
	ManageParts->add_part(ArmRight);
	ManageParts->add_part(JawLeft);
	ManageParts->add_part(JawRight);
}


int DuclyLanternjaws::activate_special()
{
	STACKTRACE;

	if ( lantern->intensity < 0.9 )
		lantern->intensity += 0.1;
	else
		lantern->intensity = 1.0;

	return TRUE;
}


Vector2 pos_laser_left()
{
	STACKTRACE;
	return 0;
}


int DuclyLanternjaws::activate_weapon()
{
	STACKTRACE;

	if ( arm_movement )
		return TRUE;			 // always use some fuel !

	if ( batt < 4 )				 // using the special with little fuel is no use.
		return FALSE;

	arm_movement = 1;

	// also, add a growing laser - arc :)

	int N = 25;

	int col = tw_makecol(255,0,0);
	game->add( ArcLeft  = new LaserArc(this, Vector2(-20,-55), col, N) );
	col = tw_makecol(255,0,0);
	game->add( ArcRight = new LaserArc(this, Vector2( 20,-55), col, N) );

	return TRUE;
}


void DuclyLanternjaws::calculate()
{
	STACKTRACE;
	ManageParts->oldpos = pos;
	ManageParts->oldvel = vel;

	Ship::calculate();
								 // this can apply extra changes to this ship pos,vel !
	ManageParts->calculate_manager();

	if ( arm_movement ) {
		if ( arm_time < arm_period && fire_weapon && batt > 0 ) {
			arm_time += frame_time * 1E-3;
			arm_angle = (0.5 - 0.5*cos(arm_time * PI2 / arm_period)) * arm_maxangle;

			// update the arms:

								 // anti-cl.w.
			ArmLeft->offset_angle  = -arm_angle;
								 // clockwise
			ArmRight->offset_angle =  arm_angle;

			// update the range of the laser-arcs:
			// angle = 0 is along the ship axis; increasing angle goes clockwise

			// the update and arc should depend on the angle of the arms
			// R and amin must be calculated from the arm position, relative to the ship

			Vector2 apos1, apos2;
			apos1 = ArmRight->offset_pos;
								 // the whole arm-length, since relpos = at the base of the arm
			apos2 = apos1 + rotate(Vector2(100,-10), arm_angle);

			double a, R;
			//			acorr = 30 * ANGLE_RATIO;
								 // this can be negative, it's ok
			R = apos2.y / cos(arm_angle - arc_corr);
			a = arm_angle + PI/2 - arc_corr;

			double dx, dy;
			dx = R * sin(arm_angle - arc_corr) + apos2.x;
			dy = 0;

			double b;
			if ( arm_time < laser_grow_time )
				b = a * (1 - arm_time / laser_grow_time);
			else
				b = 0.0;

			ArcRight->angle_min = b;
			ArcRight->angle_max = a;
			ArcRight->R = R;
			ArcRight->relpos = Vector2( dx, dy);

			ArcLeft->angle_min = -a;
			ArcLeft->angle_max = -b;
			ArcLeft->R = R;
			ArcLeft->relpos = Vector2( dx, -dy);

		} else {
			arm_time = 0;
			arm_movement = 0;

								 // anti-cl.w.
			ArmLeft->offset_angle  = 0;
								 // clockwise
			ArmRight->offset_angle = 0;

			if ( ArcLeft && ArcLeft->exists() ) {
				ArcLeft->state = 0;
				ArcLeft = 0;
			}
			if ( ArcRight && ArcRight->exists() ) {
				ArcRight->state = 0;
				ArcRight = 0;
			}
		}
	}

	// always move the jaws :)

	double t, a, period1, period2, offset;
	t = game->game_time * 1E-3;

	period1 = 3.3;
	period2 = 5.0;
								 // in seconds
	offset = PI2 * sin(t * PI2 / period1);
	a = sin(offset + t * PI2 / period2);

	// this offset is the angle of the jaw, relative to the mother
	JawLeft->offset_angle  = -a;
	JawRight->offset_angle =  a;

}


LaserArc::LaserArc(DuclyLanternjaws *creator, Vector2 lpos, int lcolor, int N)
:
SpaceLocation(creator, creator->pos+lpos, 0.0)
{
	STACKTRACE;
	mother = creator;

	// this is just the manager; no physical presence
	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;

	relpos = rotate(lpos, PI/2);
	Nseg = N;
	laser_color = lcolor;

	// angles:
	// 0  = to the right of the center
	// 90 = below the center

	angle_min =    0 * ANGLE_RATIO;
	angle_max =   45 * ANGLE_RATIO;
	R = 500.0;

	pos = mother->pos + rotate(relpos, mother->angle);
	vel = mother->vel;

	// create the arc segments :

	laser_o = new SpaceLineArc* [Nseg];

	int i;
	for ( i = 0; i < Nseg; ++i ) {
		laser_o[i] = new SpaceLineArc(this, 0, 0, 0, laser_color);
		game->add(laser_o[i]);
								 // this is damage per second
		laser_o[i]->damage_factor = mother->specialDamage * frame_time*1E-3;
	}

	update_lasersegs();

}


LaserArc::~LaserArc()
{
	/* should not be here! Cause in a physics-delete operation, the objects that we refer to here,
	could already be dead! - geo  [and gosh, I wrote this myself..]
	int i;
	for ( i = 0; i < Nseg; ++i )
	{
		if ( laser_o[i] && laser_o[i]->exists() )
			laser_o[i]->state = 0;
	}
	*/

	delete[] laser_o;

}


void LaserArc::update_lasersegs()
{
	STACKTRACE;
	int i;
	double angle_step;

	angle_step = (angle_max - angle_min) / Nseg;

	for ( i = 0; i < Nseg; ++i ) {
		if ( laser_o[i] && !laser_o[i]->exists() )
			laser_o[i] = 0;

		if (!laser_o[i]) {
			tw_error("LaserArc: laser segment has died.");
		}

		double a, a1, a2, L;
		a1 = mother->angle + angle_min + i * angle_step;
		a2 = a1 + angle_step;

		Vector2 rpos1, rpos2;
		rpos1 = R * unit_vector(a1);
		rpos2 = R * unit_vector(a2);

		a = (rpos2 - rpos1).atan();
		L = (rpos2 - rpos1).magnitude();

		laser_o[i]->set_props(pos + rpos1, a, L);

		// just in case someone changes frame_time ?
		// also, it should depend on the laser seg ( the close they are together,
		// the bigger the chance that 2 segments will hit). Default damage is for
		// a laser of length 1 for 1 second.
		laser_o[i]->damage_factor = mother->specialDamage * frame_time*1E-3 * L / 1.0;
		// this is damage per second
	}
}


void LaserArc::calculate()
{
	STACKTRACE;

	SpaceLocation::calculate();

	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;
		return;
	}

	pos = mother->pos + rotate(relpos, mother->angle);
	vel = mother->vel;

	update_lasersegs();

	//	calculate(); - Ack!

	//	angle = mother->angle + rel_angle;

	/*
	laser_time += frame_time * 1E-3;
	if ( laser_time >= laser_period )
		state = 0;
		*/
}


SpaceLineArc::SpaceLineArc(LaserArc *creator, Vector2 lpos, double langle,
double llength, int lcolor)
:
SpaceLine(creator, lpos, langle, llength, lcolor)
{
	STACKTRACE;
	mother = creator;
}


void SpaceLineArc::calculate()
{
	STACKTRACE;
	SpaceLine::calculate();

	if (mother && !mother->exists())
		mother = 0;

	if (!mother)
		state = 0;
}


void SpaceLineArc::set_props(Vector2 opos, double oangle, double olength)
{
	STACKTRACE;
	pos = opos;
	angle = oangle;
	length = olength;
}


int SpaceLineArc::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	return 0;
}


void SpaceLineArc::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

	// copied from space_line:
	int i;
	i = iround_down(damage_factor / 2);
	if (i >= BOOM_SAMPLES)
		i = BOOM_SAMPLES - 1;
	play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	damage(other, damage_factor);

	return;
}


// ok ... well, now a lantern and laser blaster

Lantern::Lantern(DuclyLanternjaws *creator, Vector2 orelpos, double langle)
:
SpaceLocation(creator, creator->pos, langle)
{
	STACKTRACE;
	mother = creator;

	relpos = rotate(orelpos, PI/2);
	pos = creator->pos + relpos;

	relangle = PI;
	//	laserspread = 0.1 * PI;
	//	weaponAccel = scale_velocity(1.0);	// very small extra acceleration.
	laserspread = mother->lanternLaserSpread;
	weaponAccel = mother->lanternAccel;

	//	intensity = 0.2;
	//	int_decrease = 0.05;	// decrease per second
	intensity = mother->lanternIntensity;
								 // decrease per second
	int_decrease = mother->lanternIntensityIncrease;

	//	maxradius = 50;
	//	maxdensity = 20;		// 100 laser parts per seconds; each lasts 1 seconds
	maxradius = mother->lanternMaxRadius;
								 // 100 laser parts per seconds; each lasts 1 seconds
	maxdensity = mother->lanternMaxDensity;

	// this is just a manager - make it intangible; otherwise collision detection of
	// spacelines goes crazy.
	this->collide_flag_anyone = 0;
	this->collide_flag_sameteam = 0;
	this->collide_flag_sameship = 0;
}


void Lantern::calculate()
{
	STACKTRACE;
	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;
		return;
	}

	pos = mother->pos + rotate(relpos, mother->angle);
	vel = mother->vel;
								 // used in syncing laser beams
	angle = mother->angle + relangle;
	SpaceLocation::calculate();

	double radius, density;

								 // lifetime of the beams?
	radius = intensity * maxradius;
								 // number of new beams per second
	density = intensity * maxdensity;

	/*
	// intensity slowly decreases, unless you hit "fire"

	if ( !mother->fire_weapon )
	//		intensity += int_decrease * frame_time*1E-3;	// is handled by the mother
	//	else
		intensity -= int_decrease * frame_time*1E-3;

	if ( intensity < 0 )
		intensity = 0;
		*/
	// no, it's better if the laser stops at once if the player stops firing.
	if ( !mother->fire_special )
		intensity = 0;

	//	int i;
								 // average occurrence per second
	if ( tw_random(1.0) < frame_time*1E-3 * density ) {
		double a, L;

		L = tw_random(0.0, radius);
		if ( L < 5 )
			L = 5;

		int col;
		int i, r, g, b;
		i = tw_random(mother->lanternColorRandomAdd);
		r = mother->lanternColorR + i; if ( r > 255) r = 255;
		g = mother->lanternColorG  + i; if ( g > 255) g = 255;
		b = mother->lanternColorB + i; if ( b > 255) b = 255;
		col = tw_makecol(r, g, b);

		double dam = mother->lanternDamage;;
		int cnt = 500;
		//dam = 1.0;

								 // 1 in 10 times
		if (tw_random(1.0) < 0.1) {
			a = tw_random(-laserspread, laserspread);
			double v;
			v = 1.0;
								 // dangerous thing
			add ( new PulseLaser2(this, a, col, L, dam, cnt, this, 0, v));

			// some force feedback
			mother->accelerate(this, angle+a+PI, weaponAccel);
		} else {
			a = tw_random(-PI, PI);
			L *= ( 0.1 + 0.9 * fabs(cos(0.5*a)) );
			a += angle;
								 // harmless thing
			add( new Laser(this, a, col, L, 0*dam, cnt, this, 0, 1) );
		}

	}

}


PulseLaser2::PulseLaser2(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage,
int lfcount, SpaceLocation *opos, Vector2 rpos, double rvelocity)
:
SpaceLine(creator, opos->normal_pos(), opos->angle+langle, lrange, lcolor),
frame(0),
frame_count(lfcount),
lpos(opos),
rel_pos(rpos)
{
	STACKTRACE;

	// angle conventions fucked up??
	rel_pos.x *= -1;
	pos = normalize(pos + rotate(rel_pos, -PI/2+opos->get_angle()));

	id |= SPACE_LASER;
	damage_factor = ldamage;

	vel = /*lpos->get_vel() +*/ rvelocity * unit_vector(angle);

	if (!(lpos && lpos->exists())) {
		lpos = 0;
		state = 0;
	}
}


void PulseLaser2::calculate()
{
	STACKTRACE;
	if (!lpos && lpos->exists()) {
		lpos = 0;
		state = 0;
		return;
	}
	if ((frame < frame_count) && (lpos->exists())) {
		//		pos = lpos->normal_pos() + rotate(rel_pos, lpos->get_angle() - PI/2);
		//		vel = lpos->get_vel();
		SpaceLine::calculate();
		frame += frame_time;
	}
	else state = 0;
	return;
}


void PulseLaser2::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	// copied from space_line:
	int i;
	i = iround_down(damage_factor / 2);
	if (i >= BOOM_SAMPLES)
		i = BOOM_SAMPLES - 1;
	play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	damage(other, damage_factor);

	state = 0;					 // this is different from space_line :)
	return;
}


REGISTER_SHIP(DuclyLanternjaws)
