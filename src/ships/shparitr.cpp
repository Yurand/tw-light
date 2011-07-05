/* $Id: shparitr.cpp,v 1.14 2004/03/24 23:51:39 yurand Exp $ */
/********************************************please leave this comment tag in**
 *******************************************************************************
 *******************************************************************************
 *  Arilou Trapper.  Ship with cloak that increases speed and reduces damage. **
 *  Has a disabling weapon and retro turning thrusters.                       **
 *******************************************************************************
 *                                                                            **
 *  Coded by MAZ (maz@creativelygenius.com)                                   **
 *                                                                            **
 *******************************************************************************
 *******************************************************************************
 *  version 0.2                                                               **
 *******************************************************************************
 *******************************************************************************
 *  Still have to add code for animating engage_subquasi graphics, and better **
 *  animation/graphics for the trap weapon.                                   **
 *******************************************************************************
 *******************************************************************************
 ******************************************************************************/

#include "../ship.h"
REGISTER_FILE

/*************************************
 **********Weapon (Trap) Class*********
 **************************************
 *************************************/

// Causes ship to freeze up, draining energy from ships batteries based on rate defined
// in shparitr.ini as drainDelay (of type int).  Once batteries are drained ship returns
// to normal.

class trapShip : public SpaceObject
{
	Ship *ship;
	int drainRate;
	int drainDelay;
	int drainAmount;
	int soundDelay;

	public:
		//  trapShip(Ship *creator, Ship *oship, int drainDelayRate, int drainAmountof,
		//                SpaceSprite *osprite, int ofcount, int ofsize);
		trapShip(SpaceLocation *creator, Ship *oship, int drainDelayRate, int drainAmountof,
			SpaceSprite *osprite, int ofcount, int ofsize);
		virtual void calculate();
};

trapShip::trapShip(SpaceLocation *creator, Ship *oship, int drainDelayRate, int drainAmountof, SpaceSprite *osprite, int ofcount, int ofsize) :
SpaceObject(creator, oship->normal_pos(), 0.0, osprite), ship(oship)
{
	STACKTRACE;
	play_sound2(data->sampleExtra[1]);
	collide_flag_anyone = 0;
	drainRate = 0;
	drainDelay = drainDelayRate;
	drainAmount = drainAmountof;
	soundDelay = 0;

	// limiting case, where the enemy recharges faster than the weapon drains.
	while (drainAmount * ship->recharge_rate < ship->recharge_amount * drainDelay*frame_time) {
		drainDelay -= 1;
		if (drainDelay < 1) {
			drainDelay = 1;
			break;
		}
	}

}


void trapShip::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		ship = 0;				 // you've to do it yourself cause you return before the other calculate call !!
		state = 0;
		return;
	}
	if (ship->batt > 0) {
		pos = ship->normal_pos();
		ship->nextkeys &= ~(keyflag::left | keyflag::right | keyflag::special | keyflag::thrust | keyflag::back | keyflag::fire | keyflag::altfire);

		// must be unconditional, outside the loop (otherwise virtually 1 extra loop/waittime is added)
		++drainRate;
		if (drainRate >= drainDelay) {
			ship->batt -= drainAmount;
			//drainRate = 0;
			drainRate -= drainDelay;
		}
	} else {
		state = 0;
		return;
	}
	if (soundDelay < 25)		 // Needed to make sound loop properly :)
		soundDelay++;
	else {
		play_sound2(data->sampleExtra[1]);
		soundDelay = 0;
	}
	SpaceObject::calculate();
}


// Homing shot that causes ship freeze/energy drain as defined in class trapShip.

class quasiTrap : public HomingMissile
{
	int vDelay;
	int drainDelay;
	int drainAmount;
	double accel;

	public:
		quasiTrap(Vector2 opos, double oangle, double trapInitialVelocity, double trapAccel, double orange, int drainDelayRate,
			int drainAmountof, double otrate, Ship *oship, SpaceSprite *osprite);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

quasiTrap::quasiTrap(Vector2 opos, double oangle, double trapInitialVelocity, double trapAccel, double orange, int drainDelayRate,
int drainAmountof, double otrate, Ship *oship, SpaceSprite *osprite) :
HomingMissile( oship, opos, oangle, trapInitialVelocity, 0, orange, 999, otrate, oship,
osprite, oship->target)
{
	STACKTRACE;
	vDelay = 0;
	drainDelay = drainDelayRate;
	drainAmount = drainAmountof;
	accel = trapAccel;
}


void quasiTrap::calculate()
{
	STACKTRACE;
	if (vDelay < 5)
		vDelay++;
	else {
		v += accel;
		vDelay = 0;
	}
	HomingMissile::calculate();
}


void quasiTrap::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->isShip())
		//add(new trapShip( ship, (Ship *)(other), drainDelay, drainAmount, data->spriteWeaponExplosion, 20, 50));
		add(new trapShip( this, (Ship *)(other), drainDelay, drainAmount, data->spriteWeaponExplosion, 20, 50));
	// the creator should be "this", otherwise "converted weapons" whose ship pointer is
	// perturbed can crash the game; the data points wrongly then...
	state = 0;
	return;
}


