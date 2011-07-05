/* $Id: shpneccr.cpp,v 1.17 2005/08/02 00:23:46 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

#include "../melee.h"
#include "../melee/mshot.h"
#include "../melee/mship.h"
#include "../melee/manim.h"

class NechanziCruiser;

class NechanziMissile : public Missile
{
	public:
	public:
		NechanziMissile(double ox, double oy, double oangle, double ov, int
			odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double
			Relativity);
		Ship* creator;
		double relativity;
		double facingAngle;
		double framesToIgnition;
		int isActivated;
		int isBurning;
		int isCoasting;
		int missileType;		 //0, 1, or 2
		double acceleration;
		double mass;
		double ignitionSpeed;
		double framesOfBurn;
		double framesOfCoasting;
		double maxSpeed;
		double damageAfterIgnition;
		double armourAfterIgnition;
		int stunFrames;
		virtual void calculate(void);
		virtual void inflict_damage(SpaceObject *other);
};
/*
class NechanziStun: public SpaceObject {
public:
  Ship *ship;
  int   stunframe;
  int   stunframe_count;
  int   frame_step;
  int   frame_size;
  int   frame_count;

  public:
  NechanziStun(Ship *oship, SpaceSprite *osprite, int ofcount,
	int ofsize, int disableFrames);

  virtual void calculate();
};
*/

class NechanziCruiser : public Ship
{
	public:
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;
		//double       weaponTurnRate;

		///double       specialRange1;
		///double       specialRange2;
		///double       specialRange3;
		double       specialVelocity1;
		double       specialVelocity2;
		double       specialVelocity3;
		double       specialMaxSpeed1;
		double       specialMaxSpeed2;
		double       specialMaxSpeed3;
		int          specialDamage1;
		int          specialDamage2;
		int          specialDamage3;
		int          specialArmour1;
		int          specialArmour2;
		int          specialArmour3;
		int          specialFramesToIgnition1;
		int          specialFramesToIgnition2;
		int          specialFramesToIgnition3;
		int          specialFramesOfThrust1;
		int          specialFramesOfThrust2;
		int          specialFramesOfThrust3;
		int          specialFramesOfCoasting1;
		int          specialFramesOfCoasting2;
		int          specialFramesOfCoasting3;
		double       specialAcceleration1;
		double       specialAcceleration2;
		double       specialAcceleration3;
		double       specialReleaseAngle1;
		double       specialReleaseAngle2;
		double       specialReleaseAngle3A;
		double       specialReleaseAngle3B;
		double       specialHotspotRate;
		double       specialMass;
		double       specialRelativity;
		double       specialStunFrames;

		int specialFired;
		int deathTimer;
		int isDead;

	public:
		NechanziCruiser(Vector2 opos, double angle, ShipData *data, unsigned int
			code);

	protected:
		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void death();
};

