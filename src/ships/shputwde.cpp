/* $Id: shputwde.cpp,v 1.2 2006/02/20 23:04:35 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

/* Copy of the Velron Cruiser
And Phedar Patrol Ship
*/

class UtwigDefender : public Ship
{
	public:
	protected:

		double   shipSpecialDrain;

		double      weaponRangeMin, weaponRangeMax;
		double      weaponVelocityMin, weaponVelocityMax;
		double      weaponRelativity;
		int         weaponDamageMin, weaponDamageMax;
		int         weaponArmourMin, weaponArmourMax;
		double      weaponRecoil, weaponRecoilMaxSpeed;

		double      minBattForThrust;
		bool        inverseLights;

		double  specialVelocity, specialTurnRate, specialAccelRate;
		int     specialFrames;

		double  energizetimer, energizetimemax;
		int     energizepersonalarmour;

	public:
		UtwigDefender(Vector2 opos, double angle, ShipData *data, unsigned
			int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();

		//	virtual void calculate_turn_left();
		//	virtual void calculate_turn_right();
		//	virtual void calculate_thrust();

		virtual void calculate_hotspots();
		virtual void calculate();

		virtual Color crewPanelColor(int k = 0);
		virtual int handle_damage(SpaceLocation *src, double normal, double direct=0);
};

class UtwigDefenderBolt : public Shot
{
	public:
		int     maxDamage, maxArmour, referenceDamage;
		double  relative_damage;

	public:

		UtwigDefenderBolt(SpaceLocation *creator, double ox, double oy, double oangle,
			double ov, int odamage, int rdamage, double orange, int oarmour,
			SpaceSprite *osprite, double relativity);

		virtual void calculate();
		virtual int  handle_damage(SpaceLocation* source, double normal, double
			direct);
		virtual void animateExplosion();
};

class UtwigDefenderCrewPodPP : public SpaceObject
{
	public:
	public:
		int frame_count;
		int frame_size;
		int frame_step;

		//	double velocity;
		int    life;
		int    lifetime;

		double maxvelocity;
		double turnrate;
		double accelrate;

		UtwigDefenderCrewPodPP(Vector2 opos, double oangle, int oLifeTime,
			Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize, double ov,
			double oturnrate, double oaccelrate);

		virtual void calculate();
		virtual int sameTeam(SpaceLocation *other);

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
};

UtwigDefender::UtwigDefender(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	minBattForThrust = (tw_get_config_float("Ship", "MinBattForThrust", 8));

	shipSpecialDrain = tw_get_config_int("Ship", "SpecialDrain",1);

	weaponRangeMin    = scale_range(tw_get_config_float("Weapon", "RangeMin", 10));
	weaponRangeMax    = scale_range(tw_get_config_float("Weapon", "RangeMax", 60));

	weaponVelocityMin = scale_velocity(tw_get_config_float("Weapon", "VelocityMin", 65));
	weaponVelocityMax = scale_velocity(tw_get_config_float("Weapon", "VelocityMax", 95));

	weaponRelativity  = tw_get_config_float("Weapon", "Relativity", 0.0);

	weaponRecoil      = scale_velocity(tw_get_config_float("Weapon", "Recoil", 20));
	weaponRecoilMaxSpeed = scale_velocity(tw_get_config_float("Weapon", "RecoilMaxSpeed", 40));
	if (weaponRecoilMaxSpeed <= 0) weaponRecoilMaxSpeed = MAX_SPEED;

	weaponDamageMin   = tw_get_config_int("Weapon", "DamageMin", 4);
	weaponDamageMax   = tw_get_config_int("Weapon", "DamageMax", 18);

	weaponArmourMin   = tw_get_config_int("Weapon", "ArmourMin", 2);
	weaponArmourMax   = tw_get_config_int("Weapon", "ArmourMax", 10);

	inverseLights     = (tw_get_config_int("Ship", "InverseLights", 1) != 0);

	energizepersonalarmour = 0;

	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 10));
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 1));
	specialAccelRate = scale_acceleration(tw_get_config_float("Special", "AccelRate", 1));
	specialFrames   = tw_get_config_int("Special", "Frames", 0);
	energizetimemax = tw_get_config_float("Special", "Timer", 0) * 1000.0;
	energizetimer = 0;
}


