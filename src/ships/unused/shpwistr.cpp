/* $Id: shpwistr.cpp,v 1.15 2005/08/14 16:14:32 geomannl Exp $ */

/*
Ok ... well, there's the monotron already, but nevertheless, this ship may
also be fun.
Re: imo the monotron sucks.
*/

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

/*
static double cross_product( Vector2 a, Vector2 b )
{
	STACKTRACE;
	return a.x*b.y - a.y*b.x;	// this is the z-value of a vector (0,0,z). I assume z-value of a and b = 0
}
*/

class WissumTripod : public Ship
{
	public:
		Vector2 oldvel, old_dvel;

	public:
		double  default_accel_rate, default_speed_max;

		double  weaponRange, specialRange;
		double  weaponVelocity, specialVelocity;
		double  weaponDamage, specialDamage;
		double  weaponArmour, specialArmour;
		double  specialDrainFuelDump;

		//  double	bombHotspotRate, bombVelocity, bombAccelRate,
		//			bombDamage, bombLifeTime, bombArmour, bombAngleCorrLimit;

		double  moveangles[3];
		int     engineactive[3];
		int     Nactive, ifiregun;

		WissumTripod(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();

		virtual void calculate_thrust();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();

		virtual void calculate_hotspots();
		//  virtual void animate(Frame *space);

		virtual void calculate();
		virtual void animate_predict(Frame *frame, int time);

		int fire_guns(int fire_type);

		virtual double handle_speed_loss(SpaceLocation *source, double normal);
};

class FuelDump : public Shot
{
	public:
	public:

		int     num_mirvs;
		double  orig_damage_factor;

		FuelDump(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
			double odamage, double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, double relativity, int num);

