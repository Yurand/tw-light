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
#include "melee/mshot.h"
REGISTER_FILE
#include <string.h>

class AktunComSat : public SpaceObject
{
	int frame;
	Ship *ship;

	double lRange;
	int    lDamage;
	int    lFrames;
	int    lRechargeRate;
	int    lRecharge;
	int    lColor;
	int    armour;

	public:
		AktunComSat(double oangle, double orange, int odamage, int oframes,
			int orechargerate, int ocolor, int oarmour, Ship *oship,
			SpaceSprite *osprite);

		virtual void calculate();
		virtual int handle_damage(SpaceLocation* source, double normal, double direct);
		//virtual void handle_damage(SpaceLocation *source);
		virtual int canCollide(SpaceLocation *other);
		virtual void death();
};

class AktunGunner : public Ship
{
	int    weaponColor;
	double weaponRange1, weaponRange2, weaponDrain2;
	int    weaponDamage;

	double specialRange;
	double specialVelocity;
	int    specialDamage;
	int    specialArmour;

	double extraRange;
	int    extraDamage;
	int    extraFrames;
	int    extraRechargeRate;
	int    extraColor;
	int    extraArmour;
	int    extraDrain;

	public:
		AktunGunner(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag);

		int num_ComSats;
		int max_ComSats;
		AktunComSat **ComSat;

		virtual int activate_weapon();
		virtual int activate_special();

};

class AktunLaser : public Laser
{
	Ship *owner;

	public:
		AktunLaser(SpaceLocation *creator, double langle, int lweaponColor, double lrange, int ldamage, int lfcount,
			SpaceLocation *opos, Vector2 relpos, Ship *lowner);

		virtual void inflict_damage(SpaceObject *other);
};

AktunGunner::AktunGunner(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag) :
Ship(opos, shipAngle, shipData, shipCollideFlag)

{
	STACKTRACE;
	weaponColor  = tw_get_config_int("Weapon", "Color", 0);
	weaponRange1  = scale_range(tw_get_config_float("Weapon", "Range1", 0));
	weaponDamage = tw_get_config_int("Weapon", "Damage", 0);

	weaponDrain2 = tw_get_config_float("Weapon", "WeaponDrain2", 0);
	weaponRange2 = scale_range(tw_get_config_float("Weapon", "Range2", 0));

	specialRange         = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity      = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage        = tw_get_config_int("Special", "Damage", 0);
	specialArmour        = tw_get_config_int("Special", "Armour", 0);

	extraRange        = scale_range(tw_get_config_float("Extra", "Range", 0));
	extraFrames       = tw_get_config_int("Extra", "Frames", 0);
	extraDamage       = tw_get_config_int("Extra", "Damage", 0);
	extraDrain        = tw_get_config_int("Extra", "Drain", 0);
	extraRechargeRate = tw_get_config_int("Extra", "RechargeRate", 0);
	extraColor        = tw_get_config_int("Extra", "Color", 0);
	extraArmour       = tw_get_config_int("Extra", "Armour", 0);

	max_ComSats = tw_get_config_int( "Extra", "Number", 0);
	num_ComSats = 0;
	ComSat = new AktunComSat*[max_ComSats];
	for (int i = 0; i < max_ComSats; i += 1) {
		ComSat[i] = NULL;
	}
}


int AktunGunner::activate_weapon()
{
	STACKTRACE;
	if (fire_special)
		return(FALSE);

	// the simplest weapon
	game->add(new AktunLaser(this, angle,
		weaponColor, weaponRange1, weaponDamage, weapon_rate,
		this, Vector2(-10, 8), this));

	// the more expensive weapon: test if there's enough energy left after the first
	// drain, then subtract the extra drain (the default drain is subracted earlier).
	if (batt >= weapon_drain + weaponDrain2) {
		game->add(new AktunLaser(this, angle,
			weaponColor, weaponRange2, weaponDamage, weapon_rate,
			this, Vector2(10, 8), this));
		batt -= weaponDrain2;
	}
	return(TRUE);
}