int UtwigDefender::activate_weapon()
{
	STACKTRACE;
								 //get power level
	double tmpR = (batt-weapon_drain)/(double)batt_max;
	batt = weapon_drain;

								 //velocity
	double  wv = weaponVelocityMin + (weaponVelocityMax - weaponVelocityMin) * tmpR;
								 //damage
	int     wd = weaponDamageMin + (int)ceil( (weaponDamageMax - weaponDamageMin) * tmpR );

	game->add(new UtwigDefenderBolt(this, 0, 46+16, angle, wv, wd, weaponDamageMax,
		weaponRangeMin + (int)ceil( (weaponRangeMax - weaponRangeMin) * tmpR ),
		weaponArmourMin + (int)ceil( (weaponArmourMax - weaponArmourMin) * tmpR ),
		data->spriteWeapon, weaponRelativity));

	accelerate (this, angle + PI, weaponRecoil * wv * wd / (weaponDamageMax * weaponVelocityMax), weaponRecoilMaxSpeed);
	//accelerate_gravwhip (this, angle + PI, weaponRecoil * wv * wd /   (weaponDamageMax * weaponVelocityMax), weaponRecoilMaxSpeed);

	//finally, the recoil is roughly proportional to the power level squared  (mass ~ damge, and physical recoil ~ mass*velocity)

	return(TRUE);
}


Color UtwigDefender::crewPanelColor(int k)
{
	STACKTRACE;
	// change the crew color, if needed
	if ( energizepersonalarmour ) {
		double a = energizetimer / energizetimemax;
		Color c = {255,200*a,200*a};
		return c;
	} else {
		return Ship::crewPanelColor(k);
	}
}


int UtwigDefender::activate_special()
{
	STACKTRACE;
	energizepersonalarmour = TRUE;
	energizetimer = energizetimemax;

	update_panel = 1;			 // update the panel!

	return(TRUE);
}


/*
void UtwigDefender::calculate_turn_left()
{
	STACKTRACE;
	if (turn_left) {
		if (batt >= minBattForThrust)
			turn_step -= (turn_rate) * frame_time;
		else {
			double tmp = 1.0 - (batt / (double)minBattForThrust);
			turn_step -= (1.0 - tmp * tmp) * turn_rate * frame_time; } }
}

void UtwigDefender::calculate_turn_right()
{
	STACKTRACE;
	if (turn_right) {
		if (batt >= minBattForThrust)
			turn_step += (turn_rate) * frame_time;
		else {
			double tmp = 1.0 - (batt/(double)minBattForThrust);
			turn_step += (1.0 - tmp * tmp) * turn_rate * frame_time; } }
}

void UtwigDefender::calculate_thrust()
{
	STACKTRACE;
	if (batt >= minBattForThrust) Ship::calculate_thrust();
}
*/

void UtwigDefender::calculate_hotspots()
{
	STACKTRACE;
	if (batt >= minBattForThrust) Ship::calculate_hotspots();
}


void UtwigDefender::calculate()
{
	STACKTRACE;
	Ship::calculate();
	if (batt < minBattForThrust) {
		if (inverseLights)
			sprite_index += 64;
	} else {
		if (!inverseLights)
			sprite_index += 64;
	}

	//set InverseLights to 0 if you want lights ON whent the engine is ON
	//set InverseLights to 1 if you want lights ON whent the engine is OFF

	if ( energizepersonalarmour ) {
		if ( energizetimer > 0 ) {
			energizetimer -= frame_time;
		} else {
			energizetimer = 0;
			energizepersonalarmour = 0;
			update_panel = 1;
		}

	}
}


int UtwigDefender::handle_damage(SpaceLocation *src, double normal, double direct)
{
	STACKTRACE;

	double totdam = normal + direct;
	// you also have to deal with negative damage, i.e., crew increase - that's
	// especially important for this ship !!

	if ( totdam < 0 ) {
		crew -= totdam;

		if ( crew > crew_max )
			crew = crew_max;

		return iround(totdam);
	}

	double impact_angle;
	if (src->vel != 0)
		impact_angle = src->vel.atan();
	else
		impact_angle = trajectory_angle(src) + PI;

	if ( energizepersonalarmour ) {
		double evac = 0;
								 // yeah, the last one may also leave !!
		while ( evac < totdam && crew >= 1 ) {
								 // otherwise, it's hard to die ;)

			Vector2 D;
			//D = -unit_vector(vel) * this->size.x;
			//D += tw_random(Vector2(50,50)) - Vector2(25,25);

			double a = impact_angle + 0.25 * PI * (1.0 - random(2.0));
			// travel in the "same" direction as the impact - which means, you'll travel in the opposite direction of fire.
			D = unit_vector(a) * 0.8*size.x;

			UtwigDefenderCrewPodPP *cp = new UtwigDefenderCrewPodPP(
				this->normal_pos() + D, a,
				specialFrames, this, data->spriteSpecial, 32,
				specialFrames, specialVelocity,
				specialTurnRate, specialAccelRate);

			//cp->vel = 0.5 * vel;
			cp->target = this;

			add(cp);

			-- crew;

			++evac;
		}

	} else {
		Ship::handle_damage(src, normal, direct);
	}

	// don't use == 0, but use < 1, cause of possible partial damage ? I assume, 0.8 crew is also dead crew...
	if ( crew < 1 ) {
		crew = 0;
								 // use the "default" die procedure
		Ship::handle_damage(src, 0, 0);
	}

	return iround(totdam);
}


