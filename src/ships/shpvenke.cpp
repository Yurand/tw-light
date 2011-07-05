/* $Id: shpvenke.cpp,v 1.12 2004/03/24 23:51:43 yurand Exp $ */
#include "../ship.h"
#include "../melee/mview.h"
#include <math.h>

REGISTER_FILE

class VenKekThrust;

class VenKekFrigate : public Ship
{
	public:

		double shipRawHotspotRate;

		double weaponRange;
		double weaponVelocity;
		int    weaponDamage;
		int    weaponArmour;
		double weaponRelativity;
		double weaponMultiplicity;
		int weaponAngleVariance;

		double specialAddedDamagePerSecond;
		int    specialDuration;
		int    specialNoTurn;
		int    specialNoFullTurn;
		//double specialAddedAccel;
		//double specialAddedSpeedMax;
		//double specialAddedForwardAccel;
		//double specialAddedForwardSpeedMax; // need to add.
		//double specialAddedTurn; // probably negative!
		//double specialAddedFullTurn;

		double specialAddedAccelMultiplier;
		double specialAddedSpeedMaxMultiplier;
		double specialAddedTurnMultiplier;

		double thrustDamagePerSecond;
		double thrustPowerPerSecond;
		double thrustFullTurnMultiplier;
		double thrustForwardAccelMultiplier;
		double thrustForwardSpeedMaxMultiplier;

		double sparksRelativity;
		int    sparksMultiplicity;
		int    sparksAngleVarianceNormal;
		int    sparksAngleVarianceAfterburner;
		double sparksPerSecondNormal;
		double sparksPerSecondAfterburner;
		double sparksSpeedNormal;
		double sparksSpeedAfterburner;
		double sparksRangeNormal;
		double sparksRangeAfterburner;
		double sparksArmourNormal;
		double sparksArmourAfterburner;
		double sparksDamageNormal;
		double sparksDamageAfterburner;

		int ThrustIsCharging;
		double ThrustPowerLevel;
		int thrustSequence;
		int thrustSequenceCount;

		int launchPoint;

	public:
		int afterburner;
		int afterburner_counter;

		int spark_counter;
		int spark_threshhold;
		double sparkAngleOffset;

		int thrustFacingNumber;	 //-2, -1, 0, 1, or 2
		VenKekThrust* Thrust;
		VenKekFrigate(Vector2 opos, double angle, ShipData *data, unsigned int code);
		virtual ~VenKekFrigate(void);
	protected:
		void createThrust();
		void createSpark(int n);
		void destroyThrust();
		virtual void calculate_thrust();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		void calculate_turn_left();
		void calculate_turn_right();

	private:

};

class VenKekShot : public Missile
{
	public:
		VenKekShot(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, double orelativity);
		int spriteToUse;
		virtual void calculate();
};

class VenKekThrust : public Missile
{
	public:
		int facingNumber;
		int launched;
		int powerLevel;
		int afterburner;
		int thrustSequence;
		double damagePerSecond;
		VenKekFrigate* creator;
		VenKekThrust(VenKekFrigate* creator, Vector2 relPos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, double orelativity);
		virtual void calculate();
		void resetRange(void);
		virtual bool die(void);
		virtual void inflict_damage(SpaceObject* other);
		virtual int handle_damage(SpaceLocation* source, double normal, double direct = 0);
		virtual ~VenKekThrust(void);
};

