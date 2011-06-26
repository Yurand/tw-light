/* $Id: shplyrwa.cpp,v 1.15 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

class LyrmristuWaBolt1 : public Missile
{
	public:
	public:
		LyrmristuWaBolt1(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double relativity);
};

class LyrmristuWaBolt2 : public Missile
{
	public:
	public:
		LyrmristuWaBolt2(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double relativity);
};

class LyrmristuWaSphere : public Shot
{
	public:
	public:
		double powerLevel;
		LyrmristuWaSphere **PP;
		LyrmristuWaSphere(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, LyrmristuWaSphere **P);
		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
		virtual void destroy();
		virtual bool die(void);
		virtual void death();
	private:

};

class LyrmristuWarDestroyer : public Ship
{
	public:
	public:
		double       weaponRange1;
		double       weaponVelocity1;
		int          weaponDamage1;
		int          weaponArmour1;
		double       weaponRelativity1;
		double         weaponAngleSpread1;
		double       weaponRange2;
		double       weaponVelocity2;
		int          weaponDamage2;
		int          weaponArmour2;
		double       weaponRelativity2;

		int    specialColor;
		double specialRange;
		int    specialFrames;

		double       specialVelocity;
		int          specialDamage;
		int          specialArmour;
		double       specialTurnRate;
		int          specialMaxDamage;
		LyrmristuWaSphere *weaponObject;

	public:
		LyrmristuWarDestroyer(Vector2 opos, double angle, ShipData *data, unsigned
			int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual int handle_damage(SpaceLocation *source, double normal, double
			direct);

};

LyrmristuWarDestroyer::LyrmristuWarDestroyer(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponRange1    = scale_range(tw_get_config_float("Weapon", "Range1", 0));
	weaponVelocity1 = scale_velocity(tw_get_config_float("Weapon", "Velocity1", 0));
	weaponDamage1   = tw_get_config_int("Weapon", "Damage1", 0);
	weaponArmour1   = tw_get_config_int("Weapon", "Armour1", 0);
	weaponRelativity1 = tw_get_config_float("Weapon", "Relativity1", 0);
	weaponAngleSpread1 = tw_get_config_float("Weapon","AngleSpread1", 0) * ANGLE_RATIO;
	weaponRange2    = scale_range(tw_get_config_float("Weapon", "Range2", 0));
	weaponVelocity2 = scale_velocity(tw_get_config_float("Weapon", "Velocity2", 0));
	weaponDamage2   = tw_get_config_int("Weapon", "Damage2", 0);
	weaponArmour2   = tw_get_config_int("Weapon", "Armour2", 0);
	weaponRelativity2 = tw_get_config_float("Weapon", "Relativity2", 0);

	specialColor  = tw_get_config_int("Special", "Color", 0);
	specialRange  = scale_range(tw_get_config_float("Special", "Range", 0));
	specialFrames = tw_get_config_int("Special", "Frames", 0);
	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "DamageArmour", 0);
	specialArmour   = tw_get_config_int("Special", "DamageArmour", 0);
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));
	specialMaxDamage = tw_get_config_int("Special","MaxDamage",0);
	weaponObject=NULL;
}


int LyrmristuWarDestroyer::activate_weapon()
{
	STACKTRACE;
	game->add(new LyrmristuWaBolt2(size.y*(0.0), (size.y * 0.6),
		angle, weaponVelocity2, weaponDamage2, weaponRange2, weaponArmour2,
		this, data->spriteWeapon, weaponRelativity2));
	game->add(new LyrmristuWaBolt1(size.y*(-0.4), (size.y * 0.6),
		angle-weaponAngleSpread1, weaponVelocity1, weaponDamage1, weaponRange1, weaponArmour1,
		this, data->spriteWeapon, weaponRelativity1));
	game->add(new LyrmristuWaBolt1(size.y*(0.4), (size.y * 0.6),
		angle+weaponAngleSpread1, weaponVelocity1, weaponDamage1, weaponRange1, weaponArmour1,
		this, data->spriteWeapon, weaponRelativity1));

	return(TRUE);
}


int LyrmristuWarDestroyer::activate_special()
{
	STACKTRACE;
	if (weaponObject==NULL) {
		weaponObject = (new LyrmristuWaSphere(
			0.0, 0.0, angle, specialVelocity, specialDamage, specialRange,
			specialArmour, this, data->spriteSpecial, &weaponObject));
		game->add(weaponObject);
	}
	else if (weaponObject->damage_factor >= specialMaxDamage) return(FALSE);
	else {
		weaponObject->damage_factor += specialDamage;
		weaponObject->armour += specialArmour;
		weaponObject->powerLevel = weaponObject->damage_factor;
	}
	return(TRUE);
}


int LyrmristuWarDestroyer::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (weaponObject==NULL)
		return Ship::handle_damage(source, normal, direct);
	else if (weaponObject->state==0)
		return Ship::handle_damage(source, normal, direct);
	else {
		if (weaponObject->armour < normal) {
			normal -= weaponObject->armour;
			weaponObject->armour = 0;
			weaponObject->damage_factor = 0;
			weaponObject->state = 0;
			weaponObject->powerLevel = 0;
		}else
		if (weaponObject->armour == normal) {
			weaponObject->armour = 0;
			weaponObject->damage_factor = 0;
			normal = 0;
			weaponObject->state = 0;
			weaponObject->powerLevel = 0;
		}else
		if (weaponObject->armour > normal) {
			weaponObject->armour -= normal;
			weaponObject->damage_factor = weaponObject->armour;
			weaponObject->powerLevel = weaponObject->damage_factor;
			normal = 0;
		}
		return Ship::handle_damage(source, normal, direct);
	}
}


LyrmristuWaSphere::LyrmristuWaSphere(double ox, double oy, double oangle,
double ov, int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
LyrmristuWaSphere **P)
:
Shot(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship, osprite)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	powerLevel = damage_factor;
	PP=P;
	if (armour<damage_factor) damage_factor=armour;
	if (damage_factor<armour) armour=damage_factor;
	sprite_index = iround((damage_factor - 1)/ 2);
	if (sprite_index>9) sprite_index=9;
	if (sprite_index<0) sprite_index=0;
}


void LyrmristuWaSphere::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		ship = 0;				 // not really needed
		state = 0;
		return;
	}

	if (ship && (ship->target) && (!ship->target->isInvisible())) {
		angle = ship->get_angle() -0;
		vel = v * unit_vector(angle);
	}
	pos = ship->normal_pos();
	angle = ship->angle;
	vel = ship->get_vel();
	Shot::calculate();
	if (state!=0) {
		sprite_index = iround(damage_factor - 1);
		if (sprite_index>15) sprite_index=15;
		if (sprite_index<0) sprite_index=0;
	}
	if (powerLevel<1) state=0;	 //should be the only gateway to kill it!
}


int LyrmristuWaSphere::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int x;
	x = Shot::handle_damage(source, normal, direct);
	if (source->isShot()) {
		source->state = 0;		 //an intact field kills at least one incoming shot.
		source->damage_factor = 0;
	}
	if (armour<damage_factor) damage_factor=armour;
	powerLevel = damage_factor;
	return x;
}


void LyrmristuWaSphere::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	int startingTargetArmour=0, endingTargetArmour=0;
	int originalCrew=-1;
	int originalStrength;
	originalStrength = iround(this->damage_factor);
	Ship* S1;
	if (other->isShip()) {
		S1 = (Ship*)other;
		originalCrew = iround(S1->crew);
	}
	if (other->isShot()) startingTargetArmour = iround(((Shot*)other)->armour);
	Shot::inflict_damage(other);
	if (other->isShot()) {
		endingTargetArmour = iround(((Shot*)other)->armour);
		((Shot*)other)->damage_factor -= (startingTargetArmour - endingTargetArmour);
		if (((Shot*)other)->damage_factor<0) ((Shot*)other)->damage_factor = 0;
	}
	if (other->isAsteroid()&&damage_factor>1) {
		state=1; damage_factor--; powerLevel=damage_factor;
	}
	if (originalCrew>0 && originalCrew<originalStrength) {
		damage_factor = originalStrength-originalCrew;
		armour = damage_factor;
		powerLevel=damage_factor;
		state = 1;
	}
	return;
}


bool LyrmristuWaSphere::die(void)
{
	STACKTRACE;
	if (powerLevel>0) {
		state = 1;
		damage_factor = powerLevel;
		armour = powerLevel;
		return(false);
	}
	else
		*PP=NULL;
	return(Shot::die());
}


void LyrmristuWaSphere::destroy(void)
{
	STACKTRACE;
	*PP=NULL;
	Shot::destroy();
}


void LyrmristuWaSphere::death()
{
	STACKTRACE;
	*PP=NULL;
}


LyrmristuWaBolt1::LyrmristuWaBolt1(double ox, double oy, double oangle,
double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
double relativity)
:
Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour,
oship,osprite, relativity)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	sprite_index = (get_index(ship->get_angle()) +
		(0 * 64));

}


LyrmristuWaBolt2::LyrmristuWaBolt2(double ox, double oy, double oangle,
double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
double relativity)
:
Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour,
oship,osprite, relativity)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	sprite_index = (get_index(ship->get_angle()) +
		(1 * 64));

}


REGISTER_SHIP(LyrmristuWarDestroyer)
