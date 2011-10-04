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

#include "util/aastr.h"

#include "shpchmav.h"

ChmmrAvatar::ChmmrAvatar(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange  = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponDamage = tw_get_config_int("Weapon", "Damage", 0);

	specialForce = scale_velocity(tw_get_config_float("Special", "Force", 0));
	specialRange = scale_range(tw_get_config_float("Special", "Range", 0));

	extraRange        = scale_range(tw_get_config_float("Extra", "Range", 0));
	extraFrames       = tw_get_config_int("Extra", "Frames", 0);
	extraDamage       = tw_get_config_int("Extra", "Damage", 0);
	extraRechargeRate = tw_get_config_int("Extra", "RechargeRate", 0);
	extraColor        = tw_get_config_int("Extra", "Color", 0);
	extraArmour       = tw_get_config_int("Extra", "Armour", 0);

	uninterrupted_fire = false;
}


void ChmmrAvatar::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if ((uninterrupted_fire) && ((!fire_weapon) || weapon_low)) {
		uninterrupted_fire = false;
		add(new Animation(this,
			pos + unit_vector(angle) * (tw_random(weaponRange-15) + 25 + 25)
			+ Vector2(tw_random(-25,25), tw_random(-25,25)),
			data->spriteWeaponExplosion, (random()%4)*10, 10, 75, DEPTH_EXPLOSIONS));
	};

}


int ChmmrAvatar::activate_weapon()
{
	STACKTRACE;
	add(new ChmmrLaser(angle, weaponRange, weaponDamage, weapon_rate, this, Vector2(0.0, 25.0) ));
	if (tw_random(150) < frame_time) {
		add(new Animation(this,
			pos + unit_vector(angle) * (tw_random(weaponRange-15) + 25 + 25)
			+ Vector2(tw_random(-25,25), tw_random(-25,25)),
			data->spriteWeaponExplosion, (random()%4)*10, 10, 25+tw_random(50), DEPTH_EXPLOSIONS, tw_random(1.0) + tw_random(1.0))
			);
		add(new Animation(this,
			pos + unit_vector(angle) * (tw_random(weaponRange-15) + 25 + 25)
			+ Vector2(tw_random(-25,25), tw_random(-25,25)),
			data->spriteWeaponExplosion, (random()%4)*10, 10, 50, DEPTH_EXPLOSIONS)
			);
	}

	uninterrupted_fire = true;

	return(TRUE);
}


int ChmmrAvatar::activate_special()
{
	STACKTRACE;
	if (target && target->exists() && (!target->isInvisible()) && (target->mass > 0) && (distance(target) < specialRange)) {
		add(new ChmmrBeam(this, special_rate));
		target->accelerate(this, target->trajectory_angle(this), specialForce / target->mass, MAX_SPEED);
		return (true);
	}
	return false;
}


void ChmmrAvatar::materialize()
{
	STACKTRACE;
	Ship::materialize();
	add(new ChmmrZapSat(0.0, extraRange,
		extraDamage, extraFrames, extraRechargeRate, extraColor, extraArmour, this,
		data->spriteExtra));
	add(new ChmmrZapSat(PI2/3, extraRange,
		extraDamage, extraFrames, extraRechargeRate, extraColor, extraArmour, this,
		data->spriteExtra));
	add(new ChmmrZapSat(PI2*2/3, extraRange,
		extraDamage, extraFrames, extraRechargeRate, extraColor, extraArmour, this,
		data->spriteExtra));
}


ChmmrLaser::ChmmrLaser(double langle, double lrange, int ldamage,
int lfcount, SpaceLocation *opos, Vector2 rel_pos)
:
Laser(opos, langle, pallete_color[hot_color[random() % HOT_COLORS]], lrange,
ldamage, lfcount, opos, rel_pos, true)
{
	STACKTRACE;
}


ChmmrBeam::ChmmrBeam(Ship *oship, int oframes) :
SpaceObject(oship, oship->normal_pos(), oship->get_angle(),
meleedata.sparkSprite),
frame(0),
frame_count(oframes),
ship(oship),
target(oship->target)
{
	STACKTRACE;
	set_depth(DEPTH_HOTSPOTS);
	collide_flag_anyone = 0;
	if (!(ship && ship->exists())) {
		state = 0;
		return;
	}

	target = ship->target;
	if (!(target && target->exists()) || (target->isInvisible())) {
		state = 0;
		return;
	}
}


