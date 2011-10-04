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

#define trace_lifetime 200.0
#define trace_rate 50
#define max_trace_number 10

class TauSliderTrace;

class TauSlider  :  public Ship
{

	TauSliderTrace* tr[max_trace_number];

	double        weaponRange;
	double        weaponVelocity;
	int           weaponLength;

	double        range0, range1, range2;
	double        velocity0, velocity1, velocity2;
	int           special_charge, max_charge, critical_charge;
	double        sub_velocity, sub_range;

	double        jd, jv;		 //, jvx, jvy;
	Vector2       jvp;
	int           ct;
	int           trace_recharge;

	bool          holding_special;
	bool          in_jump;
	bool          just_exited;

	int           collide_flag_reserve;

	public:

		TauSlider(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual  int  activate_weapon();
		virtual void  calculate_fire_special();
		virtual  int  activate_special();
		virtual void  calculate();
		virtual  int  handle_damage(SpaceLocation* source, int normal, int direct);
		virtual  int  canCollide(SpaceLocation *other);
		virtual  int  translate(double dx, double dy);
		virtual  int  accelerate(SpaceLocation *source, double angle, double vel, double max_speed);
		virtual void  animate(Frame *space);
		virtual  int  isProtected();
		virtual void  calculate_hotspots();
};

class TauSliderLaser : public SpaceLine
{
	double        range, d, v;

	public:

		TauSliderLaser  (double ox, double oy, double oangle, double ov, double oragne,
			double olength, SpaceLocation *creator);

		virtual void  calculate();
		virtual void  inflict_damage(SpaceObject *other);
};

class TauSliderTrace : public SpaceLocation
{
	public:
		int lifetime, s_i;
		TauSliderTrace(Ship* oship);
		virtual void calculate();
};

TauSlider::TauSlider (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	//        hotspot_position = 0.22;

	weaponRange             = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity          = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponLength            = tw_get_config_int("Weapon", "Length", 0);

	range0                  = scale_range(tw_get_config_float("Special", "Range0", 0));
	range1                  = scale_range(tw_get_config_float("Special", "Range1", 0));
	range2                  = scale_range(tw_get_config_float("Special", "Range2", 0));
	velocity0                = scale_velocity(tw_get_config_float("Special", "Velocity0", 0));
	velocity1                = scale_velocity(tw_get_config_float("Special", "Velocity1", 0));
	velocity2                = scale_velocity(tw_get_config_float("Special", "Velocity2", 0));
	critical_charge         = tw_get_config_int("Special", "MinCount", 0);
	max_charge              = tw_get_config_int("Special", "MaxCount", 1);
	if (max_charge < 0) max_charge = 999999999;
	sub_range               = tw_get_config_float("Special", "SubRange", 0);
	sub_velocity            = tw_get_config_float("Special", "SubVelocity", 0);

	in_jump                 = false;
	holding_special         = false;
	special_charge          = 0;
	just_exited             = false;
	collide_flag_reserve    = collide_flag_anyone;
	trace_recharge = 0;
	ct = 0;

	for (int i=0; i<max_trace_number; i++)
		tr[i] = NULL;
}


int TauSlider::activate_weapon()
{
	STACKTRACE;
	if (in_jump) return false;
	add(new TauSliderLaser(+21, 15, angle - 15*ANGLE_RATIO,
		weaponVelocity, weaponRange, weaponLength, this));
	add(new TauSliderLaser(+7, 15, angle - 5*ANGLE_RATIO,
		weaponVelocity, weaponRange, weaponLength, this));
	add(new TauSliderLaser(-21, 15, angle + 15*ANGLE_RATIO,
		weaponVelocity, weaponRange, weaponLength, this));
	add(new TauSliderLaser(-7, 15, angle + 5*ANGLE_RATIO,
		weaponVelocity, weaponRange, weaponLength, this));
	return true;
}


void TauSlider::calculate_fire_special()
{
	STACKTRACE;
	special_low = false;

	if (fire_special) {
		if (in_jump) return;
		holding_special = true;
		if (special_recharge <= 0) {
			if (batt < special_drain) {
				special_low = true;
				activate_special();
				return;
			} else {
				special_charge++;
				batt -= special_drain;
				special_recharge += special_rate;
				play_sound2(data->sampleSpecial[0]);
				if (special_charge >= max_charge)
					activate_special();
			}
		}
	} else {
		if (holding_special)
			if (!in_jump)
				activate_special();
		holding_special = false;
	}
}


int TauSlider::activate_special()
{
	STACKTRACE;
	if (special_charge >= critical_charge) {
		in_jump = true;
		collide_flag_anyone = 0;
		layer = LAYER_HOTSPOTS;
		double a = special_charge - sub_velocity;
		jv = velocity0 + a * (velocity1 + a * velocity2);
		//jvx = jv * cos(angle*ANGLE_RATIO);
		//jvy = jv * sin(angle*ANGLE_RATIO);
		jvp = jv * unit_vector(angle);
		a = special_charge - sub_range;
		jd = range0 + a * (range1 + a * range2);
		trace_recharge = 0;
		play_sound2(data->sampleSpecial[2]);
	}
	else    play_sound2(data->sampleSpecial[1]);

	special_charge = 0;
	return true;
}


void TauSlider::calculate()
{
	STACKTRACE;
	just_exited = false;
	Ship::calculate();
	if (in_jump) {
		trace_recharge -= frame_time;
		if (trace_recharge < 0) {
			trace_recharge += trace_rate;
			ct = (ct + 1) % max_trace_number;
			tr[ct] = new TauSliderTrace(this);
			add(tr[ct]);
		}
		//x = normalize(x + jvx * frame_time, X_MAX);
		//y = normalize(y + jvy * frame_time, Y_MAX);
		pos = normalize(pos + jvp * frame_time, map_size);
		jd -= jv * frame_time;
		if (jd <= 0) {
			in_jump = false;
			layer = LAYER_SHIPS;
			collide_flag_anyone = collide_flag_reserve;
		}
	}
}


int TauSlider::handle_damage(SpaceLocation *source, int normal, int direct)
{
	STACKTRACE;
	if (in_jump) {
		return 0;
	}
	if (just_exited && source->isPlanet())
		normal = iround(crew);
	return Ship::handle_damage(source, normal, direct);
}


int TauSlider::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	return ((!in_jump)&&Ship::canCollide(other));
}


