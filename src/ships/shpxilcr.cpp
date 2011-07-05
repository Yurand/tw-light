/* $Id: shpxilcr.cpp,v 1.14 2004/03/24 23:51:43 yurand Exp $ */
/*
I used some code from the Orz Nemesis for this ship: for the turret
Rest is "original" - as far as I can be original anyway ;)
*/

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class XillzCrescentOrbiter;

class XillzCrescent : public Ship
{
	double    weaponRange;
	double    weaponVelocity;
	int       weaponDamage;
	double    weaponArmour;
	double    specialArmour, specialMass, specialAccelRate, specialVelocity, specialRange,
		specialOrbitDecayFactor, specialWellSize;

	XillzCrescentOrbiter  **orbiters;
	int                   Norbiters, MaxOrbiters, klastcreated;

	public:
		XillzCrescent(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:

		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
};

class XillzCrescentOrbiter : public SpaceObject
{
	public:

		SpaceObject *centre;
		double      armour, R, accel, passiveRange, OrbitDecayFactor, WellSize;

		Vector2 relpos, relvel;	 // orbiting is done relative to the ship

		XillzCrescentOrbiter(SpaceLocation *creator, Vector2 opos,
			double oangle, SpaceSprite *osprite, double orange,
			double oarmour, double omass, double oaccel, double velocity,
			double odecay, double oWellSize);

