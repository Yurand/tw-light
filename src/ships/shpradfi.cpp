/* $Id: shpradfi.cpp,v 1.7 2004/03/24 23:51:42 yurand Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class RadeanFirestorm : public Ship
{
	double  weaponRange, weaponVelocity, weaponDamage, weaponArmour, battmultiplier;

	bool shipmode;

	public:
		RadeanFirestorm(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:

		virtual void calculate();
		virtual void animate(Frame *f);

		virtual int activate_weapon();
		virtual int activate_special();

		virtual int handle_damage(SpaceLocation* source, double normal, double direct);
};

RadeanFirestorm::RadeanFirestorm(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	battmultiplier = get_config_float("Special", "battmultiplier", 2.0);

	shipmode = 0;
}


int RadeanFirestorm::activate_weapon()
{
	STACKTRACE;

	if (!shipmode) {
		Vector2 rpos;
		rpos = Vector2(0,40);	 // * unit_vector(angle);

		add(new Missile(this, rpos, angle,
			weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon, 0.0));

		return TRUE;
	} else
	return FALSE;
}


int RadeanFirestorm::activate_special()
{
	STACKTRACE;
	// toggle between 0 and 1.
	shipmode = !shipmode;
	return TRUE;
}


int RadeanFirestorm::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	if (shipmode) {
		// damage also adds to your battery.
		handle_fuel_sap(this, -battmultiplier * normal);
	}

	// handle crew damage as usual
	return Ship::handle_damage(source, normal, direct);
}


void RadeanFirestorm::calculate()
{
	STACKTRACE;
	Ship::calculate();

	sprite_index = get_sprite_index();
	if (shipmode)
		sprite_index += 64;
}


void RadeanFirestorm::animate(Frame *f)
{
	STACKTRACE;
	// animate the ship
	Ship::animate(f);

	// if in mode 0, ie a weapon can be fired, animate a growing spark:
	if (!shipmode) {
		Vector2 rpos;
		int index;
		double r;

		rpos = pos + 40 * unit_vector(angle);

		r = batt / weapon_drain;
		if (r > 0.999)
			r = 0.999;
		index =  int( r * data->spriteSpecial->frames());

		data->spriteSpecial->animate(rpos, index, f);
	}
}


REGISTER_SHIP(RadeanFirestorm)
