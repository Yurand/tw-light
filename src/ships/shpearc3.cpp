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
#include "util/aastr.h"
REGISTER_FILE

class EarthlingCruiserMk3Beam;

class EarthlingCruiserMk3 : public Ship
{
	double      weaponRange;
	double      weaponVelocity;
	double      weaponDamage;
	double      weaponArmour;
	double      weaponSpread;
	double      weaponTrackingAngle;
	double      weaponTrackingRate;

	double      launch_angle;

	double      specialRange;
	double      specialDamage;
	double      specialDamageShots;
	int         specialFrames;

	public:
		EarthlingCruiserMk3(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual int  activate_weapon();
		virtual int  activate_special();
		virtual void calculate();
};

class EarthlingCruiserMk3Shot : public Shot
{
	public:
		EarthlingCruiserMk3Shot(SpaceLocation *creator, Vector2 opos, double oangle,
			double ov, double odamage, double orange, double oarmour, SpaceSprite *osprite);
		virtual void animate(Frame *space);
		virtual void soundExplosion();
};

class EarthlingCruiserMk3Beam : public SpaceLine
{
	protected:
		double          frame, frame_count;
		SpaceLocation   *lpos;
		Vector2         rel_pos;
		double          damage_shots;
		double          base_length;
		bool            got_spark;
		int             switch_counter;
	public:
		EarthlingCruiserMk3Beam(SpaceLocation *creator, Vector2 rpos, double lrange,
			double ldamage, double sdamage, int lfcount, SpaceObject *tgt);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual void animate(Frame *space);
};

EarthlingCruiserMk3::EarthlingCruiserMk3(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weaponRange         = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour        = tw_get_config_int("Weapon", "Armour", 0);
	weaponSpread        = tw_get_config_float("Weapon", "Spread", 0) * ANGLE_RATIO;
	weaponTrackingAngle = tw_get_config_float("Weapon", "TrackingAngle", 0.01) * ANGLE_RATIO;
	weaponTrackingRate  = scale_turning(tw_get_config_float("Weapon", "TrackingRate", 0));

	launch_angle        = 0;

	specialRange        = scale_range(tw_get_config_float("Special", "Range", 0));
	specialDamage       = tw_get_config_float("Special", "Damage", 0);
	specialDamageShots  = tw_get_config_int("Special", "DamageShots", 0);
	specialFrames       = tw_get_config_int("Special", "Frames", 0);

	weapon_sample       = -1;
	special_sample      = -1;

}


void EarthlingCruiserMk3::calculate()
{
	STACKTRACE;
	Ship::calculate();

	double track_angle = 0;
	double track_min = PI2;
	double d_a;
	SpaceObject *tgt = 0;

								 //!!!
	pos += unit_vector(angle) * 28;
	int i;
	for (i=0; i < targets->N; i++) {
		tgt = targets->item[i];
		if ((tgt != this) && (!tgt->sameShip(this)) && (!tgt->sameTeam(this))
		&& (!tgt->isInvisible()) && (distance(tgt) <= 1.2*(weaponRange + abs(vel)*game->shot_relativity))) {

			d_a = trajectory_angle(tgt);
			double s = unit_vector(d_a + PI/2).dot(tgt->get_vel() - vel * game->shot_relativity) / weaponVelocity;
			if (fabs(s) > 0.99)
				track_angle = 0;
			else {
				s = atan(s/sqrt(1-s*s));
				s = normalize(s + d_a - angle - turn_step, PI2);
				if (s > PI) s -= PI2;
				if (fabs(s) <= weaponTrackingAngle) {
					d_a = s - launch_angle;
					if (tgt == target) {
						track_angle = s;
						break;
					}
					if (fabs(d_a) < track_min) {
						track_angle = s;
						track_min = fabs(d_a);
					}
				}
			}
		}
	}
								 //!!!
	pos -= unit_vector(angle) * 28;

	d_a = track_angle - launch_angle;
	double ta = weaponTrackingRate * frame_time;
	if (fabs(d_a) < ta) ta = fabs(d_a);
	if (d_a > 0) launch_angle += ta;
	else launch_angle -= ta;

}


int EarthlingCruiserMk3::activate_weapon()
{
	STACKTRACE;
	add(new EarthlingCruiserMk3Shot(this, Vector2(0.0, 28.0),
		normalize(angle + launch_angle + turn_step + tw_random(-weaponSpread, +weaponSpread), PI2),
		weaponVelocity, weaponDamage, weaponRange, weaponArmour, data->spriteWeapon));
	return true;
}


int EarthlingCruiserMk3::activate_special()
{
	STACKTRACE;
	bool fire = false;;
	SpaceObject *o;
	double rng = 1e40;
	SpaceObject *tgt = NULL;

								 //!!!
	pos -= unit_vector(angle) * 6;
	Query q;
	for (q.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
	bit(LAYER_CBODIES), specialRange); q.current; q.next()) {
		o = q.currento;
		if (!o->isInvisible() && !o->sameTeam(this) && (o->collide_flag_anyone&bit(LAYER_LINES))
		&& (distance(o) < rng)) {
			tgt = o;
			rng = distance(o);
		}
	}
	q.end();
								 //!!!
	pos += unit_vector(angle) * 6;

