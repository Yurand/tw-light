/* $Id: shpgahmo.cpp,v 1.12 2006/04/22 09:24:53 geomannl Exp $ */
#include "../ship.h"
#include "../melee/mview.h"

REGISTER_FILE

class GahmurMonitor : public Ship
{
	public:
	public:
		double       shipWeaponDrainPerSecond;
		double       shipSpecialInitialDrain;
		double       shipSpecialDrainPerSecond;
		double       shipSpecialLockedDrainPerSecond;

		int          weaponMinChargeTime;
		int          weaponMaxChargeTime;
		double       weaponMinChargeRange;
		double       weaponMaxChargeRange;
		double       weaponMinChargeBeamRange;
		double       weaponMaxChargeBeamRange;
		double       weaponMinChargeVelocity;
		double       weaponMaxChargeVelocity;
		double       weaponSpeedChangeFactor;
		double       weaponMinChargeDamage;
		double       weaponMaxChargeDamage;
		double       weaponMinChargeBeamDamage;
		double       weaponMaxChargeBeamDamage;
		double       weaponRange;
		double       weaponVelocity;
		double       weaponDamage;
		double       weaponHome;
		double       weaponMinBatteryToCharge;
		double       weaponMinChargeBeamDivergenceAngle;
		double       weaponMaxChargeBeamDivergenceAngle;
		int          weaponBeamEnabled;
		int          weaponStopsDynamo;
		double       weaponVulnerabilityFactor;
		int          weaponAlwaysFireOnEmptyBattery;
		int          weaponAlwaysFireOnMaxChargeTime;

		double       specialNormalDamagePerSecond;
		double       specialDirectDamagePerSecond;
		double       specialLengthMultiplier;
		double       specialPowerMultiplier;
		double       specialDecayMultiplier;
		int          specialStopsDynamo;
		double       specialInitialEnergy;
		double       specialMaxLength;
		double       specialMaxEnergy;
		double       specialVelocityCouplingFactor;

		bool         isCharging;
		bool         wasFiredTooEarly;
		bool         isLaunching;
		bool         isBeaming;
		int          chargingTime;
		int          oldChargingTime;
		int          tractorLockTime;
		int          oldTractorLockTime;
		bool         isTractoring;
		double       tractorPower;
		double       tractorLength;
		double       tractorAngle;
		int          tractorSpriteIndex;
		int          last_fire_special;
		int          last_fire_weapon;
		SpaceObject* tractorTarget;
		SpaceObject* oldTractorTarget;
		SpaceObject* holdoverTractorTarget;
		SpaceLocation* tractorSource;
		double       defaultFiringAngle;

	public:
		GahmurMonitor(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void PowerTractor();
		virtual void DecayTractor();
		virtual void CalculateTractor();
		virtual void CalculateTractorEffect();
		virtual void LaunchPlasma();
		virtual void BeamPlasma();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void death();
		virtual bool die();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);

};

class GahmurPlasma : public HomingMissile
{
	public:

		static SpaceSprite *spriteWeaponExplosion;
		int frame_count;
		int max_damage;
		int sprite_index_override;

	public:
		double vulnerabilityFactor;

		double speedChangeFactor;
		GahmurPlasma(Vector2 opos, double oangle, double ov, int odamage,
			double orange, double otrate, Ship *oship, SpaceSprite *osprite, int ofcount);

		virtual void SetMaxDamage(double omaxDamage);
		virtual void calculate();

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		void animate (Frame *frame);

};

class GahmurTractor:public Laser
{
	public:
	public:
		GahmurTractor(GahmurMonitor *ocreator, double langle, int lcolor, double lrange, double ldamage,
			int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle);
		virtual void inflict_damage(SpaceObject *other);

		virtual int canCollide(SpaceLocation *other);

		virtual void calculate();
		GahmurMonitor* creator;
		int explosionSpriteIndex;
		double normalDamagePerSecond;
		double directDamagePerSecond;
};

