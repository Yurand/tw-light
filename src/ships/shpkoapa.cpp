/* $Id: shpkoapa.cpp,v 1.13 2005/08/02 00:23:46 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

class KoanuaMissile : public Missile
{
	public:
	public:
		KoanuaMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double
			Relativity);
		Ship* creator;
		double relativity;
		double facingAngle;
		double framesToIgnition;
		int isBurning;
		int isCoasting;
		double acceleration;
		double mass;
		double ignitionSpeed;
		double framesOfBurn;
		double framesOfCoasting;
		double maxSpeed;
		double damageAfterIgnition;
		double armourAfterIgnition;
		void calculate(void);
};

class KoanuaPatrolShip : public Ship
{
	public:
	public:

		double       shipSpeedMax;
		double       shipTurnRate;
		double       shipAccelRate;
		double       rawHotspotRate;
		double       shipSpeedMaxLossPerTurboUsed;
		double       shipAccelLossPerTurboUsed;
		double       shipTurnRateGainPerTurboUsed;
		int          shipSpecialDrain;
		int          weaponGroupOneActive;
		int          weaponGroupTwoActive;

		double       weaponRange;
		double       weaponVelocity1;
		double       weaponMaxSpeed;
		int          weaponDamage1;
		int          weaponDamage2;
		int          weaponArmour1;
		int          weaponArmour2;
		double       weaponRelativity;
		int          weaponFramesToIgnition;
		int          weaponFramesOfThrust;
		int          weaponFramesOfCoasting;
		double       weaponAcceleration;
		double       weaponReleaseAngle1;
		double       weaponReleaseAngle2;
		double       weaponReleaseAngle3;
		double       weaponHotspotRate;
		double       weaponMass;

		double    specialTurnRate;
		double    specialSpeedMax;
		double    specialAccelRate;
		int       specialFramesPerBattery;

		int turboOn;
		int turboTimeLeft;
		int numberOfTurbosUsed;
		int framesToRestartSound;

	public:
		KoanuaPatrolShip(Vector2 opos, double angle, ShipData *data, unsigned int
			code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_thrust();
		virtual double handle_speed_loss(SpaceLocation* source, double normal);

};

KoanuaPatrolShip::KoanuaPatrolShip(Vector2 opos, double angle,
ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	rawHotspotRate = tw_get_config_float("Ship", "HotspotRate", 0);

	shipSpeedMax = scale_velocity(tw_get_config_float("Ship", "SpeedMax",0));
	shipAccelRate = scale_acceleration(tw_get_config_float("Ship", "AccelRate", 0), rawHotspotRate);
	shipTurnRate = scale_turning(tw_get_config_float("Ship", "TurnRate", 0));
	shipSpeedMaxLossPerTurboUsed = tw_get_config_float("Ship", "SpeedMaxLossPerTurboUsed", 0);
	shipAccelLossPerTurboUsed = tw_get_config_float("Ship", "AccelLossPerTurboUsed", 0);
	shipTurnRateGainPerTurboUsed = tw_get_config_float("Ship", "TurnRateGainPerTurboUsed", 0);
	shipSpecialDrain = tw_get_config_int("Ship", "SpecialDrain", 0);

	weaponGroupOneActive = tw_get_config_int("Weapon", "GroupOneActive", 0);
	weaponGroupTwoActive = tw_get_config_int("Weapon", "GroupTwoActive", 0);
	weaponHotspotRate = tw_get_config_float("Weapon", "HotspotRate", 0);
	weaponFramesOfCoasting = tw_get_config_int("Weapon", "FramesOfCoasting", 0);
	weaponVelocity1 = scale_velocity(tw_get_config_float("Weapon", "Velocity1", 0));
	weaponDamage1   = tw_get_config_int("Weapon", "Damage1", 0);
	weaponArmour1   = tw_get_config_int("Weapon", "Armour1", 0);
	weaponMaxSpeed = scale_velocity(tw_get_config_float("Weapon", "MaxSpeed", 0));
	weaponDamage2   = tw_get_config_int("Weapon", "Damage2", 0);
	weaponArmour2   = tw_get_config_int("Weapon", "Armour2", 0);
	weaponRelativity = tw_get_config_float("Weapon", "Relativity", 0);
	weaponFramesToIgnition = tw_get_config_int("Weapon", "FramesToIgnition", 0);
	weaponFramesOfThrust = tw_get_config_int("Weapon", "FramesOfThrust", 0);
	weaponReleaseAngle1 = tw_get_config_float("Weapon", "ReleaseAngle1", 0)*ANGLE_RATIO;
	weaponReleaseAngle2 = tw_get_config_float("Weapon", "ReleaseAngle2", 0)*ANGLE_RATIO;
	weaponReleaseAngle3 = tw_get_config_float("Weapon", "ReleaseAngle3", 0)*ANGLE_RATIO;
	weaponAcceleration = scale_acceleration(tw_get_config_float("Weapon", "AccelRate",0), weaponHotspotRate);
	weaponMass = tw_get_config_float("Weapon", "Mass", 0);

	specialTurnRate  = scale_turning(tw_get_config_int("Special", "TurnRate", 0));
	specialAccelRate = scale_acceleration(tw_get_config_float("Special", "AccelRate", 0), rawHotspotRate);
	specialSpeedMax    = scale_velocity(tw_get_config_float("Special", "SpeedMax", 0));

	specialFramesPerBattery   = tw_get_config_int("Special", "FramesPerBattery", 0);
	turboTimeLeft=0;
	numberOfTurbosUsed=0;
	turboOn=0;
	framesToRestartSound = 999999;
}


int KoanuaPatrolShip::activate_special()
{
	STACKTRACE;

	turboTimeLeft=iround(specialFramesPerBattery * batt);
	framesToRestartSound = iround(specialFramesPerBattery * batt - 4500);
	if (framesToRestartSound<=0) framesToRestartSound = 0;
	batt = shipSpecialDrain;
	turn_rate = specialTurnRate;
	accel_rate = specialAccelRate;
	speed_max = specialSpeedMax;
	turboOn=TRUE;
	return(TRUE);
}


int KoanuaPatrolShip::activate_weapon()
{
	STACKTRACE;
	KoanuaMissile* K;
	if (weaponGroupOneActive) {
		K = new KoanuaMissile(0,
			angle+weaponReleaseAngle1+turn_step, weaponVelocity1, weaponDamage1,
			scale_range(1000), weaponArmour1,
			this, data->spriteWeapon, weaponRelativity);
		K->framesToIgnition = weaponFramesToIgnition;
		K->framesOfBurn = weaponFramesOfThrust;
		K->framesOfCoasting = weaponFramesOfCoasting;
		K->facingAngle = angle+turn_step;
		K->creator=this;
		K->damageAfterIgnition = weaponDamage2;
		K->armourAfterIgnition = weaponArmour2;
		K->acceleration = weaponAcceleration;
		K->mass = weaponMass;
		K->maxSpeed = weaponMaxSpeed;
		game->add(K);
	}
	if (weaponGroupTwoActive) {
		K = new KoanuaMissile(0,
			angle+weaponReleaseAngle2+turn_step, weaponVelocity1, weaponDamage1,
			scale_range(1000), weaponArmour1,
			this, data->spriteWeapon, weaponRelativity);
		K->framesToIgnition = weaponFramesToIgnition;
		K->framesOfBurn = weaponFramesOfThrust;
		K->framesOfCoasting = weaponFramesOfCoasting;
		K->facingAngle = angle+turn_step;
		K->creator=this;
		K->damageAfterIgnition = weaponDamage2;
		K->armourAfterIgnition = weaponArmour2;
		K->acceleration = weaponAcceleration;
		K->mass = weaponMass;
		K->maxSpeed = weaponMaxSpeed;
		game->add(K);

		K = new KoanuaMissile(0,
			angle+weaponReleaseAngle3+turn_step, weaponVelocity1, weaponDamage1,
			scale_range(1000), weaponArmour1,
			this, data->spriteWeapon, weaponRelativity);
		K->framesToIgnition = weaponFramesToIgnition;
		K->framesOfBurn = weaponFramesOfThrust;
		K->framesOfCoasting = weaponFramesOfCoasting;
		K->facingAngle = angle+turn_step;
		K->creator=this;
		K->damageAfterIgnition = weaponDamage2;
		K->armourAfterIgnition = weaponArmour2;
		K->acceleration = weaponAcceleration;
		K->mass = weaponMass;
		K->maxSpeed = weaponMaxSpeed;
		game->add(K);
	}
	return(TRUE);
}


void KoanuaPatrolShip::calculate()
{
	STACKTRACE;

	if (turboTimeLeft>0) {
		turboTimeLeft -= frame_time;
		//this->nextkeys |= keyflag::thrust;
		if (control)
			control->keys |= keyflag::thrust;
	}
	if (turboTimeLeft<=0&&turboOn==TRUE) {
		turboTimeLeft = 0;
		turn_rate = shipTurnRate;
		accel_rate = shipAccelRate;
		speed_max = shipSpeedMax;
		turboOn=FALSE;
		numberOfTurbosUsed++;
	}
	Ship::calculate();
	if (turboOn==TRUE)
		sprite_index = (get_index(ship->get_angle()) + (1 * 64));
	else
		sprite_index = (get_index(ship->get_angle()) + (0 * 64));

}


double KoanuaPatrolShip::handle_speed_loss(SpaceLocation *source, double normal)
{
	STACKTRACE;
	//vux limpets affect both normal speed and turbocharge speed,
	//turbo is not immune to slowdown!
	double speed_loss = normal;
	if (speed_loss > 0.0) {
		double sl = (30/(mass+30)) * speed_loss;
		if (sl > 1) tw_error ("Speed loss too large\n(%f)", sl);
		shipAccelRate *= 1 - sl * shipAccelRate / (shipAccelRate + scale_acceleration(2,4));
		shipSpeedMax *= 1 - sl * shipSpeedMax / (shipSpeedMax + scale_velocity(10));
		shipTurnRate *=  1 - sl * shipTurnRate / (shipTurnRate + scale_turning(4));
		specialAccelRate *= 1 - sl * specialAccelRate / (specialAccelRate + scale_acceleration(2,4));
		specialSpeedMax *= 1 - sl * specialSpeedMax / (specialSpeedMax + scale_velocity(10));
		specialTurnRate *=  1 - sl * specialTurnRate / (specialTurnRate + scale_turning(4));
		speed_loss = 0;
	}
	return Ship::handle_speed_loss(source, normal);
}


void KoanuaPatrolShip::calculate_turn_left()
{
	STACKTRACE;
	Ship::calculate_turn_left();
}


void KoanuaPatrolShip::calculate_turn_right()
{
	STACKTRACE;
	Ship::calculate_turn_right();
}


void KoanuaPatrolShip::calculate_thrust()
{
	STACKTRACE;
	Ship::calculate_thrust();
}


KoanuaMissile::KoanuaMissile(Vector2 opos, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double relativity)
:
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship,osprite, relativity)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	isBurning = FALSE;
	isCoasting = FALSE;
	facingAngle=oangle;

	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void KoanuaMissile::calculate(void)
{
	STACKTRACE;
	if (isBurning==FALSE && isCoasting==FALSE)
	if (framesToIgnition>=0) {
		framesToIgnition -= frame_time;
		sprite_index = (get_index(facingAngle) +
			(0 * 64));
	} else {
		framesToIgnition = 0;
		isBurning=TRUE;
		damage_factor = damageAfterIgnition;
		armour = armourAfterIgnition;
		range = 99999;
		sprite_index = (get_index(facingAngle) +
			(1 * 64));
		if (TRUE) play_sound2(this->creator->data->sampleWeapon[1]);

	}
	if (isBurning==TRUE && isCoasting==FALSE) {
		if (framesOfBurn>=0) {
			framesOfBurn -= frame_time;

			if (mass > 0)
				accelerate_gravwhip (this, facingAngle, acceleration / mass, maxSpeed);
		} else {
			framesOfBurn = 0;
			isBurning = FALSE;
			isCoasting = TRUE;
			sprite_index = (get_index(facingAngle) +
				(0 * 64));
		}
	}
	else if (isCoasting==TRUE) {
		framesOfCoasting -= frame_time;
		if (framesOfCoasting<=0) state=0;
	}
	Missile::calculate();

}


REGISTER_SHIP(KoanuaPatrolShip)
