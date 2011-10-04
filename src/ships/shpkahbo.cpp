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

#include "melee/mmain.h"

#define SMALL_BOOMERANG 1
#define MEDIUM_BOOMERANG 2
#define LARGE_BOOMERANG 3

class KahrBoomerang;
class KahrSmall;
class KahrMedium;
class KahrLarge;

class KahrBoomerang : public Ship
{

	int          weaponChoice;

	double       weapon1Range;
	double       weapon1Turn;
	double       weapon1Velocity;
	int          weapon1Drain;
	int          weapon1Damage;
	int          weapon1Armour;
	int          weapon1Rate;

	double       weapon2Range;
	double       weapon2Turn;
	double       weapon2Velocity;
	int          weapon2Drain;
	int          weapon2Damage;
	int          weapon2Armour;
	int          weapon2Rate;

	double       weapon3Range;
	double       weapon3Turn;
	double       weapon3Velocity;
	int          weapon3Drain;
	int          weapon3Damage;
	int          weapon3Armour;
	int          weapon3Rate;

	KahrLarge    *boomerangL;

	public:

		int                 num_medium_boomerangs;

		KahrBoomerang(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void calculate_fire_special();
};

class KahrSmall : public Shot
{
	public:
		KahrSmall(double ox,double oy,double oangle, double ov, double oturn,
			int odamage, double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, int ofcount, int ofsize);

		int frame;
		int frame_step;
		int frame_size;
		int frame_count;
		double maxspeed;
		double srange;
		int returning;
		double turning;
		int turned;

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

class KahrMedium : public Shot
{
	public:
		KahrMedium(double ox,double oy,double oangle, double ov, double oturn,
			int odamage, double orange, int oarmour, KahrBoomerang *oship,
			SpaceSprite *osprite, int ofcount, int ofsize);
		virtual void death();

		int frame;
		int frame_step;
		int frame_size;
		int frame_count;
		double maxspeed;
		double srange;
		int returning;
		double turning;
		int turned;

		KahrBoomerang     *kahrship;

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

class KahrLarge : public Shot
{
	public:
		KahrLarge(double ox,double oy,double oangle, double ov, double oturn,
			int odamage, double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, int ofcount, int ofsize);