int AktunGunner::activate_special()
{
	STACKTRACE;
	//	if ((fire_weapon) && (batt >= extraDrain))
	//	{
	if (num_ComSats == max_ComSats) {
		num_ComSats -= 1;
		ComSat[0]->state = 0;
		memcpy(&ComSat[0], &ComSat[1], sizeof(AktunComSat*) * num_ComSats);
		ComSat[num_ComSats] = NULL;
	}
	AktunComSat *tmp = new AktunComSat(0.0, extraRange,
		extraDamage, extraFrames, extraRechargeRate, extraColor, extraArmour, this,
		data->spriteExtra);
	ComSat[num_ComSats] = tmp;
	num_ComSats += 1;
	//		batt -= extraDrain;
	game->add( tmp );
	//		return(FALSE);
	//	}

	/*
	game->add(new Missile(this,
	Vector2(0.0, size.y / 2.0), angle, specialVelocity, specialDamage, specialRange,
	specialArmour, this, data->spriteSpecial) );
	*/
	return(TRUE);
}


AktunLaser::AktunLaser(SpaceLocation *creator, double langle, int lweaponColor, double lrange, int ldamage,
int lfcount, SpaceLocation *opos, Vector2 relpos, Ship *lowner) :
Laser(creator, langle, tw_get_palete_color(lweaponColor), lrange,
ldamage, lfcount, opos, relpos, true),
owner(lowner)
{
	STACKTRACE;
}


void AktunLaser::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	sound.stop(owner->data->sampleWeapon[0]);
	Laser::inflict_damage(other);
	sound.stop((SAMPLE *)(melee[1].dat));
	sound.play(owner->data->sampleWeapon[1]);
}


AktunComSat::AktunComSat(double oangle, double orange, int odamage,
int oframes, int orechargerate, int ocolor, int oarmour, Ship *oship,
SpaceSprite *osprite) :
SpaceObject(oship, 0, 0, osprite),
ship(oship),
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
	collide_flag_anyone = ALL_LAYERS - bit(LAYER_CBODIES);
	angle = oangle;
	pos = ship->normal_pos();
	if (!(ship && ship->exists())) state = 0;
}


void AktunComSat::calculate()
{
	STACKTRACE;

	if (!(ship && ship->exists())) {
		state = 0;
		return;
	}

	SpaceObject::calculate();

	sprite_index++;
	if (sprite_index == 40)
		sprite_index = 0;

	if (lRecharge > 0) {
		lRecharge -= frame_time;
		return;
	}

	vel *= 1 - .0005 * frame_time;

	if (magnitude_sqr(vel) < 0.05 * 0.05) {
		vel = 0;
	}

	Query q;
	for (q.begin(this, OBJECT_LAYERS &~ bit(LAYER_CBODIES), lRange); q.currento; q.next()) {
		if (!q.currento->isInvisible() && !q.currento->sameTeam(this)) {
			SpaceLocation *l;
			l = new PointLaser(this, tw_get_palete_color(lColor), 1, lFrames,
				this, q.currento, 0);
			game->add(l);
			if (l->exists()) {
				sound.play(ship->data->sampleExtra[0]);
				lRecharge += lRechargeRate;
				break;
			}
			else l->state = 0;
		}
	}
	return;
}


int AktunComSat::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	return SpaceObject::canCollide(other);
}


int AktunComSat::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	double tot;
	tot = normal+direct;
	if ( tot > 0 ) {
		armour -= iround(tot);

		if (armour <= 0) {
			armour = 0;
			state = 0;
			game->add(new Animation(this, pos,
				meleedata.kaboomSprite, 0, KABOOM_FRAMES, 50, LAYER_EXPLOSIONS));
			sound.stop(data->sampleExtra[0]);
			sound.play(data->sampleExtra[0]);
		}
	}
	return 1;
}


void AktunComSat::death()
{
	STACKTRACE;
	for (int i = 0; i < ((AktunGunner*)ship)->num_ComSats; i++ ) {
		if (((AktunGunner*)ship)->ComSat[i] == this) {
			((AktunGunner*)ship)->ComSat[i] = NULL;
			((AktunGunner*)ship)->num_ComSats -= 1;
			memmove(&((AktunGunner*)ship)->ComSat[i], &((AktunGunner*)ship)->ComSat[i+1],
				(((AktunGunner*)ship)->num_ComSats-i) * sizeof(AktunComSat*));
			return;
		}
	}
}


REGISTER_SHIP(AktunGunner)
