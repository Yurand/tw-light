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

#include "frame.h"

REGISTER_FILE

class BahaoidBabyBuzzsaw;

//#define MAX_NUM_MINES 2

class BahaoidBuzzsaw : public Ship
{
	double  weaponRange;
	double  weaponSpeed;
	int     weaponDamage;
	int     weaponColor;

	double  specialRange;
	int     specialArmour, specialNumber;

	int     can_turn;

	public:
		BahaoidBuzzsaw(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag);

		int     nummines;

	protected:
		virtual void calculate();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual int activate_weapon();
		virtual int activate_special();
};

class BahaoidBabyBuzzsaw : public AnimatedShot
{
	double  MineRange;

	double  weaponRange;
	double  weaponSpeed;
	int     weaponDamage;
	int     weaponColor;
	int     weaponRate, weapon_time;
	int     fully_dropped;

	double  turn_step;

	public:
		BahaoidBabyBuzzsaw(Vector2 opos, double ov, double oangle, int odamage,
			int oarmour, Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize,
			double miner, double rangey, double speedy, int damagey, int colory, int ratey);

		virtual void calculate();
		virtual int handle_damage(SpaceLocation* source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
};

BahaoidBuzzsaw::BahaoidBuzzsaw(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag) :
Ship(opos, shipAngle, shipData, shipCollideFlag)
{
	STACKTRACE;

	weaponRange     = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponSpeed     = tw_get_config_float("Weapon", "Speed", 1) * ANGLE_RATIO;
	weaponDamage    = tw_get_config_int("Weapon", "Damage", 0);
	weaponColor     = tw_get_config_int("Weapon", "Color", 2);

	can_turn        = TRUE;

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	nummines        = 0;
	specialNumber   = tw_get_config_int("Special", "Number", 0);
}


void BahaoidBuzzsaw::calculate_turn_left()
{
	STACKTRACE;
	if (can_turn) {
		Ship::calculate_turn_left();
	}
}


void BahaoidBuzzsaw::calculate_turn_right()
{
	STACKTRACE;
	if (can_turn) {
		Ship::calculate_turn_right();
	}
}


int BahaoidBuzzsaw::activate_weapon()
{
	STACKTRACE;
	double xone = 0.8660254038;
	double yone = 0.5;

	int w, h;
	w = iround(size.x);
	h = iround(size.y);

	double da;
	da = 60.0 * ANGLE_RATIO;

	game->add(new Laser(this, angle + da, tw_get_palete_color(weaponColor),
		weaponRange, weaponDamage, weapon_rate, this, Vector2(0.0, (h / 2.0))));

	game->add(new Laser(this, angle + 2*da, tw_get_palete_color(weaponColor),
		weaponRange, weaponDamage, weapon_rate, this, Vector2((w / 2.0)*xone, (h / 2.0)*yone)));

	game->add(new Laser(this, angle + 3*da, tw_get_palete_color(weaponColor),
		weaponRange, weaponDamage, weapon_rate, this, Vector2((w / 2.0)*xone, -(h / 2.0)*yone)));

	game->add(new Laser(this, angle + 4*da, tw_get_palete_color(weaponColor),
		weaponRange, weaponDamage, weapon_rate, this, Vector2(0.0, -(h / 2.0))));

	game->add(new Laser(this, angle + 5*da, tw_get_palete_color(weaponColor),
		weaponRange, weaponDamage, weapon_rate, this, Vector2(-(w / 2.0)*xone, -(h / 2.0)*yone)));

	game->add(new Laser(this, angle + 6*da, tw_get_palete_color(weaponColor),
		weaponRange, weaponDamage, weapon_rate, this, Vector2(-(w / 2.0)*xone, (h / 2.0)*yone)));

	return(TRUE);
}


int BahaoidBuzzsaw::activate_special()
{
	STACKTRACE;
	if (nummines>=specialNumber) return(FALSE);

	game->add(new BahaoidBabyBuzzsaw(0, 0, angle, 0, specialArmour, this,
		data->spriteSpecial, 1, 30, specialRange, weaponRange, weaponSpeed, weaponDamage, weaponColor, weapon_rate));

	nummines++;
	return TRUE;
}


void BahaoidBuzzsaw::calculate()
{
	STACKTRACE;

	if (fire_weapon) {
		can_turn = FALSE;
		turn_step += weaponSpeed * frame_time;
	} else {
		can_turn = TRUE;
	}

	Ship::calculate();
}


BahaoidBabyBuzzsaw::BahaoidBabyBuzzsaw(Vector2 opos, double ov, double oangle, int odamage, int oarmour,
Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize, double miner,
double rangey, double speedy, int damagey, int colory, int ratey):
AnimatedShot(oship, opos, oangle, ov, odamage, -1.0, oarmour, oship,
osprite, ofcount, ofsize)
{
	STACKTRACE;
	weaponRange     = rangey;
	weaponSpeed     = speedy;
	weaponDamage    = damagey;
	weaponColor     = colory;
	weaponRate  = ratey;
	weapon_time = 0;

	MineRange   = miner;

	turn_step   = 0.0;
	fully_dropped = FALSE;
}


int BahaoidBabyBuzzsaw::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	armour -= normal + direct;