VenKekFrigate::VenKekFrigate(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	shipRawHotspotRate = get_config_float("Ship", "HotspotRate", 0);

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponRelativity = get_config_float("Weapon", "Relativity", 0);
	weaponMultiplicity = get_config_int("Weapon", "Multiplicity" ,1);
	weaponAngleVariance = get_config_int("Weapon", "AngleVariance", 0);

	specialAddedDamagePerSecond = get_config_float("Special", "AddedDamagePerSecond", 0);
	specialDuration = get_config_int("Special", "Duration", 9);
	specialNoFullTurn = get_config_int("Special", "NoFullTurn", 0);
	specialNoTurn = get_config_int("Special", "NoTurn", 0);
	//specialAddedAccel = scale_acceleration(get_config_float("Special", "AddedAccel", 0), shipRawHotspotRate);
	//specialAddedForwardAccel = scale_acceleration(get_config_float("Special", "AddedForwardAccel", 0), shipRawHotspotRate);
	//specialAddedSpeedMax = scale_velocity(get_config_float("Special", "AddedSpeedMax", 0));
	//specialAddedForwardSpeedMax = scale_velocity(get_config_float("Special", "AddedForwardSpeedMax", 0));
	//specialAddedTurn = scale_turning(get_config_float("Special", "AddedTurn", 0));
	//specialAddedFullTurn = scale_turning(get_config_float("Special", "AddedFullTurn", 0));

	specialAddedAccelMultiplier = get_config_float("Special", "AddedAccelMultiplier", 0);
	specialAddedSpeedMaxMultiplier = get_config_float("Special", "AddedSpeedMaxMultiplier", 0);
	specialAddedTurnMultiplier = get_config_float("Special", "AddedTurnMultiplier", 0);

	thrustDamagePerSecond = get_config_float("Thrust", "DamagePerSecond", 0);
	thrustPowerPerSecond = get_config_float("Thrust", "PowerPerSecond", 0);
	thrustFullTurnMultiplier = (get_config_float("Thrust", "FullTurnMultiplier", 0));
	thrustForwardAccelMultiplier = get_config_float("Thrust", "ForwardAccelMultiplier", 0);
	thrustForwardSpeedMaxMultiplier = (get_config_float("Thrust" ,"ForwardSpeedMaxMultiplier", 0));

	sparksRelativity = get_config_float("Sparks", "Relativity", 0);
	sparksMultiplicity = get_config_int("Sparks", "Multiplicity", 0);
	sparksAngleVarianceNormal = get_config_int("Sparks", "AngleVarianceNormal", 0);
	sparksAngleVarianceAfterburner = get_config_int("Sparks", "AngleVarianceAfterburner", 0);
	sparksPerSecondNormal = get_config_float("Sparks", "SparksPerSecondNormal", 0);
	sparksPerSecondAfterburner = get_config_float("Sparks", "SparksPerSecondAfterburner", 0);
	sparksSpeedNormal = scale_velocity(get_config_float("Sparks", "SpeedNormal", 0));
	sparksSpeedAfterburner = scale_velocity(get_config_float("Sparks", "SpeedAfterburner", 0));
	sparksRangeNormal = scale_range(get_config_float("Sparks", "RangeNormal", 0));
	sparksRangeAfterburner = scale_range(get_config_float("Sparks", "RangeAfterburner", 0));
	sparksArmourNormal = get_config_float("Sparks", "ArmourNormal", 0);
	sparksArmourAfterburner = get_config_float("Sparks", "ArmourAfterburner", 0);
	sparksDamageNormal = get_config_float("Sparks", "DamageNormal", 0);
	sparksDamageAfterburner = get_config_float("Sparks", "DamageAfterburner", 0);

	launchPoint = -1;
	ThrustIsCharging = 0;
	ThrustPowerLevel = 0.00001;
	Thrust = NULL;
	thrustFacingNumber = 0;
	afterburner = FALSE;
	afterburner_counter = 0;
	thrustSequence = 0;
	thrustSequenceCount = 0;
	spark_counter = 0;
	spark_threshhold = 0;
	sparkAngleOffset = 0;

}


VenKekFrigate::~VenKekFrigate(void)
{
								 // possible crash problem.
	if (Thrust!=NULL && Thrust->exists()) Thrust->state = 0;
}


void VenKekFrigate::calculate()
{
	STACKTRACE;
	double powerNeeded;
	if (afterburner) {
		spark_threshhold = (int)((double)1000 / (double)sparksPerSecondAfterburner);
	} else {
		spark_threshhold = (int)((double)1000 / (double)sparksPerSecondNormal);
	}
	spark_counter += frame_time;
	if (spark_counter > spark_threshhold) {
		spark_counter -= spark_threshhold;
		this->createSpark(this->sparksMultiplicity);
	}
	thrustSequenceCount += frame_time;
	if (thrustSequenceCount>60) {
		thrustSequenceCount -= 60;
		thrustSequence = (++thrustSequence)%4;
	}
	if (afterburner) {
		afterburner_counter += frame_time;
		if (afterburner_counter > this->specialDuration) {
			afterburner = FALSE;
			afterburner_counter = 0;
		}
	}
	powerNeeded = frame_time / 1000.0 * thrustPowerPerSecond;
	if (thrust || turn_right || turn_left)
		this->batt -= powerNeeded;
	thrustFacingNumber = 0;
	if (turn_left && !turn_right && !thrust) thrustFacingNumber = 2;
	if (turn_left && !turn_right && thrust) thrustFacingNumber = 1;
	if (!turn_left && !turn_right && thrust) thrustFacingNumber = 0;
	if (turn_right && !turn_left && thrust) thrustFacingNumber = -1;
	if (turn_right && !turn_left && !thrust) thrustFacingNumber = -2;
	if (Thrust!=NULL) {
		Thrust->resetRange();
		Thrust->facingNumber = thrustFacingNumber;
	}
	Ship::calculate();
	if (turn_right || turn_left || thrust || afterburner) {
		this->createThrust();
		Thrust->afterburner = this->afterburner;
		Thrust->thrustSequence = this->thrustSequence;
	}
	else
		this->destroyThrust();
}


