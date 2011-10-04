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

//ForevianShocker lousy code by Richardyzo@ig.com.br

#include "ship.h"
REGISTER_FILE

class ForevianShocker : public Ship
{
	double      weaponRange, weaponVelocity;
	int         weaponDamage, weaponArmour;
	int         weapon_type;
	int         weapon_frame_size;

	double      specialRange, specialVelocity;
	int         specialDamage, specialArmour;
	int         specialFrames;
	int         shockingFrames;
	int         shockVar;
	int         specialLoss;

	bool        new_special;
	int         special_frame_size, special_frame_num;

	SpaceSprite *special_sprite;

	public:
		ForevianShocker(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
};

class ForevianShockwave : public AnimatedShot
{
	SpaceObject *last_hit;  int loss;

	public:
		ForevianShockwave(double ox, double oy, double oangle, double ov, int odamage, double orange, int oarmour, Ship *oship, SpaceLocation *opos, SpaceSprite *osprite, int ofcount, int ofsize, int ol);
		virtual void inflict_damage(SpaceObject *other);
};

ForevianShocker::ForevianShocker(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weaponRange     = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity  = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage    = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour    = tw_get_config_int("Weapon", "Armour", 0);
	weapon_type     = tw_get_config_int("Weapon", "Type", 0);

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	specialFrames   = tw_get_config_int("Special", "Frames", 0);

	specialLoss     = tw_get_config_int("Special", "Loss", 0);

	shockingFrames  = 0;
	shockVar        = 0;

	new_special     = (tw_get_config_int("Special", "NewGFX", 0) != 0);
	weapon_frame_size   = iround(weaponRange/weaponVelocity/32);

	if (new_special) {
		special_sprite= data->spriteExtraExplosion;
		special_frame_size = iround(specialRange/specialVelocity/32);
		special_frame_num = 32;
	} else {
		special_sprite = data->spriteSpecial;
		special_frame_size = 50;
		special_frame_num = 2;
	}
}


int ForevianShocker::activate_weapon()
{
	STACKTRACE;

	SpaceSprite *tmp_s = data->spriteWeapon;
	if (weapon_type==2) tmp_s = data->spriteExtra;
	play_sound2(data->sampleWeapon[0]);
	if (weapon_type == 0) {
		if (random()%2)
			tmp_s = data->spriteExtra;
		add(new AnimatedShot(this, Vector2(27.0,20.0), angle , weaponVelocity,  weaponDamage, weaponRange, weaponArmour, this, tmp_s, 32, weapon_frame_size, 1.0));
		if (weapon_type == 0) {
			if (random()%2)  tmp_s = data->spriteExtra;
			else    tmp_s = data->spriteWeapon;
		}
	}
	add(new AnimatedShot(this, Vector2(-27.0,20.0), angle , weaponVelocity, weaponDamage, weaponRange, weaponArmour, this, tmp_s, 32, weapon_frame_size, 1.0)); return(TRUE);
}


int ForevianShocker::activate_special()
{
	STACKTRACE;
	if (shockingFrames == 0) {
		play_sound2(data->sampleSpecial[0]);
		shockingFrames = specialFrames;
		shockVar = 1;
		recharge_amount = 0;
	}
	return true;
}


void ForevianShocker::calculate()
{
	STACKTRACE;
	if (shockVar == 1) {
		if (shockingFrames > 0)
			shockingFrames -= frame_time;

		//if (shockVar && shockingFrames <= 2500 )
		if (shockVar && shockingFrames == 2500 ) {
			add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
		}
		if (shockingFrames == 2000 ) {
			play_sound2(data->sampleSpecial[0]);
			add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
		}
		if (shockingFrames == 1500 ) {
			play_sound2(data->sampleSpecial[0]);
			add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
		}
		if (shockingFrames == 1000 ) {
			play_sound2(data->sampleSpecial[0]);
			add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
		}
		if (shockingFrames == 500 ) {
			play_sound2(data->sampleSpecial[0]);
			add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
		}
		if (shockingFrames == 50 ) {
			play_sound2(data->sampleSpecial[1]);

			add( new ForevianShockwave(0, 70, angle , specialVelocity, specialDamage, specialRange, specialArmour, this, this, special_sprite, special_frame_num, special_frame_size, specialLoss));

			shockingFrames = 0;
			shockVar = 0;
			recharge_amount = 1;
		}
	}

	Ship::calculate();
}


ForevianShockwave::ForevianShockwave(double ox, double oy, double oangle, double ov, int odamage, double orange, int oarmour, Ship *oship, SpaceLocation *opos, SpaceSprite *osprite, int ofcount, int ofsize, int ol) :
AnimatedShot(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship, osprite, ofcount, ofsize)
{
	STACKTRACE;
	collide_flag_anyone = OBJECT_LAYERS&~bit(LAYER_SHOTS);
	collide_flag_sameship = collide_flag_anyone;
	last_hit = oship;
	loss = ol;
}


void ForevianShockwave::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other==last_hit)
		return;
	play_sound2(data->sampleSpecial[2]);
	if (!other->sameShip(this))
		damage(other, 0, damage_factor);
	add( new Animation(this, pos, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
	if ((damage_factor-=loss) <= 0) {
		die(); return;
	}

	last_hit=other;

	Query q;
	double r0=1e40;
	SpaceObject *tgt = NULL;
	for (q.begin(this, OBJECT_LAYERS&~bit(LAYER_SHOTS), range-d); q.currento; q.next()) {
		if ((q.currento==last_hit) || q.currento->isPlanet() || !(q.currento->canCollide(this)&&canCollide(q.currento)) ) continue; if (distance(q.currento)<r0) {
			tgt = q.currento;
			r0 = distance(tgt);
		}
	}
	q.end();

	if (tgt)
		changeDirection(trajectory_angle(tgt));

	return;
}


REGISTER_SHIP(ForevianShocker)
