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

class ThraddashTorch : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		double       specialThrust;
		int          specialDamage;
		int          specialArmour;

	public:
		ThraddashTorch(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate_thrust();
};

class ThraddashFlame : public Animation
{
	int armour;

	public:
		ThraddashFlame(Vector2 opos, int odamage, int oarmour, Ship *oship,
			SpaceSprite *osprite, int ofcount, int ofsize);

		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

ThraddashTorch::ThraddashTorch(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialThrust = scale_velocity(tw_get_config_float("Special", "Thrust", 0));
	specialDamage = tw_get_config_int("Special", "Damage", 0);
	specialArmour = tw_get_config_int("Special", "Armour", 0);
}


int ThraddashTorch::activate_weapon()
{
	STACKTRACE;
	add(new Missile(this, Vector2(0.0, 0.5*get_size().y),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}


int ThraddashTorch::activate_special()
{
	STACKTRACE;
	accelerate(this, angle, specialThrust, MAX_SPEED);
	add(new ThraddashFlame(
	//    x ,//- cos(angle) * (width() / 2.0),
	//    y ,//- sin(angle) * (height() / 2.0),
		pos - unit_vector(angle) * size.x/2.5,
		specialDamage, specialArmour, this, data->spriteSpecial, 39, 100));
	return(TRUE);
}


void ThraddashTorch::calculate_thrust()
{
	STACKTRACE;
	if (special_recharge <= 0) Ship::calculate_thrust();
}


ThraddashFlame::ThraddashFlame(Vector2 opos, int odamage, int oarmour,
Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize) :
Animation(oship, opos, osprite, 0, ofcount, ofsize, DEPTH_SHOTS),
armour(oarmour)
{
	STACKTRACE;
	layer = LAYER_SHOTS;
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	damage_factor = odamage;

	attributes &= ~ATTRIB_UNDETECTABLE;

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
	}
}


void ThraddashFlame::calculate()
{
	STACKTRACE;
	Animation::calculate();
}


int ThraddashFlame::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int total = iround(normal + direct);
	armour -= total;

	if ((armour <= 0) || (source->isObject() && ((SpaceObject*)source)->mass)) {
		state = 0;
								 //normal_x(), normal_y(),
		add(new Animation(this, normal_pos(),
			meleedata.sparkSprite, 0, SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
	}
	return total;
}


REGISTER_SHIP(ThraddashTorch)
