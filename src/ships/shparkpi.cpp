/* $Id: shparkpi.cpp,v 1.21 2006/06/05 19:03:43 geomannl Exp $ */

#include <allegro.h>
#include "../ship.h"
#include "../melee/mview.h"
REGISTER_FILE

//#include "sc1ships.h"
class ArkanoidPincerShip;

class ArkanoidPincer : public SpaceObject
{
	public:
	public:
		SpaceObject* shipStruck;
		int damageFactor;
		int armour;
		int isInvulnerableWhenOpening;
		int isInvulnerableWhenClosing;
		int isInvulnerableWhenResting;
		int isAlive;
		int transmitsDamageToShip;
		ArkanoidPincerShip* creator;
		ArkanoidPincer** pointerToMe;
		ArkanoidPincer* otherPincer;
		double angleShift;
		double angleSkew;
		ArkanoidPincer(ArkanoidPincerShip* ocreator, Vector2 opos,
			double oangle, SpaceSprite *osprite);
		virtual void calculate();
		virtual void collide(SpaceObject *other);
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage (SpaceLocation *source, double normal, double direct);
		virtual void animate(Frame* space);
		virtual void death(void);
		virtual bool die(void);
		void MoveToRelativeLocation(Ship* ocreator, double ox, double oy);
		void MoveToRelativeLocationPolar(Ship* ocreator, double oangle, double odistance);
		int spriteIndexSkew;
};

class ArkanoidPincerShip : public Ship
{
	public:
		int          shipHotspotRate;
		double       shipSpeedMax;
		double       shipAccelRate;
		double       shipTurnRate;
		int          shipMinBattForRegrowth;
		int          shipTimeForRegrowth;
		int          shipMass;
		int          regrowthCount;

		double       weaponAngleOpen;
		double       weaponAngleClosed;
		double       weaponSkewAngleOpen;
		double       weaponSkewAngleClosed;
		int          weaponTimeToClose;
		int          weaponTimeToOpen;
		double       weaponRangeOpen;
		double       weaponRangeClosed;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponCrushDamage;
		int          weaponArmour;
		double       weaponMass;
		int          weaponIsInvulnerableWhenClosing;
		int          weaponIsInvulnerableWhenOpening;
		int          weaponIsInvulnerableWhenResting;
		int          weaponTransmitsDamageToShip;

		int          specialFramesPerEnergy;
		int          specialIsRelativistic;
		int          specialZeroesNormalVelocity;
		double       specialVelocity;
		int          specialTurnDuringScuttle;
		int          specialReverseTurnDuringScuttle;
		double       specialTurnRate;
		int          specialScuttleSpeedSlowDown;
		int          specialScuttleTurnSlowDown;

		int framesJaws;
		int areJawsClosing;
		int areJawsOpening;
		int currentRangeJaws;
		int targetCrushed;
		int isScuttling;
		int isScuttlingOld;
		int scuttlingFrameCount;

		Vector2 baseV;			 //storage for pre-scuttling velocity
		double fractionJawsOpen;
		double angleShift;
		double angleSkew;
		double jawsRange;

	public:
		ArkanoidPincerShip(Vector2 opos, double angle, ShipData *data, unsigned int code);
		ArkanoidPincer* pincerL;
		ArkanoidPincer* pincerR;
		double jawAngle;
		int getSpriteIndex(void);
		virtual int handle_damage (SpaceLocation *source, double normal, double direct);
	protected:
		virtual void animate(Frame* space);
		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_thrust();
		virtual void death();
		virtual double handle_speed_loss(SpaceLocation* source, double normal);
		virtual void collide(SpaceObject *other);
		void ClosePincers(void);
		void OpenPincers(void);
		void Regrow(void);
};