int TauSlider::translate(double dx, double dy)
{
	STACKTRACE;
	if (!in_jump) return Ship::translate(dx, dy);
	return false;
}


int TauSlider::accelerate(SpaceLocation *source, double oangle, double vel, double omax_speed)
{
	STACKTRACE;
	if (!in_jump) return Ship::accelerate(source, oangle, vel, omax_speed);
	return false;
}


void TauSlider::animate(Frame* space)
{
	STACKTRACE;
	int i,j,r,g,b;
	double a;
	for (i=max_trace_number; i>=0; i--) {
		j = (ct + i) % max_trace_number;
		if (tr[j]!=NULL) {
			if (!tr[j]->exists()) {
				tr[j] = NULL;
				continue;
			}
			a = tr[j]->lifetime / trace_lifetime;
			r = (int)floor(195 - a*500);
			if (r < 0) r = 0;
			g = (int)floor(195 - a*500);
			if (g < 0) g = 0;
			b = (int)floor(255 - a*200);
			if (b < 0) b = 0;
			sprite->animate_character(tr[j]->normal_pos(),tr[j]->s_i,tw_makecol(r,g,b),space);
		}
	}

	if (!in_jump) Ship::animate(space);
	else    sprite->animate_character(pos, sprite_index,tw_makecol(215,215,255),space);
}


int TauSlider::isProtected()
{
	STACKTRACE;
	return (in_jump);
}


void TauSlider::calculate_hotspots()
{
	STACKTRACE;
	if (!in_jump) Ship::calculate_hotspots();
}


TauSliderLaser::TauSliderLaser (double ox, double oy, double oangle, double ov, double orange, double olength, SpaceLocation *creator) :
SpaceLine(creator, creator->normal_pos(), oangle, olength, tw_makecol(255,255,255)), range(orange),
d(0),
v(ov)
{
								 // * ANGLE_RATIO;
	double alpha = creator->get_angle();
	//double tx = cos(alpha), ty = sin(alpha);
	//x += oy * tx - ox * ty;
	//y += oy * ty + ox * tx;
	pos += rotate( Vector2(oy,ox), alpha );
	damage_factor = 1;
	//vx = ov * cos (angle * ANGLE_RATIO) + creator->get_vx();
	//vy = ov * sin (angle * ANGLE_RATIO) + creator->get_vy();
	vel = ov * unit_vector(angle) + creator->get_vel();
}


void TauSliderLaser::calculate()
{
	STACKTRACE;
	double a = (d) / range;
	int r = (int)floor(255 - a*400);
	if (r < 0) r = 0;
	int g = (int)floor(255 - a*400);
	if (g < 0) g = 0;
	int b = (int)floor(255 - a*200);
	if (b < 0) b = 0;
	color = tw_makecol(r, g, b);
	SpaceLine::calculate();
	d += v * frame_time;
	if (d > range) state = 0;
}


void TauSliderLaser::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	SpaceLine::inflict_damage(other);
	state = 0;
}


TauSliderTrace::TauSliderTrace(Ship* oship) :
SpaceLocation(oship, oship->normal_pos(), oship->get_angle())
{
	STACKTRACE;
	collide_flag_anyone = 0;
	lifetime = 0;
	//vx = oship->get_vx();
	//vy = oship->get_vy();
	vel = oship->get_vel();
								 //>getSpriteIndex();
	s_i = oship->get_sprite_index();
}


void TauSliderTrace::calculate()
{
	STACKTRACE;
	if ((lifetime += frame_time) >= trace_lifetime)
		state = 0;
}


REGISTER_SHIP(TauSlider)
