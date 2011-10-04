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

#include "frame.h"

#include "shputwju.h"

UtwigJugger::UtwigJugger(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	fortitude = 0;
}


void UtwigJugger::calculate()
{
	STACKTRACE;
	Ship::calculate();
	if (!fire_special)
		special_recharge = 0;
}


void UtwigJugger::calculate_fire_weapon()
{
	STACKTRACE;
	weapon_low = FALSE;

	if (fire_weapon) {
		if ((weapon_recharge > 0) || (special_recharge > 0))
			return;
		if (fire_special && batt) return;

		add(new Missile(this, Vector2(34.0, 11.0),
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon));
		add(new Missile(this, Vector2(-34.0, 11.0),
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon));
		add(new Missile(this, Vector2(18.0, 20.0),
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon));
		add(new Missile(this, Vector2(-18.0, 20.0),
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon));
		add(new Missile(this, Vector2(6.0, 27.0),
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon));
		add(new Missile(this, Vector2(-6.0, 27.0),
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon));

		weapon_recharge = weapon_rate;

		play_sound2(data->sampleWeapon[0]);
	}
}


void UtwigJugger::animate(Frame *space)
{
	STACKTRACE;
	if (special_recharge > 0) {
		sprite->animate_character(pos, sprite_index, tw_get_palete_color(hot_color[((int)(special_recharge/10)) % HOT_COLORS]), space);
	}
	else sprite->animate( pos, sprite_index, space);
	return;
}


int UtwigJugger::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (special_recharge > 0) {
		batt += normal;
		if (batt > batt_max) batt = batt_max;
		normal = 0;
	}
	if (fortitude && (normal > 0)) {
		if (normal > random(3)) normal -= fortitude;
		if (normal < 0) normal = 0;
	}
	return Ship::handle_damage(source, normal, direct);
}


double UtwigJugger::isProtected() const
{
	return (special_recharge > 0) ? 1 : 0;
}


REGISTER_SHIP(UtwigJugger)
