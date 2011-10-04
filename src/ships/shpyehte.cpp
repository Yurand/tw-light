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

class YehatTerminator : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		int          specialFrames;
		int          shieldFrames;

	public:
		YehatTerminator(Vector2 opos, double angle, ShipData *data, unsigned int code);
		virtual double isProtected() const;

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void animate(Frame *space);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

YehatTerminator::YehatTerminator(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialFrames = tw_get_config_int("Special", "Frames", 0);
	shieldFrames  = 0;
}


int YehatTerminator::activate_weapon()
{
	STACKTRACE;
	add(new Missile(this, Vector2(-24.0, 14),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	add(new Missile(this, Vector2(24.0, 14),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}


int YehatTerminator::activate_special()
{
	STACKTRACE;
	shieldFrames = (shieldFrames % frame_time) + specialFrames;
	return(TRUE);
}


void YehatTerminator::calculate()
{
	STACKTRACE;
	if (shieldFrames > 0)
		shieldFrames-= frame_time;
	Ship::calculate();
}


void YehatTerminator::animate(Frame *space)
{
	STACKTRACE;
	if (shieldFrames > 0)
		data->spriteSpecial->animate( pos, sprite_index, space);
	else
		sprite->animate( pos, sprite_index, space);
}


int YehatTerminator::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (shieldFrames > 0) normal = 0;
	return Ship::handle_damage(source, normal, direct);
}


double YehatTerminator::isProtected() const
{
	return (shieldFrames > 0);
}


REGISTER_SHIP(YehatTerminator)