	if (tgt) {
		game->add(new EarthlingCruiserMk3Beam(this, Vector2(0,-6), specialRange,
			specialDamage, specialDamageShots, specialFrames, tgt));
		play_sound(data->sampleSpecial[0]);
		fire = true;
	}

	return(fire);
}


EarthlingCruiserMk3Shot::EarthlingCruiserMk3Shot(SpaceLocation *creator, Vector2 opos, double oangle,
double ov, double odamage, double orange, double oarmour, SpaceSprite *osprite) :
Shot(creator, opos, oangle, ov, odamage, orange, oarmour, creator, osprite)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 20;
	explosionFrameSize  = 25;
	explosionSample = data->sampleWeapon[1];
	sprite_index = 32;

	attributes &= ~ATTRIB_STANDARD_INDEX;
	play_sound(data->sampleWeapon[0]);
}


void EarthlingCruiserMk3Shot::animate(Frame *space)
{
	STACKTRACE;
	if (exists())
		sprite->animate(pos, sprite_index = (int)(31.99 * d / range), space);
}


void EarthlingCruiserMk3Shot::soundExplosion()
{
	STACKTRACE;
	play_sound(explosionSample);
	return;
}


EarthlingCruiserMk3Beam::EarthlingCruiserMk3Beam(SpaceLocation *creator, Vector2 rpos, double lrange,
double ldamage, double sdamage, int lfcount, SpaceObject *tgt) :
SpaceLine(creator, creator->normal_pos(), 0, lrange, 0),
frame(0), frame_count(lfcount), lpos(creator), rel_pos(rpos), damage_shots(sdamage)

{
	STACKTRACE;
	set_depth(DEPTH_EXPLOSIONS);
	target = tgt;
	base_length = length;
	rel_pos.x *= -1;
	pos = normalize(pos + rotate(rel_pos, -PI/2+lpos->get_angle()));
	vel = lpos->get_vel();
	id |= SPACE_LASER;
	damage_factor = ldamage;
	angle = trajectory_angle(target);
	if (!target->canCollide(this) || !canCollide(target)) state = 0;

	if (!(lpos && lpos->exists())) {
		lpos = 0;
		state = 0;
	}

	color = tw_makecol(100+tw_random()%105,100+tw_random()%105,255);
	got_spark = false;
	switch_counter = 0;
}


void EarthlingCruiserMk3Beam::calculate()
{
	STACKTRACE;
	if (!(lpos && lpos->exists())) {
		lpos = 0;
		state = 0;
	}

	if ((frame < frame_count) && (lpos && lpos->exists())) {
		pos = lpos->normal_pos() + rotate(rel_pos, lpos->get_angle() - PI/2);
		vel = lpos->get_vel();
		SpaceLine::calculate();
		frame += frame_time;
	}
	else
		state = 0;

	if ((!target) && (switch_counter <= 0)) {
		SpaceObject *o;
		double rng = 1e40;
		SpaceObject *tgt = NULL;
		Query a;
		for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
		bit(LAYER_CBODIES), base_length); a.current; a.next()) {
			o = a.currento;
			if (!o->isInvisible() && !o->sameTeam(this) && (o->collide_flag_anyone&bit(LAYER_LINES))
			&& (distance(o) < rng)) {
				tgt = o;
				rng = distance(o);
			}
		}
		if (tgt) {
			target = tgt;
			//			switch_counter = 55;
			got_spark = false;
		}
	}

	if (target && (distance(target) <= base_length)) {
		length = base_length;
		if (target->exists() && canCollide(target) && target->canCollide(this)) {
			angle = trajectory_angle(target);
		}
		if (!target->exists()) target = NULL;
	} else {
		target = NULL;
		if (switch_counter <= 0)
			//			die();
			length = 0;
		else
			switch_counter -= frame_time;
	}

	color = tw_makecol(100+tw_random()%105,100+tw_random()%105,255);
}


void EarthlingCruiserMk3Beam::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->isShot())
		damage(other, damage_shots*frame_time/frame_count);
	else
		damage(other, damage_factor*frame_time/frame_count);

	int aa = get_tw_aa_mode();
	if (!((aa & AA_BLEND) && (aa & AA_ALPHA) && (!got_spark) && !(aa & AA_NO_AA))) {
		physics->add(new Animation( this,
			pos + edge(), meleedata.sparkSprite, 0,
			SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
		got_spark = true;
	}

	return;
}


void EarthlingCruiserMk3Beam::animate(Frame *space)
{
	STACKTRACE;
	int aa = get_tw_aa_mode();
	SpaceLine::animate(space);
	if ((aa & AA_BLEND) && (aa & AA_ALPHA) && !(aa & AA_NO_AA) && (length < base_length*0.9999) && (target)) {
		int _old_trans = aa_get_trans();
		aa_set_trans(rand()%156);
		data->spriteSpecial->animate(pos, 0, space);
		aa_set_trans(rand()%156);
		data->spriteSpecialExplosion->animate(pos+edge(), 0, space);
		aa_set_trans(_old_trans);
	}
}


REGISTER_SHIP(EarthlingCruiserMk3)
