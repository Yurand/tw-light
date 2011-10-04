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

class RekojAssassin : public Ship
{
	double weaponRange;
	int    weaponDamage;
	double weaponVelocity;
	int    weaponArmour;

	bool   specialMatchSpeed;

	int    extraColor;
	double extraRange;
	int    extraDamage;
	int    extraAngle;
	int    extraDelay;
	int    extraBlip;
	int    extraArcLimit;

	int      normal_recharge;
	//	int    normal_rate;

	public:
		RekojAssassin(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
};

RekojAssassin::RekojAssassin(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)

{
	STACKTRACE;
	weaponRange           = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponDamage      = tw_get_config_int("Weapon", "Damage", 0);
	weaponVelocity  = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponArmour      = tw_get_config_int("Weapon", "Armour", 0);

	specialMatchSpeed = false;

	extraColor            = tw_get_config_int("Extra", "Color", 0);
	extraRange            = scale_range(tw_get_config_float("Extra", "Range", 0));
	extraDamage           = tw_get_config_int("Extra", "Damage", 0);
	extraDelay          = tw_get_config_int("Extra", "Delay", 0);
	extraArcLimit       = tw_get_config_int("Extra", "ArcLimit", 0);
	extraBlip               = 0;

	normal_recharge = recharge_amount;
	//	normal_rate			= recharge_rate;

}


int RekojAssassin::activate_weapon()
{
	STACKTRACE;
								 // sqrt(vx*vx + vy*vy);
	double shipVelocity = magnitude(vel);

	add(new Missile(this, Vector2(0.0, get_size().y / 2.0),
		angle, fabs(shipVelocity)+weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));

	return(TRUE);
}


int RekojAssassin::activate_special()
{
	STACKTRACE;
	specialMatchSpeed = !specialMatchSpeed;
	return(TRUE);
}


void RekojAssassin::calculate()
{
	STACKTRACE;

	Ship::calculate();

	if (target && target->exists()) {

		if (distance(target) > weaponRange)
			specialMatchSpeed = false;

		if (specialMatchSpeed) {
			nextkeys &= ~(keyflag::thrust);
			recharge_step=0;
			//recharge_rate=0;//Bad!
			recharge_amount=0;

			//				vx = target->get_vx();
			//				vy = target->get_vy();
			vel = target->get_vel();
		} else specialMatchSpeed = false;
	}
	else specialMatchSpeed = false;

	if (batt==0)  specialMatchSpeed = false;

	if (!specialMatchSpeed) {
		//recharge_rate=normal_rate;
		recharge_amount=normal_recharge;
	}

	//Laser finder
	if (target && target->exists())
	if (target->isInvisible()) {
		extraBlip++;
		if (extraBlip==extraDelay) {
			double angleDiff;
			double blipAngle;

								 //reference is angle of ship
			angleDiff = trajectory_angle(target) - angle;

			if (angleDiff < 0) angleDiff += PI2;

			if (angleDiff > PI && angleDiff < (PI2 - extraArcLimit)) blipAngle = angle-extraArcLimit;
			else if (angleDiff > extraArcLimit && angleDiff < PI) blipAngle = angle+extraArcLimit;
			else blipAngle = trajectory_angle(target);

			blipAngle = normalize(blipAngle,PI2);

			extraBlip=0;
			add(new Laser(this, blipAngle,
				tw_get_palete_color(extraColor), extraRange, extraDamage, weapon_rate,
				this, Vector2(get_size().x/11, get_size().y/2.07), true));
		}
	}

}


REGISTER_SHIP(RekojAssassin)
