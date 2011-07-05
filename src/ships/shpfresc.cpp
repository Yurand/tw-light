/* $Id: shpfresc.cpp,v 1.10 2005/07/11 00:25:31 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

class FreinSchizm : public Ship
{
	public:
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		double       specialRange;
		double       specialVelocity;
		int          specialDamage;
		int          specialArmour;
		double       specialTurnRate;

	public:
		FreinSchizm(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
};

class SchizmHealingbolt : public HomingMissile
{
	public:
	public:
		SchizmHealingbolt(SpaceLocation *creator, Vector2 rpos, double oangle,
			double ov, double odamage, double orange, double oarmour, double oturnrate,
			SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget);

		virtual void inflict_damage(SpaceObject *other);
};

FreinSchizm::FreinSchizm(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));
}


int FreinSchizm::activate_weapon()
{
	STACKTRACE;
	Shot *tmp = new Missile( this,
		Vector2(0.0, size.y / 1.5), angle, weaponVelocity, weaponDamage, weaponRange,
		weaponArmour, this, data->spriteSpecial);
	add(tmp);

	tmp->explosionSprite = data->spriteWeaponExplosion;
	tmp->explosionSample = 0;
	tmp->explosionFrameCount = data->spriteWeaponExplosion->frames();
	tmp->explosionFrameSize = 100;
	return(TRUE);
}


int FreinSchizm::activate_special()
{
	STACKTRACE;

	Shot *tmp = new SchizmHealingbolt( this, Vector2(0.0, size.y / 2.0),
		angle, specialVelocity, specialDamage, specialRange, specialArmour, specialTurnRate,
		this, data->spriteWeapon, target);
	add(tmp);

	tmp->explosionSprite = data->spriteSpecialExplosion;
	tmp->explosionSample = 0;
	tmp->explosionFrameCount = data->spriteSpecialExplosion->frames();
	tmp->explosionFrameSize = 100;

	return(TRUE);
}


//HomingMissile::HomingMissile(SpaceLocation *creator, Vector2 rpos,
//	double oangle, double ov, double odamage, double orange, double oarmour,
//	double otrate, SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget)

SchizmHealingbolt::SchizmHealingbolt(SpaceLocation *creator, Vector2 rpos, double oangle,
double ov, double odamage, double orange, double oarmour, double oturnrate,
SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget)
:
HomingMissile(creator, rpos, oangle, ov, odamage, orange, oarmour, oturnrate, opos, osprite, otarget)
{
	STACKTRACE;
}


void SchizmHealingbolt::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!(ship && ship->exists()))
		return;

	// check if the "other" is a ship, and already at max capacity, otherwise, damage
	// your own ship NO, THAT SUCKED.
	// different strategy now: if either has <max crew, add 1 crew to the schizm and
	// 1 to the enemy (to save the balance of birth and rebirth in the universe). The
	// lost and restless souls of those that died in battle, are revived that way.

	if (other->isShip() &&
		((Ship*)other)->crew <= ((Ship*)other)->crew_max - 1
	&& ship->crew <= ship->crew_max - 1 ) {
		HomingMissile::inflict_damage(ship);
		HomingMissile::inflict_damage(other);
		die();
	} else {
		die();
	}

	//	add(new Animation(this, pos, data->spriteSpecial,
	//						0, data->spriteSpecial->frames(), 100, DEPTH_EXPLOSIONS, specialScale));
}


REGISTER_SHIP(FreinSchizm)
