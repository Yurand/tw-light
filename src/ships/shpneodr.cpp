/* $Id: shpneodr.cpp,v 1.13 2005/08/23 22:18:11 geomannl Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class NeoDrain : public Ship
{
	public:
		double  weaponRange, weaponVelocity, weaponDamage, weaponArmour;
		double  specialRange, specialDamage;
		int     specialColor;
		double  weapon_drain_ref;

		bool shipmode;

	public:
		NeoDrain(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();

};

class LaserDrain : public Laser
{
	public:
		void inflict_damage(SpaceObject *other);
	public:
		LaserDrain(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage,
			int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle);
};

NeoDrain::NeoDrain(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialColor  = tw_get_config_int("Special", "Color", 0);
	specialRange  = scale_range(tw_get_config_float("Special", "Range", 0));
	specialDamage = tw_get_config_int("Special", "Damage", 0);

	weapon_drain_ref = weapon_drain;
}


int NeoDrain::activate_weapon()
{
	STACKTRACE;

	add(new Missile(this, Vector2(36,20), angle,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, 0.0));

	return TRUE;
}


int NeoDrain::activate_special()
{
	STACKTRACE;

	add(new LaserDrain(this, angle,
		pallete_color[specialColor], specialRange, specialDamage, special_rate,
		this, Vector2(-10,42), true));

	return TRUE;
}


void NeoDrain::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if (special_recharge > 0)
		// special is being used
		weapon_drain = iround(2 * weapon_drain_ref);
	else
		weapon_drain = iround(weapon_drain_ref);
}


LaserDrain::LaserDrain(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage,
int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle)
:
Laser(creator, langle, lcolor, lrange, ldamage, lfcount, opos, rpos, osinc_angle)
{
	STACKTRACE;
	// nother extra needed.
}


void LaserDrain::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	other->handle_fuel_sap(this, damage_factor);
}


REGISTER_SHIP(NeoDrain)