UtwigDefenderBolt::UtwigDefenderBolt(SpaceLocation *creator, double ox, double oy,
double oangle, double ov, int odamage, int rdamage, double orange,
int oarmour, SpaceSprite *osprite, double relativity) :
Shot(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour,
creator, osprite, relativity),
maxDamage(odamage), maxArmour(oarmour), referenceDamage(rdamage)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 40;
	explosionFrameSize = 12;
	relative_damage = 0;
	sprite_index = (int)floor( 40 * (referenceDamage - maxDamage) / (double)referenceDamage );
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void UtwigDefenderBolt::calculate()
{
	STACKTRACE;
	Shot::calculate();
	if (!exists()) return;		 //to avoid (d > r)

	//helpers
	double tmpR, tmp;
	tmpR = (range - d) / range;

	//calculating current damage
	tmp = maxDamage * tmpR;
	damage_factor = (int)(tmp);
	damage_factor += (int)( (tmp - damage_factor) * 2 );

	//current sprite_index is proportinal to the damage_factor
	sprite_index = (int)floor( 40 * (referenceDamage - tmp) / referenceDamage );

	//calculating current range
	tmp = maxArmour * tmpR;

	armour = (int)(tmp);
	armour += (int)( (tmp - armour) * 2 );

	if (armour < 1) armour = 1;	 //just in case
}


int UtwigDefenderBolt::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	if (normal+direct > 0) {
		relative_damage += (normal + direct) / (double)armour;
		if (relative_damage >= 1)
			return Shot::handle_damage(this, 9999, 9999);
		else
			return iround(normal + direct);
	}
	return 0;
}


void UtwigDefenderBolt::animateExplosion()
{
	STACKTRACE;
								 // just in case
	if (damage_factor <= 0) return;

	int start_frame = (int)(sprite_index / 1.7);
	int frame_count = explosionFrameCount - start_frame;

	game->add(new Animation(this, pos,
		explosionSprite, start_frame, frame_count,
		(int)(500.0/frame_count), DEPTH_EXPLOSIONS));

	return;
}


UtwigDefenderCrewPodPP::UtwigDefenderCrewPodPP(Vector2 opos, double oangle, int oLifeTime,
Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize, double ov,
double oturnrate, double oaccelrate)
:
SpaceObject(oship, opos, oangle, osprite),
frame_count(ofcount),
frame_size(ofsize),
frame_step(0),
life(0),
lifetime(oLifeTime)
{
	STACKTRACE;
	turnrate  = oturnrate * frame_time;
	accelrate = oaccelrate * frame_time;

	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);

	isblockingweapons = false;

	attributes &= ~ATTRIB_STANDARD_INDEX;

	// starting velocity: maxed
	maxvelocity = ov;
	vel = unit_vector(angle) * maxvelocity;
	// make the starting velocity relative to your ship
	vel += ship->vel;
}


int UtwigDefenderCrewPodPP::sameTeam(SpaceLocation *other)
{
	STACKTRACE;
	return true;
}


void UtwigDefenderCrewPodPP::calculate()
{
	STACKTRACE;
	frame_step += frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if (sprite_index == frame_count) sprite_index = 0;
	}

	life += frame_time;
	if (life >= lifetime) {
		die();
		return;
	}

	if (!target) {
		die();
		return;
	}

	double a = trajectory_angle(target) - angle;
	while (a < -PI) a += PI2;
	while (a >  PI) a -= PI2;
	// between -PI and +PI

	if (a < 0)
		angle -= turnrate;
	else
		angle += turnrate;

	accelerate(this, angle, accelrate, maxvelocity);

	SpaceObject::calculate();

}


void UtwigDefenderCrewPodPP::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->isShip() && other->damage_factor == 0) {
		//		sound.stop(data->sampleExtra[0]);
		//		sound.play(data->sampleExtra[0]);
		damage(other, 0, -1);
		state = 0;
	}
}


int UtwigDefenderCrewPodPP::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	state = 0;					 // this is extra; eg. if hit by a asteroid or so.
	return 0;
}


REGISTER_SHIP(UtwigDefender)