ArkanoidPincerShip::ArkanoidPincerShip(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	//message.print(1500,2,"ArkanoidPincerShipCreator1");
	pincerL = NULL; pincerR=NULL;
	shipMass = tw_get_config_int("Ship", "Mass", 0);
	shipTurnRate = scale_turning(tw_get_config_float("Ship", "TurnRate", 0));
	shipHotspotRate = tw_get_config_int("Ship", "HotspotRate", 0);
	shipSpeedMax = scale_velocity(tw_get_config_float("Ship", "SpeedMax", 0));
	shipAccelRate = scale_acceleration(tw_get_config_float("Ship", "AccelRate",0),shipHotspotRate);
	shipMinBattForRegrowth= tw_get_config_int("Ship", "MinBattForRegrowth",0);
	shipTimeForRegrowth = tw_get_config_int("Ship", "TimeForRegrowth", 0);

	weaponAngleOpen    = tw_get_config_float("Weapon", "AngleOpen", 0) * ANGLE_RATIO;
	weaponAngleClosed  = tw_get_config_float("Weapon", "AngleClosed", 0) * ANGLE_RATIO;
	weaponSkewAngleOpen = tw_get_config_float("Weapon", "SkewAngleOpen", 0) * ANGLE_RATIO;
	weaponSkewAngleClosed = tw_get_config_float("Weapon", "SkewAngleClosed", 0) * ANGLE_RATIO;
	weaponTimeToClose  = tw_get_config_int("Weapon", "TimeToClose", 0);
	weaponTimeToOpen   = tw_get_config_int("Weapon", "TimeToOpen", 0);
	weaponRangeOpen    = scale_range(tw_get_config_float("Weapon", "RangeOpen", 0));
	weaponRangeClosed  = scale_range(tw_get_config_float("Weapon", "RangeClosed", 0));
	weaponDamage       = tw_get_config_int("Weapon", "Damage", 0);
	weaponCrushDamage       = tw_get_config_int("Weapon", "CrushDamage", 0);
	weaponArmour       = tw_get_config_int("Weapon", "Armour", 0);
	weaponMass         = tw_get_config_float("Weapon", "Mass", 0);
	//  message.print(3000,7,"weaponMass = %f", weaponMass);
	weaponIsInvulnerableWhenClosing = tw_get_config_int("Weapon", "IsInvulnerableWhenClosing", 0);
	weaponIsInvulnerableWhenOpening = tw_get_config_int("Weapon", "IsInvulnerableWhenOpening", 0);
	weaponIsInvulnerableWhenResting = tw_get_config_int("Weapon", "IsInvulnerableWhenResting", 0);
	weaponTransmitsDamageToShip = tw_get_config_int("Weapon", "TransmitsDamageToShip",0);

	specialFramesPerEnergy = tw_get_config_int("Special", "FramesPerEnergy", 0);
	specialIsRelativistic   = tw_get_config_int("Special", "IsRelativistic", 0);
	specialZeroesNormalVelocity = tw_get_config_int("Special", "ZeroesNormalVelocity", 0);
	//specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialTurnDuringScuttle   = tw_get_config_int("Special", "TurnDuringScuttle", 0);
	specialReverseTurnDuringScuttle   = tw_get_config_int("Special", "ReverseTurnDuringScuttle", 0);
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));
	specialScuttleSpeedSlowDown = tw_get_config_int("Special", "ScuttleSpeedSlowDown", 0);
	specialScuttleTurnSlowDown = tw_get_config_int("Special", "ScuttleTurnSlowDown", 0);
	pincerL = new ArkanoidPincer(this, pos, this->angle, data->spriteWeapon);
	pincerR = new ArkanoidPincer(this, pos, this->angle, data->spriteWeapon);
	pincerL->transmitsDamageToShip = weaponTransmitsDamageToShip;
	pincerL->isInvulnerableWhenClosing = weaponIsInvulnerableWhenClosing;
	pincerL->isInvulnerableWhenOpening = weaponIsInvulnerableWhenOpening;
	pincerL->isInvulnerableWhenResting = weaponIsInvulnerableWhenResting;
	pincerR->transmitsDamageToShip = weaponTransmitsDamageToShip;
	pincerR->isInvulnerableWhenClosing = weaponIsInvulnerableWhenClosing;
	pincerR->isInvulnerableWhenOpening = weaponIsInvulnerableWhenOpening;
	pincerR->isInvulnerableWhenResting = weaponIsInvulnerableWhenResting;
	pincerL->otherPincer = pincerR;
	pincerR->otherPincer = pincerL;
	pincerL->mass = weaponMass;
	pincerR->mass = weaponMass;
	pincerL->spriteIndexSkew = 0;
	pincerL->angleSkew = weaponSkewAngleOpen;
	pincerL->pointerToMe = &pincerL;
	pincerR->spriteIndexSkew = 64;
	pincerR->angleSkew = -weaponSkewAngleOpen;
	pincerR->pointerToMe = &pincerR;

	game->add(pincerL);
	game->add(pincerR);

	jawAngle=30;
	jawAngle=weaponAngleOpen;
	angleShift=weaponAngleOpen;
	angleSkew=weaponSkewAngleOpen;
	framesJaws = 0;
	areJawsClosing = 0;
	areJawsOpening = 0;
	jawsRange=weaponRangeOpen;
	fractionJawsOpen = 1.00;
	targetCrushed = 0;
	isScuttling = FALSE;
	isScuttlingOld = FALSE;
	scuttlingFrameCount = 0;
	baseV = 0;
	regrowthCount=0;
	//message.print(1500,2,"ArkanoidPincerShipCreator2");

	if (!pincerL->exists() || !pincerR->exists()) {
		tw_error("The pincer must not die!!");
	}
}


