/* $Id: shpvelcr.cpp,v 1.9 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

class VelronCruiser : public Ship
{
	protected:

		double   shipSpecialDrain;

		double      weaponRangeMin, weaponRangeMax;
		double      weaponVelocityMin, weaponVelocityMax;
		double      weaponRelativity;
		int         weaponDamageMin, weaponDamageMax;
		int         weaponArmourMin, weaponArmourMax;
		double      weaponRecoil, weaponRecoilMaxSpeed;

		double      get_aim(SpaceObject *tgt, double min_angle, double max_angle);
		int         fire_def_shot(double lowAngle, double highAngle, double
			defaultAngle);

		double      specialRange, specialVelocity, specialRelativity;
		int         specialDamage, specialArmour;
		double      specialMinAngle, specialMaxAngle, specialMidAngle;
		double      specialLaunchAngleDeflectionRange;
		int         specialDrain;
		int         specialDrainDivisor;
		int         specialDrainCounter;
		int     specialAlternatingFire;

		double      minBattForThrust;
		bool        inverseLights;
		int     defenseGunToFire;

	public:
		VelronCruiser(Vector2 opos, double angle, ShipData *data, unsigned
			int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_thrust();
		virtual void calculate_hotspots();
		virtual void calculate();

};

class VelronCrBolt : public Shot
{
	int     maxDamage, maxArmour, referenceDamage;
	double  relative_damage;

	public:

		VelronCrBolt(SpaceLocation *creator, double ox, double oy, double oangle,
			double ov, int odamage, int rdamage, double orange, int oarmour,
			SpaceSprite *osprite, double relativity);

		virtual void calculate();
		virtual int  handle_damage(SpaceLocation* source, double normal, double
			direct);
		virtual void animateExplosion();
};

class VelronCrDefShot : public Shot
{

	public:

		VelronCrDefShot(SpaceLocation *creator, double ox, double oy, double
			oangle,
			double ov, int odamage, double orange, int oarmour,
			SpaceSprite *osprite, double relativity);

		virtual void calculate();
};

VelronCruiser::VelronCruiser(Vector2 opos, double angle, ShipData *data,
unsigned int code)  :
Ship(opos, angle, data, code)
{
	STACKTRACE;

	minBattForThrust = (get_config_float("Ship", "MinBattForThrust", 8));

	shipSpecialDrain = get_config_int("Ship", "SpecialDrain",1);

	weaponRangeMin    = scale_range(get_config_float("Weapon", "RangeMin", 10));
	weaponRangeMax    = scale_range(get_config_float("Weapon", "RangeMax", 60));

	weaponVelocityMin = scale_velocity(get_config_float("Weapon", "VelocityMin", 65));
	weaponVelocityMax = scale_velocity(get_config_float("Weapon", "VelocityMax", 95));

	weaponRelativity  = get_config_float("Weapon", "Relativity", 0.0);

	weaponRecoil      = scale_velocity(get_config_float("Weapon", "Recoil", 20));
	weaponRecoilMaxSpeed = scale_velocity(get_config_float("Weapon", "RecoilMaxSpeed", 40));
	if (weaponRecoilMaxSpeed <= 0) weaponRecoilMaxSpeed = MAX_SPEED;

	weaponDamageMin   = get_config_int("Weapon", "DamageMin", 4);
	weaponDamageMax   = get_config_int("Weapon", "DamageMax", 18);

	weaponArmourMin   = get_config_int("Weapon", "ArmourMin", 2);
	weaponArmourMax   = get_config_int("Weapon", "ArmourMax", 10);

	specialRange      = scale_range(get_config_float("Special", "Range", 6));
	specialDamage     = get_config_int("Special", "Damage", 1);
	specialVelocity   = scale_velocity(get_config_float("Special", "Velocity", 100));
	specialArmour     = get_config_int("Special", "Armour", 1);
	specialRelativity = get_config_float("Special", "Relativity", 1.0);

	specialMinAngle   = get_config_float("Special", "MinAngle",15) * ANGLE_RATIO;
	specialMidAngle   = get_config_float("Special", "MidAngle",90) * ANGLE_RATIO;
	specialMaxAngle   = get_config_float("Special", "MaxAngle",180) * ANGLE_RATIO;

	specialLaunchAngleDeflectionRange = get_config_float("Special", "LaunchAngleDeflectionRange", 2.0) * PI/180;
	specialDrain = get_config_int("Ship", "SpecialDrain", 1);
	specialDrainDivisor = get_config_int("Ship", "SpecialDrainDivisor", 2);

	inverseLights     = (get_config_int("Ship", "InverseLights", 1) != 0);

	specialDrainCounter = specialDrainDivisor;
	specialAlternatingFire = get_config_int("Special", "AlternatingFire", 0);

}


int VelronCruiser::activate_weapon()
{
	STACKTRACE;
								 //get power level
	double tmpR = (batt-weapon_drain)/(double)batt_max;
	batt = weapon_drain;

								 //velocity
	double  wv = weaponVelocityMin + (weaponVelocityMax - weaponVelocityMin) * tmpR;
								 //damage
	int     wd = weaponDamageMin + (int)ceil( (weaponDamageMax - weaponDamageMin) * tmpR );

	game->add(new VelronCrBolt(this, 0, 46+16, angle, wv, wd, weaponDamageMax,
		weaponRangeMin + (int)ceil( (weaponRangeMax - weaponRangeMin) * tmpR ),
		weaponArmourMin + (int)ceil( (weaponArmourMax - weaponArmourMin) * tmpR ),
		data->spriteWeapon, weaponRelativity));

	accelerate (this, angle + PI, weaponRecoil * wv * wd / (weaponDamageMax * weaponVelocityMax), weaponRecoilMaxSpeed);
	//accelerate_gravwhip (this, angle + PI, weaponRecoil * wv * wd /   (weaponDamageMax * weaponVelocityMax), weaponRecoilMaxSpeed);

	//finally, the recoil is roughly proportional to the power level squared  (mass ~ damge, and physical recoil ~ mass*velocity)

	return(TRUE);
}


int VelronCruiser::activate_special()
{
	STACKTRACE;
	specialDrainCounter--;
	if (specialDrainCounter<=0)
		specialDrainCounter = specialDrainDivisor;
	else
		batt = batt + specialDrain;

	Vector2 opos = pos;

	double ox = 11.5;
	double oy = -16.5;

	pos = normalize(opos + rotate(Vector2(-ox, oy), -PI/2+angle));
	if (specialAlternatingFire==0||defenseGunToFire==0)
		VelronCruiser::fire_def_shot(specialMinAngle, specialMaxAngle, specialMidAngle);

	pos = normalize(opos + rotate(Vector2(ox, oy), -PI/2+angle));

	if (specialAlternatingFire==0||defenseGunToFire==1)
		VelronCruiser::fire_def_shot(PI2-specialMaxAngle, PI2-specialMinAngle, PI2-specialMidAngle);

	pos = opos;
	if (defenseGunToFire==0) defenseGunToFire=1;
	else defenseGunToFire=0;

	return(TRUE);
}


void VelronCruiser::calculate_turn_left()
{
	STACKTRACE;
	if (turn_left) {
		if (batt >= minBattForThrust)
			turn_step -= (turn_rate) * frame_time;
		else {
			double tmp = 1.0 - (batt / (double)minBattForThrust);
			turn_step -= (1.0 - tmp * tmp) * turn_rate * frame_time;
		}
	}
}


void VelronCruiser::calculate_turn_right()
{
	STACKTRACE;
	if (turn_right) {
		if (batt >= minBattForThrust)
			turn_step += (turn_rate) * frame_time;
		else {
			double tmp = 1.0 - (batt/(double)minBattForThrust);
			turn_step += (1.0 - tmp * tmp) * turn_rate * frame_time;
		}
	}
}


void VelronCruiser::calculate_thrust()
{
	STACKTRACE;
	if (batt >= minBattForThrust) Ship::calculate_thrust();
}


void VelronCruiser::calculate_hotspots()
{
	STACKTRACE;
	if (batt >= minBattForThrust) Ship::calculate_hotspots();
}


void VelronCruiser::calculate()
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
}


double VelronCruiser::get_aim(SpaceObject *tgt, double min_angle, double max_angle)
{
	STACKTRACE;
	if (tgt == NULL)
		return (-1000);

	Vector2 tv = tgt->get_vel() - specialRelativity * vel;
	double tvx = tv.x;
	double tvy = tv.y;
	tv = min_delta(tgt->normal_pos(), pos);
	double rx  = tv.x;
	double ry  = tv.y;
	double r2  = rx*rx + ry*ry;
	double u2  = specialVelocity;
	u2 *= u2;
	double d2v = u2 - (tvx*tvx + tvy*tvy);
	double t = (rx*tvx + ry*tvy);
	double q, p;
	if (fabs(d2v/u2) > 0.01 ) {
		q = t*t + r2*d2v;
		if (q > 0) q = sqrt(q);
		else    return (-1000);
		p = (t+q)/d2v;
		q = (t-q)/d2v;
		if (p > 0) t = p;
		else       t = q;
		if (t < 0) return (-1000);
	} else {
		if (fabs(t)<1e-6) return (-1000);
		else    t = - 0.5 * r2 / t;
		if (t < 0) return (-1000);
	}
	if (t * specialVelocity > specialRange) return(-1000);
	t = normalize((atan3(tvy*t + ry, tvx*t + rx)) - angle, PI2);
	double d_a = normalize(t - min_angle, PI2);
	if (d_a > PI) d_a -= PI2;
	if (d_a > 0) {
		d_a = normalize(t - max_angle, PI2);
		if (d_a > PI) d_a -= PI2;
		if (d_a < 0)
			return (t);
	}
	return (-1000);
}


int VelronCruiser::fire_def_shot(double lowAngle, double highAngle, double defaultAngle)
{
	STACKTRACE;
	SpaceObject *o;
	double distance2, bestDistance2 = 99999999;
	double angleShift, relativeAngle;
	double firingAngle = defaultAngle;

	angleShift = ((double)(random()%2001-1000)/1000.0) * specialLaunchAngleDeflectionRange;

	Query a;
	for (a.begin(this, OBJECT_LAYERS, specialRange); a.current; a.next()) {
		o = a.currento;
		if ( (!o->isInvisible()) && !o->sameTeam(this)
		&& (o->collide_flag_anyone & bit(LAYER_SHOTS))) {
			distance2 = distance(o); relativeAngle = get_aim(o, lowAngle, highAngle);
			if ((distance2 < bestDistance2) && (relativeAngle > -1000)) {
				bestDistance2 = distance2;
				firingAngle = relativeAngle;
			}
		}
	}

	add(new VelronCrDefShot(this, 0.0, 0.0, normalize(angle + firingAngle + angleShift, PI2), specialVelocity,
		specialDamage, specialRange, specialArmour, data->spriteSpecial, specialRelativity));

	return(TRUE);
}


VelronCrBolt::VelronCrBolt(SpaceLocation *creator, double ox, double oy,
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
}


void VelronCrBolt::calculate()
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


int VelronCrBolt::handle_damage(SpaceLocation* source, double normal, double direct)
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


void VelronCrBolt::animateExplosion()
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


VelronCrDefShot::VelronCrDefShot(SpaceLocation *creator, double ox, double oy,
double oangle, double ov, int odamage, double orange, int oarmour,
SpaceSprite *osprite, double relativity) :
Shot(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, creator, osprite, relativity)
{
	STACKTRACE;
}


void VelronCrDefShot::calculate()
{
	STACKTRACE;
	Shot::calculate();
	if (!exists()) return;
	if (d/range > 0.8)
		sprite_index = (int)( ( 5 * ((d/range) - 0.8) ) * 4 );
}


REGISTER_SHIP(VelronCruiser)