GahmurTractor::GahmurTractor(GahmurMonitor *ocreator, double langle, int lcolor, double lrange, double ldamage,
int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle)
:
Laser(ocreator, langle, lcolor, lrange, ldamage,
lfcount, opos, rpos, osinc_angle)
{
	STACKTRACE;
	creator = ocreator;
	normalDamagePerSecond = ocreator->specialNormalDamagePerSecond;
	directDamagePerSecond = ocreator->specialDirectDamagePerSecond;
	set_depth(DEPTH_SPECIAL);
	data->spriteSpecialExplosion = creator->data->spriteSpecialExplosion;
	explosionSpriteIndex = 0;
}


void GahmurTractor::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	double x1, x2;
	x1 = (normalDamagePerSecond / 1000.0) * (double)frame_time;
	x2 = (directDamagePerSecond / 1000.0) * (double)frame_time;
	damage(other, x1, x2);

	if (creator && creator->exists()) {
		if (creator->tractorTarget==NULL) {
			creator->tractorTarget = other;
			creator->tractorLength = other->distance(creator->tractorSource);
			creator->tractorAngle = creator->tractorSource->trajectory_angle(other);
		} else {
			if (creator->distance(creator->tractorTarget) > creator->distance(other)) {
				creator->tractorTarget = other;
				creator->holdoverTractorTarget = other;
				if (other->state==0) creator->isTractoring = false;
			} else {
				creator->holdoverTractorTarget = creator->tractorTarget;;
				if (!creator->tractorTarget->exists()) creator->isTractoring = false;
			}
		}
	}
	collide_flag_anyone = collide_flag_sameship = collide_flag_sameteam = 0;
	physics->add(new Animation( this,
		pos + edge(), data->spriteSpecialExplosion, explosionSpriteIndex,
		1, 15, DEPTH_EXPLOSIONS));

	//note:  Laser::inflict_damage is NOT called!
}


void GahmurTractor::calculate()
{
	STACKTRACE;
	if (creator && !creator->exists())
		creator = 0;

	Laser::calculate();
}


int GahmurTractor::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	if (other->isShot())
		return false;

	if (other->isLine())
		return false;

	return Laser::canCollide(other);
}


