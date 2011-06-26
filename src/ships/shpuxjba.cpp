/* $Id: shpuxjba.cpp,v 1.9 2005/08/14 16:14:32 geomannl Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class Uxjoz : public Ship
{
	public:
		double  weaponRange, weaponVelocity, weaponDamage, weaponArmour, weaponMass;
		double  specialRange, specialVelocity, specialDamage, specialArmour, specialTurnRate;

	public:
		Uxjoz(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:

		virtual int activate_weapon();
		virtual int activate_special();

};

Uxjoz::Uxjoz(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponMass     = tw_get_config_float("Weapon", "Mass", 0);

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));

}


int Uxjoz::activate_weapon()
{
	STACKTRACE;

	// shaky
	double a = 0.04 * PI;
	turn_step += tw_random(2*a) - a;

	Shot *s;
	s = new Shot(this, Vector2(0,100), angle,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, 0.0);
	s->mass = weaponMass;

	s->explosionSprite = data->spriteWeaponExplosion;
	s->explosionSample = 0;
	s->explosionFrameCount = data->spriteWeaponExplosion->frames();
	s->explosionFrameSize = 100;

	add(s);

	return TRUE;
}


int Uxjoz::activate_special()
{
	STACKTRACE;
	//HomingMissile::HomingMissile(SpaceLocation *creator, Vector2 rpos,
	//	double oangle, double ov, double odamage, double orange, double oarmour,
	//	double otrate, SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget)

	int i;
	for ( i = 0; i < 2; ++i ) {
		double da;

		if (i)
			da = 1;
		else
			da = -1;

		HomingMissile *h;
		h = new HomingMissile(this,
			Vector2(da*50, 0.0), angle+da*PI/2, specialVelocity, specialDamage, specialRange,
			specialArmour, specialTurnRate, this, data->spriteSpecial, target);

		h->explosionSprite = data->spriteSpecialExplosion;
		h->explosionSample = 0;
		h->explosionFrameCount = data->spriteSpecialExplosion->frames();
		h->explosionFrameSize = 100;

		game->add(h);

	}

	return TRUE;
}


REGISTER_SHIP(Uxjoz)
