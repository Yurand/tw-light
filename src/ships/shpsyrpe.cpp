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

#include "shpsyrpe.h"

SyreenPenetrator::SyreenPenetrator(Vector2 opos, double angle, ShipData *data, unsigned int code)
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
	specialFrames   = tw_get_config_int("Special", "Frames", 0);
}


int SyreenPenetrator::activate_weapon()
{
	STACKTRACE;
	Missile *m;
	m = new Missile(this, Vector2(0.0, size.y / 2.0 + 10),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon);
	add(m);
	m->collide_flag_sameship = ALL_LAYERS;

	return(TRUE);
}


int SyreenPenetrator::activate_special()
{
	STACKTRACE;
	double minDist;
	minDist = specialRange + (size.x / 2.0);
	int j;
	for (j = 0; j < targets->N; j += 1) {
		Ship *target = (Ship*) targets->item[j];
		if (!target->isShip()) continue;
		if (!control->valid_target(target)) continue;
		int callDamage;
		double r;
		if (target) r = minDist - distance(target) - target->get_size().x / 2.0;
		else r = -1;

		Color col = target->crewPanelColor();
		if ((col.g < 192) || (col.r > 16) || (col.b > 16)) continue;
		if (r > 0) {
			callDamage = (int)(r * (double)(specialDamage) / specialRange);
			if (callDamage > specialDamage) callDamage = specialDamage;
			callDamage += (random() % specialDamage);
			if ((target->getCrew() - callDamage) < 1) callDamage = iround(target->getCrew() - 1);
			int old = iround(target->getCrew());
			damage(target, 0, callDamage);
			callDamage = iround(old - target->getCrew());
			for(int i = 0; i < callDamage; i++) {
				add(new CrewPod(
					target->normal_pos() + (unit_vector(trajectory_angle(target) - PI) *
					target->size.x) + random(Vector2(50,50)) - Vector2(25,25),
					specialVelocity, specialFrames, this, data->spriteSpecial, 32,
					50));
			}
		}
	}
	return(TRUE);
}


CrewPod::CrewPod(Vector2 opos, double oVelocity, int oLifeTime,
Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize) :
SpaceObject(oship, opos, 0.0, osprite),
frame_count(ofcount),
frame_size(ofsize),
frame_step(0),
velocity(oVelocity),
life(0),
lifetime(oLifeTime)
{
	STACKTRACE;
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);

	isblockingweapons = false;
}


int CrewPod::sameTeam(SpaceLocation *other)
{
	STACKTRACE;
	return true;
}


void CrewPod::calculate()
{
	STACKTRACE;
	frame_step += frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if (sprite_index == frame_count) sprite_index = 0;
	}

	life += frame_time;
	if (life >= lifetime) {
		state = 0;
		return;
	}

	if (ship && ship->exists()) {
		vel = unit_vector(trajectory_angle(ship)) * velocity;
	}
	else ship = NULL;

	SpaceObject::calculate();
}


void CrewPod::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->isShip() && other->damage_factor == 0) {
		sound.stop(data->sampleExtra[0]);
		sound.play(data->sampleExtra[0]);
		damage(other, 0, -1);
		state = 0;
	}
}


int CrewPod::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	state = 0;
	return 0;
}


REGISTER_SHIP(SyreenPenetrator)