		int frame;
		int frame_step;
		int frame_size;
		int frame_count;
		double srange;
		int returning;
		double turning;
		double maxspeed;
		int turned;

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

KahrBoomerang::KahrBoomerang(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weapon1Range    = scale_range(tw_get_config_float("WeaponS", "Range", 0));
	weapon1Turn     = scale_turning(tw_get_config_float("WeaponS","Turn", 0));
	weapon1Velocity = scale_velocity(tw_get_config_float("WeaponS", "Velocity", 0));
	weapon1Drain    = tw_get_config_int("WeaponS", "Drain", 0);
	weapon1Damage   = tw_get_config_int("WeaponS", "Damage", 0);
	weapon1Armour   = tw_get_config_int("WeaponS", "Armour", 0);
	weapon1Rate     = scale_frames(tw_get_config_float("WeaponS", "Rate", 0));

	weapon2Range    = scale_range(tw_get_config_float("WeaponM", "Range", 0));
	weapon2Turn     = scale_turning(tw_get_config_float("WeaponM","Turn", 0));
	weapon2Velocity = scale_velocity(tw_get_config_float("WeaponM", "Velocity", 0));
	weapon2Drain    = tw_get_config_int("WeaponM", "Drain", 0);
	weapon2Damage   = tw_get_config_int("WeaponM", "Damage", 0);
	weapon2Armour   = tw_get_config_int("WeaponM", "Armour", 0);
	weapon2Rate     = scale_frames(tw_get_config_float("WeaponM", "Rate", 0));

	weapon3Range    = scale_range(tw_get_config_float("WeaponL", "Range", 0));
	weapon3Turn     = scale_turning(tw_get_config_float("WeaponL","Turn", 0));
	weapon3Velocity = scale_velocity(tw_get_config_float("WeaponL", "Velocity", 0));
	weapon3Drain    = tw_get_config_int("WeaponL", "Drain", 0);
	weapon3Damage   = tw_get_config_int("WeaponL", "Damage", 0);
	weapon3Armour   = tw_get_config_int("WeaponL", "Armour", 0);
	weapon3Rate     = scale_frames(tw_get_config_float("WeaponL", "Rate", 0));

	weapon_rate    = weapon1Rate;
	weapon_drain   = weapon1Drain;
	weaponChoice   = SMALL_BOOMERANG;
	num_medium_boomerangs = 0;
	boomerangL    = NULL;
}


int KahrBoomerang::activate_weapon()
{
	STACKTRACE;
	int chance;
	int answer = FALSE;
	if (weaponChoice == SMALL_BOOMERANG) {
		add(new KahrSmall( 0.0, 0.0 , angle,
			weapon1Velocity, weapon1Turn, weapon1Damage, weapon1Range,
			weapon1Armour, this, data->spriteWeapon, 64, 3));
		weapon1Turn = (weapon1Turn * -1.0);
		answer = TRUE;
	}
	if (weaponChoice == MEDIUM_BOOMERANG) {
		if (num_medium_boomerangs < 4) {
			add(new KahrMedium( 0.0, 0.0, angle, weapon2Velocity,
				(weapon2Turn*-1.0), weapon2Damage, weapon2Range, weapon2Armour,
				this, data->spriteSpecial, 32, 4));
			num_medium_boomerangs += 1;
			answer = TRUE;
		} else answer = FALSE;
	}
	if (weaponChoice == LARGE_BOOMERANG) {
		chance = random() % 2;
		if (chance) weapon3Turn = weapon3Turn * -1;
		if (boomerangL == NULL) {
			boomerangL = new KahrLarge(0.0, 0.0, angle, weapon3Velocity,
				weapon3Turn, weapon3Damage, weapon3Range, weapon3Armour,
				this, data->spriteExtra, 64, 3);
			add(boomerangL);
			answer = TRUE;
		} else answer = FALSE;
	}
	return answer;
}


int KahrBoomerang::activate_special()
{
	STACKTRACE;
	int answer;
	if (weaponChoice == SMALL_BOOMERANG) {
		weapon_drain = weapon2Drain;
		weapon_rate = weapon2Rate;
		weaponChoice = MEDIUM_BOOMERANG;
		answer = TRUE;
	}
	else if (weaponChoice == MEDIUM_BOOMERANG) {
		weapon_drain = weapon3Drain;
		weapon_rate = weapon3Rate;
		weaponChoice = LARGE_BOOMERANG;
		answer = TRUE;
	}
	else if (weaponChoice == LARGE_BOOMERANG) {
		weapon_drain = weapon1Drain;
		weapon_rate = weapon1Rate;
		weaponChoice = SMALL_BOOMERANG;
		answer = TRUE;
	} else answer = FALSE;
	return (answer);
}


void KahrBoomerang::calculate()
{
	STACKTRACE;
	if ((boomerangL != NULL) && (!boomerangL->exists()))
		boomerangL=NULL;
	Ship::calculate();
	if ((!fire_weapon) && ((boomerangL != NULL))
	&& (!(boomerangL->returning))) {
		boomerangL->returning = TRUE;
		if ((turn_left) && (boomerangL->turning > 0.0))
			boomerangL->turning = ((boomerangL->turning) * -1.0);
		else if ((turn_right) && (boomerangL->turning < 0.0))
			boomerangL->turning = ((boomerangL->turning) * -1.0);
	}
}


void KahrBoomerang::calculate_fire_special()
{
	STACKTRACE;
	if (weaponChoice == SMALL_BOOMERANG) {
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(1), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(2), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(3), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(4), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(5), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(6), 36, 0, 36, 0, 19, 30);
	}
	else if (weaponChoice == MEDIUM_BOOMERANG) {
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(1), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(2), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(3), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(4), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(5), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(6), 36, 0, 36, 0, 19, 30);
	}
	else if (weaponChoice == LARGE_BOOMERANG) {
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(1), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(2), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(3), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(4), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(5), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(6), 36, 0, 36, 0, 19, 30);
	}
	Ship::calculate_fire_special();
}


KahrSmall::KahrSmall(double ox,double oy,double oangle, double ov,
double oturn, int odamage, double orange, int oarmour, Ship *oship,
SpaceSprite *osprite, int ofcount, int ofsize) :
Shot(oship, Vector2(ox,oy), oangle, ov, odamage, -1.0 , oarmour, oship, osprite)

{
	STACKTRACE;
	mass = 0.25;
	frame = 0;
	frame_step = 0;
	frame_size = ofsize;
	frame_count = ofcount;
	maxspeed = ov;
	srange = orange;
	returning = FALSE;
	turning = oturn;
	turned = FALSE;
}


void KahrSmall::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		state = 0;
		ship = 0;
		return;
	}
	if (distance(ship) > srange)
		returning = TRUE;
	if (returning) {
		collide_flag_sameship |= bit(LAYER_SHIPS);
		if (normalize(normalize(trajectory_angle(ship), PI2) - normalize(angle,PI2),PI2) > (15 * ANGLE_RATIO))
			angle += turning * frame_time;
		else turned = TRUE;
		if (turned) angle = trajectory_angle(ship);
		double v, alpha;
		alpha = atan(vel);
		alpha = alpha + PI;
		alpha = normalize(alpha, PI2);

		v = maxspeed;

		if ((fabs(min_delta(alpha, angle, PI2)) > 0.5))
			vel = unit_vector(angle) * v;
	}
	Shot::calculate();
	frame_step += frame_time;
	if (frame_step >= frame_size) {
		frame_step -= frame_size;
		if (turning > 0) frame--;
		else frame++;
		if (frame == frame_count)
			frame = 0;
		if (frame == -1)
			frame = (frame_count - 1);
		sprite_index=frame;
	}
}