GahmurMonitor::GahmurMonitor(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	shipWeaponDrainPerSecond =  tw_get_config_float("Ship", "WeaponDrainPerSecond", 0);
	shipSpecialInitialDrain = tw_get_config_float("Ship", "SpecialInitialDrain", 0);
	shipSpecialDrainPerSecond = tw_get_config_float("Ship", "SpecialDrainPerSecond", 0);
	shipSpecialLockedDrainPerSecond = tw_get_config_float("Ship", "SpecialLockedDrainPerSecond", 0);

	weaponMinChargeRange    = scale_range(tw_get_config_float("Weapon", "MinChargeRange", 0));
	weaponMaxChargeRange    = scale_range(tw_get_config_float("Weapon", "MaxChargeRange", 0));
	weaponMinChargeBeamRange= scale_range(tw_get_config_float("Weapon", "MinChargeBeamRange", 0));
	weaponMaxChargeBeamRange= scale_range(tw_get_config_float("Weapon", "MaxChargeBeamRange", 0));

	weaponMinChargeVelocity = scale_velocity(tw_get_config_float("Weapon", "MinChargeVelocity", 0));
	weaponMaxChargeVelocity = scale_velocity(tw_get_config_float("Weapon", "MaxChargeVelocity", 0));

	weaponMinChargeDamage =   tw_get_config_float("Weapon", "MinChargeDamage", 0);
	weaponMaxChargeDamage =   tw_get_config_float("Weapon", "MaxChargeDamage", 0);
	weaponMinChargeBeamDamage = tw_get_config_float("Weapon", "MinChargeBeamDamage", 0);
	weaponMaxChargeBeamDamage = tw_get_config_float("Weapon", "MaxChargeBeamDamage", 0);

	weaponSpeedChangeFactor = tw_get_config_float("Weapon", "SpeedChangeFactor", 0.0);
	weaponMinChargeTime =     tw_get_config_int("Weapon", "MinChargeTime", 0);
	weaponMaxChargeTime =     tw_get_config_int("Weapon", "MaxChargeTime", 0);
	weaponMinBatteryToCharge = tw_get_config_float("Weapon", "MinBatteryToCharge", 0);
	weaponMinChargeBeamDivergenceAngle = tw_get_config_float("Weapon", "MinChargeBeamDivergenceAngle", 0) * PI / 180;
	weaponMaxChargeBeamDivergenceAngle = tw_get_config_float("Weapon", "MaxChargeBeamDivergenceAngle", 0) * PI / 180;
	weaponBeamEnabled = tw_get_config_int("Weapon", "BeamEnabled", 0);
	weaponStopsDynamo =       tw_get_config_int("Weapon", "StopsDynamo", 0);
	weaponHome     =          scale_turning(tw_get_config_float("Weapon", "Homing", 0));
	weaponVulnerabilityFactor = tw_get_config_float("Weapon", "VulnerabilityFactor", 0);
	weaponAlwaysFireOnEmptyBattery = tw_get_config_int("Weapon", "AlwaysFireOnEmptyBattery", 0);
	weaponAlwaysFireOnMaxChargeTime = tw_get_config_int("Weapon", "AlwaysFireOnMaxChargeTime", 0);

	specialNormalDamagePerSecond  = tw_get_config_float("Special", "NormalDamagePerSecond", 0);
	specialDirectDamagePerSecond  = tw_get_config_float("Special", "DirectDamagePerSecond", 0);
	specialLengthMultiplier = tw_get_config_float("Special", "LengthMultiplier", 1.0);
	specialPowerMultiplier =  tw_get_config_float("Special", "PowerMultiplier", 1.0);
	specialDecayMultiplier =  tw_get_config_float("Special", "DecayMultiplier", 1.0);
	specialStopsDynamo =      tw_get_config_int("Special", "StopsDynamo", 0);
	specialInitialEnergy =   (tw_get_config_float("Special", "InitialEnergy", 0));
	specialMaxLength =        scale_range(tw_get_config_float("Special", "MaxLength", 0));
	specialMaxEnergy =        tw_get_config_float("Special", "MaxEnergy", 0);
	specialVelocityCouplingFactor = tw_get_config_float("Special", "VelocityCouplingFactor", 0);

	isCharging = false;
	isLaunching = false;
	isBeaming = false;
	isTractoring = false;
	chargingTime = 0;
	oldChargingTime = 0;
	tractorLockTime = 0;
	oldTractorLockTime = 0;
	tractorPower = 0;
	tractorLength = 0;
	tractorAngle = 0;
	tractorTarget = NULL;
	oldTractorTarget = NULL;
	holdoverTractorTarget = NULL;
	tractorSource = new SpaceLocation(this, Vector2(0,0),0);
	tractorSpriteIndex = 0;
	last_fire_special = 0;
	last_fire_weapon = 0;
	wasFiredTooEarly = false;
	defaultFiringAngle = 0.0001;
}


int GahmurMonitor::activate_weapon()
{
	STACKTRACE;
	return(FALSE);				 //entirely handled in the calculate section.
}


int GahmurMonitor::activate_special()
{
	STACKTRACE;
	return(FALSE);				 //entirely handled in the calculate section.
}