void ArkanoidPincerShip::death()
{
	STACKTRACE;

	Ship::death();

	//message.print(1500,13,"ShipDeath1");
	if (pincerL && pincerL->exists()) {
		pincerL->creator = NULL;
		pincerL->otherPincer = NULL;
		pincerL->state = 0;
		pincerL = NULL;
	}
	if (pincerR && pincerR->exists()) {
		pincerR->creator = NULL;
		pincerR->otherPincer = NULL;
		pincerR->state = 0;
		pincerR = NULL;
	}
	//message.print(1500,13,"ShipDeath2");
}


void ArkanoidPincerShip::animate(Frame* space)
{
	STACKTRACE;
	//message.print(1500,6,"ArkanoidPincerShipAnimate1 %d", this->sprite_index);
	Ship::animate(space);
	//message.print(1500,6,"ArkanoidPincerShipAnimate2 %d", this->sprite_index);
}


void ArkanoidPincerShip::calculate(void)
{
	STACKTRACE;
	if (!pincerL->exists() || !pincerR->exists()) {
		tw_error("The pincer must not die!!");
	}
	//message.print(1500,9,"angle = %f turn_step = %f",this->angle, this->turn_step);
	Ship::calculate();
	isScuttlingOld = isScuttling;
	if (fire_special&&this->batt>0) isScuttling=TRUE;
	else isScuttling=FALSE;
	if (isScuttling)
		scuttlingFrameCount += frame_time;
	if (scuttlingFrameCount>specialFramesPerEnergy) {
		batt--; scuttlingFrameCount -= specialFramesPerEnergy;
	}
	if (isScuttling==TRUE && isScuttlingOld==FALSE) {
		baseV = vel;
		turn_rate = specialTurnRate;
	}
	if (isScuttling==FALSE && isScuttlingOld==TRUE) {
		turn_rate = shipTurnRate;
		vel = baseV;
		if (specialZeroesNormalVelocity) {
			vel =0;
		}
	}
	//if (isScuttling&&(turn_left==turn_right)) //line in question
	//  this->vx = baseVX; this->vy = baseVY;
	if (areJawsClosing) this->ClosePincers();
	else if (areJawsOpening) this->OpenPincers();
	if (pincerL)pincerL->angleShift = -jawAngle;
	if (pincerR)pincerR->angleShift = jawAngle;
	if (pincerL)pincerL->MoveToRelativeLocationPolar(this, -jawAngle, jawsRange);
	if (pincerR)pincerR->MoveToRelativeLocationPolar(this, jawAngle, jawsRange);

	if (( ( !pincerR || pincerR->isAlive==FALSE) ||
		( !pincerL || pincerL->isAlive==FALSE) ) &&
	batt >= shipMinBattForRegrowth) {

		regrowthCount += frame_time;
		if (regrowthCount>shipTimeForRegrowth) {
			Regrow();
			regrowthCount -= shipTimeForRegrowth;
		}
	}
	//message.print(1500,9,"ArkanoidPincerShipEndCalculate");
}


