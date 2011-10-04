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

#include <assert.h>
#include "ship.h"
REGISTER_FILE

#include "frame.h"

class DraxMine;

class DraxGryphon : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	int          weaponoffset;
	double       weaponRelativity;

	double       specialRange;
	double       specialVelocity;
	int          specialDamage;
	int          specialArmour;
	int          specialTimer;
	double       specialRadius;
	int          numMines;
	int          maxMines;
	double       specialSeek;
	double       specialVSeek;

	DraxMine  **weaponObject;

	public:
		DraxGryphon(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:
		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
};

class DraxMine : public AnimatedShot
{

	double       MineRange;
	int          MineTimer;
	double       MineRadius;
	int          MineMoving;
	double       MineSeek;
	double       MineSeekVel;

	public:
		DraxMine(double ox, double oy,double ov, double oangle, int odamage,
			int oarmour, Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize,
			double miner, int minet, double minera, double mines, double minesv);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

DraxGryphon::DraxGryphon(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponoffset   = 0;
	weaponRelativity = tw_get_config_float("Weapon", "Relativity", 0);
	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	specialTimer    = tw_get_config_int("Special","Timer", 0);
	specialRadius   = scale_range(tw_get_config_float("Special", "Radius", 0));
	specialSeek     = scale_range(tw_get_config_float("Special", "Seek", 0));
	specialVSeek    = scale_velocity(tw_get_config_float("Special", "VelSeek", 0));
	numMines        = 0;
	maxMines        = tw_get_config_int("Special", "Number", 1);
	weaponObject = new DraxMine*[maxMines];
	for (int i = 0; i < maxMines; i += 1) {
		weaponObject[i] = NULL;
	}

}


int DraxGryphon::activate_weapon()
{
	STACKTRACE;
	weaponoffset++;

	if (weaponoffset > 3)
		weaponoffset = 0;

	add(new Missile(this, Vector2((size.x/4)+(weaponoffset*4),
		(size.y*.25)),angle, weaponVelocity, weaponDamage,
		weaponRange, weaponArmour,this, data->spriteWeapon, weaponRelativity));
	add(new Missile(this, Vector2((-size.x/4)-(weaponoffset*4),
		(size.y*.25)),angle, weaponVelocity, weaponDamage,
		weaponRange, weaponArmour,this, data->spriteWeapon, weaponRelativity));
	return(TRUE);
}


int DraxGryphon::activate_special()
{
	STACKTRACE;

	if (numMines == maxMines) {
		weaponObject[0]->state = 0;
		numMines -= 1;
		for (int i = 0; i < numMines; i += 1) {
			weaponObject[i] = weaponObject[i + 1];
		}
		weaponObject[numMines] = NULL;
	}
	weaponObject[numMines] = new DraxMine(0.0, -(size.y / 2.0),specialVelocity, (angle + PI),
		specialDamage, specialArmour, this, data->spriteSpecial, 32, 20, specialRange,
		specialTimer, specialRadius, specialSeek, specialVSeek);
	add(weaponObject[numMines]);
	numMines += 1;
	return(TRUE);
}


void DraxGryphon::calculate()
{
	STACKTRACE;

	int j = 0;
	for (int i = 0; i < numMines; i += 1) {

		weaponObject[i-j] = weaponObject[i];
		if (!weaponObject[i]->exists()) j += 1;
		if (j) weaponObject[i] = NULL;
	}
	numMines -= j;

	Ship::calculate();
}


DraxMine::DraxMine(double ox,double oy,double ov, double oangle, int odamage, int oarmour,
Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize, double miner,
int minet, double minera, double mines, double minesv) :
AnimatedShot(oship, Vector2(ox,oy), oangle, ov, odamage, -1.0, oarmour, oship,
osprite, ofcount, ofsize)
{
	STACKTRACE;
	MineMoving = TRUE;
	MineRange = miner;
	MineTimer = minet;
	MineRadius = minera;
	MineSeek = mines;
	MineSeekVel = minesv;

	isblockingweapons = false;
}


void DraxMine::calculate()
{
	STACKTRACE;

	AnimatedShot::calculate();

	if (!exists()) return;

	// this is only needed to make the mine move a short distance away
	// from the ship, just after it's spawned.
	if (MineMoving) {
		MineRange -= frame_time;
		if (MineRange <= 0) {
			MineMoving = FALSE;
			AnimatedShot::stop();
		}
	}

	Query a;
	double r=99999;
	int Seek=FALSE;

	// re-direct homingmissiles
	for (a.begin(this, bit(LAYER_SHOTS),MineRadius); a.current; a.next()) {
		if (a.currento->isShot()) {
			Shot *o = NULL;
			o = (Shot *) a.currento;
			if ((distance(o) < MineRadius) && (o->canCollide(this))
				&& (o->isHomingMissile()))
				((HomingMissile *) o)->target = this;
		}
	}

	// find the closest shot(f) which the mine seeks out (if it is visible).
	Shot *f = NULL;
	for (a.begin(this, bit(LAYER_SHOTS),MineSeek); a.current; a.next()) {
		if (a.current->exists() && a.currento->isShot()) {
			if ((distance(a.current) < r) && (!a.current->sameTeam(this)) &&
			(!a.current->isInvisible())) {
				f = (Shot *) a.currento;
				r = distance(f);
				Seek = TRUE;
			}
		}
	}

	// only become active after the delay factor, when it's moved away from
	// the ship.
	if (!MineMoving) {
		if (Seek) {
			angle = intercept_angle2(pos, 0, MineSeekVel,
				f->normal_pos(), f->get_vel());
			v = MineSeekVel;
			vel = MineSeekVel * unit_vector(angle);

		} else {
			AnimatedShot::stop();
		}
	}

	MineTimer -= frame_time;
	if (MineTimer < 0)
		state = 0;
	return;
}


void DraxMine::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->isShip())
		damage_factor = 0;

	AnimatedShot::inflict_damage(other);
	return;
}


REGISTER_SHIP(DraxGryphon)