		virtual void calculate();
};

/* removed cause they sucked :)
class TripodBomb : public SpaceObject
{
public:
public:
	WissumTripod *mother;
	SpaceObject *findme;

	double	hotspot_rate, hotspot_frame, armour,
			speed_max, lifetime, existtime, accel_rate;

	Vector2	enemy_vel;

	TripodBomb(WissumTripod *creator, Vector2 relpos, double relangle, SpaceSprite *osprite);

	virtual void calculate();
	virtual int  handle_damage(SpaceLocation *source, double normal, double direct = 0);
	virtual void inflict_damage(SpaceObject *other);

	void calculate_hotspots(double a);
};
*/

WissumTripod::WissumTripod(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_float("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_float("Weapon", "Armour", 0);

	specialRange        = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity     = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage       = tw_get_config_float("Special", "Damage", 0);
	specialArmour       = tw_get_config_float("Special", "Armour", 0);

	/*
	bombHotspotRate = scale_frames(tw_get_config_float("Bomb", "HotspotRate", 0));
	bombVelocity    = scale_velocity(tw_get_config_float("Bomb", "Velocity", 0));
	bombAccelRate   = scale_acceleration(tw_get_config_float("Bomb", "AccelRate", 0));
	bombLifeTime    = scale_frames(tw_get_config_float("Bomb", "LifeTime", 0));
	bombDamage      = tw_get_config_float("Bomb", "Damage", 0);
	bombArmour      = tw_get_config_float("Bomb", "Armour", 0);
	bombAngleCorrLimit = tw_get_config_float("Bomb", "Armour", 0);	// in degrees.
	*/

	specialDrainFuelDump = tw_get_config_float("Ship", "SpecialDrainExtra", 0);
	//	tw_error("special drain extra = %d", int(specialDrainFuelDump));

	// these are the only angles at which the ship can provide thrust.
	moveangles[0] = 210.0 * ANGLE_RATIO;
	moveangles[1] =  90.0 * ANGLE_RATIO;
	moveangles[2] = -30.0 * ANGLE_RATIO;

	engineactive[0] = 0;
	engineactive[1] = 0;
	engineactive[2] = 0;
	Nactive = 0;
	ifiregun = 0;

	//	Nactiveangles = 1;
	//	iactiveangle = 0;
	//	angle = moveangles[iactiveangle];

	default_accel_rate = accel_rate;
	default_speed_max = speed_max;

	sprite_index = 0;

	// for pred.
	oldvel = vel;
	old_dvel = 0;
}


void WissumTripod::calculate_thrust()
{
	STACKTRACE;
	if ( thrust )
		engineactive[1] = 1;
	else
		engineactive[1] = 0;
}


void WissumTripod::calculate_turn_right()
{
	STACKTRACE;
	turn_step = 0;
	if ( turn_right )
		engineactive[0] = 1;
	else
		engineactive[0] = 0;
}


void WissumTripod::calculate_turn_left()
{
	STACKTRACE;
	turn_step = 0;
	if ( turn_left )
		engineactive[2] = 1;
	else
		engineactive[2] = 0;
}


int WissumTripod::fire_guns(int fire_type)
{
	STACKTRACE;
	int testfired;

	int ilast = ifiregun;

	testfired = 0;
	for (;;) {

		++ifiregun;
		if ( ifiregun > 2 )
			ifiregun = 0;

		if ( (engineactive[ifiregun] && fire_type == 1)
		|| (!engineactive[ifiregun] && fire_type == 2) ) {
			double a;
			a = angle;
								 // cause the vector-offs. is rotated by angle,
			angle = moveangles[ifiregun];
								 // and if 2 engines are on, angle isn't either of the directions

			// some scatter
			double rana, da;
			da = 10 * ANGLE_RATIO;
			rana = tw_random(-da, da);

			add(new Shot(this, Vector2(0, 15),
				angle + rana, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
				this, data->spriteWeapon, 1.0));

			testfired = 1;
			angle = a;
			break;
		}

		if ( ifiregun == ilast ) // in case none is active ....
			break;
	}

	if ( testfired )
		return TRUE;
	else
		return FALSE;
}


int WissumTripod::activate_weapon()
{
	STACKTRACE;
	return fire_guns(1);
}


int WissumTripod::activate_special()
{
	STACKTRACE;

	// this is handled implicitly in activate_weapon, this must return true
	// so that fire_special is set; this must consume very little fuel,
	// for "normal use".

	if ( Nactive < 3 ) {

		fire_guns(2);

		if (weapon_sample >= 0)
			play_sound2(data->sampleWeapon[weapon_sample]);

		return TRUE;			 // ok, the special was used
	}

	// a special case for the special is, when all 3 thrusters have been
	// activated, do something like the hotspot special. This consumes
	// extra fuel.

	if ( Nactive == 3 && batt >= specialDrainFuelDump ) {
		int i;
		for ( i = 0; i < 3; ++i ) {
			double a;
			a = angle;
			angle = moveangles[i];

			game->add( new FuelDump(this, Vector2(0,-25),
				angle+PI, specialVelocity,
				specialDamage, specialRange, specialArmour,
				this, this->data->spriteExtra, 1.0, 0) );

			angle = a;
		}

		// extra fuel (note that special_drain is also subtracted, later on):
		batt -= specialDrainFuelDump - special_drain;
		if ( batt < 0 )
			batt = 0;

		play_sound(data->sampleExtra[0]);

		return TRUE;			 // ok, the 3 things were launched
	}

	return FALSE;
}


double WissumTripod::handle_speed_loss(SpaceLocation *source, double normal)
{
	STACKTRACE;

	accel_rate = default_accel_rate;
	speed_max = default_speed_max;

	Ship::handle_speed_loss(source, normal);

	default_accel_rate = accel_rate;
	default_speed_max = speed_max;

	return 1;
}


void WissumTripod::calculate()
{
	STACKTRACE;
	int k;

	Ship::calculate();

	k = 0;

	int i;

	for ( i = 0; i < 3; ++i ) {
		if (engineactive[i]) {
			if ( k == 0 )
				angle = moveangles[i];
			else {
				double da;
				da = moveangles[i] - angle;
				while ( da >  PI )      da -= PI2;
				while ( da < -PI )      da += PI2;
								 // this gives the average between the two
				angle += 0.5 * da;
			}
			++k;
		}
	}

	Nactive = k;

	if ( k == 2 ) {
		accel_rate = 2 * default_accel_rate;
		speed_max = 2 * default_speed_max;
	} else {
		accel_rate = default_accel_rate;
		speed_max = default_speed_max;
	}

	// calculate thrust:
	if ( k == 1 || k == 2 ) {
		thrust = 1;
		Ship::calculate_thrust();
	}

	// determine which one to draw (1 orientation, but different engine graphics)
	sprite_index = 0;
	// note: Nactiveangles = 1 or 2

}


void WissumTripod::calculate_hotspots()
{
	STACKTRACE;
	// add hotspots to all active engines
	int i;

	if ((Nactive > 0) && (hotspot_frame <= 0)) {
		for ( i = 0; i < 3; ++i ) {
			if ( engineactive[i] )
				game->add(new Animation(this,
					normal_pos() - unit_vector(moveangles[i]) * size.x / 2.5,
					meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
		}

		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0)
		hotspot_frame -= frame_time;
	return;
}


void WissumTripod::animate_predict(Frame *frame, int time)
{
	STACKTRACE;
	// just do the linear prediction (is most stable)
	//SpaceObject::animate_predict(frame, time);
	SpaceObject::animate(frame);
}


FuelDump::FuelDump(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
double odamage, double orange, double oarmour, SpaceLocation *opos,
SpaceSprite *osprite, double relativity, int num)
:
Shot(creator, rpos, oangle, ov,
odamage, orange, oarmour, opos,
osprite, relativity)
{
	STACKTRACE;
	num_mirvs = num;
	sprite_index = num_mirvs;	 // each mirv has a different sprite
	orig_damage_factor = damage_factor;
								 // num starts at 0
	damage_factor = int(damage_factor / (num+1));
	if ( damage_factor <= 0 )
		damage_factor = 1;
}


void FuelDump::calculate()
{
	STACKTRACE;
	Shot::calculate();

	if (!state && num_mirvs < sprite->frames()-1) {
		++num_mirvs;
		// spawn 2 new coredumps, with half the damage, and at different angles

		double da = 45 * ANGLE_RATIO;

		game->add( new FuelDump(this, Vector2(0,0), angle+da, v,
			orig_damage_factor, range, armour/2,
			this, sprite, 1.0, num_mirvs) );

		game->add( new FuelDump(this, Vector2(0,0), angle-da, v,
			orig_damage_factor, range, armour/2,
			this, sprite, 1.0, num_mirvs) );

	}
}


/*

TripodBomb::TripodBomb(WissumTripod *creator, Vector2 relpos,
					   double relangle, SpaceSprite *osprite)
:
SpaceObject(creator, creator->pos+relpos, creator->angle+relangle, osprite)
{
	STACKTRACE;
	mother = creator;
	findme = mother->target;

	speed_max = mother->bombVelocity;
	vel = mother->vel + speed_max * unit_vector(angle);

	enemy_vel = findme->vel;

	accel_rate = mother->bombAccelRate;

	lifetime = mother->bombLifeTime;
	existtime = 0;

	hotspot_rate = mother->bombHotspotRate;
	hotspot_frame = 0;

	armour = mother->bombArmour;
	damage_factor = mother->bombDamage;

	mass = 0;
}

void TripodBomb::calculate_hotspots(double a)
{
	STACKTRACE;
	if (hotspot_frame <= 0) {
		game->add(new Animation(this,
			normal_pos() - unit_vector(a) * size.x / 2.5,
			game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0)
		hotspot_frame -= frame_time;
	return;
}

void TripodBomb::calculate()
{
	STACKTRACE;

	existtime += frame_time;

	if ( (!(mother && mother->exists())) || existtime > lifetime )
	{
		state = 0;
		return;
	}

	findme = mother->target;

	if ( findme && findme->exists() )
	{

		// from average direction of the enemy, and current heading,
		// calculate the position closest to the enemies path.

		Vector2 V1, P1, P2;

		//enemy_vel = 0.9 * enemy_vel * 0.1 * findme->vel;
		enemy_vel = findme->vel;

		V1 = enemy_vel;
		P1 = findme->pos;

		P2 = pos;

		double T1;	// time, assuming constant velocity and heading, for enemy to reach closest point to the bomb

		Vector2 DP;
		DP = min_delta(P2 - P1, map_size);

		double a;

		if ( V1.magnitude() > 0.1 * speed_max )
		{

			T1 = DP.dot(V1) / V1.dot(V1);

			Vector2 Ptarget;
			Ptarget = P1 + 0.5 * fabs(T1) * V1;

			DP = min_delta(Ptarget - P2, map_size);

			// heading for smallest distance to enemy ship
			a = DP.atan();

		} else
			a = trajectory_angle(findme);

		// relative to top angle
		a +=  PI/2;

		while (a < -PI)		a += PI2;
		while (a >  PI)		a -= PI2;

		// which thruster(s) will be active?
		int k;
		k = int(a / ANGLE_RATIO);	// in degrees

		int m;
		m = 1;//20;	// extra activation angle

		// check each angle arc:

		// ... for acceleration
		double v;
		v = accel_rate * frame_time;

		if ( k < -120+m || k > 120-m )
		{
			// active top engine
			// add hotspots
			a = -PI/2;
			calculate_hotspots(a);
			//		vel += v * unit_vector(a+PI);
			accelerate_gravwhip(this, a+PI, v, speed_max);
		}

		if ( k > -120-m && k < 0+m )
		{
			// active bottom right one
			a = PI/6;
			calculate_hotspots(a);
			//		vel += v * unit_vector(a+PI);
			accelerate_gravwhip(this, a+PI, v, speed_max);
		}

		if ( k > 0-m && k < 120+m )
		{
			// activate bottom left one
			a = PI - PI/6;
			calculate_hotspots(a);
			//		vel += v * unit_vector(a+PI);
			accelerate_gravwhip(this, a+PI, v, speed_max);
		}

	}

	sprite_index = 0;

	SpaceObject::calculate();

	sprite_index = 0;

}

int TripodBomb::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	armour -= (normal + direct);
	if ( armour <= 0 )
	{
		state = 0;
		play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
		game->add(new Animation(this, pos, game->kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
	}
	return (normal+direct);
}

void TripodBomb::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	SpaceObject::inflict_damage(other);

	state = 0;
//	play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
//	game->add(new Animation(this, pos, game->kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
}

*/

REGISTER_SHIP(WissumTripod)