NechanziCruiser::NechanziCruiser(Vector2 opos, double angle, ShipData *data,
unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	//weaponTurnRate = scale_turning(tw_get_config_float("Weapon", "TurnRate", 0));

	specialHotspotRate = tw_get_config_int("Special", "HotspotRate",0);
	specialMass = tw_get_config_int("Special", "Mass", 0);
	///specialRange1  =   scale_range(tw_get_config_float("Special", "Range1", 0));
	///specialRange2  =   scale_range(tw_get_config_float("Special", "Range2", 0));
	///specialRange3  =   scale_range(tw_get_config_float("Special", "Range3", 0));
	specialVelocity1 = scale_velocity(tw_get_config_float("Special", "Velocity1", 0));
	specialVelocity2 = scale_velocity(tw_get_config_float("Special", "Velocity2", 0));
	specialVelocity3 = scale_velocity(tw_get_config_float("Special", "Velocity3", 0));
	specialDamage1   = tw_get_config_int("Special", "Damage1", 0);
	specialDamage2   = tw_get_config_int("Special", "Damage2", 0);
	specialDamage3   = tw_get_config_int("Special", "Damage3", 0);
	specialArmour1   = tw_get_config_int("Special", "Armour1", 0);
	specialArmour2   = tw_get_config_int("Special", "Armour2", 0);
	specialArmour3   = tw_get_config_int("Special", "Armour3", 0);
	specialMaxSpeed1 = scale_velocity(tw_get_config_float("Special", "MaxSpeed1", 0));
	specialMaxSpeed2 = scale_velocity(tw_get_config_float("Special", "MaxSpeed2", 0));
	specialMaxSpeed3 = scale_velocity(tw_get_config_float("Special", "MaxSpeed3", 0));
	specialFramesToIgnition1   = tw_get_config_int("Special", "FramesToIgnition1", 0);
	specialFramesToIgnition2   = tw_get_config_int("Special", "FramesToIgnition2", 0);
	specialFramesToIgnition3   = tw_get_config_int("Special", "FramesToIgnition3", 0);
	specialFramesOfThrust1   = tw_get_config_int("Special", "FramesOfThrust1", 0);
	specialFramesOfThrust2   = tw_get_config_int("Special", "FramesOfThrust2", 0);
	specialFramesOfThrust3   = tw_get_config_int("Special", "FramesOfThrust3", 0);
	specialFramesOfCoasting1   = tw_get_config_int("Special", "FramesOfCoasting1", 0);
	specialFramesOfCoasting2   = tw_get_config_int("Special", "FramesOfCoasting2", 0);
	specialFramesOfCoasting3   = tw_get_config_int("Special", "FramesOfCoasting3", 0);
	specialAcceleration1 = scale_acceleration(tw_get_config_float("Special", "Acceleration1", 0), specialHotspotRate);
	specialAcceleration2 = scale_acceleration(tw_get_config_float("Special", "Acceleration2", 0), specialHotspotRate);
	specialAcceleration3 = scale_acceleration(tw_get_config_float("Special", "Acceleration3", 0), specialHotspotRate);
	specialReleaseAngle1 = tw_get_config_float("Special", "ReleaseAngle1", 0) * ANGLE_RATIO;
	specialReleaseAngle2 = tw_get_config_float("Special", "ReleaseAngle2", 0) * ANGLE_RATIO;
	specialReleaseAngle3A = tw_get_config_float("Special", "ReleaseAngle3A", 0) * ANGLE_RATIO;
	specialReleaseAngle3B = tw_get_config_float("Special", "ReleaseAngle3B", 0) * ANGLE_RATIO;
	specialRelativity = tw_get_config_float("Special", "Relativity", 0);
	specialStunFrames = tw_get_config_int("Special", "StunFrames", 0);
	specialFired=0;
	deathTimer=0;
	isDead = FALSE;
}


void NechanziCruiser::death(void)
{
	STACKTRACE;
	isDead = TRUE;
	if (deathTimer<100) {
		state = 1;
		return;
	}
	Ship::death();
}