int quasiTrap::handle_damage(SpaceLocation *source, double norm, double direct)
{
	STACKTRACE;
	return 1;
}


/*************************************
 ******Thruster Animation Class********
 **************************************
 *************************************/
class ArilouTrapperThrust : public PositionedAnimation
{
	int base_frame;
	public:
		ArilouTrapperThrust(SpaceLocation *creator, double ox, double oy, SpaceSprite *osprite, double offset);
};

ArilouTrapperThrust::ArilouTrapperThrust(SpaceLocation *creator, double ox, double oy,
SpaceSprite *osprite, double offset) :
PositionedAnimation(creator, creator, Vector2(ox,oy), osprite,
0, 1, 25, LAYER_SHOTS)
{
	STACKTRACE;
	sprite_index = get_index((follow->get_angle())+offset);
}


/*************************************
 **************Ship Class**************
 **************************************
 *************************************/

class ArilouTrapper : public Ship
{
	int    regenrateFrames;
	int    regenrateCount;
	int    regenerating;
	int    rechargeAmount;
	int    drainDelay;
	int    drainAmount;
	double trapTurn;
	double weaponRange;
	double weaponVelocity;
	int    weaponDamage;
	int    weaponArmour;
	int    weaponDuration;
	double trapInitialVelocity;
	double trapAccel;
	int    normalRechargeAmount;
	double quasiVelocity;
	int    cloak;
	int    cloak_frame;

	public:
		static int cloak_color[3];
		Color crewPanelColor(int k = 0);
		ArilouTrapper(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
		virtual double isInvisible() const;

								 // Takes half damage when in cloak (subquasi)
		virtual int handle_damage(SpaceLocation* source, double normal, double direct);

		virtual int activate_weapon();
		virtual void calculate_fire_special();
		virtual void calculate_hotspots();
		virtual void calculate();// Thrust backwards @ 1/2 normal thrust when left and right held  // Regenerates crew (shields) when battery is full  // Some cloak related code.

		void calculate_thrust(); // This override increases max speed by a factor of 2.5 when in cloak (subquasi)
								 // This override adds a thrust 45deg opposite of turn equal to 1/3 the normal thrust
		void calculate_turn_left();
								 // This override adds a thrust 45deg opposite of turn equal to 1/3 the normal thrust
		void calculate_turn_right();

		virtual void animate(Frame *space);
};

int ArilouTrapper::cloak_color[3] = { 15, 10, 2 };
Color ArilouTrapper::crewPanelColor(int k)
{
	STACKTRACE;
	Color c = { 66,66,255 }
	;
	return c;
}


ArilouTrapper::ArilouTrapper(Vector2 opos, double shipAngle, ShipData *shipData,
unsigned int code) : Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weapon_sample -= 1;
	regenrateFrames = 400;
	regenerating    = FALSE;
	rechargeAmount = get_config_int("Ship", "rechargeAmount", 0);
	drainDelay     = get_config_int("Trap", "drainDelay", 0);
	drainAmount    = get_config_int("Trap", "drainAmount", 0);
	trapTurn       = get_config_float("Trap", "trapTurn", 0);
	trapInitialVelocity = get_config_float("Trap", "initialVelocity", 0);
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	trapAccel      = get_config_float("Trap", "Accel", 0);
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponDuration = get_config_int("Weapon", "Duration", 0);
	quasiVelocity  = scale_velocity(get_config_float("Ship","quasiVelocity", 0));
	normalRechargeAmount = recharge_amount;
	cloak = FALSE;
	cloak_frame = 0;

}


double ArilouTrapper::isInvisible() const
{
	if (cloak_frame >= 300) return(1);
	return 0;
}


