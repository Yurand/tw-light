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

#include <allegro.h>
#include "ship.h"
REGISTER_FILE

class EarthlingCruiser2 : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	double       weaponTurnRate;
	double       weaponSpread;
	int          weaponFocus;

	int    specialColor;
	double specialRange;
	int    specialFrames;
	int    specialDamage;

	public:
		EarthlingCruiser2(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
};

class EarthlingFusionBlast : public AnimatedShot
{
	public:
		EarthlingFusionBlast(double ox, double oy, double oangle, double ov,
			int odamage, double orange, int oarmour, double otrate, SpaceLocation *ocreator,
			SpaceObject *otarget, SpaceSprite *osprite, int ofcount, int ofsize, int ofocus);
		virtual void calculate();
	protected:
		int focus;
		int growing;
		double turn_rate;
};

EarthlingCruiser2::EarthlingCruiser2(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponTurnRate = scale_turning(tw_get_config_float("Weapon", "TurnRate", 0));
	weaponSpread   = tw_get_config_float("Weapon", "Spread", 10)*ANGLE_RATIO;
	weaponFocus    = tw_get_config_int("Weapon", "Focus", 2);

	specialColor  = tw_get_config_int("Special", "Color", 0);
	specialRange  = scale_range(tw_get_config_float("Special", "Range", 0));

	specialFrames = tw_get_config_int("Special", "Frames", 0);
	specialDamage = tw_get_config_int("Special", "Damage", 0);

}


int EarthlingCruiser2::activate_weapon()
{
	STACKTRACE;
	add(new EarthlingFusionBlast(0.0, size.y*0.45, angle, weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, weaponTurnRate, this,
		target, data->spriteSpecial, 6, 70, weaponFocus));
	add(new EarthlingFusionBlast(0.0, size.y*0.45, angle + weaponSpread, weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, weaponTurnRate, this,
		target, data->spriteSpecial, 6, 70, weaponFocus));
	add(new EarthlingFusionBlast(0.0, size.y*0.45, angle - weaponSpread, weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, weaponTurnRate, this,
		target, data->spriteSpecial, 6, 70, weaponFocus));
	return(TRUE);
}


int EarthlingCruiser2::activate_special()
{
	STACKTRACE;
	int fire = FALSE;
	SpaceObject *o;

	Query q;
	for (q.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
	bit(LAYER_CBODIES), specialRange); q.current; q.next()) {
		o = q.currento;
		if (!o->isInvisible() && !o->sameTeam(this) && (o->collide_flag_anyone&bit(LAYER_LINES))) {
			SpaceLocation *l = new PointLaser(this, pallete_color[specialColor], 1,
				specialFrames, this, o, Vector2(0.0,0.0));
			add(l);
			if (l->exists()) {
				fire = TRUE;
				l->set_depth(LAYER_EXPLOSIONS);
			}
		}
	}
	q.end();
	if (fire) sound.play((SAMPLE *)(melee[MELEE_BOOM + 0].dat));

	return(fire);
}


EarthlingFusionBlast::EarthlingFusionBlast(double ox, double oy, double oangle, double ov,
int odamage, double orange, int oarmour, double otrate, SpaceLocation *ocreator,
SpaceObject *otarget, SpaceSprite *osprite, int ofcount, int ofsize, int ofocus) :
AnimatedShot(ocreator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, ocreator,
osprite, ofcount, ofsize)
{
	STACKTRACE;
	growing = true;
	turn_rate = otrate;
	target = otarget;
	focus = ofocus;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 11;
	explosionFrameSize  = 35;
	explosionSample = data->sampleWeapon[1];
}


void EarthlingFusionBlast::calculate()
{
	STACKTRACE;
	Shot::calculate();
	frame_step -= frame_time;
	while (frame_step < 0) {
		frame_step += frame_size;
		sprite_index++;
		if (sprite_index == frame_count) {
			sprite_index = 0;
			if (growing) {
				sprite = data->spriteWeapon;
				frame_size = 150;
				frame_step = frame_size;
				growing = false;
			}
		}
	}

	if (target==NULL) return;
	else    if (!target->exists()) return;
	if ((!target->isInvisible())&&(ship)) {

		double d_a = normalize(trajectory_angle(target) - angle, PI2);
		if (d_a > PI) d_a -= PI2;
		double maneur_v;
		if (d * focus > range) maneur_v = turn_rate * frame_time;
		else             maneur_v = turn_rate *frame_time * focus * d / range;
		if (fabs(d_a) < maneur_v)
			angle = trajectory_angle(target);
		else
		if (d_a > 0) angle += maneur_v;
			else         angle -= maneur_v;
		changeDirection(angle);
	}

}


REGISTER_SHIP(EarthlingCruiser2)
