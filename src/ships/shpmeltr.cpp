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

class MelnormeTrader : public Ship
{
	public:
		double       weaponRange;
		double       weaponRangeUp;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;
		SpaceObject *weaponObject;

		double       specialRange;
		double       specialVelocity;
		int          specialFrames;
		int          specialArmour;

	public:
		MelnormeTrader(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual void calculate();

		virtual int activate_weapon();
		virtual int activate_special();
};

class MelnormeShot : public Shot
{
	double v;
	int frame;
	int frame_step;
	int frame_size;
	int frame_count;
	int charge_frame;
	int charge_phase;
	int released;
	double RangeUp;

	public:
		MelnormeShot(Vector2 opos, double oangle, double ov, int odamage,
			double orange, double rangeup, int oarmour, Ship *oship, SpaceSprite *osprite,
			int ofcount, int ofsize);

		virtual void calculate();
		virtual void animateExplosion();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
};

class MelnormeDisable : public SpaceObject
{
	Ship *ship;
	int   disableframe;
	int   disableframe_count;
	int   frame_step;
	int   frame_size;
	int   frame_count;

	public:
		MelnormeDisable(Ship *creator, Ship *oship, SpaceSprite *osprite, int ofcount,
			int ofsize, int disableFrames);

		virtual void calculate();
};

class MelnormeSpecial : public AnimatedShot
{
	int disableFrames;
	SpaceSprite *disableSprite;

	public:
		MelnormeSpecial(Vector2 opos, double oangle, double ov,
			int oframes, double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, int ofsize, int ofcount);

		void inflict_damage(SpaceObject *other);
		void animateExplosion();
};

MelnormeTrader::MelnormeTrader(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponRangeUp  = scale_range(tw_get_config_float("Weapon", "RangeUp", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponObject   = NULL;

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialFrames   = tw_get_config_int("Special", "Frames", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
}


void MelnormeTrader::calculate()
{
	STACKTRACE;
	Ship::calculate();
	if (weaponObject) {
		if (!weaponObject->exists()) weaponObject = NULL;
		if (fire_weapon) recharge_step += frame_time; else weaponObject = NULL;
	}
}


int MelnormeTrader::activate_weapon()
{
	STACKTRACE;
	if (weaponObject)
		return(FALSE);
	add(weaponObject = new MelnormeShot(
		Vector2(0.0, (get_size().y /*height()*/ / 2.0)), angle, weaponVelocity, weaponDamage, weaponRange, weaponRangeUp,
		weaponArmour, this, data->spriteWeapon, 10, 50));
	return(TRUE);
}


int MelnormeTrader::activate_special()
{
	STACKTRACE;
	add( new MelnormeSpecial(
		Vector2(0.0, get_size().y /*(height()*/ / 2.0), angle, specialVelocity, specialFrames,
		specialRange, specialArmour, this, data->spriteSpecial, 20, 50));
	return(TRUE);
}


MelnormeShot::MelnormeShot(Vector2 opos, double oangle, double ov,
int odamage, double orange, double rangeup, int oarmour, Ship *oship, SpaceSprite *osprite,
int ofcount, int ofsize) :
Shot(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite),
v(ov),
frame(0),
frame_step(0),
frame_size(ofsize),
frame_count(ofcount),
charge_frame(0),
charge_phase(0),
released(FALSE),
RangeUp(rangeup)
{
	STACKTRACE;
	//  vx = ship->get_vx();
	//  vy = ship->get_vy();
	vel = ship->get_vel();
	set_depth(DEPTH_SHIPS+0.5);
}


void MelnormeShot::calculate()
{
	STACKTRACE;
	if (released) {
		Shot::calculate();
	} else {
		SpaceObject::calculate();
	}
	if (!exists()) return;
	frame_step += frame_time;
	if (frame_step >= frame_size) {
		frame_step -= frame_size;
		frame++;
		if (frame == frame_count) {
			frame = 0;
			if ((!released) && (charge_phase < 3)) {
				charge_frame++;
				if (charge_frame == 5) {
					charge_frame = 0;
					charge_phase++;
					damage_factor *= 2;
					armour *= 2;
					range += RangeUp;
					play_sound(data->sampleWeapon[0]);
				}
			}
		}
	}
	sprite_index = (charge_phase * 10) + frame;
	if (!released) {
		if (!ship || !ship->fire_weapon) {
			//			vx = cos(angle) * v;
			//			vy = sin(angle) * v;
			vel = v * unit_vector(angle);
			released = TRUE;
			play_sound(data->sampleSpecial[0]);
		} else {
			angle = ship->get_angle();
			//			x = ship->normal_x() + (cos(angle) * (ship->width() / 2.0));
			//			y = ship->normal_y() + (sin(angle) * (ship->height() / 2.0));
			pos = ship->normal_pos() + 0.5 * product(unit_vector(angle), ship->get_size());
			//			vx = ship->get_vx();
			//			vy = ship->get_vy();
			vel = ship->get_vel();

		}
	}
	return;
}


void MelnormeShot::animateExplosion()
{
	STACKTRACE;
	add( new Animation(this, pos,
		data->spriteWeaponExplosion, (charge_phase * 20), 20, 25,
		DEPTH_EXPLOSIONS));
}


int MelnormeShot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int old = iround(armour);
	Shot::handle_damage(source, normal, direct);
	if (!released && (armour > 0)) armour = old;
	return iround(old - armour);
}


void MelnormeShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->exists()) return;
	damage(other, damage_factor);

	// this can only die on enemy non-shots, if it's not red ...
	if (charge_phase < 3)
		if (other->isblockingweapons) state = 0;

	if (state == 0) {
		animateExplosion();
		soundExplosion();
	}
	return;
}


MelnormeDisable::MelnormeDisable(Ship *creator, Ship *oship, SpaceSprite *osprite,
int ofcount, int ofsize, int disableFrames) :
SpaceObject(creator, oship->normal_pos(), 0.0, osprite),
ship(oship),
disableframe(0),
disableframe_count(disableFrames),
frame_step(0),
frame_size(ofsize),
frame_count(ofcount)
{
	STACKTRACE;
	collide_flag_anyone = 0;
	set_depth(DEPTH_EXPLOSIONS);
}


void MelnormeDisable::calculate()
{
	STACKTRACE;
	frame_step+= frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if (sprite_index == frame_count)
			sprite_index = 0;
	}
	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}
	//	x = ship->normal_x();
	//	y = ship->normal_y();
	pos = ship->normal_pos();
	ship->nextkeys &= ~(keyflag::left | keyflag::right | keyflag::special);
	ship->nextkeys |= keyflag::right;
	disableframe += frame_time;
	if (disableframe >= disableframe_count) state = 0;
	SpaceObject::calculate();
}


MelnormeSpecial::MelnormeSpecial(Vector2 opos, double oangle,
double ov, int oframes, double orange, int oarmour, Ship *oship,
SpaceSprite *osprite, int ofcount, int ofsize) :
AnimatedShot(oship, opos, oangle, ov, 0, orange, oarmour, oship, osprite,
ofcount, ofsize),
disableFrames(oframes),
disableSprite(data->spriteExtra)
{
	STACKTRACE;
	collide_flag_anyone = bit(LAYER_SHIPS);
}


void MelnormeSpecial::animateExplosion()
{
	STACKTRACE;
}


void MelnormeSpecial::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->isShip()) add(new MelnormeDisable( ship,
			(Ship *)(other), disableSprite, 20, 50, disableFrames));
	state = 0;
	return;
}


REGISTER_SHIP(MelnormeTrader)