int ArilouTrapper::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if ( cloak == TRUE ) {
		normal = normal/2;
		direct = direct/2;
	}
	play_sound2(data->sampleExtra[0]);
	return Ship::handle_damage(source, normal, direct);
}


int ArilouTrapper::activate_weapon()
{
	STACKTRACE;
	if (cloak) {
		play_sound2(data->sampleSpecial[0]);
		cloak = FALSE;
		play_sound2(data->sampleWeapon[1]);
		add(new quasiTrap(Vector2(0.0, (size.y * 1)), angle, trapInitialVelocity, trapAccel, weaponRange*1.5, drainDelay,
			drainAmount, trapTurn, this, data->spriteWeaponExplosion));
	} else {
		play_sound2(data->sampleWeapon[0]);
		add(new AnimatedShot(this, Vector2(0.0, (size.y/3.0)), angle , weaponVelocity,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon, 10,
			12, 1.0));
	}
	return(TRUE);
}


void ArilouTrapper::calculate_fire_special()
{
	STACKTRACE;
	special_low = FALSE;
	if (fire_special) {
		if ((batt < special_drain) && (!cloak)) {
			special_low = TRUE;
			return;
		}
		if (special_recharge > 0)
			return;
		if (cloak) {
			play_sound2(data->sampleSpecial[0]);
			cloak = FALSE;
			recharge_amount = normalRechargeAmount;
		} else {
			play_sound2(data->sampleSpecial[0]);
			cloak = TRUE;
			batt -= special_drain;
			recharge_amount = 1;
		}
		special_recharge = special_rate;
	}
}


void ArilouTrapper::calculate_hotspots()
{
	STACKTRACE;
	if (!cloak)
		Ship::calculate_hotspots();
}


void ArilouTrapper::calculate()
{
	STACKTRACE;
	if (regenerating) {
		if ((batt < batt_max) || (crew >= crew_max ))
			regenerating = FALSE;
		else {
			if ((regenrateCount -= frame_time) < 0) {
				crew += rechargeAmount;
				regenrateCount = regenrateFrames;
			}
		}
	}
	else if (!(regenerating) && (crew < crew_max) && (batt == batt_max )) {
		regenerating = TRUE;
		regenrateCount = regenrateFrames;
	}

	if ((cloak) && (cloak_frame < 300))
		cloak_frame+= frame_time;
	if ((!cloak) && (cloak_frame > 0))
		cloak_frame-= frame_time;

	if (turn_right && turn_left) {
		accelerate_gravwhip(this, angle + PI, accel_rate * frame_time / 2, speed_max);
		if (!cloak) {
			add(new ArilouTrapperThrust(this, 0, -17, data->spriteSpecial, PI*3/4));
			add(new ArilouTrapperThrust(this, 0, 17, data->spriteSpecial, -PI*3/4));
		}
	}
	Ship::calculate();
}


void ArilouTrapper::calculate_thrust()
{
	STACKTRACE;
	if (thrust) {
		if (cloak)
			accelerate_gravwhip(this, angle, accel_rate * frame_time, quasiVelocity);
		else {
			add(new ArilouTrapperThrust(this, -28, 0, data->spriteSpecial, 0));
			accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);
		}
	}
	return;
}


void ArilouTrapper::calculate_turn_left()
{
	STACKTRACE;
	if (turn_left && !turn_right) {
		accelerate_gravwhip(this, angle + PI*3/4, accel_rate * frame_time/3, speed_max);
		if (!cloak)
			add(new ArilouTrapperThrust(this, 0, -17, data->spriteSpecial, PI*3/4));
	}
	Ship::calculate_turn_left();
}


void ArilouTrapper::calculate_turn_right()
{
	STACKTRACE;
	if (turn_right && !turn_left) {
		accelerate_gravwhip(this, angle - PI*3/4, accel_rate * frame_time/3, speed_max);
		if (!cloak)
			add(new ArilouTrapperThrust(this, 0, 17, data->spriteSpecial, -PI*3/4));
	}
	Ship::calculate_turn_right();
}


void ArilouTrapper::animate(Frame *space)
{
	STACKTRACE;
	if ((cloak_frame > 0) && (cloak_frame < 300))
		sprite->animate_character( pos, sprite_index, pallete_color[cloak_color[(int)(cloak_frame / 100)]], space);
	else
	if ((cloak_frame >= 300))
		sprite->animate_character( pos, sprite_index, pallete_color[0], space);
	else
		Ship::animate(space);
}


REGISTER_SHIP(ArilouTrapper)