void GahmurMonitor::calculate()
{
	STACKTRACE;
	double requiredWeaponEnergy;
	double requiredSpecialEnergy;
	if (state==0) {
		LaunchPlasma();
		Ship::calculate();
		return;
	}
	requiredWeaponEnergy = (((double)shipWeaponDrainPerSecond / 1000.0) * (double)frame_time);
	requiredSpecialEnergy = (((double)shipSpecialDrainPerSecond / 1000.0) * (double)frame_time);

	if (((fire_weapon==0)||(weaponBeamEnabled==0))&&(fire_special==1)&&(last_fire_special==0)) {
		if (batt>=shipSpecialInitialDrain || isTractoring) {
			isTractoring = !isTractoring;
			if (isTractoring) {
				batt -= shipSpecialInitialDrain;
				tractorPower = specialInitialEnergy;
				tractorAngle = angle;
			}
		}
	}

	if ((fire_weapon==1) && (last_fire_weapon==0) && (batt>weaponMinBatteryToCharge)) {
		isCharging = true;
		isLaunching = false;
		isBeaming = false;
								 //maybe need THIS line!
		if (!wasFiredTooEarly) chargingTime = 0;
	}

	if (((fire_weapon==0) && (last_fire_weapon==1))||wasFiredTooEarly) {
		if ((chargingTime>=weaponMinChargeTime)&&isCharging) {
			isCharging = false;
			if (fire_special && weaponBeamEnabled) {
				isBeaming = true;
				isLaunching = false;
				wasFiredTooEarly = false;
			} else {
				isBeaming = false;
				isLaunching = true;
				wasFiredTooEarly = false;
			}
		} else {
			if (isCharging) {
				wasFiredTooEarly = true;
				isCharging = true;
			}
		}
	}

	if (isCharging) {
		if (batt>requiredWeaponEnergy && chargingTime < weaponMaxChargeTime) {
			chargingTime += frame_time;
			batt -= requiredWeaponEnergy;
			update_panel = TRUE;
		} else {
			if (fire_weapon==0) {
								 // a kludge
				isCharging = false;
				wasFiredTooEarly = false;
			}
			if (weaponAlwaysFireOnEmptyBattery) {
				isCharging = false;
				wasFiredTooEarly = false;
				if (fire_special) {
					isLaunching = false;
					isBeaming = true;
				} else {
					isLaunching = true;
					isBeaming = false;
				}
			}
		}
	} else {
								 //not charging
		//chargingTime = 0;
	}

	if ((int)chargingTime/(int)350 != (int)oldChargingTime/(int)350) {
		double fracDone = (double)chargingTime / (double)weaponMaxChargeTime;
		play_sound2((data->sampleWeapon[0]), iround(150 + 150 * fracDone), iround(1000.0 + 300 * (fracDone-.5)));
	}
	if (chargingTime>=weaponMaxChargeTime) {
		chargingTime = weaponMaxChargeTime;
		if (weaponAlwaysFireOnMaxChargeTime) {
			isCharging = false;
			wasFiredTooEarly = false;
			if (fire_special) {
				isLaunching = false;
				isBeaming = true;
			} else {
				isLaunching = true;
				isBeaming = false;
			}
		}
	}

	if (isLaunching) {
		LaunchPlasma();
		isLaunching = false;
		isBeaming = false;
		isCharging = false;
	}
	if (isBeaming) {
		BeamPlasma();
		isLaunching = false;
		isBeaming = false;
		isCharging = false;
	}
	if (isCharging==false) {
		chargingTime = 0;
		oldChargingTime = -1000;
	}
								 //should freeze the dynamo
	if (isCharging&&weaponStopsDynamo)recharge_step = recharge_rate;
								 //should freeze the dynamo
	if (isTractoring&&specialStopsDynamo)recharge_step = recharge_rate;

	if (isTractoring)
		PowerTractor();
	else
		DecayTractor();

	CalculateTractor();
	CalculateTractorEffect();
	last_fire_special = fire_special;
	last_fire_weapon = fire_weapon;
	oldChargingTime = chargingTime;
	holdoverTractorTarget = NULL;
	Ship::calculate();
}


