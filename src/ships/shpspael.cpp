/*
This file is part of "TW-Light"
					http://tw-light.appspot.com/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "ship.h"
REGISTER_FILE

#include "shpspael.h"

SpathiEluder::SpathiEluder(Vector2 opos, double angle, ShipData *data, unsigned int code)
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


int SpathiEluder::activate_weapon()
{
	STACKTRACE;
	add(new Missile( this, Vector2(0.0, size.y / 2.0),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}


int SpathiEluder::activate_special()
{
	STACKTRACE;
	SpaceLocation *tmp = new HomingMissile( this,
		Vector2(0.0, -size.y / 1.5), angle + PI, specialVelocity, specialDamage, specialRange,
		specialArmour, specialTurnRate, this, data->spriteSpecial, target);
	tmp->collide_flag_sameship |= bit(LAYER_SHIPS);
	tmp->collide_flag_sameteam |= bit(LAYER_SHIPS);
	add(tmp);
	return(TRUE);
}


REGISTER_SHIP(SpathiEluder)
