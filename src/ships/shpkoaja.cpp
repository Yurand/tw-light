/* $Id: shpkoaja.cpp,v 1.6 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

class KoanuaJavelinMissile : public Missile
{
	public:
		KoanuaJavelinMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double
			Relativity);
		Ship* creator;
		double relativity;
		double facingAngle;
		double framesToIgnition;
		int isBurning;
		int isCoasting;
		double acceleration;
		double ignitionSpeed;
		double framesOfBurn;
		double framesOfCoasting;
		double maxSpeed;
		double damageAfterIgnition;
		double armourAfterIgnition;
		void calculate(void);
};

class KoanuaJavelin : public Ship
{
	public:

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

		double    specialRange;
		double    specialDamage;
		double    specialArmour;
		double    specialVelocity;
		int       specialMultiplicity;
		double    specialAngleSpread;
		double    specialVelocitySpread;
		double    specialRangeSpread;

		int specialOriginShift;

	public:
		KoanuaJavelin(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_thrust();
		virtual double handle_speed_loss(SpaceLocation* source, double normal);

};

KoanuaJavelin::KoanuaJavelin(Vector2 opos, double angle,
ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponGroupOneActive = get_config_int("Weapon", "GroupOneActive", 0);
	weaponGroupTwoActive = get_config_int("Weapon", "GroupTwoActive", 0);
	weaponHotspotRate = get_config_float("Weapon", "HotspotRate", 0);
	weaponFramesOfCoasting = get_config_int("Weapon", "FramesOfCoasting", 0);
	weaponVelocity1 = scale_velocity(get_config_float("Weapon", "Velocity1", 0));
	weaponDamage1   = get_config_int("Weapon", "Damage1", 0);
	weaponArmour1   = get_config_int("Weapon", "Armour1", 0);
	weaponMaxSpeed = scale_velocity(get_config_float("Weapon", "MaxSpeed", 0));
	weaponDamage2   = get_config_int("Weapon", "Damage2", 0);
	weaponArmour2   = get_config_int("Weapon", "Armour2", 0);
	weaponRelativity = get_config_float("Weapon", "Relativity", 0);
	weaponFramesToIgnition = get_config_int("Weapon", "FramesToIgnition", 0);
	weaponFramesOfThrust = get_config_int("Weapon", "FramesOfThrust", 0);
	weaponReleaseAngle1 = get_config_float("Weapon", "ReleaseAngle1", 0)*ANGLE_RATIO;
	weaponReleaseAngle2 = get_config_float("Weapon", "ReleaseAngle2", 0)*ANGLE_RATIO;
	weaponReleaseAngle3 = get_config_float("Weapon", "ReleaseAngle3", 0)*ANGLE_RATIO;
	weaponAcceleration = scale_acceleration(get_config_float("Weapon", "AccelRate",0), weaponHotspotRate);
	weaponMass = get_config_float("Weapon", "Mass", 0);

	specialRange  = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage = (get_config_float("Special", "Damage", 0));
	specialArmour = get_config_float("Special", "Armour", 0);
	specialMultiplicity = get_config_int("Special", "Multiplicity", 0);
	specialAngleSpread = get_config_float("Special", "AngleSpread", 0);
	specialVelocitySpread = scale_velocity(get_config_float("Special", "VelocitySpread", 0));
	specialRangeSpread = scale_range(get_config_float("Special", "RangeSpread", 0));
	specialOriginShift = 1;
}


int KoanuaJavelin::activate_special()
{
	STACKTRACE;
	int i;
	double angleShift, velocityShift, rangeShift;
	Shot* S;
	for(i=0; i<8; i++) {
		angleShift = random(-this->specialAngleSpread, this->specialAngleSpread) * PI / 180;
		velocityShift = (random(-this->specialVelocitySpread, this->specialVelocitySpread));
		rangeShift = (random(-this->specialRangeSpread, this->specialRangeSpread));
		//angleShift = 0; velocityShift = 0; rangeShift = 0;
		S = new Shot(this, Vector2(0.3 * this->size.x * specialOriginShift, 0.4 * this->size.y),
			this->angle + angleShift, this->specialVelocity + velocityShift,
			this->specialDamage, this->specialRange, this->specialArmour,
			this, this->data->spriteSpecial);
		//S = new Shot(this, Vector2(0,0), this->angle,
		// scale_velocity(80), 1, scale_range(40),
		// 1 ,this, data->spriteSpecial);
		game->add(S);
		specialOriginShift = -specialOriginShift;
	}

	return(TRUE);
}


int KoanuaJavelin::activate_weapon()
{
	STACKTRACE;
	KoanuaJavelinMissile* K;
	if (weaponGroupOneActive) {
		K = new KoanuaJavelinMissile(0,
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
		K = new KoanuaJavelinMissile(0,
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

		K = new KoanuaJavelinMissile(0,
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


void KoanuaJavelin::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if (special_recharge>0)
		sprite_index = (get_index(ship->get_angle()) + (1 * 64));
	else
		sprite_index = (get_index(ship->get_angle()) + (0 * 64));

}


double KoanuaJavelin::handle_speed_loss(SpaceLocation *source, double
normal)
{
	STACKTRACE;
	return Ship::handle_speed_loss(source, normal);
}


void KoanuaJavelin::calculate_turn_left()
{
	STACKTRACE;
	Ship::calculate_turn_left();
}


void KoanuaJavelin::calculate_turn_right()
{
	STACKTRACE;
	Ship::calculate_turn_right();
}


void KoanuaJavelin::calculate_thrust()
{
	STACKTRACE;
	Ship::calculate_thrust();
}


KoanuaJavelinMissile::KoanuaJavelinMissile(Vector2 opos, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
double relativity)
:
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship,osprite, relativity)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	isBurning = FALSE;
	isCoasting = FALSE;
	facingAngle=oangle;

}


void KoanuaJavelinMissile::calculate(void)
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


REGISTER_SHIP(KoanuaJavelin)
