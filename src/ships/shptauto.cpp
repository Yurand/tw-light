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
#include "melee/mview.h"

REGISTER_FILE

class TauTor : public Ship
{
	double        weaponRange;
	double        weaponVelocity;
	int           weaponDamage;
	int           weaponArmour;
	double        specialRange;
	double        specialVelocity;
	int           specialDamage;
	int           specialArmour;
	bool          TauTor_state;
	bool          can_switch;
	double        accel_boost, speed_boost;
	int           extra_drain, extra_rate;
	int           switch_drain;
	int           extra_recharge;
	double        engine_drain, residual_drain;
	int           transition_time, transition_count;

	public:

		TauTor     (Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual  int  activate_weapon();
		virtual void  calculate_fire_special();
		virtual void  calculate();
		virtual void  calculate_thrust();
		virtual void  calculate_hotspots();
};

class TauTorTwister : public AnimatedShot
{
	// SAMPLE* sample_midd;

	public:

		TauTorTwister(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, SpaceLocation *creator, SpaceLocation *lpos,
			SpaceSprite *osprite, int ofcount, int ofsize, double relativity = 0);

};

class TauTorPlasma : public AnimatedShot
{
	public:
		TauTorPlasma (Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, SpaceLocation *creator, SpaceLocation *lpos,
			SpaceSprite *osprite, int ofcount, int ofsize, double relativity = 0);

};

TauTor::TauTor(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)

{
	STACKTRACE;
	weaponRange     = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity  = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage    = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour    = tw_get_config_int("Weapon", "Armour", 0);

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);

	switch_drain    = tw_get_config_int("Extra", "StartDrain", 1);
	engine_drain    = tw_get_config_float("Extra", "EngineDrain", 0)/1000.0;
	accel_boost     = tw_get_config_float("Extra", "AccelBoost", 1.0);
	speed_boost     = tw_get_config_float("Extra", "SpeedBoost", 1.0);
	transition_time = tw_get_config_int("Extra", "TransitionTime", 1);
	transition_count= 0;

	TauTor_state  = false;
	can_switch      = true;

	extra_drain     = weapon_drain;
	extra_rate      = weapon_rate;
	extra_recharge  = recharge_amount;

	residual_drain  = 0.0;
}


int TauTor::activate_weapon()
{
	STACKTRACE;
	if (transition_count > 0) return false;

	if (TauTor_state) {
		add(new TauTorPlasma(Vector2(14.0, 24.0), angle,
			vel.magnitude() + specialVelocity, specialDamage,
			specialRange, specialArmour, this, this, data->spriteSpecial, 6, 50));
		add(new TauTorPlasma(Vector2(-14.0, 24.0), angle,
			vel.magnitude() + specialVelocity, specialDamage,
			specialRange, specialArmour, this, this, data->spriteSpecial, 6, 50));
		weapon_sample = 0;
	} else {
		add(new TauTorTwister(Vector2(0.0, 24.0), angle, weaponVelocity, weaponDamage,
			weaponRange, weaponArmour, this, this, data->spriteWeapon, 6, 50));
		weapon_sample = 1;
	}
	return(TRUE);
}


void TauTor::calculate_fire_special()
{
	STACKTRACE;
	special_low = false;
	if (fire_special && can_switch && (transition_count <=0)) {
		can_switch = false;
		if (TauTor_state) {
			play_sound(data->sampleExtra[1]);
			residual_drain = 0.0;
			recharge_amount = extra_recharge;
			weapon_rate  = extra_rate;
			weapon_drain = extra_drain;
			sprite = data->spriteShip;
			TauTor_state = false;
		}
		else    if (batt > switch_drain) {
			transition_count = transition_time;
			residual_drain = 0.0;
			recharge_amount = 0;
			play_sound(data->sampleExtra[0]);
			weapon_rate  = special_rate;
			weapon_drain = special_drain;
		}
		else    special_low = true;
	}
}


void TauTor::calculate()
{
	STACKTRACE;

	if (transition_count > 0) {
		if ((transition_count -= frame_time) <= 0) {
			sprite = data->spriteExtraExplosion;
			TauTor_state = true;
		}
		residual_drain += switch_drain * frame_time / (double)transition_time;
	}
	int a = (int)floor(residual_drain);
	batt -= a;
	residual_drain -= a;
	if (batt < 0) batt = 0;

	if (TauTor_state && (batt == 0)) {
		play_sound(data->sampleExtra[1]);
		residual_drain = 0.0;
		recharge_amount = extra_recharge;
		weapon_rate  = extra_rate;
		weapon_drain = extra_drain;
		sprite = data->spriteShip;
		TauTor_state = false;
	}

	Ship::calculate();
	if (!fire_special) can_switch = true;
}


void TauTor::calculate_thrust()
{
	STACKTRACE;

	if (thrust && (transition_count <=0)) {
		if (TauTor_state && (batt > 0)) {
			residual_drain += engine_drain * frame_time;
			accelerate(this, angle, accel_rate * frame_time * accel_boost, speed_max * speed_boost);
		}
		else    accelerate(this, angle, accel_rate * frame_time, speed_max);
	}
}


void TauTor::calculate_hotspots ()
{
	if ((thrust) && (hotspot_frame <= 0) && (transition_count <=0)) {
		if (TauTor_state && (batt > 0))
			add(new Animation(this,
			//normal_x() - (cos(angle * ANGLE_RATIO) * 21),
			//normal_y() - (sin(angle * ANGLE_RATIO) * 21),
				normal_pos() - 21 * unit_vector(angle),
				data->spriteExtra, 0, 12, time_ratio, LAYER_HOTSPOTS));
		else    add(new Animation(this,
			//normal_x() - (cos(angle * ANGLE_RATIO) * 21),
			//normal_y() - (sin(angle * ANGLE_RATIO) * 21),
				normal_pos() - 21 * unit_vector(angle),
				meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, LAYER_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0) hotspot_frame -= frame_time;
	return;
}


TauTorTwister::TauTorTwister(Vector2 opos, double oangle, double ov, int odamage,
double orange, int oarmour, SpaceLocation *creator, SpaceLocation *lpos,
SpaceSprite *osprite, int ofcount, int ofsize, double relativity) :
AnimatedShot(creator, opos, oangle, ov, odamage, orange, oarmour,
lpos, osprite, ofcount, ofsize, relativity)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 12;
	explosionFrameSize  = 50;
	//sample_midd         = data->sampleSpecial[0];
}


TauTorPlasma::TauTorPlasma(Vector2 opos, double oangle, double ov, int odamage,
double orange, int oarmour, SpaceLocation *creator, SpaceLocation *lpos,
SpaceSprite *osprite, int ofcount, int ofsize, double relativity) :
AnimatedShot(creator, opos, oangle, ov, odamage, orange, oarmour,
lpos, osprite, ofcount, ofsize, relativity)
{
	STACKTRACE;
	explosionSprite     = data->spriteSpecialExplosion;
	explosionFrameCount = 12;
	explosionFrameSize  = 50;

}


REGISTER_SHIP(TauTor)
