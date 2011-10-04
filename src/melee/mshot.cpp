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

#include "melee.h"
REGISTER_FILE
#include "id.h"
#include "mgame.h"
#include "mshot.h"
#include "mship.h"
#include "manim.h"

// Define shot spark

Shot::Shot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
double odamage, double orange, double oarmour, SpaceLocation *opos,
SpaceSprite *osprite, double relativity)
:
SpaceObject(creator, opos->normal_pos(),
oangle, osprite),
v(ov),
d(0.0),
range(orange),
armour(oarmour),
explosionSprite(meleedata.sparkSprite),
explosionSample(NULL),
explosionFrameCount(SPARK_FRAMES),
explosionFrameSize(scale_frames(0))
{
	STACKTRACE;
	layer = LAYER_SHOTS;
	set_depth(DEPTH_SHOTS);
	attributes |= ATTRIB_SHOT;

	// angle conventions fucked up??
	rpos.x *= -1;
	pos = normalize(pos + rotate(rpos, -PI/2+opos->get_angle()));

	vel = (v * unit_vector(angle)) + opos->get_vel() * relativity;

	damage_factor = odamage;

	id |= SPACE_SHOT;

	if (range < 0) range = 99999999999999.0;

	isblockingweapons = false;
}


void Shot::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();

	if (!(ship && ship->exists())) {
		state = 0;
		return;
	}

	d += v * frame_time;
	if (d >= range) state = 0;
	return;
}


void Shot::animate(Frame *space)
{
	STACKTRACE;
	SpaceObject::animate(space);
	return;
}


int Shot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (!exists()) return 0;
	if ((normal > 0) || (direct > 0)) {
		armour -= normal;
		armour -= direct;
		if (armour <= 0) {
			normal += armour;
			armour = 0;
			state = 0;
			animateExplosion();
			soundExplosion();
		}
	}
	return 1;
}


void Shot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->exists()) return;
	damage(other, damage_factor);
	//if (!other->isShot()) state = 0;
	if (other->isblockingweapons) state = 0;
	if (state == 0) {
		animateExplosion();
		soundExplosion();
	}
	return;
}


void Shot::death()
{
	STACKTRACE;
	/*	animateExplosion();
		soundExplosion(); */
}


void Shot::animateExplosion()
{
	STACKTRACE;
	game->add(new Animation(this, normal_pos(),
		explosionSprite, 0, explosionFrameCount,
		explosionFrameSize, DEPTH_EXPLOSIONS));
	return;
}


void Shot::soundExplosion()
{
	STACKTRACE;
	if (explosionSample) {
		play_sound2(explosionSample);
	}
	else if (damage_factor > 0) {
		int i = iround_down(damage_factor / 2);
		if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	}
	return;
}


void Shot::stop()
{
	STACKTRACE;
	vel = Vector2(0,0);
	return;
}


void Shot::destroy()
{
	STACKTRACE;
	state = 0;
	return;
}


void Shot::changeDirection (double nangle)
{
	//v = sqrt(vx*vx + vy*vy);
	//	vx += (v * cos(nangle)) - (v * cos(angle));
	//	vy += (v * sin(nangle)) - (v * sin(angle));
	vel = v * unit_vector(nangle);
	angle = normalize(nangle,PI2);
	return;
}


int Shot::isHomingMissile()
{
	STACKTRACE;
	return ((id & BASE_MASK3) == SPACE_HOMING_MISSILE);
}


AnimatedShot::AnimatedShot(SpaceLocation *creator, Vector2 rpos,
double oangle, double ov, double odamage, double orange, double oarmour, SpaceLocation *opos,
SpaceSprite *osprite, int ofcount, int ofsize, double relativity)
:
Shot(creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite, relativity),
frame_count(ofcount),
frame_size(ofsize),
frame_step(ofsize)
{
	STACKTRACE;
}


void AnimatedShot::calculate()
{
	STACKTRACE;
	Shot::calculate();
	frame_step -= frame_time;
	while (frame_step < 0) {
		frame_step += frame_size;
		sprite_index++;
		if (sprite_index == frame_count) sprite_index = 0;
	}
	return;
}


Missile::Missile(SpaceLocation *creator, Vector2 rpos, double oangle,
double ov, double odamage, double orange, double oarmour,
SpaceLocation *opos, SpaceSprite *osprite, double relativity)
:
Shot(creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite, relativity)
{
	STACKTRACE;
	sprite_index = get_index(angle);
}


void Missile::changeDirection (double oangle)
{
	Shot::changeDirection(oangle);
	sprite_index = get_index(oangle);
	return;
}


HomingMissile::HomingMissile(SpaceLocation *creator, Vector2 rpos,
double oangle, double ov, double odamage, double orange, double oarmour,
double otrate, SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget)
:
Missile(creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite, 0),
turn_rate(otrate),
turn_step(0.0)
{
	STACKTRACE;
	target = otarget;
	id = SPACE_HOMING_MISSILE;
}