int ArkanoidPincerShip::activate_weapon()
{
	STACKTRACE;
	//message.print(1500,5,"ActivateWeapon1");
	if (areJawsOpening)return(FALSE);
	if (areJawsClosing)return(FALSE);
	if (!areJawsOpening) {
		targetCrushed = 0;
		areJawsClosing = 1;
		areJawsOpening = 0;
		if (pincerL) {
			if (fractionJawsOpen>0.95) pincerL->damage_factor = weaponDamage;
			pincerL->shipStruck = NULL;
		}
		if (pincerR) {
			if (fractionJawsOpen>0.95) pincerR->damage_factor = weaponDamage;
			pincerR->shipStruck = NULL;
		}
	}
	//jawAngle += 0.05 * frame_time;
	//this->ClosePincers();
	//message.print(1500,5,"ActivateWeapon2");
	return(TRUE);
}


int ArkanoidPincerShip::activate_special()
{
	STACKTRACE;
	//message.print(1500,5,"ActivateSpecial");
	return(FALSE);
}


void ArkanoidPincerShip::calculate_turn_left()
{
	STACKTRACE;
	//message.print(1500,6,"CalcTurnLeft1");
	double motionAngle;
	if (isScuttling==FALSE) {
		Ship::calculate_turn_left();
		return;
	}
	else if (turn_left) {
		if (specialTurnDuringScuttle)
			Ship::calculate_turn_left();
		if (specialReverseTurnDuringScuttle)
			Ship::calculate_turn_right();
		motionAngle = this->angle - PI/2;
		if (specialIsRelativistic)
			vel = baseV + unit_vector(motionAngle)*specialVelocity;
		else
			vel = unit_vector(motionAngle)*specialVelocity;
	}
	//else tw_error("calc turn left called without turning left!");
	//if ((!turn_left)&&(!turn_right))
	//  this->vx = baseVX; this->vy = baseVY;
	//message.print(1500,6,"CalcTurnLeft2");
}


void ArkanoidPincerShip::calculate_turn_right()
{
	STACKTRACE;
	//message.print(1500,6,"CalcTurnRight1");
	double motionAngle;
	if (isScuttling==FALSE) {
		Ship::calculate_turn_right();
		return;
	}
	else if (turn_right) {
		if (specialTurnDuringScuttle)
			Ship::calculate_turn_right();
		if (specialReverseTurnDuringScuttle)
			Ship::calculate_turn_left();
		motionAngle = this->angle + PI/2;
		if (specialIsRelativistic)
			vel = baseV + unit_vector(motionAngle)*specialVelocity;
		else
			vel = unit_vector(motionAngle)*specialVelocity;
	}
	//if ((!turn_left)&&(!turn_right))
	//  this->vx = baseVX; this->vy = baseVY;
	else if (!turn_left) {
		//tw_error("calc turn left called without turning right!");
		if (specialIsRelativistic)
			vel = baseV;
		else
			vel = 0;
	}
	//message.print(1500,6,"CalcTurnRight2");
}


void ArkanoidPincerShip::calculate_thrust()
{
	STACKTRACE;
	//message.print(1500,6,"CalcThrust1");

	if (isScuttling==FALSE) {
		Ship::calculate_thrust();
	}
	else if (turn_left==FALSE&&turn_right&&FALSE)
		{vel = baseV;}
}


double ArkanoidPincerShip::handle_speed_loss(SpaceLocation* source, double normal)
{
	STACKTRACE;
	double speed_loss = normal;
	if (speed_loss > 0.0) {
		double sl = (30/(mass+30)) * speed_loss;
		if (sl > 1)
			tw_error("Speed loss too large\n(%f)", sl);

		//accel_rate *= 1 - sl * accel_rate / (accel_rate + scale_acceleration(2,4));
		//hotspot_rate = (int)(hotspot_rate / (1 - sl * accel_rate / (accel_rate + scale_acceleration(2,4)) ) );
		specialVelocity *= 1 - sl * specialVelocity / (specialVelocity + scale_velocity(10));
		shipSpeedMax *= 1 - sl * shipSpeedMax / (shipSpeedMax + scale_velocity(10));
		specialTurnRate *=  1 - sl * specialTurnRate / (specialTurnRate + scale_turning(4));
	}
	//message.print(1500,6,"HandleSpeedLoss");
	return Ship::handle_speed_loss(source, normal);
}


