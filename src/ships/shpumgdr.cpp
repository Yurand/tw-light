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

class UmgahDrone : public Ship
{
	public:
		int          weaponDamage;
		int          weaponCone;
		int          specialRate;

	public:

		int          firing;
		int          damage_type;

		UmgahDrone(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate_thrust();
		virtual void calculate();
};

class UmgahCone : public SpaceObject
{
	double dist;

	double damage;
	double residual_damage;
	int damage_type;

	UmgahDrone *umgahship;

	public:
		UmgahCone(double odist, int odamage, UmgahDrone *oship, SpaceSprite *osprite);

		virtual bool change_owner(SpaceLocation *new_owner);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual void animate(Frame* space);
		virtual int canCollide(SpaceLocation* other);

};

UmgahDrone::UmgahDrone(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	specialRate    = special_rate;
	damage_type    = tw_get_config_int("Weapon", "DamageType", 0);

	firing = false;
	add(new UmgahCone(81, weaponDamage, this, data->spriteWeapon));
}


int UmgahDrone::activate_weapon()
{
	STACKTRACE;
	firing = true;
	return(TRUE);
}


int UmgahDrone::activate_special()
{
	STACKTRACE;
	special_rate = specialRate;
	vel = 0;
	pos -= (unit_vector(angle) * size.x * 2.0);
	if (!thrust) special_rate = 50;
	return(TRUE);
}


void UmgahDrone::calculate_thrust()
{
	STACKTRACE;
	if (special_recharge <= 0)
		Ship::calculate_thrust();
}


void UmgahDrone::calculate()
{
	STACKTRACE;
	if (fire_weapon)
		recharge_step = recharge_rate;
	else
		firing = false;

	Ship::calculate();
}


UmgahCone::UmgahCone(double odist, int odamage, UmgahDrone *oship,
SpaceSprite *osprite) :
SpaceObject(oship, oship->normal_pos(), 0.0, osprite),
dist(odist)
{
	STACKTRACE;
	layer = LAYER_SHOTS;
	set_depth(DEPTH_SHIPS - 0.1);

	id = SPACE_SHOT;
	damage_factor = 1;

	damage_type = oship->damage_type;
	damage = (odamage/1000.0);
	residual_damage = 1e-5;

	pos = ship->normal_pos() + (unit_vector(ship->get_angle()) * dist);
	angle = ship->get_angle();
	sprite_index = get_index(angle);

	collide_flag_sameship = 0;

	isblockingweapons = false;

	umgahship = oship;
}


bool UmgahCone::change_owner(SpaceLocation *new_owner)
{
	STACKTRACE;
	return false;
}


void UmgahCone::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	if (!(umgahship && umgahship->exists()))
		umgahship = NULL;

	pos = ship->normal_pos() + (unit_vector(ship->get_angle()) * dist);
	vel = ship->get_vel();
	angle = ship->get_angle();
	sprite_index = get_index(angle);

	if (damage_type == 2) {
		residual_damage = fabs(residual_damage);
		if (residual_damage <= 0) residual_damage = 1e-5;
	}
}


void UmgahCone::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	switch (damage_type) {
		case 0:
		{
			residual_damage = damage * frame_time / 2.0;
			while (residual_damage >= 1) {
				residual_damage -= 1;
				SpaceObject::inflict_damage(other);
			};
			residual_damage *= 1e4;
			if (residual_damage > random(10000))
				SpaceObject::inflict_damage(other);
		}; break;
		case 1:
		{
			residual_damage += damage * frame_time / 2.0;
			while (residual_damage >= 1) {
				residual_damage -= 1;
				SpaceObject::inflict_damage(other);
			};
		}; break;
		case 2: if (residual_damage > 0) {
			residual_damage += damage * frame_time;
			while (residual_damage >= 1) {
				residual_damage -= 1;
				SpaceObject::inflict_damage(other);
			};
			residual_damage *= -1;
		}; break;
	}

}


int UmgahCone::canCollide(SpaceLocation* other)
{
	STACKTRACE;
	//  calc_base();
	if (umgahship && umgahship->exists())
		return (umgahship->firing & !other->isPlanet());
	else
		return false;
}


void UmgahCone::animate(Frame* space)
{
	STACKTRACE;
	//  calc_base();
	if (!umgahship || !umgahship->exists() || !umgahship->firing) return;
	int si = sprite_index;
								 //graphics
	sprite_index += ((rand()%6) << 6);
	SpaceObject::animate(space);
	sprite_index = si;
}


REGISTER_SHIP(UmgahDrone)