void HomingMissile::calculate()
{
	STACKTRACE;
	Missile::calculate();
	if (target && !target->isInvisible()) {
		double d_a = normalize(trajectory_angle(target) - (angle + turn_step), PI2);
		if (d_a > PI) d_a -= PI2;
		double ta = turn_rate * frame_time;
		if (fabs(d_a) < ta) ta = fabs(d_a);
		if (d_a > 0) turn_step += ta;
		else turn_step -= ta;
		while(fabs(turn_step) > PI2/64/2) {
			if (turn_step < 0.0) {
				angle -= PI2/64;
				turn_step += PI2/64;
			}
			else if (turn_step > 0.0) {
				angle += PI2/64;
				turn_step -= PI2/64;
			}
		}
		angle = normalize(angle, PI2);
		vel = v * unit_vector(angle);
	}

	sprite_index = (int)(angle / (PI2/64)) + 16;
	sprite_index &= 63;

	return;
}


void HomingMissile::animate(Frame *space)
{
	STACKTRACE;
	int old_sprite_index = sprite_index;
	Vector2 old_vel = vel;

	double ta = 0;
	if (target && !target->isInvisible()) {
		Vector2 tpos = target->normal_pos() + target->vel * 0;
		double da = normalize(atan3(min_delta(tpos,pos)) - (angle + turn_step));
		if (da > PI) da -= PI2;
		double ta = turn_rate * 0;
		if (fabs(da) < ta) ta = da;
		else if (da > 0) ta = -ta;
	}

	angle += ta;
	sprite_index = get_index(angle + ta);

	vel = v * unit_vector(angle + ta/2);

	Shot::animate(space);

	sprite_index = old_sprite_index;
	vel = old_vel;
	return;
}


Laser::Laser(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage,
int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle)
:
SpaceLine(creator, opos->normal_pos(),
langle, lrange, lcolor),
frame(0),
frame_count(lfcount),
lpos(opos),
rel_pos(rpos),
sinc_angle(osinc_angle)
{
	STACKTRACE;

	// angle conventions fucked up??
	rel_pos.x *= -1;
	pos = normalize(pos + rotate(rel_pos, -PI/2+opos->get_angle()));

	id |= SPACE_LASER;
	damage_factor = ldamage;
	relative_angle = angle - lpos->get_angle();

	/*
		double alpha;
		alpha = (pos->get_angle());
		double tx, ty;
		tx = cos(alpha);
		ty = sin(alpha);
	// angle conventions fucked up??
		x = pos->normal_x() + pos_y * tx - pos_x * ty;
		y = pos->normal_y() + pos_y * ty + pos_x * tx;*/
	vel = lpos->get_vel();

	if (!lpos->exists()) state = 0;

	collide_flag_sameteam = ALL_LAYERS;
}


void Laser::calculate()
{
	STACKTRACE;
	if ((frame < frame_count) && (lpos->exists())) {
		pos = lpos->normal_pos() + rotate(rel_pos, lpos->get_angle() - PI/2);
		vel = lpos->get_vel();
		if (sinc_angle) angle = normalize(lpos->get_angle() + relative_angle, PI2);
		SpaceLine::calculate();
		frame += frame_time;
	}
	else state = 0;
	return;
}


PointLaser::PointLaser(SpaceLocation *creator, int lcolor, double ldamage,
int lfcount, SpaceLocation *lsource, SpaceObject *ltarget, Vector2 rel_pos) :
Laser(creator, lsource->trajectory_angle(ltarget), lcolor, lsource->distance(ltarget), ldamage, lfcount, lsource, rel_pos),
target(ltarget)
{
	STACKTRACE;
	collide_flag_anyone = bit(target->layer);
	collide_flag_sameteam = bit(target->layer);
	collide_flag_sameship = bit(target->layer);
	vel = lpos->get_vel();
	angle = trajectory_angle(target);

	if (!target->canCollide(this) || !canCollide(target)) state = 0;
	if (!lpos->exists()) state = 0;
}


int PointLaser::canCollide(SpaceObject *other)
{
	STACKTRACE;
	if (other != target) return false;
	return Laser::canCollide(other);
}


void PointLaser::calculate()
{
	STACKTRACE;
	double alpha;
	alpha = (lpos->get_angle());
	Laser::calculate();
	if (target) {
		if (target->exists() && canCollide(target) && target->canCollide(this)) {
			inflict_damage(target);
			//			length = distance(target);
		}
		angle = trajectory_angle(target);
		if (!target->exists()) target = NULL;
	}
	return;
}


TimedShot::TimedShot(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
double ovel, double otime, double oarmour, double odamage)
:
SpaceObject(creator, 0, 0, osprite),
armour(oarmour),
existtime(0),
maxtime(otime)
{
	STACKTRACE;
	angle = creator->angle + orelangle;
	pos = creator->pos + rotate(orelpos, creator->angle - PI/2);
	vel = ovel * unit_vector(angle);

	layer = LAYER_SHOTS;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = 0;

	damage_factor = odamage;

	isblockingweapons = false;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void TimedShot::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();

	if (!(ship && ship->exists())) {
		state = 0;
		return;
	}

	if (existtime >= maxtime) {
		state = 0;
		return;
	}

	existtime += frame_time * 1E-3;

	// always orient the shot in the direction of movement.
	angle = vel.angle();
	sprite_index = get_index(angle);
}


void TimedShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	// copied from Shot::infli...

	if (!other->exists()) return;

	damage(other, damage_factor);

	//if (!other->isShot()) state = 0;
	if (other->isblockingweapons) state = 0;

	if (state == 0) {
		//animateExplosion();
		//soundExplosion();
	}
}


int TimedShot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	armour -= (normal + direct);

	if (armour <= 0) {
		armour = 0;
		state = 0;
	}

	return true;
}
