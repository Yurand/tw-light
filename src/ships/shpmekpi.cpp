/* $Id: shpmekpi.cpp,v 1.14 2004/03/24 23:51:42 yurand Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class MeknikChainsaw;
class LaserInform;

class MeknikPincer : public Ship
{
	double  rockRange, rockVelocity, rockDamage, rockArmour;
	double  weaponColor, weaponRange, weaponFrames, weaponDamage;
	double specialTurnperiod, specialDamage, specialDamageperiod;

	int Nsaws;
	MeknikChainsaw **saw;
	double sawangle, *sawrefangle;

	public:
		MeknikPincer(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
		virtual ~MeknikPincer();

		void getinformed(int itype, SpaceLocation *other);
		int special_state();

	protected:

		virtual void calculate();
		//virtual void animate(Frame *f);

		virtual int activate_weapon();
		virtual int activate_special();

		virtual int handle_damage(SpaceLocation* source, double normal, double direct);

};

class MeknikChainsaw : public SpaceObject
{
	double  damage, damageperiod, dist, maxdist;
	double  lifetime, sawtime, inflict_time;
	MeknikPincer *refship;

	public:
		MeknikChainsaw(MeknikPincer *oship, double odist, double a,
			double damage, double damageperiod,
			SpaceSprite *ospr);

	protected:

		virtual void calculate();
		//	virtual void animate(Frame *f);

		//	virtual int handle_damage(SpaceLocation* source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);

};

class LaserInform : public Laser
{
	MeknikPincer *mother;
	public:
		LaserInform(MeknikPincer *creator, double langle, int lcolor, double lrange, double ldamage,
			int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle);

		virtual void inflict_damage(SpaceObject *other);
		virtual void calculate();
};

MeknikPincer::MeknikPincer(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;
	weaponColor  = get_config_int("Weapon", "Color", 0);
	weaponRange  = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage = get_config_int("Weapon", "Damage", 0);

	rockRange    = scale_range(get_config_float("Quirk", "Range", 0));
	rockVelocity = scale_velocity(get_config_float("Quirk", "Velocity", 0));
	rockDamage   = get_config_int("Quirk", "Damage", 0);
	rockArmour   = get_config_int("Quirk", "Armour", 0);

	specialTurnperiod = get_config_float("Special", "Turnperiod", 1.0);
	specialDamage = get_config_float("Special", "Damage", 1.0);
	specialDamageperiod = get_config_float("Special", "Damageperiod", 1.0);

	Nsaws = 3;
	saw = new MeknikChainsaw* [Nsaws];
	sawrefangle = new double [Nsaws];

	int i;
	for ( i = 0; i < Nsaws; ++i ) {
		saw[i] = 0;
		sawrefangle[i] = i * PI2 / Nsaws;
	}
	sawangle = 0.0;
}


MeknikPincer::~MeknikPincer()
{
	delete saw;
	delete sawrefangle;
}


void MeknikPincer::getinformed(int itype, SpaceLocation *other)
{
	STACKTRACE;
	// if you've destroyed (i=1) an asteroid, then...
	if (itype == 1 && other->isAsteroid()) {
		//damage(source, 1000, 1000);
		add(new Shot(this, Vector2(0, -size.y/2),
			angle+PI, rockVelocity, rockDamage, rockRange, rockArmour,
			this, data->spriteWeapon));

		if (vel.length() < speed_max)
			vel += rockVelocity * unit_vector(angle);
	}
}


int MeknikPincer::special_state()
{
	STACKTRACE;
	return special_low;
}


int MeknikPincer::activate_weapon()
{
	STACKTRACE;
	// activate laser (s)

	int i;
	double D = 0.125 * PI;

	for ( i = -2; i <= 2; ++i) {
		double R, da;
		Vector2 rpos;

		da = i * D;

		R = 50;
		rpos = Vector2(0, 5) + R * unit_vector(PI/2-da);

		add(new LaserInform(this, angle + da,
			makecol(128,0,64), weaponRange / (1+fabs((double)i)), weaponDamage, weapon_rate,
			this, rpos, true));
	}

	return TRUE;
}


int MeknikPincer::activate_special()
{
	STACKTRACE;
	// for re-activation, require minimum batt.
	if (!saw[0])
		if (batt < 4)
			return FALSE;

	int i;
	for ( i = 0; i < Nsaws; ++i ) {
		int d;

		//if ( i == 0 )
		//	d = 1;
		//else
		//	d = -1;
		d = 1;					 // same direction.

		if (!saw[i]) {
			saw[i] = new MeknikChainsaw(this, 60.0, sawangle + sawrefangle[i],
				specialDamage, specialDamageperiod,
				data->spriteSpecial);
			add(saw[i]);
		}
	}

	return TRUE;
}


void MeknikPincer::calculate()
{
	STACKTRACE;

	int i;
	for ( i = 0; i < Nsaws; ++i ) {
		if ( !(saw[i] && saw[i]->exists()) )
			saw[i] = 0;
		else
			saw[i]->angle = sawangle + sawrefangle[i];
	}

	for ( i = 0; i < Nsaws; ++i )
		if (saw[i])
			break;

	if (i < Nsaws) {			 // if there's a saw alive, increase angle.
		sawangle += PI2 * frame_time*1E-3 / specialTurnperiod;
		if (angle > PI)
			angle -= PI2;
		if (angle < -PI)
			angle += PI2;
	}

	Ship::calculate();
}


int MeknikPincer::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	/*
	if (source->isAsteroid())
	{
		damage(source, 1000, 1000);
		add(new Shot(this, Vector2(0, -size.y/2),
			angle+PI, rockVelocity, rockDamage, rockRange, rockArmour,
			this, data->spriteWeapon));

		vel += rockVelocity * unit_vector(angle);
	}
	*/

	return Ship::handle_damage(source, normal, direct);
}