void GahmurMonitor::LaunchPlasma()
{
	STACKTRACE;
	GahmurPlasma* GP;
	double ta, rta, fracDone;
	Vector2 SV, TV;
	if (chargingTime<weaponMinChargeTime) {
		chargingTime = 0;
		return;					 //flushed...
	}
	if (weaponMinChargeTime == weaponMaxChargeTime) fracDone = 0.5;
	else fracDone = (double)(chargingTime - weaponMinChargeTime) / (double)(weaponMaxChargeTime - weaponMinChargeTime);
	if (fracDone<0.00) fracDone = 0.00;
	if (fracDone>1.00) fracDone = 1.00;
	weaponDamage = (1.0-fracDone) * weaponMinChargeDamage + fracDone * weaponMaxChargeDamage;
	weaponRange = (1.0-fracDone) * weaponMinChargeRange + fracDone * weaponMaxChargeRange;
	weaponVelocity = (1.0-fracDone) * weaponMinChargeVelocity + fracDone * weaponMaxChargeVelocity;

	if (target)ta = trajectory_angle(target);
	else {
		ta = angle + defaultFiringAngle;
		defaultFiringAngle = -defaultFiringAngle;
	}
	rta = angle - ta;
	while(rta<0) rta += PI2;
	while(rta>PI2) rta-= PI2;
	if (rta>PI) SV = unit_vector(0) * size.y * 0.24;
	else SV = unit_vector(PI) * size.y * 0.24;
	TV = unit_vector(-ta+angle+PI/2) * size.y * 0.20;
	GP = (new GahmurPlasma(TV+SV, ta,
		weaponVelocity, iround(weaponDamage), weaponRange, weaponHome, this,
		data->spriteWeapon, 64));
	GP->speedChangeFactor = weaponSpeedChangeFactor;
	GP->SetMaxDamage(weaponDamage);
	GP->vulnerabilityFactor = weaponVulnerabilityFactor;
	game->add(GP);
	chargingTime = 0;
	oldChargingTime = -1000;
	isCharging = false;
	play_sound2((data->sampleWeapon[1]), iround(120.0 + 400.0 * fracDone), iround(1500.0 - (1000.0 * fracDone)));

}


void GahmurMonitor::BeamPlasma()
{
	STACKTRACE;
	double fracDone;
	double weaponBeamDivergenceAngle;
	Vector2 SV, TV;
	if (chargingTime<weaponMinChargeTime) {
		chargingTime = 0;
		return;					 //flushed...
	}
	if (weaponMinChargeTime == weaponMaxChargeTime) fracDone = 0.5;
	else fracDone = (double)(chargingTime - weaponMinChargeTime) / (double)(weaponMaxChargeTime - weaponMinChargeTime);
	if (fracDone<0.00) fracDone = 0.00;
	if (fracDone>1.00) fracDone = 1.00;
	weaponDamage = (1.0-fracDone) * weaponMinChargeBeamDamage + fracDone * weaponMaxChargeBeamDamage;
	weaponRange = (1.0-fracDone) * weaponMinChargeBeamRange + fracDone * weaponMaxChargeBeamRange;
	weaponBeamDivergenceAngle = (1.0-fracDone) * weaponMinChargeBeamDivergenceAngle + fracDone * weaponMaxChargeBeamDivergenceAngle;
	game->add(new Laser(this, angle+weaponBeamDivergenceAngle, tw_get_palete_color(5), weaponRange, weaponDamage,
		100, this, Vector2(size.x * 0.27, size.x * 0.13), true));
	game->add(new Laser(this, angle-weaponBeamDivergenceAngle, tw_get_palete_color(5), weaponRange, weaponDamage,
		100, this, Vector2(size.x * -0.27, size.x * 0.13), true));
	chargingTime = 0;
	oldChargingTime = -1000;
	isCharging = false;
	play_sound2((data->sampleExtra[0]), iround(150 + 150 * fracDone), iround(1000.0 * (1.5 - fracDone)));
}


