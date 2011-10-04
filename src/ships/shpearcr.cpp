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

#include "shpearcr.h"

EarthlingCruiser::EarthlingCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponTurnRate = scale_turning(tw_get_config_float("Weapon", "TurnRate", 0));

	specialColor  = tw_get_config_int("Special", "Color", 0);
	specialRange  = scale_range(tw_get_config_float("Special", "Range", 0));
	specialFrames = tw_get_config_int("Special", "Frames", 0);
	specialDamage = tw_get_config_int("Special", "Damage", 0);

}


int EarthlingCruiser::activate_weapon()
{
	STACKTRACE;
	game->add(new EarthlingMissile(
		Vector2(0.0, (size.y * 1.0)), angle, weaponVelocity, weaponDamage, weaponRange,
		weaponArmour, weaponTurnRate, this, data->spriteWeapon));
	return(TRUE);
}


int EarthlingCruiser::activate_special()
{
	STACKTRACE;
	int fire = FALSE;
	SpaceObject *o;

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
	bit(LAYER_CBODIES), specialRange); a.current; a.next()) {
		o = a.currento;
		if ( (!o->isInvisible()) && !o->sameTeam(this) && (o->collide_flag_anyone & bit(LAYER_LINES))) {
			SpaceLocation *l = new PointLaser(this, tw_get_palete_color(specialColor), 1,
				specialFrames, this, o, Vector2(0.0, 10.0));
			game->add(l);
			if (l->exists()) {
				fire = TRUE;
				l->set_depth(LAYER_EXPLOSIONS);
			}
		}
	}
	if (fire) play_sound((SAMPLE *)(melee[MELEE_BOOM + 0].dat));

	return(fire);
}


EarthlingMissile::EarthlingMissile(Vector2 opos, double oangle,
double ov, int odamage, double orange, int oarmour, double otrate,
Ship *oship, SpaceSprite *osprite) :
HomingMissile(oship, opos, oangle, ov, odamage, orange, oarmour, otrate,
oship, osprite, oship->target)
{
	STACKTRACE;
	collide_flag_sameship = bit(LAYER_SHIPS) | bit(LAYER_SHOTS);
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
}


REGISTER_SHIP(EarthlingCruiser)
