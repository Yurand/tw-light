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

class ArilouSkiff : public Ship
{
	public:
		int    weaponColor;
		double weaponRange;
		int    weaponFrames;
		int    weaponDamage;

		int just_teleported;

		SpaceSprite *specialSprite;
		double       specialFrames;

	public:
		ArilouSkiff(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		virtual void inflict_damage(SpaceObject *other);
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void calculate_gravity();
		virtual int accelerate(SpaceLocation *source, double angle, double velocity,
			double max_speed);
		virtual int accelerate_gravwhip(SpaceLocation *source, double angle, double velocity,
			double max_speed);
};

ArilouSkiff::ArilouSkiff(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	specialSprite = data->spriteSpecial;

	weaponColor  = tw_get_config_int("Weapon", "Color", 0);
	weaponDamage = tw_get_config_int("Weapon", "Damage", 0);
	weaponRange  = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponFrames = tw_get_config_int("Weapon", "Frames", 0);

	specialFrames = tw_get_config_float("Special", "Frames", 0);

	just_teleported = 0;
}


void ArilouSkiff::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (just_teleported && other->mass) {
		damage(this, 0, 999);
	}
	else Ship::inflict_damage(other);
	return;
}


int ArilouSkiff::activate_weapon()
{
	STACKTRACE;
	SpaceObject *o = NULL;

	double r = 99999;

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS), weaponRange + 200); a.current; a.next()) {
		if ((distance(a.current) < r) && !a.current->isInvisible()) {
			o = a.currento;
			r = distance(o);
		}
	}

	if (o) r = trajectory_angle(o); else r = angle;
	game->add(new Laser(this, r, tw_get_palete_color(weaponColor),
		weaponRange, weaponDamage, weaponFrames, this));
	return TRUE;
}


int ArilouSkiff::activate_special()
{
	STACKTRACE;

	game->add(new Animation(this, pos,
		specialSprite, 0, 40, iround(specialFrames/40), DEPTH_HOTSPOTS-0.1));

	Vector2 d = Vector2 (
		random(-1500.0, 1500.0),
		random(-1500.0, 1500.0)
		);
	translate(d);
	just_teleported = 1;

	game->add(new Animation(this, pos,
		specialSprite, 0, 40, iround(specialFrames/40), DEPTH_HOTSPOTS-0.1));
	return(TRUE);
}


void ArilouSkiff::calculate()
{
	STACKTRACE;
	just_teleported = 0;
	Ship::calculate();

	if (!thrust) {
		vel *= 1 - frame_time * 0.02;
	}
}


void ArilouSkiff::calculate_gravity()
{
	STACKTRACE;
}


int ArilouSkiff::accelerate(SpaceLocation *source, double angle, double velocity,
double max_speed)
{
	STACKTRACE;
	if (source == this)
		return Ship::accelerate(source, angle, velocity, max_speed);
	return false;
}


int ArilouSkiff::accelerate_gravwhip(SpaceLocation *source, double angle, double velocity,
double max_speed)
{
	STACKTRACE;
	if (source == this)
		return Ship::accelerate(source, angle, velocity, max_speed);
	return false;
}


REGISTER_SHIP(ArilouSkiff)
