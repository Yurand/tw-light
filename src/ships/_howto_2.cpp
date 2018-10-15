/* $Id$ */ 

#include "../ship.h"
REGISTER_FILE

/*Ship-Howto 2. How to make a ship fire a (default) weapon
*/

class HowTo_2 : public Ship
{
public:
IDENTITY(HowTo_2);
	// parameters we need to define weapon behaviour
	double	weaponRange, weaponVelocity, weaponDamage;
	int		weaponArmour;
public:
	HowTo_2(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag);

	/* This is called by the Ship class, when the fire-button is pressed. By default
	is does nothing, but you can overload it to spawn some weapon.
	*/
	virtual int activate_weapon();
};

HowTo_2::HowTo_2(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag)
:
Ship(opos, shipAngle, shipData, shipCollideFlag)
{
	// the range. The scale_range scales from "sc2" values to pixel values.
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	// the velocity. The scale_velocity scales from "sc2" values to pixel/second values
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	// the damage done when it hits some object. For normal weapon types, this subtracts the specified amount of crew if the.
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	// armour = how many hit points it absorbs, before it dies.
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	// for debugging purpose
	debug_id = 4;
};

int HowTo_2::activate_weapon()
{
	Missile *m;
	// how far away from the ship does this missile start its violent existence.
	Vector2 offset = Vector2(0.0, size.y / 2.0 + 10);

	// creates a new missile, but this is just some memory allocation.
	m = new Missile(this, offset,
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon);

	// adds to the physics (class Physics - but you can also use game->add, which adds to the physics anyway)
	// Thus this object is put in a list, and its subroutines can then invoked during game iteration.
	physics->add(m);

	// meaningless.
	return 0;
}


REGISTER_SHIP(HowTo_2);