int ArkanoidPincerShip::getSpriteIndex(void)
{
	STACKTRACE;
	return(sprite_index);
}


void ArkanoidPincerShip::collide(SpaceObject *other)
{
	STACKTRACE;
	if (pincerL)
		if (other==pincerL) return;
	if (pincerR)
		if (other==pincerR) return;
	//if (other==this->pincerL||other==this->pincerR) return;
	SpaceObject::collide(other);
}


int ArkanoidPincerShip::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	int x;
	//message.print(1500,6,"ArkanoidPincerShipHandleDamage1");
	x = Ship::handle_damage(source, normal, direct);
	if (!exists()) {			 //(state==0) {
		if (pincerL)pincerL->state = 0;
		if (pincerR)pincerR->state = 0;
	}
	//message.print(1500,6,"ArkanoidPincerShipHandleDamage2");
	return(x);
}


void ArkanoidPincerShip::OpenPincers(void)
{
	STACKTRACE;
	//message.print(1500,11,"OpenPincers1");
	fractionJawsOpen += (double)frame_time / (double)weaponTimeToOpen;
	if (fractionJawsOpen>1.00) {
		fractionJawsOpen = 1.00;
		areJawsOpening = 0;
		if (pincerL) pincerL->mass = 0;
		if (pincerR) pincerR->mass = 0;
	}
	jawAngle = weaponAngleOpen * fractionJawsOpen + weaponAngleClosed * (1-fractionJawsOpen);
	jawsRange = weaponRangeOpen * fractionJawsOpen + weaponRangeClosed * (1-fractionJawsOpen);
	if (pincerL)pincerL->angleSkew = weaponSkewAngleOpen * fractionJawsOpen + weaponSkewAngleClosed * (1-fractionJawsOpen);
	if (pincerR)pincerR->angleSkew = -(weaponSkewAngleOpen * fractionJawsOpen + weaponSkewAngleClosed * (1-fractionJawsOpen));
	//if (fractionJawsOpen<0.90)mass = weaponMass;
	//message.print(1500,11,"OpenPincers2");
}


void ArkanoidPincerShip::ClosePincers(void)
{
	STACKTRACE;
	//message.print(1500,11,"ClosePincers1");
	if (pincerL) pincerL->mass = 0;
	if (pincerR) pincerR->mass = 0;
	fractionJawsOpen -= (double)frame_time / (double)weaponTimeToClose;
	if (fractionJawsOpen<0.00) {
		fractionJawsOpen = 0.00;
		areJawsClosing = 0;
		areJawsOpening = 1;		 //auto opening... may change this later.
		//mass = shipMass;
	}
	jawAngle = weaponAngleOpen * fractionJawsOpen + weaponAngleClosed * (1-fractionJawsOpen);
	jawsRange = weaponRangeOpen * fractionJawsOpen + weaponRangeClosed * (1-fractionJawsOpen);
	if (pincerL!=NULL)pincerL->angleSkew = weaponSkewAngleOpen * fractionJawsOpen + weaponSkewAngleClosed * (1-fractionJawsOpen);
	if (pincerR!=NULL)pincerR->angleSkew = -(weaponSkewAngleOpen * fractionJawsOpen + weaponSkewAngleClosed * (1-fractionJawsOpen));
	//message.print(1500,11,"ClosePincers2");

}


void ArkanoidPincerShip::Regrow(void)
{
	STACKTRACE;
	//message.print(1500,11,"Regrow1");
	if (pincerL) {
		if (pincerL->isAlive==FALSE) {
			pincerL->isAlive=TRUE;
			pincerL->damage_factor=0;
			pincerL->armour = weaponArmour;
			pincerL->collide_flag_anyone = ALL_LAYERS;
			pincerL->collide_flag_sameship = 0;
			pincerL->collide_flag_sameteam = 0;
			pincerL->mass = weaponMass;
			return;
		}
	} else {
		tw_error("pincerL does not exist!");
	}

	if (pincerR) {
		if (pincerR->isAlive==FALSE) {
			pincerR->isAlive=TRUE;
			pincerR->damage_factor=0;
			pincerR->armour = weaponArmour;
			pincerR->collide_flag_anyone = ALL_LAYERS;
			pincerR->collide_flag_sameship = 0;
			pincerR->collide_flag_sameteam = 0;
			pincerR->mass = weaponMass;
			return;
		}
	} else {
		tw_error("pincerR does not exist!");
	}

	//message.print(1500,11,"Regrow2");
}


