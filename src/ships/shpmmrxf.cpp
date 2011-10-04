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

#define T_FORM 0
#define Y_FORM 1

class MmrnmhrmXForm : public Ship
{
	public:
		struct
		{
			public:
				double       speed_max;
				double       accel_rate;
				double       turn_rate;
				int          recharge_amount;
				int          recharge_rate;
				int          weapon_drain;
				int          weapon_rate;
				int          hotspot_rate;
				SpaceSprite *sprite;
		} form_data[2];

		int form;

		int        laserColor;
		double     laserRange;
		double     laserAngle;
		int        laserDamage;
		SpaceLine *laser1;
		SpaceLine *laser2;
		int        laserSpark;

		double missileRange;
		double missileVelocity;
		int    missileDamage;
		int    missileArmour;
		double missileTurnRate;

	public:
		MmrnmhrmXForm(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		void calculate();
		double handle_speed_loss(SpaceLocation *source, double normal);

		int activate_weapon();
		int activate_special();
};

MmrnmhrmXForm::MmrnmhrmXForm(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code),
laser1(NULL),
laser2(NULL),
laserSpark(0)
{
	STACKTRACE;
	form_data[T_FORM].speed_max       = speed_max;
	form_data[T_FORM].accel_rate      = accel_rate;
	form_data[T_FORM].turn_rate       = turn_rate;
	form_data[T_FORM].recharge_amount = recharge_amount;
	form_data[T_FORM].recharge_rate   = recharge_rate;
	form_data[T_FORM].weapon_drain    = weapon_drain;
	form_data[T_FORM].weapon_rate     = weapon_rate;
	form_data[T_FORM].hotspot_rate    = hotspot_rate;
	form_data[T_FORM].sprite          = sprite;

	laserColor  = tw_get_config_int("Weapon1", "Color", 0);
	laserDamage = tw_get_config_int("Weapon1", "Damage", 0);
	laserRange  = scale_range(tw_get_config_float("Weapon1", "Range", 0));

	missileRange    = scale_range(tw_get_config_float("Weapon2", "Range", 0));
	missileVelocity = scale_velocity(tw_get_config_float("Weapon2", "Velocity", 0));
	missileDamage   = tw_get_config_int("Weapon2", "Damage", 0);
	missileArmour   = tw_get_config_int("Weapon2", "Armour", 0);
	missileTurnRate = scale_turning(tw_get_config_float("Weapon2", "TurnRate", 0));

	weapon_drain    = tw_get_config_int("Special", "WeaponDrain", 0);
	weapon_rate     = scale_frames(tw_get_config_float("Special", "WeaponRate", 0));
	double raw_hotspot_rate = tw_get_config_float("Special", "HotspotRate", 0);
	hotspot_rate    = scale_frames(raw_hotspot_rate);
	recharge_amount = tw_get_config_int("Special", "RechargeAmount", 0);
	recharge_rate   = scale_frames(tw_get_config_float("Special", "RechargeRate", 0));
	turn_rate       = scale_turning(tw_get_config_float("Special", "TurnRate", 0));
	speed_max       = scale_velocity(tw_get_config_float("Special", "SpeedMax", 0));
	accel_rate      = scale_acceleration(tw_get_config_float("Special", "AccelRate", 0), raw_hotspot_rate);

	form_data[Y_FORM].speed_max       = speed_max;
	form_data[Y_FORM].accel_rate      = accel_rate;
	form_data[Y_FORM].turn_rate       = turn_rate;
	form_data[Y_FORM].recharge_amount = recharge_amount;
	form_data[Y_FORM].recharge_rate   = recharge_rate;
	form_data[Y_FORM].weapon_drain    = weapon_drain;
	form_data[Y_FORM].weapon_rate     = weapon_rate;
	form_data[Y_FORM].hotspot_rate    = hotspot_rate;
	form_data[Y_FORM].sprite = sprite = data->spriteSpecial;

	laserAngle = asin(28 / laserRange);

	form = Y_FORM;
	activate_special();
}


void MmrnmhrmXForm::calculate()
{
	STACKTRACE;
	if (laserSpark > 0) {
		laserSpark-= frame_time;
		if ((laserSpark <= 0) && (laserSpark > -frame_time)) {
			if ((laser1->damage_factor > 0) && (laser2->damage_factor > 0))
				add(new Animation(this,
					laser1->normal_pos() + laser1->edge(),
					meleedata.sparkSprite, 0, SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
			laser1 = NULL;
			laser2 = NULL;
		}
	}

	Ship::calculate();
}


int MmrnmhrmXForm::activate_weapon()
{
	STACKTRACE;
	if (form == T_FORM) {
		add(laser1 = new Laser(this, angle + laserAngle,
			tw_get_palete_color(laserColor), laserRange, laserDamage, weapon_rate, this,
			Vector2(-28.0, 2.0), true));
		add(laser2 = new Laser(this, angle - laserAngle,
			tw_get_palete_color(laserColor), laserRange, laserDamage, weapon_rate, this,
			Vector2(28.0, 2.0), true));
		laserSpark = weapon_rate;
	}
	else if (form == Y_FORM) {
		add(new HomingMissile(this, Vector2(-13.0, 2.0),
			angle - 25.0 * ANGLE_RATIO, missileVelocity, missileDamage, missileRange,
			missileArmour, missileTurnRate, this, data->spriteWeapon, target));
		add(new HomingMissile(this, Vector2(13.0, 2.0),
			angle + 25.0 * ANGLE_RATIO, missileVelocity, missileDamage, missileRange,
			missileArmour, missileTurnRate, this, data->spriteWeapon, target));
	}
	return(TRUE);
}


int MmrnmhrmXForm::activate_special()
{
	STACKTRACE;
	form_data[form].speed_max       = speed_max;
	form_data[form].accel_rate      = accel_rate;
	form_data[form].turn_rate       = turn_rate;
	form_data[form].recharge_amount = recharge_amount;
	form_data[form].recharge_rate   = recharge_rate;
	form_data[form].weapon_drain    = weapon_drain;
	form_data[form].weapon_rate     = weapon_rate;
	form_data[form].hotspot_rate    = hotspot_rate;
	form_data[form].sprite          = sprite;
	form = ((!form) & 1);
	speed_max       = form_data[form].speed_max;
	accel_rate      = form_data[form].accel_rate;
	turn_rate       = form_data[form].turn_rate;
	recharge_amount = form_data[form].recharge_amount;
	recharge_rate   = form_data[form].recharge_rate;
	weapon_drain    = form_data[form].weapon_drain;
	weapon_rate     = form_data[form].weapon_rate;
	hotspot_rate    = form_data[form].hotspot_rate;
	sprite          = form_data[form].sprite;
	weapon_sample   = form;
	return(TRUE);
}


double MmrnmhrmXForm::handle_speed_loss(SpaceLocation *source, double normal)
{
	STACKTRACE;
	Ship::handle_speed_loss(source, normal);
	if (normal) {
		activate_special();
		Ship::handle_speed_loss(source, normal);
		activate_special();
	}
	return normal;
}


REGISTER_SHIP(MmrnmhrmXForm)
