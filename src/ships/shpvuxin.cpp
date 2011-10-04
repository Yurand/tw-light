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

class VuxIntruder : public Ship
{
	public:
		int    weaponColor;
		double weaponRange;
		int    weaponDamage;

		double specialRange;
		double specialVelocity;
		double specialSlowdown;
		int    specialArmour;

	public:
		VuxIntruder(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual SpaceLocation *get_ship_phaser() ;
		void relocate() ;
		virtual void animate(Frame *space);
};

class VuxLimpet : public AnimatedShot
{
	double slowdown_factor;

	public:
		VuxLimpet(Vector2 opos, double ov, double slowdown, double orange,
			int oarmour, Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

VuxIntruder::VuxIntruder(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)

{
	STACKTRACE;
	weaponColor  = tw_get_config_int("Weapon", "Color", 0);
	weaponRange  = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponDamage = tw_get_config_int("Weapon", "Damage", 0);

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialSlowdown = tw_get_config_float("Special", "Slowdown", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
}


int VuxIntruder::activate_weapon()
{
	STACKTRACE;
	add(new Laser(this, angle,
		tw_get_palete_color(weaponColor), weaponRange, weaponDamage, weapon_rate,
		this, Vector2(size.x/11, (size.y / 2.07)), true));
	return(TRUE);
}


int VuxIntruder::activate_special()
{
	STACKTRACE;
	add(new VuxLimpet(Vector2(0, -size.y / 2.8),
		specialVelocity, specialSlowdown, specialRange, specialArmour, this,
		data->spriteSpecial, 100, 5));
	return(TRUE);
}


VuxLimpet::VuxLimpet(Vector2 opos, double ov, double slowdown,
double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
int ofsize, int ofcount) :
AnimatedShot(oship, opos, 0.0, ov, 0, orange, oarmour, oship, osprite,
ofcount, ofsize),
slowdown_factor(slowdown)
{
	STACKTRACE;
	if ((ship->target) && (!ship->target->isInvisible()))
		angle = trajectory_angle(ship->target);
	else
		angle = ship->get_angle() - PI;

	vel    = v * unit_vector(angle);

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
	}
}


void VuxLimpet::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		state = 0;
		return;
	}

	if ((ship->target) && (!ship->target->isInvisible())) {
		angle = trajectory_angle(ship->target);
		vel = v * unit_vector(angle);
	}
	AnimatedShot::calculate();
}


void VuxLimpet::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->isShip()) {
		if (other->damage_factor || other->mass) state = 0;
		return;
	}

	Ship *target = (Ship *) other;

	play_sound(data->sampleSpecial[1]);

	//MYCODE begin
	int hx,hy,tries, col;

	if ( target->spritePanel ) {
		BITMAP *bmp = target->spritePanel->get_bitmap(0);
		//find a random spot on the target ship where it "exist"
		tries = 0;
		while (tries < 10) {
								 //graphics
			hx = 18 + (rand() % 27);
								 //graphics
			hy = 15 + (rand() % 36);
			tries++;
			col = tw_getpixel(bmp,hx,hy);
			if (col == tw_bitmap_mask_color(bmp)) continue;
			if (col == 0) continue;
			if (col == -1) continue;
			if (col == tw_get_palete_color(8)) continue;
			break;
		}

		// draw the Limplet on the ship panel
		//BITMAP *s = sprite->get_bitmap(sprite_index);
		//stretch_sprite(bmp, s, hx - s->w/4, hy - s->h/4,	s->w/2, s->h/2 );
		sprite->draw(Vector2(hx,hy)-sprite->size()/4,
			sprite->size()/2,
			sprite_index, bmp);
		target->update_panel = TRUE;
	}

	target->handle_speed_loss(this, slowdown_factor);
	state = 0;
}


void VuxIntruder::relocate()
{
	STACKTRACE;
	if ( control ) target = control->target;
	if (target && (distance(target) > 500)) {
		pos = target->normal_pos() + (unit_vector(angle) * 125.0);
		angle = trajectory_angle(target);
		if (angle > PI2) angle -= PI2;
		if (angle < 0) angle += PI2;
	}
	sprite_index = get_index(angle);
	angle = (iround(angle / (PI2/64))) * (PI2/64);
	return;
}


void VuxIntruder::animate(Frame *space)
{
	STACKTRACE;

	double back_x=size.x/3.60, back_y=-size.y/2.33,
		frnt_x=size.x/5.55, frnt_y=+size.y/17.01,
		back_y_1=-size.y/2.06;

	int s_index = get_index(angle);

	if (turn_right)
		data->spriteWeapon->animate(pos +
			rotate(Vector2(-frnt_x, frnt_y), angle-PI/2),
								 //graphics
			s_index + ((rand()%3) << 6), space);
	if (turn_left)
		data->spriteWeapon->animate(pos +
			rotate(Vector2(frnt_x,frnt_y), angle-PI/2),
								 //graphics
			s_index + ((rand()%3) << 6), space);

	s_index += 32; s_index &= 63;

	if (thrust) {
		data->spriteExtra->animate(pos +
			rotate(Vector2(back_x, back_y_1), angle-PI/2),
								 //graphics
			s_index + ((rand()%3) << 6), space);
		data->spriteExtra->animate(pos +
			rotate(Vector2(-back_x, back_y_1), angle-PI/2),
								 //graphics
			s_index + ((rand()%3) << 6), space);
	} else {
		if (turn_left)
			data->spriteWeapon->animate(pos +
				rotate(Vector2(-back_x, back_y), angle-PI/2),
								 //graphics
				s_index + ((rand()%3) << 6), space);
		if (turn_right)
			data->spriteWeapon->animate(pos +
				rotate(Vector2(back_x, back_y), angle-PI/2),
								 //graphics
				s_index + ((rand()%3) << 6), space);
	}

	Ship::animate(space);
};

class VuxPhaser : public Phaser
{
	public:
		VuxPhaser(Vector2 opos, Vector2 n, VuxIntruder *ship,
			SpaceSprite *sprite, int osprite_index, int *ocolors,
			int onum_colors, int ofsize, int steps, int step_time) ;

		VuxIntruder *vuxship;

		virtual void calculate();
};

VuxPhaser::VuxPhaser(Vector2 opos, Vector2 _n, VuxIntruder *ship,
SpaceSprite *sprite, int osprite_index, int *ocolors,
int onum_colors, int ofsize, int steps, int step_size)
:
Phaser(ship, opos, _n, ship, sprite, osprite_index, ocolors, onum_colors, ofsize, steps, step_size)
{
	STACKTRACE;
	vuxship = ship;
}


void VuxPhaser::calculate()
{
	STACKTRACE;
	Phaser::calculate();
	if (!ship) return;
								 //((VuxIntruder*)ship)->relocate();
	if ( !(ship->attributes & ATTRIB_INGAME )) vuxship->relocate();
	angle = ship->get_angle();
	sprite_index = get_index(angle);
	rel_pos = unit_vector(angle) * rel_pos.length();
	pos = normalize(ship->normal_pos() - rel_pos);
	return;
}


SpaceLocation *VuxIntruder::get_ship_phaser()
{
	STACKTRACE;
	return new VuxPhaser(
		pos - unit_vector(angle) * PHASE_MAX * size.x,
		unit_vector(angle) * PHASE_MAX * size.x,
		this, sprite, sprite_index, hot_color, HOT_COLORS,
		PHASE_DELAY, PHASE_MAX, PHASE_DELAY);
}


REGISTER_SHIP(VuxIntruder)
