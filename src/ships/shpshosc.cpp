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

#include "shpshosc.h"

ShofixtiScout::ShofixtiScout(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialRange  = scale_range(tw_get_config_float("Special", "Range", 0));
	specialScale = tw_get_config_float("Special", "Scale", 1.0);
	specialFrames = tw_get_config_int("Special", "Frames", 0);
	specialDamage = tw_get_config_int("Special", "Damage", 0);
	flipSwitch    = FALSE;
	glory         = 0;

}


int ShofixtiScout::activate_weapon()
{
	STACKTRACE;
	add(new Missile(this, Vector2(0.0, size.y / 2.0),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}


void ShofixtiScout::calculate_fire_special()
{
	STACKTRACE;
	int gloryDamage;

	if (fire_special) {
		if (!flipSwitch) {
			flipSwitch = TRUE;
			glory++;
			if (glory == 3) {
				Query q;
				for (q.begin(this, OBJECT_LAYERS, specialRange); q.currento; q.next()) {
					if (q.currento->canCollide(this)) {
						gloryDamage = (int)ceil((specialRange - distance(q.currento)) / specialRange * specialDamage);
						damage(q.current, 0, gloryDamage);
					}
				}
				q.end();
				add(new Animation(this, pos, data->spriteSpecial,
					0, specialFrames, 50, DEPTH_EXPLOSIONS, specialScale));
				play_sound2(data->sampleSpecial[0]);
				damage(this, 0, 999);
			} else {
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(2));
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(3));
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(4));
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(5));
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(1));
				/*				blit(data->bitmapPanel[5 + glory], panelBitmap[1], 0, 23, 0, 23,
										CAPTAIN_WIDTH, 7);
								blit(data->bitmapPanel[5 + glory], panelBitmap[2], 0, 23, 0, 23,
										CAPTAIN_WIDTH, 7);
								blit(data->bitmapPanel[5 + glory], panelBitmap[3], 0, 23, 0, 23,
										CAPTAIN_WIDTH, 7);
								blit(data->bitmapPanel[5 + glory], panelBitmap[4], 0, 23, 0, 23,
										CAPTAIN_WIDTH, 7);
								blit(data->bitmapPanel[5 + glory], panelBitmap[5], 0, 23, 0, 23,
										CAPTAIN_WIDTH, 7);*/
			}
		}
	}
	else flipSwitch = FALSE;
	return;
}


REGISTER_SHIP(ShofixtiScout)