void GahmurMonitor::PowerTractor()
{
	STACKTRACE;
	double energy;
	if (tractorTarget) {
		tractorLockTime += frame_time;
		energy = (shipSpecialLockedDrainPerSecond / 1000.0) * (double)frame_time;
	}
	else {
		energy = (shipSpecialDrainPerSecond / 1000.0) * (double)frame_time;
		tractorLockTime = 0;  oldTractorLockTime = 0;
	}
	if (batt>energy) {
		tractorPower += (specialPowerMultiplier / 1000.0) * (double)frame_time;
		batt -= energy;
	}
	else
		isTractoring = false;
	if (tractorPower>specialMaxEnergy)
		tractorPower=specialMaxEnergy;
}


void GahmurMonitor::DecayTractor()
{
	STACKTRACE;
	tractorPower -= (specialDecayMultiplier / 1000.0) * (double)frame_time;
	if (tractorPower<0.0) {
		tractorLockTime = 0;
		tractorLength = 0;
		tractorPower = 0.0;
		oldTractorTarget = NULL;
		holdoverTractorTarget = NULL;
		tractorTarget = NULL;
		tractorAngle = 0;
		isTractoring = false;
	}
}


void GahmurMonitor::CalculateTractor()
{
	STACKTRACE;

	if (tractorTarget && !tractorTarget->exists())
		tractorTarget = 0;

	GahmurTractor* GT;

	tractorSource->pos = pos + unit_vector(angle) * size.y * 0.3;

	if (tractorPower<0.0001)
		return;

	tractorLength = scale_range(1) * tractorPower * specialLengthMultiplier;

	if (tractorLength>specialMaxLength)
		tractorLength=specialMaxLength;

	if (tractorTarget != NULL) {
		if (tractorTarget->exists())
			tractorAngle = tractorSource->trajectory_angle(tractorTarget);
		else
			isTractoring = false;

		GT = new GahmurTractor(this, tractorAngle, tw_get_palete_color(6+(int)isTractoring), tractorLength, 0,
			50, tractorSource, Vector2(0,0), true);

		if (tractorTarget->exists()) {
			if (tractorSource->distance(tractorTarget)>tractorLength + scale_range(0.5))
				tractorTarget = NULL;
		}

		game->add(GT);
		if (oldTractorTarget!=tractorTarget)
			play_sound2((data->sampleSpecial[0]));
		if ((++tractorSpriteIndex)>5) tractorSpriteIndex = 0;
		GT->explosionSpriteIndex = tractorSpriteIndex;
	}
	else {						 //tractorTarget == NULL here
		GT = new GahmurTractor(this, angle, tw_get_palete_color(9+(int)isTractoring), tractorLength, 0,
			50, tractorSource, Vector2(0,0), true);
		tractorAngle = angle;
		game->add(GT);
		if (oldTractorTarget) isTractoring = false;
	}
	GT->normalDamagePerSecond = specialNormalDamagePerSecond;
	GT->directDamagePerSecond = specialDirectDamagePerSecond;
	oldTractorTarget = tractorTarget;

	if ((oldTractorLockTime/1000 != tractorLockTime/1000) && (holdoverTractorTarget)) {
		play_sound2((data->sampleSpecial[1]), 200);
	}

	oldTractorLockTime = tractorLockTime;
}


void GahmurMonitor::CalculateTractorEffect()
{
	STACKTRACE;
	Vector2 TM;
	double tmass;
	double vtransfer;
	if (tractorTarget==NULL || (!tractorTarget->exists())) {
		//tractorLockTime = 0; //added?
		//oldTractorLockTime = 0; //added???
		//oldTractorTarget = NULL; //added??
		//holdoverTractorTarget = NULL;
		//tractorTarget = NULL;
		//isTractoring = false;
		return;
	}
	if (tractorTarget->isShot()) {
		tractorTarget->pos = pos + unit_vector(tractorAngle) * tractorLength;
		tractorTarget->set_vel ( vel );
		return;
	}
	if (tractorTarget->mass>0.01) {
		//tractorTarget->pos = pos + unit_vector(tractorAngle) * tractorLength;
		tmass = mass + tractorTarget->mass;

		if (tmass > 0)
			TM = (vel * mass + tractorTarget->vel * tractorTarget->mass)/tmass;
		else
			TM = vel;

		vtransfer = (specialVelocityCouplingFactor * frame_time) / (1+(specialVelocityCouplingFactor * frame_time));
		vel = vel * (1-vtransfer) + TM * vtransfer;
		tractorTarget->set_vel(tractorTarget->vel * (1-vtransfer) + TM * vtransfer);
		return;
	}
}


