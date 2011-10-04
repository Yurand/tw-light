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

#include <math.h>
#include "shpzfpst.h"

ZoqFotPikStinger::ZoqFotPikStinger(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	specialDamage = tw_get_config_int("Special", "Damage", 0);
	if (tw_get_config_int("Special", "Licking", 0) == 0) specialDamage *= -1;
}


int ZoqFotPikStinger::activate_weapon()
{
	STACKTRACE;
	add(new ZoqFotPikShot(
		Vector2(0.0, 0.5*get_size().y), angle + ANGLE_RATIO * random(-10.0, 10.0),
		weaponVelocity, weaponDamage, weaponRange, weaponArmour, this,
		data->spriteWeapon, 6));
	return(TRUE);
}


int ZoqFotPikStinger::activate_special()
{
	STACKTRACE;
	add(new ZoqFotPikTongue(
		39, specialDamage, this, data->spriteSpecial, 6, 50));
	return(TRUE);
}


ZoqFotPikShot::ZoqFotPikShot(Vector2 opos, double oangle,
double ov, int odamage, double orange, int oarmour, Ship *oship,
SpaceSprite *osprite, int num_frames) :
Shot(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite),
frame_count(num_frames)
{
	STACKTRACE;
}


void ZoqFotPikShot::calculate()
{
	STACKTRACE;
	Shot::calculate();
	sprite_index = (int)((d / range) * (double)(frame_count - 1));
}


ZoqFotPikTongue::ZoqFotPikTongue(double odist, int odamage, Ship *oship,
SpaceSprite *osprite, int ofcount, int ofsize) :
SpaceObject(oship, oship->normal_pos()/*, oship->normal_y()*/, 0.0,
osprite),
dist(odist),
ship(oship),
lick_factor(odamage),
frame(0),
frame_size(ofsize),
frame_count(ofcount),
frame_step(0)
{
	STACKTRACE;
	layer = LAYER_SHOTS;
	set_depth(DEPTH_SHOTS);
	damage_factor = abs(lick_factor);
	//  x = ship->normal_x() + (cos(ship->get_angle()) * dist);
	//  y = ship->normal_y() + (sin(ship->get_angle()) * dist);
	pos = ship->normal_pos() + dist * unit_vector(ship->get_angle());
	//  vx = ship->get_vx(); vy = ship->get_vy();
	vel = ship->get_vel();
	sprite_index = get_index(ship->get_angle());
	sprite_index += (64 * frame);

	isblockingweapons = true;
}


void ZoqFotPikTongue::calculate()
{
	STACKTRACE;
	int current_frame = frame;

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	frame_step+= frame_time;
	while(frame_step >= frame_size) {
		frame_step -= frame_size;
		frame++;
		if (frame == frame_count)
			state = 0;
	}
	if ((current_frame != frame) && (lick_factor > 0))
		damage_factor = lick_factor;

	//  x = ship->normal_x() + cos(ship->get_angle()) * dist;
	//  y = ship->normal_y() + sin(ship->get_angle()) * dist;
	pos = ship->normal_pos() + dist * unit_vector(ship->get_angle());
	//  vx = ship->get_vx(); vy = ship->get_vy();
	vel = ship->get_vel();
	sprite_index = get_index(ship->get_angle());
	sprite_index += (64 * frame);

	SpaceObject::calculate();
}


void ZoqFotPikTongue::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	SpaceObject::inflict_damage(other);
	damage_factor = 0;

	// in order to remove the variable damage:
	lick_factor = 0;
}


REGISTER_SHIP(ZoqFotPikStinger)