ArkanoidPincer::ArkanoidPincer(ArkanoidPincerShip *ocreator, Vector2 opos,
double oangle, SpaceSprite *osprite) :
SpaceObject((SpaceObject*)ocreator, opos, oangle, osprite)
{
	STACKTRACE;
	//message.print(1500,13,"ArkanoidPincerCreate1");

	layer = LAYER_SHIPS;

	creator=NULL; damage_factor=0;
	angleShift=0; mass=0; armour=0;
	creator = ocreator;
	if (creator)
		damage_factor = creator->weaponDamage;
	if (creator)
		vel = creator->get_vel();
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	if (creator)angleShift=creator->weaponAngleOpen;
	if (creator)mass = creator->weaponMass;
	shipStruck=NULL;
	if (creator)armour = creator->weaponArmour;
	isAlive = TRUE;
	//message.print(1500,13,"ArkanoidPincerCreate2");

	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void ArkanoidPincer::death(void)
{
	STACKTRACE;
	//message.print(1500,14,"ArkanoidPincerDeath1");
	//state=1;
	//isAlive = 0;
	SpaceObject::death();

	if (creator) {
		*pointerToMe = NULL;
		if (otherPincer)
			this->otherPincer->otherPincer = NULL;
	}
	//message.print(1500,14,"ArkanoidPincerDeath2");
}


bool ArkanoidPincer::die(void)
{
	STACKTRACE;
	//message.print(1500,14,"ArkanoidPincerDie1");
	isAlive=FALSE;
	state=1;
	collide_flag_anyone = 0;	 // ok, it becomes passive, as if it's not there.
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	//  x = SpaceObject::die();
	//message.print(1500,14,"ArkanoidPincerDie2");
	if (!exists()) {
		tw_error("The pincer must not die!!");
	}
	return true;
}


void ArkanoidPincer::animate(Frame* space)
{
	STACKTRACE;
	//message.print(1500,4,"ArkanoidPincerAnimate1 %d", this->sprite_index);
	if (isAlive) SpaceObject::animate(space);
	//message.print(1500,4,"ArkanoidPincerAnimate2 %d", this->sprite_index);
}


void ArkanoidPincer::MoveToRelativeLocation(Ship* ocreator, double ox, double oy)
{
	STACKTRACE;
	double ddx, ddy, aRad;
	//message.print(1500,15,"MoveToRelativeLocation1");
	if (ocreator==NULL) return;
	vel = ocreator->get_vel();
	aRad = (ocreator->get_angle() + ocreator->turn_step);
	ddx = cos(aRad) * oy - sin(aRad) * ox;
	ddy = cos(aRad) * ox + sin(aRad) * oy;

	pos = ocreator->normal_pos() + Vector2(ddx, ddy);
	//message.print(1500,15,"MoveToRelativeLocation2");

}


void ArkanoidPincer::MoveToRelativeLocationPolar(Ship* ocreator, double oangle, double odistance)
{
	STACKTRACE;
	double aRad;
	//message.print(1500,15,"MoveToRelativeLocationPolar1");
	if (!creator) return;
	if (!creator->exists()) return;
	if (creator->state==0) return;
	vel = creator->vel;
	aRad = (creator->angle + creator->turn_step + oangle);
	pos = creator->normal_pos() + unit_vector(aRad) * odistance;
	//ArkanoidPincer::MoveToRelativeLocation(ocreator, dx, dy);
	//message.print(1500,15,"MoveToRelativeLocationPolar2");
}


void ArkanoidPincer::calculate(void)
{
	STACKTRACE;
	double angleDirection;
	//message.print(1500,15,"ArkanoidPincerCalculate1");
	angleDirection = creator->angle + creator->turn_step + angleShift + angleSkew;
	//sprite_index = (normalize((ship->get_angle() / 5.625) + 16, 64) + (1 * 64));
	sprite_index = (get_index(angleDirection) + spriteIndexSkew);
	if (sprite_index<0) sprite_index=0;
	if (sprite_index>127) sprite_index=127;
	//sprite_index = spriteIndexSkew;
	if (!exists()) {
		tw_error("The pincer must not die!!");
	}
	SpaceObject::calculate();
	//message.print(1500,15,"ArkanoidPincerCalculate2");
	if (!exists()) {
		tw_error("The pincer must not die!!");
	}
}


void ArkanoidPincer::collide(SpaceObject *other)
{
	STACKTRACE;
	Vector2 vv;
	if (!isAlive) return;
	if (other==this->creator) return;
	if (other==this->otherPincer) return;
	vv = vel;
	SpaceObject::collide(other);
	if (vv!=vel) {
		if (creator) {
			creator->set_vel ( vel );
			//      message.print(4000,15,"old %f -- %f new %f -- %f",vxx, vyy, this->vx, this->vy);
		}
	}
}


void ArkanoidPincer::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	int i;
	i = iround(damage_factor / 2);
	if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
	//double oldMass;
	//oldMass = mass;
	//message.print(1500,15,"ArkanoidPincerInflictDamage1");
	if (!creator) return;
								 // jaws must be closing to inflict damage
	if (!creator->areJawsClosing) return;
	//if (creator->areJawsClosing)mass = 0; //?? debug attempt.
	if (this->otherPincer!=NULL)
	if (this->otherPincer->shipStruck==NULL && damage_factor>0) {
		this->otherPincer->damage_factor += creator->weaponCrushDamage;
		//creator->targetCrushed=1; //should now be irrelevant
	}
	if (damage_factor>0) SpaceObject::inflict_damage(other);
	else return;				 //new addition
	if (other->isShip()) {
		this->damage_factor=0;
		this->shipStruck=other;
		//tw_error("Ship Struck!");
	}
	//if (creator->areJawsClosing)mass = oldMass; //?? debug attempt.
	collide_flag_anyone = ALL_LAYERS;
	play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	physics->add(new Animation( other,
		other->normal_pos(), meleedata.sparkSprite, 0,
		SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));

	//message.print(1500,15,"ArkanoidPincerInflictDamage2");
}