MeknikChainsaw::MeknikChainsaw(MeknikPincer *oship, double odist, double a,
double odamage, double odamageperiod,
SpaceSprite *ospr)
:
SpaceObject(oship, oship->pos+odist*unit_vector(a), a, ospr)
{
	STACKTRACE;
	refship = oship;
	//turnperiod = oturnperiod;
	damage = odamage;
	damageperiod = odamageperiod;

	maxdist = odist;
	dist = 0.5 * maxdist;

	angle = a;

	lifetime = 0;
	sawtime = 0;
	inflict_time = 0;

	mass = 0;					 // so that it can slice through without pushing it away
	layer = LAYER_SPECIAL;

	set_depth(DEPTH_SHIPS - 0.1);

	isblockingweapons = true;
}


void MeknikChainsaw::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();

	if ( !(refship && refship->exists()) ) {
		refship = 0;
		state = 0;
		return;
	}

	// if special is pressed, and batt isn't low.
	if ( !refship->fire_special && !refship->special_state() ) {
		state = 0;
		return;
	}

	double dt;

	dt = frame_time * 1E-3;
	sawtime += dt;

	double w;
	int i;

	w = 0.2;
	i = int(sawtime / w);

	if (i < 0 || i >= 3) {
		i = 0;
		//sawtime -= 3 * w;
		sawtime = 0;
	}

	/*
	angle += PI2 * dt / turnperiod;
	if (angle > PI)
		angle -= PI2;
	if (angle < -PI)
		angle += PI2;
		*/

	sprite_index = get_index(angle) + i*64;
	//sprite_index = i * 64;
	i = sprite->frames();

	if (dist < maxdist)
								 // it takes 1 second to appear fully.
		dist += 0.5 * maxdist * dt;
	else
		dist = maxdist;

	pos = refship->pos + dist * unit_vector(angle+PI/2);
	vel = refship->vel;			 // not really needed ?
}


void MeknikChainsaw::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	//SpaceObject::inflict_damage(other);
	inflict_time += frame_time * 1E-3;

	while (inflict_time > damageperiod) {
		inflict_time -= damageperiod;
		SpaceObject::damage(other, damage, 0);

		if (!other || !other->state) {
			refship->getinformed(1, other);
			break;
		}
	}
}


LaserInform::LaserInform(MeknikPincer *creator, double langle, int lcolor, double lrange, double ldamage,
int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle)
:
Laser(creator, langle, lcolor, lrange, ldamage, lfcount, opos, rpos, osinc_angle)
{
	STACKTRACE;
	mother = creator;
}


void LaserInform::calculate()
{
	STACKTRACE;
	if (!(mother && mother->exists())) {
		mother = 0;
		state = 0;
		return;
	}

	Laser::calculate();
}


void LaserInform::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	Laser::inflict_damage(other);

	if (!other || !other->exists()) {
		mother->getinformed(1, other);
	}
}


REGISTER_SHIP(MeknikPincer)