int NechanziCruiser::activate_weapon()
{
	STACKTRACE;
	game->add(new Missile(this, Vector2(size.y*(0.26), (size.y * -0.15)),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	game->add(new Missile(this, Vector2(size.y*(-0.26), (size.y * -0.15)),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}


int NechanziCruiser::activate_special()
{
	STACKTRACE;
	if (specialFired)
		return(FALSE);

	NechanziMissile* NM;
	NM = new NechanziMissile(size.y*(0.05), (size.y * 0.4),
		angle+specialReleaseAngle1+turn_step, specialVelocity1, specialDamage1, scale_range(1000), specialArmour1,
		this, data->spriteSpecial, specialRelativity);
	NM->framesToIgnition = specialFramesToIgnition1;
	NM->framesOfBurn = specialFramesOfThrust1;
	NM->framesOfCoasting = specialFramesOfCoasting1;
	NM->facingAngle = angle+turn_step;
	NM->creator=this;
	NM->damageAfterIgnition = specialDamage1;
	NM->armourAfterIgnition = specialArmour1;
	NM->acceleration = specialAcceleration1;
	NM->mass = specialMass;
	NM->maxSpeed = specialMaxSpeed1;
	NM->missileType = 2;		 // this is actually obsolete (stun removed - Geo)
								 // this is actually obsolete (stun removed - Geo)
	NM->stunFrames = iround(specialStunFrames);
	game->add(NM);

	NM = new NechanziMissile(size.y*(-0.05), (size.y * 0.4),
		angle-specialReleaseAngle1+turn_step, specialVelocity1, specialDamage1, scale_range(1000), specialArmour1,
		this, data->spriteSpecial, specialRelativity);
	NM->framesToIgnition = specialFramesToIgnition1;
	NM->framesOfBurn = specialFramesOfThrust1;
	NM->framesOfCoasting = specialFramesOfCoasting1;
	NM->facingAngle = angle+turn_step;
	NM->creator=this;
	NM->damageAfterIgnition = specialDamage1;
	NM->armourAfterIgnition = specialArmour1;
	NM->acceleration = specialAcceleration1;
	NM->mass = specialMass;
	NM->maxSpeed = specialMaxSpeed1;
	NM->missileType = 2;
	NM->stunFrames = iround(specialStunFrames);
	game->add(NM);

	NM = new NechanziMissile(size.y*(-0.10), (size.y * 0.3),
		angle-specialReleaseAngle2+turn_step, specialVelocity2, specialDamage2, scale_range(1000), specialArmour2,
		this, data->spriteSpecial, specialRelativity);
	NM->framesToIgnition = specialFramesToIgnition2;
	NM->framesOfBurn = specialFramesOfThrust2;
	NM->framesOfCoasting = specialFramesOfCoasting2;
	NM->facingAngle = angle+turn_step;
	NM->creator=this;
	NM->damageAfterIgnition = specialDamage2;
	NM->armourAfterIgnition = specialArmour2;
	NM->acceleration = specialAcceleration2;
	NM->mass = specialMass;
	NM->maxSpeed = specialMaxSpeed2;
	NM->missileType = 1;
	game->add(NM);

	NM = new NechanziMissile(size.y*(0.00), (size.y * 0.3),
		angle+turn_step, specialVelocity2, specialDamage2, scale_range(1000), specialArmour2,
		this, data->spriteSpecial, specialRelativity);
	NM->framesToIgnition = specialFramesToIgnition2;
	NM->framesOfBurn = specialFramesOfThrust2;
	NM->framesOfCoasting = specialFramesOfCoasting2;
	NM->facingAngle = angle+turn_step;
	NM->creator=this;
	NM->damageAfterIgnition = specialDamage2;
	NM->armourAfterIgnition = specialArmour2;
	NM->acceleration = specialAcceleration2;
	NM->mass = specialMass;
	NM->maxSpeed = specialMaxSpeed2;
	NM->missileType = 1;
	game->add(NM);

	NM = new NechanziMissile(size.y*(0.1), (size.y * 0.3),
		angle+specialReleaseAngle2+turn_step, specialVelocity2, specialDamage2, scale_range(1000), specialArmour2,
		this, data->spriteSpecial, specialRelativity);
	NM->framesToIgnition = specialFramesToIgnition2;
	NM->framesOfBurn = specialFramesOfThrust2;
	NM->framesOfCoasting = specialFramesOfCoasting2;
	NM->facingAngle = angle+turn_step;
	NM->creator=this;
	NM->damageAfterIgnition = specialDamage2;
	NM->armourAfterIgnition = specialArmour2;
	NM->acceleration = specialAcceleration2;
	NM->mass = specialMass;
	NM->maxSpeed = specialMaxSpeed2;
	NM->missileType = 1;
	game->add(NM);

	/*NM = new NechanziMissile(height()*(-0.2), (height() * 0.2),
	  angle-specialReleaseAngle3B+turn_step, specialVelocity3, specialDamage3,
	scale_range(1000), specialArmour3,
	  this, data->spriteSpecial, specialRelativity);
	NM->framesToIgnition = specialFramesToIgnition3;
	NM->framesOfBurn = specialFramesOfThrust3;
	NM->framesOfCoasting = specialFramesOfCoasting3;
	NM->facingAngle = angle+turn_step;
	NM->creator=this;
	NM->damageAfterIgnition = specialDamage3;
	NM->armourAfterIgnition = specialArmour3;
	NM->acceleration = specialAcceleration3;
	NM->mass = specialMass;
	NM->maxSpeed = specialMaxSpeed3;
	game->add(NM);*/

	NM = new NechanziMissile(size.y*(-0.08), (size.y * 0.2),
		angle-specialReleaseAngle3A+turn_step, specialVelocity3, specialDamage3, scale_range(1000), specialArmour3,
		this, data->spriteSpecial, specialRelativity);
	NM->framesToIgnition = specialFramesToIgnition3;
	NM->framesOfBurn = specialFramesOfThrust3;
	NM->framesOfCoasting = specialFramesOfCoasting3;
	NM->facingAngle = angle+turn_step;
	NM->creator=this;
	NM->damageAfterIgnition = specialDamage3;
	NM->armourAfterIgnition = specialArmour3;
	NM->acceleration = specialAcceleration3;
	NM->mass = specialMass;
	NM->maxSpeed = specialMaxSpeed3;
	game->add(NM);

	NM = new NechanziMissile(size.y*(0.08), (size.y * 0.2),
		angle+specialReleaseAngle3A+turn_step, specialVelocity3, specialDamage3, scale_range(1000), specialArmour3,
		this, data->spriteSpecial, specialRelativity);
	NM->framesToIgnition = specialFramesToIgnition3;
	NM->framesOfBurn = specialFramesOfThrust3;
	NM->framesOfCoasting = specialFramesOfCoasting3;
	NM->facingAngle = angle+turn_step;
	NM->creator=this;
	NM->damageAfterIgnition = specialDamage3;
	NM->armourAfterIgnition = specialArmour3;
	NM->acceleration = specialAcceleration3;
	NM->mass = specialMass;
	NM->maxSpeed = specialMaxSpeed3;
	game->add(NM);

	/*NM = new NechanziMissile(height()*(0.2), (height() * 0.2),
	  angle+specialReleaseAngle3B+turn_step, specialVelocity3, specialDamage3,
	scale_range(1000), specialArmour3,
	  this, data->spriteSpecial, specialRelativity);
	NM->framesToIgnition = specialFramesToIgnition3;
	NM->framesOfBurn = specialFramesOfThrust3;
	NM->framesOfCoasting = specialFramesOfCoasting3;
	NM->facingAngle = angle+turn_step;
	NM->creator=this;
	NM->damageAfterIgnition = specialDamage3;
	NM->armourAfterIgnition = specialArmour3;
	NM->acceleration = specialAcceleration3;
	NM->mass = specialMass;
	NM->maxSpeed = specialMaxSpeed3;
	game->add(NM);*/

	specialFired = 1;

	return(TRUE);
}


void NechanziCruiser::calculate(void)
{
	STACKTRACE;
	if (isDead) deathTimer += frame_time;
	if (deathTimer>100) state=0;
	Ship::calculate();
	if (specialFired && (!fire_special))
		specialFired = FALSE;
}


NechanziMissile::NechanziMissile(double ox, double oy, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double relativity)
:
Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship,osprite, relativity)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	isBurning = FALSE;
	isCoasting = FALSE;
	facingAngle=oangle;
	damage_factor=0;			 // no damage until it ignites
	stunFrames=0;				 //default is no stun
	missileType=0;				 //just a default.  May be set to 0, 1, or 2
	isActivated=0;

	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void NechanziMissile::calculate(void)
{
	STACKTRACE;
	if (creator==NULL)
		isActivated=1;
	else if (creator->state==0)
		isActivated=1;
	else if (((NechanziCruiser*)creator)->isDead)
		isActivated=1;
	else if (!creator->fire_special)
		isActivated=1;
	if (isBurning==FALSE && isCoasting==FALSE)
	if (framesToIgnition>=0) {
		if (isActivated)framesToIgnition -= frame_time;
		sprite_index = get_index(facingAngle) + (0 * 64) +
			(missileType * 128);
	} else {
		framesToIgnition = 0;
		isBurning=TRUE;
		damage_factor = damageAfterIgnition;
		armour = armourAfterIgnition;
		range = 99999;
		sprite_index = (get_index(facingAngle) +
			(1 * 64)) + (missileType * 128);
		if (TRUE) play_sound2(this->creator->data->sampleSpecial[1]);

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


void NechanziMissile::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	Shot::inflict_damage(other);
	//  if (missileType==2 && other->isShip() && isActivated) {
	//    game->add(new NechanziStun((Ship*)other, data->spriteExtra, 64, 50,
	//stunFrames));
	//  }
}


/*
NechanziStun::NechanziStun(Ship *oship, SpaceSprite *osprite,
  int ofcount,
  int ofsize,
  int stunFrames)
	:
  SpaceObject(oship, oship->normal_pos(), 0.0, osprite),
  ship(oship),
  stunframe(0),
  stunframe_count(stunFrames),
  frame_step(0),
  frame_size(ofsize),
  frame_count(ofcount)
{
	STACKTRACE;
	collide_flag_anyone = 0;
  layer = LAYER_EXPLOSIONS;
  isblockingweapons = false;
}

void NechanziStun::calculate() {
	STACKTRACE;
	pos = ship->pos;
	vel = ship->vel;
	frame_step+= frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if (sprite_index == frame_count)
			sprite_index = 0;
		}
	if (!(ship && ship->exists()))
	{
		ship = 0;
		state = 0;
		return;
		}

  ship->nextkeys &= ~( keyflag::fire | keyflag::special);

  stunframe += frame_time;
  if (stunframe >= stunframe_count) state = 0;
	SpaceObject::calculate();
	}
*/

REGISTER_SHIP(NechanziCruiser)