void VenKekFrigate::calculate_thrust(void)
{
	STACKTRACE;
	double powerNeeded, totalAccel, totalSpeedMax;
	if (afterburner&&(!turn_left)&&(!turn_right)) thrust=TRUE;
	if (afterburner&&specialNoFullTurn) thrust=TRUE;
	if (!thrust) return;
	powerNeeded = frame_time / 1000.0 * thrustPowerPerSecond;
	if (powerNeeded > this->batt) {
		thrust = false;
		return;
	}
	totalAccel = accel_rate;
	totalSpeedMax = speed_max;
	if ((!turn_left)&&(!turn_right)) {
		totalAccel *= thrustForwardAccelMultiplier;
		totalSpeedMax = totalSpeedMax * thrustForwardSpeedMaxMultiplier;
	}
	if (afterburner) {
		totalAccel *= specialAddedAccelMultiplier;
		totalSpeedMax = totalSpeedMax * specialAddedSpeedMaxMultiplier;
	}
	//if (afterburner&&(!turn_left)&&(!turn_right)) {
	//  totalAccel += specialAddedForwardAccel;
	//  totalSpeedMax += specialAddedForwardSpeedMax;
	//}
	accelerate_gravwhip(this, angle, (totalAccel) * frame_time, totalSpeedMax);
	//accelerate_gravwhip(this, angle, (totalAccel) * frame_time, speed_max);

}


void VenKekFrigate::calculate_turn_left()
{
	STACKTRACE;
	double powerNeeded, delta;
	if (afterburner&&specialNoTurn) turn_left = false;
	powerNeeded = frame_time / 1000.0 * thrustPowerPerSecond;
	if (powerNeeded > this->batt) {
		turn_left = false;
		return;
	}
	delta = turn_rate;
	if (!thrust) delta *= thrustFullTurnMultiplier;
	//if (afterburner&&!thrust) delta += specialAddedFullTurn;
	if (afterburner) delta *= specialAddedTurnMultiplier;
	if (turn_left)
		turn_step -= delta * frame_time;
}


void VenKekFrigate::calculate_turn_right()
{
	STACKTRACE;
	double powerNeeded, delta;
	if (afterburner&&specialNoTurn) turn_right = false;
	powerNeeded = frame_time / 1000.0 * thrustPowerPerSecond;
	if (powerNeeded > this->batt) {
		turn_right = false;
		return;
	}
	delta = turn_rate;
	if (!thrust) delta *= thrustFullTurnMultiplier;
	//if (afterburner&&!thrust) delta += specialAddedFullTurn;
	if (afterburner) delta *= specialAddedTurnMultiplier;
	if (turn_right)
		turn_step += delta * frame_time;
}


int VenKekFrigate::activate_weapon()
{
	STACKTRACE;
	double deflection, deflectionRad;
	int i;
	for(i=0;i<weaponMultiplicity;i++) {
		deflection = (random() % (weaponAngleVariance * 2 + 1))-weaponAngleVariance;
		deflectionRad = (double)deflection * ANGLE_RATIO;
		launchPoint = -launchPoint;
		game->add(new VenKekShot(size.y*launchPoint*(0.19), (size.y * 0.27),
			angle+deflectionRad, weaponVelocity, weaponDamage, weaponRange,
			weaponArmour, this, data->spriteWeapon, weaponRelativity));
	}
	return(TRUE);
}


int VenKekFrigate::activate_special()
{
	STACKTRACE;
	afterburner = TRUE;
	afterburner_counter = 0;
	return(TRUE);
}


void VenKekFrigate::createThrust(void)
{
	STACKTRACE;
	if (Thrust!=NULL) {
		Thrust->damage_factor = 1;
		Thrust->armour = 1;
		Thrust->state = 1;
		Thrust->damagePerSecond = this->thrustDamagePerSecond;
		return;
	}
	Thrust = new VenKekThrust(this, Vector2(0,this->size.x * 0.0), this->angle + PI,
		0, 1, -1, 1, this, data->spriteSpecial, 0);
	Thrust->creator = this;
	Thrust->vel = this->vel;
	Thrust->pos = this->pos;
	Thrust->damage_factor = 1;
	Thrust->armour = 1;
	game->add(Thrust);
	Thrust->afterburner = this->afterburner;
}


void VenKekFrigate::destroyThrust(void)
{
	STACKTRACE;
	if (Thrust==NULL) return;
	Thrust->state = 0;
	Thrust = NULL;
}


VenKekShot::VenKekShot(double ox, double oy, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double orelativity)
:
Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship, osprite, orelativity),
spriteToUse(odamage)
{
	STACKTRACE;
	;
}


VenKekThrust::VenKekThrust(VenKekFrigate* ocreator, Vector2 relPos, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double orelativity)
:
Missile(oship, relPos, oangle, ov, odamage, orange, oarmour, oship, osprite, orelativity)
{
	STACKTRACE;
	creator = ocreator;
	launched = FALSE;
	afterburner = creator->afterburner;
	powerLevel = 1;
	thrustSequence=0;
}