void KahrSmall::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other == ship) {
		state = 0;
		mass = 0;
	}
	else if (other->isShip()) {
		angle += PI;
		returning = TRUE;
		turned = FALSE;
		SpaceObject::inflict_damage(other);
	} else Shot::inflict_damage(other);
}


KahrMedium::KahrMedium(double ox,double oy,double oangle, double ov,
double oturn, int odamage, double orange, int oarmour, KahrBoomerang *oship,
SpaceSprite *osprite, int ofcount, int ofsize) :
Shot(oship, Vector2(ox,oy), oangle, ov, odamage, -1.0 , oarmour, oship, osprite)

{
	STACKTRACE;
	mass = 3.0;
	frame = 0;
	frame_step = 0;
	frame_size = ofsize;
	frame_count = ofcount;
	maxspeed = ov;
	srange = orange;
	returning = FALSE;
	turning = oturn;
	turned = FALSE;

	kahrship = oship;
}


void KahrMedium::death()
{
	STACKTRACE;
	if (kahrship) (kahrship)->num_medium_boomerangs -= 1;
	Shot::death();
}


void KahrMedium::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		state = 0;
		ship = 0;
		return;
	}
	if (!(kahrship && kahrship->exists()))
		kahrship = 0;

	if (distance(ship) > srange)
		returning = TRUE;
	if (returning) {
		collide_flag_sameship |= bit(LAYER_SHIPS);
		if (normalize(normalize(trajectory_angle(ship), PI2) - normalize(angle,PI2),PI2) > (15*ANGLE_RATIO))
			angle += turning * frame_time;
		else turned = TRUE;
		if (turned) angle = trajectory_angle(ship);
		double v, alpha;
		alpha = atan(vel);
		alpha = alpha + PI;
		alpha = normalize(alpha, PI2);

		v = maxspeed;

		if ((fabs(min_delta(alpha, angle, PI2)) > 0.5))
			vel = unit_vector(angle) * v;
	}
	Shot::calculate();
	frame_step += frame_time;
	if (frame_step >= frame_size) {
		frame_step -= frame_size;
		if (turning > 0) frame--;
		else frame++;
		if (frame == frame_count)
			frame = 0;
		if (frame == -1)
			frame = (frame_count - 1);
		sprite_index=frame;
	}
}


void KahrMedium::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other == ship) {
		state = 0;
		mass = 0;
	}
	else if (other->isShip()) {
		angle += PI;
		returning = TRUE;
		turned = FALSE;
		SpaceObject::inflict_damage(other);
	} else Shot::inflict_damage(other);
}


KahrLarge::KahrLarge(double ox,double oy,double oangle, double ov,
double oturn, int odamage, double orange, int oarmour, Ship *oship,
SpaceSprite *osprite, int ofcount, int ofsize) :
Shot(oship, Vector2(ox,oy), oangle, ov, odamage, -1.0 , oarmour, oship, osprite)

{
	STACKTRACE;
	mass =5.0;
	frame = 0;
	frame_step = 0;
	frame_size = ofsize;
	frame_count = ofcount;
	maxspeed = ov;
	srange = orange;
	returning = FALSE;
	turning = oturn;
	turned = FALSE;
}


void KahrLarge::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		state = 0;
		ship = 0;
		return;
	}
	if (returning) {
		collide_flag_sameship |= bit(LAYER_SHIPS);
		if (normalize(normalize(trajectory_angle(ship), PI2) - normalize(angle,PI2),PI2) > (15 * ANGLE_RATIO))
			angle += turning * frame_time;
		else turned = TRUE;
		if (turned) angle = trajectory_angle(ship);
		double v, alpha;
		alpha = atan(vel);
		alpha = alpha + PI;
		alpha = normalize(alpha, PI2);

		v = maxspeed;

		if ((fabs(min_delta(alpha, angle, PI2)) > 0.5 * ANGLE_RATIO))
			vel = unit_vector(angle) * v;
	}
	Shot::calculate();
	frame_step += frame_time;
	if (frame_step >= frame_size) {
		frame_step -= frame_size;
		if (turning > 0) frame--;
		else frame++;
		if (frame == frame_count)
			frame = 0;
		if (frame == -1)
			frame = (frame_count - 1);
		sprite_index=frame;
	}
}


void KahrLarge::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other == ship) {
		state = 0;
		mass = 0;
	}
	else if (other->mass > 0) {
		angle += PI;
		returning = TRUE;
		turned = FALSE;
		SpaceObject::inflict_damage(other);
	} else Shot::inflict_damage(other);
}


int KahrLarge::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	return Shot::handle_damage(source, 0, 0);
}


REGISTER_SHIP(KahrBoomerang)