void ChmmrBeam::calculate()
{
	STACKTRACE;

	SpaceLocation::calculate();

	if (!(ship && ship->exists())) {
		ship = 0;				 // not really needed but well.
		state = 0;
		return;
	}
	target = ship->target;

	if ((!(target && target->exists())) || (target->isInvisible())) {
		state = 0;
		return;
	}

	//	x = ship->normal_x();
	//	y = ship->normal_y();
	pos = ship->normal_pos();
	frame += frame_time;
	if (frame > frame_count) state = 0;
}


void ChmmrBeam::animate(Frame *space)
{
	STACKTRACE;
	const int beam_color[5] = { 80, 81, 82, 83, 84 };
	int i;						 //, old_trans;
	double length = target->get_vel().length() + (target->get_size().x / 4.0);
	/*
		if ((get_tw_aa_mode() & AA_BLEND) && !(get_tw_aa_mode() & AA_NO_AA)) {
			old_trans = aa_get_trans();
			for(i = 3; i >= 0 ; i--) {
				aa_set_trans(255*(i+1)/4.0);
				target->get_sprite()->animate_character(
						target->normal_pos() + (i+1) * unit_vector(trajectory_angle(target) - PI) * length,
						target->get_sprite_index(), tw_makecol(20,20,240), space);
			}
			aa_set_trans(old_trans);
		}
		else*/
	for(i = 3; i >= 0 ; i--)
		target->get_sprite()->animate_character(
			target->normal_pos() + (i+1) * unit_vector(trajectory_angle(target) - PI) * length,
			target->get_sprite_index(), pallete_color[beam_color[i]], space);

}


ChmmrZapSat::ChmmrZapSat(double oangle, double orange, int odamage,
int oframes, int orechargerate, int ocolor, int oarmour, Ship *oship,
SpaceSprite *osprite) :
SpaceObject(oship, Vector2(0.0, 0.0), 0.0, osprite),
lRange(orange),
lDamage(odamage),
lFrames(oframes),
lRechargeRate(orechargerate),
lRecharge(0),
lColor(ocolor),
armour(oarmour)
{
	STACKTRACE;
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);
	collide_flag_anyone = ALL_LAYERS &~ bit(LAYER_CBODIES);
	id |= CHMMR_SPEC;
	angle = oangle;
	//	x = ship->normal_x() + cos(angle) * 100.0;
	//	y = ship->normal_y() + sin(angle) * 100.0;
	pos = ship->normal_pos() + unit_vector(angle) * 100.0;

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
	}

	isblockingweapons = false;
}


void ChmmrZapSat::calculate()
{
	STACKTRACE;

	SpaceObject::calculate();

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	//	x = ship->normal_x() + (cos(angle) * 100.0);
	//	y = ship->normal_y() + (sin(angle) * 100.0);
	pos = ship->normal_pos() + unit_vector(angle) * 100.0;

	double da = 0.002;

	angle += da * frame_time;

	//	vx = (ship->normal_x() + (cos(angle) * 100.0) - x) / frame_time;
	//	vy = (ship->normal_y() + (sin(angle) * 100.0) - y) / frame_time;
	vel = (ship->normal_pos() + unit_vector(angle) * 100.0 - pos) / frame_time;

	if (angle >= PI2) angle -= PI2;
	sprite_index = get_index(angle);

	if (lRecharge > 0) {
		lRecharge -= frame_time;
		return;
	}

	Query q;
	for (q.begin(this, OBJECT_LAYERS &~ bit(LAYER_CBODIES), lRange); q.currento; q.next()) {
		if (!q.currento->isInvisible() && !q.currento->sameTeam(this) && (q.currento->collide_flag_anyone&bit(LAYER_LINES))) {
			add(new PointLaser(this, pallete_color[lColor], 1, lFrames,
				this, q.currento, Vector2(0.0, 0.0) ));
			sound.play((SAMPLE *)(melee[MELEE_BOOM + 0].dat));
			lRecharge += lRechargeRate;
			break;
		}
	}
	return;
}


int ChmmrZapSat::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	if (!other->damage_factor) return false;
	return SpaceObject::canCollide(other);
}


int ChmmrZapSat::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int total = iround(normal + direct);
	if (total) {
		armour -= total;
		if (armour <= 0) {
			armour = 0;
			state = 0;
			add(new Animation(this, pos,
				meleedata.kaboomSprite, 0, KABOOM_FRAMES, 50, DEPTH_EXPLOSIONS));
			sound.stop(data->sampleExtra[0]);
			sound.play(data->sampleExtra[0]);
		}
	}
	return total;
}


REGISTER_SHIP(ChmmrAvatar)