void VenKekThrust::calculate(void)
{
	STACKTRACE;
	int si;
	double rotDeg, rotRad, distMult;
	Vector2 relPos;

	if (!(creator && creator->exists())) {
		this->state = 0;
		creator = 0;
		return;
	}

	if (creator) {
		relPos = unit_vector(creator->angle) * creator->size.x;
		switch(facingNumber) {
			case(-2):
				rotDeg = 120;
				distMult = 0.65;
				creator->sparkAngleOffset = 0.5 * PI;
				break;
			case(-1):
				rotDeg = 150.5;
				distMult = 0.86;
				creator->sparkAngleOffset = 0.75 * PI;
				break;
			case(0):
				rotDeg = 180;
				distMult = 0.91;
				creator->sparkAngleOffset = PI;
				break;
			case(1):
				rotDeg = 209.5;
				distMult = 0.86;
				creator->sparkAngleOffset = 1.25 * PI;
				break;
			case(2):
				rotDeg = 240;
				distMult = 0.65;
				creator->sparkAngleOffset = 1.5 * PI;
				break;
			default:
				rotDeg = 180;
				distMult = 0.90;
				break;
		}
		rotRad = rotDeg * ANGLE_RATIO;
		relPos = relPos.rotate(rotRad) * distMult;
	}
	if (!launched && creator!=NULL && creator->exists()) {
		this->pos = creator->pos + relPos;
		this->changeDirection(creator->angle);
		this->v = creator->vel.magnitude();
		this->vel = creator->vel;
	}
	Missile::calculate();
	si = get_index(this->angle);
	si = (si + 8 * facingNumber) % 64;
	while(si<0) si += 64;
	//afterburner = TRUE;
	if (afterburner)
		//message.print(300,6,"afterburner =  %d", afterburner);
		sprite_index = si + (4 * 64) + (thrustSequence * 64);
	else
		sprite_index = si + (0 * 64) + (thrustSequence * 64);
}


void VenKekThrust::resetRange(void)
{
	STACKTRACE;
	this->d = 0;
	return;
}


bool VenKekThrust::die(void)
{
	STACKTRACE;
	state = 1;
	return(false);
}


int VenKekThrust::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	state = 1;
	return(0);
}


void VenKekThrust::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	damage_factor =  frame_time / 1000.0 * damagePerSecond;
	if (afterburner) damage_factor += frame_time / 1000.0 * creator->specialAddedDamagePerSecond;
	Shot::inflict_damage(other);
	if (other->isShip() || powerLevel <=1) {
		powerLevel = 0;
		return;
	}
	state = 1;
	this->damage_factor = 1;
	this->armour = 1;
	return;
}


VenKekThrust::~VenKekThrust(void)
{
	if (!creator) return;
	if (!creator->exists()) return;
	if (creator->state == 0) return;
	if (creator->Thrust == this) creator->Thrust = NULL;
}


void VenKekShot::calculate()
{
	STACKTRACE;
	Missile::calculate();
}


void VenKekFrigate::createSpark(int n)
{
	STACKTRACE;
	//Missile::Missile(SpaceLocation *creator, Vector2 rpos, double oangle,
	//	double ov, double odamage, double orange, double oarmour,
	//	SpaceLocation *opos, SpaceSprite *osprite, double relativity)

	Missile* M;
	int i, dAngle;
	double dAngleRad;
	if (this->Thrust==NULL) return;
	if (!this->Thrust->exists()) return;
	for(i=0; i<n; i++) {
		if (afterburner)
			dAngle = random()%(2 * sparksAngleVarianceAfterburner + 1) - sparksAngleVarianceAfterburner;
		else
			dAngle = random()%(2 * sparksAngleVarianceNormal + 1) - sparksAngleVarianceNormal;
		dAngleRad = (double)dAngle * ANGLE_RATIO;
		if (afterburner)
			M = new Missile(this->Thrust, Vector2(0,this->size.y * -0.375), this->sparkAngleOffset+this->angle + dAngleRad,
				this->sparksSpeedAfterburner, this->sparksDamageAfterburner,
				this->sparksRangeAfterburner, this->sparksArmourAfterburner,
				this, this->data->spriteExtra, this->sparksRelativity);
		else
			M = new Missile(this->Thrust, Vector2(0,this->size.y * -0.375), this->sparkAngleOffset+this->angle + dAngleRad,
				this->sparksSpeedNormal, this->sparksDamageNormal,
				this->sparksRangeNormal, this->sparksArmourNormal,
				this, this->data->spriteExtra, this->sparksRelativity);
		game->add(M);
	}

}


REGISTER_SHIP(VenKekFrigate)