	if (armour <= 0) {
		state = 0;
	}

	return 0;
}


void BahaoidBabyBuzzsaw::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if ((ship) && (other == ship)) {
		state = 0;
		//		sound.play(ship->data->sampleExtra[1], 255, 128, 1000);

		BahaoidBuzzsaw *temp = (BahaoidBuzzsaw*)ship;
		temp->nummines--;
		ship->handle_damage(this, damage_factor);
	}
}


void BahaoidBabyBuzzsaw::calculate()
{
	STACKTRACE;
	SpaceObject *o, *t = NULL;
	double oldrange = 999999;

	Query b;

	if (!fully_dropped) {
		int bahooodle = collide_flag_sameship;

		collide_flag_sameship = bit(SPACE_LAYERS) - 1;

		fully_dropped = TRUE;

		for (b.begin(this, bit(LAYER_SHIPS),scale_range(1.0)); b.current; b.next()) {
			collide_flag_sameship = bahooodle;
			fully_dropped = FALSE;
		}
	}

	AnimatedShot::calculate();

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_CBODIES) + bit(LAYER_SPECIAL),
	MineRange); a.current; a.next()) {
		o = a.currento;
		if (!o->sameTeam(this) && !o->isInvisible() && (distance(o) < oldrange) && (o->ally_flag != (unsigned int)-1) || (o->getID() == ID_ASTEROID)) {
			t = o;
			oldrange = distance(o);
		}
	}

	if (t)
		weapon_time += frame_time;

	if (t && weapon_time > weaponRate) {
		weapon_time -= weaponRate;

		double xone = 0.8660254038;
		double yone = 0.5;

		int w, h;
		w = iround(size.x);
		h = iround(size.y);

		double da;
		da = 60.0 * ANGLE_RATIO;

		game->add(new Laser(this, angle + da, tw_get_palete_color(weaponColor),
			weaponRange, weaponDamage, weaponRate,  this, Vector2(0.0, (h / 2.0))));

		game->add(new Laser(this, angle + 2*da, tw_get_palete_color(weaponColor),
			weaponRange, weaponDamage, weaponRate,  this, Vector2((w / 2.0)*xone, (h / 2.0)*yone)));

		game->add(new Laser(this, angle + 3*da, tw_get_palete_color(weaponColor),
			weaponRange, weaponDamage, weaponRate,  this, Vector2((w / 2.0)*xone, -(h / 2.0)*yone)));

		game->add(new Laser(this, angle + 4*da, tw_get_palete_color(weaponColor),
			weaponRange, weaponDamage, weaponRate,  this, Vector2(0.0, -(h / 2.0))));

		game->add(new Laser(this, angle + 5*da, tw_get_palete_color(weaponColor),
			weaponRange, weaponDamage, weaponRate,  this, Vector2(-(w / 2.0)*xone, -(h / 2.0)*yone)));

		game->add(new Laser(this, angle + 6*da, tw_get_palete_color(weaponColor),
			weaponRange, weaponDamage, weaponRate,  this, Vector2(-(w / 2.0)*xone, (h / 2.0)*yone)));

		//turn_step += weaponSpeed * frame_time;
		angle += weaponSpeed * frame_time;
		while (angle > PI)
			angle -= PI2;
	}

	/*
	while(fabs(turn_step) > 5.625 * ANGLE_RATIO)
	{
		if (turn_step < 0.0)
		{
			angle -= 5.625 * ANGLE_RATIO;
			turn_step += 5.625 * ANGLE_RATIO;
		}
		else if (turn_step > 0.0)
		{
			angle += 5.625 * ANGLE_RATIO;
			turn_step -= 5.625 * ANGLE_RATIO;
		}

		if (angle < 0.0)
		{
			angle += PI2;
		}
		if (angle >= PI2)
		{
			angle -= PI2;
		}
	}
	*/

	//sprite_index = (int)(angle / 5.625) + 16;
	//sprite_index &= 63;
	sprite_index = get_index(angle);

	return;
}


REGISTER_SHIP(BahaoidBuzzsaw)