		void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
};

XillzCrescent::XillzCrescent(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	double raw_hotspot_rate = get_config_float("Ship", "HotspotRate", 0);

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialArmour     = get_config_float("Special", "Armour", 0);
	specialMass       = get_config_float("Special", "Mass", 0);
	specialAccelRate  = scale_acceleration(get_config_float("Special", "AccelRate", 0), raw_hotspot_rate);
	specialVelocity   = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialRange      = scale_range(get_config_float("Weapon", "Range", 0));

	specialOrbitDecayFactor  = get_config_float("Special", "OrbitDecayFactor", 0);
	specialWellSize = get_config_float("Special", "WellSize", 100);

	MaxOrbiters = get_config_int("Special", "N", 0);
	Norbiters = 0;
	orbiters = new XillzCrescentOrbiter* [MaxOrbiters];
	int i;
	for ( i = 0; i < Norbiters; ++i )
		orbiters[i] = 0;		 // reset the pointer
	klastcreated = 0;
}


int XillzCrescent::activate_weapon()
{
	STACKTRACE;

	Vector2 rpos;
	double a, DX;
	DX = 20;

	rpos = Vector2(-DX, 0.0);
	a = PI/2;

	add(new Missile(this, rpos, angle + a,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, 1.0));

	rpos = Vector2(+DX, 0.0);
	a = -PI/2;

	add(new Missile(this, rpos, angle + a,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, 1.0));

	return TRUE;
}


int XillzCrescent::activate_special()
{
	STACKTRACE;
	XillzCrescentOrbiter    *xo;

	xo = new XillzCrescentOrbiter(this, pos, angle+PI, data->spriteSpecial,
		specialRange, specialArmour, specialMass, specialAccelRate, specialVelocity,
		specialOrbitDecayFactor, specialWellSize);

	add(xo);

	// ok ... and one other should disappear ...

	if ( Norbiters < MaxOrbiters ) {
		orbiters[Norbiters] = xo;
		++Norbiters;
		return TRUE;
	} else {

		int k;

		// 1: check if there is one empty entry
		for ( k = 0; k < Norbiters; ++k )
			if ( !orbiters[k] )
				break;
		if ( k != Norbiters ) {
			orbiters[k] = xo;
			return TRUE;
		}

		// 2: check if there is one without a centre
		for ( k = 0; k < Norbiters; ++k )
			if ( k != klastcreated && !orbiters[k]->centre )
				break;

		if ( k != Norbiters ) {
			klastcreated = k;
								 // it should die first
			orbiters[k]->state = 0;
			orbiters[k] = xo;	 // point to the replacement orbiter
			return TRUE;
		}

		// 3: check if there is one without a ship as centre
		for ( k = 0; k < Norbiters; ++k )
			if ( k != klastcreated && !orbiters[k]->centre->isShip() )
				break;
		if ( k != Norbiters ) {
			klastcreated = k;
								 // it should die first
			orbiters[k]->state = 0;
			orbiters[k] = xo;	 // point to the replacement orbiter
			return TRUE;
		}

		// finally, replace the next-to last on
		k = klastcreated + 1;
		while ( k >= Norbiters )
			k -= Norbiters;

		orbiters[k]->state = 0;	 // it should die first
		orbiters[k] = xo;		 // point to the replacement orbiter

		klastcreated = k;
		return TRUE;
	}
}


void XillzCrescent::calculate()
{
	STACKTRACE;
	Ship::calculate();

	// check if orbiters have died:
	int i;
	for ( i = 0; i < Norbiters; ++i )
		if ( !(orbiters[i] && orbiters[i]->exists()) )
			orbiters[i] = 0;	 // reset the pointer
}


XillzCrescentOrbiter::XillzCrescentOrbiter(SpaceLocation *creator, Vector2 opos,
double oangle, SpaceSprite *osprite, double orange,
double oarmour, double omass, double oaccel, double velocity,
double odecay, double oWellSize)
:
SpaceObject(creator, opos, oangle, osprite)
{
	STACKTRACE;
	mass = omass;
	armour = oarmour;
	accel = oaccel;

	centre = 0;

	vel = velocity * unit_vector(oangle);

	collide_flag_anyone = ALL_LAYERS;
	layer = LAYER_SPECIAL;

	passiveRange = orange;

	OrbitDecayFactor = odecay;
	WellSize = oWellSize;

	isblockingweapons = false;
}


void XillzCrescentOrbiter::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;					 // orbiters disappear when the ship dies - otherwise it becomes a mess
	}

	// enter orbit around the closest spacelocation

	if ( !(centre && centre->exists()) ) {
		centre = 0;				 // reset the pointer
		R = 1E99;

		// scan for a new center: the nearest within range
		int layers = OBJECT_LAYERS;

		Query a;
		for (a.begin(this, layers, passiveRange); a.current; a.next()) {
			SpaceObject *o = a.currento;
			if (!(o->isPlanet()) && o->mass != 0 && o != ship && (!sameTeam(o)) ) {
				if (centre == 0 || distance(centre) < R) {
					// enter orbit !
					centre = o;
					R = distance(centre);
				}

			}
		}
	}

	if ( centre ) {

		double b;
		//		if ( mass != 0 )
		//			b = mass / centre->mass;
		//		else
		//			b = 1;

		// apply a small force to the centre:
		double ang;
		Vector2 dv;
		ang = trajectory_angle(centre);

		b = WellSize / sqrt(distance(centre));
		if (b > 1)
			b = 1;

		dv = accel*frame_time * unit_vector(ang) * b - OrbitDecayFactor  * (vel - centre->vel);

		centre->vel -= dv;
		vel += dv;

		// update position due to their interaction
		centre->pos -= dv * frame_time;

		// the enemy only needs and extra positional change; your own (relative)
		// position still needs complete update:
		pos += vel * frame_time;

		if ( distance(centre) > passiveRange)
			centre = 0;

	}

	// also, disturb any other objects that come from anywhere
	// and are within reach,
	int layers = OBJECT_LAYERS;
	Query a;
	for (a.begin(this, layers, passiveRange); a.current; a.next()) {
		SpaceObject *o = a.currento;
		if ( o->isShot() ) {
			Shot *s = (Shot*) o;

			double b = WellSize / sqrt(distance(s));
			if (b > 2)
				b = 2;
			double ang = trajectory_angle(s);

			Vector2 dv;

			dv = accel*frame_time * unit_vector(ang) * b;

			s->vel = s->v * unit_vector(s->angle);
			s->vel -= dv;

			s->v = s->vel.length();
			s->angle = s->vel.atan();

		}
	}

}


int XillzCrescentOrbiter::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	double total = normal + direct;

	armour -= total;
	if (armour <= 0) {
		total -=armour;
		state = 0;
	}

	if (source && source->isShip()) {
		damage(source, 0, 4);
		state = 0;
		return iround(total);
	}

	if (source && source->isPlanet()) {
		state = 0;				 // the planet should clean up the existing orbiters
		return iround(total);
	}

	return iround(total);
}


REGISTER_SHIP(XillzCrescent)