int GahmurMonitor::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	int x;
	x = Ship::handle_damage(source, normal, direct);
	if (crew<=0.0) {
		LaunchPlasma();
	}
	return x;
}


void GahmurMonitor::death()
{
	STACKTRACE;
	//message.print(1000,10,"death!!!!!");
	//LaunchPlasma();
	Ship::death();
}


bool GahmurMonitor::die()
{
	STACKTRACE;
	//message.print(1000,11,"die!!!!!");
	//LaunchPlasma();
	return Ship::die();
}


SpaceSprite *GahmurPlasma::spriteWeaponExplosion = NULL;
GahmurPlasma::GahmurPlasma(Vector2 opos, double oangle, double ov,
int odamage, double orange, double otrate, Ship *oship,
SpaceSprite *osprite, int ofcount) :
HomingMissile( oship, opos, oangle, ov, odamage, orange, 0, otrate, oship,
osprite, oship->target),
frame_count(ofcount),
max_damage(odamage)
{
	STACKTRACE;
	spriteWeaponExplosion = data->spriteWeaponExplosion;
	speedChangeFactor = 0.0;
	attributes &= ~ATTRIB_STANDARD_INDEX;
	sprite_index_override = 0;
	vulnerabilityFactor = 0.0;
}


void GahmurPlasma::calculate()
{
	STACKTRACE;
	int damageFactorSprite;
	int base_sprite_index;
	HomingMissile::calculate();
								 //half of the friction formula... no range change, though.
	v *= 1 + speedChangeFactor * game->frame_time;
								 //here's the friction modifier for range changes.
	range = d + (range - d) * (1 + speedChangeFactor * game->frame_time);
	//note, a positive value is acceleration, a negative value deceleration!
	base_sprite_index = (int)(angle / (PI2/64)) + 16;
	base_sprite_index &= 63;
	damage_factor = max_damage - (int)((d / range) * (double)(max_damage));
	damageFactorSprite = (int)(damage_factor + 0.5);
	if (damageFactorSprite>14) damageFactorSprite = 14;
	if (damageFactorSprite<0) damageFactorSprite = 0;
	sprite_index_override = damageFactorSprite * 64 + base_sprite_index;
	sprite_index = sprite_index_override;
}


void GahmurPlasma::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	SpaceObject::inflict_damage(other);
	if (!other->isShot()) {
		if (other->exists()) {
			add(new FixedAnimation(this, other,
				spriteWeaponExplosion, 0, 20, 50, DEPTH_EXPLOSIONS));
		}
		else {
			add(new Animation(this, other->normal_pos(),
				spriteWeaponExplosion, 0, 20, 50, DEPTH_EXPLOSIONS));
		}
		state = 0;
	}
}


int GahmurPlasma::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	double total = (normal + direct)*vulnerabilityFactor;

	if (total) {
		d += total / max_damage * range;
		if (d >= range) state = 0;
	}
	return 1;
}


void GahmurPlasma::animate (Frame *frame)
{
	//sprite_index = sprite_index_override;	//xx ooh, this is SO wrong! Leads to desynches
	//if there is a better way, TELL ME!!!  --> it's already in the ::calculate routine, so it doesn't need to be here
	Shot::animate(frame);
}


void GahmurPlasma::SetMaxDamage(double omaxDamage)
{
	STACKTRACE;
	max_damage = iround(omaxDamage);
}


REGISTER_SHIP(GahmurMonitor)