int ArkanoidPincer::handle_damage (SpaceLocation *source, double normal, double direct)
{
	int x;

	if (source->isPlanet()) {
		state = 1;
		normal = armour / 3;
		armour -= iround(normal);
	}
	// sometimes, the planet makes the state=0. This corrects for that, since the damage()
	// call is done _after_ the statement state=0 by the planet damage function.

	//message.print(1500,15,"ArkanoidPincerHandleDamage1 armour= %d normal= %d", armour, normal);
	x = 0;
	if (creator) {
		if (transmitsDamageToShip)
			x = this->creator->handle_damage(source, normal, direct);
		if (isInvulnerableWhenClosing && creator->areJawsClosing) {
			state=1;
			//message.print(1500,15,"ArkanoidPincerHandleDamage Exit1");
			return(x);
		}
		else if (isInvulnerableWhenOpening && creator->areJawsOpening) {
			//message.print(1500,15,"ArkanoidPincerHandleDamage Exit2");
			state=1;
			return(x);
		}
		else if (isInvulnerableWhenResting && !creator->areJawsClosing && !creator->areJawsOpening) {
			//message.print(1500,15,"ArkanoidPincerHandleDamage Exit3");
			state=1;
			return(x);
		} else {
			//message.print(1500,12,"Fall-Through");
			armour -= iround(normal + direct);
		}
	} else {
		//message.print(1500,12,"No Creator -- Doing the subtraction!");
		armour -= iround(normal + direct);
	}
	if (armour<=0) {
		//tw_error("Pincer took damage!");
		isAlive=FALSE;
		state=1;
		collide_flag_anyone = 0; // ok, it becomes passive, as if it's not there.
		collide_flag_sameship = 0;
		collide_flag_sameteam = 0;
	}
	if (!exists()) {
		tw_error("The pincer must not die!!");
	}
	//message.print(1500,15,"ArkanoidPincerHandleDamage2 armour=%d normal=%d", armour, normal);
	return iround(normal + direct);
}


REGISTER_SHIP(ArkanoidPincerShip)
