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

/*
Yet another cobbled-together ship from spare parts of other ships:
Cruiser, Avatar, Xform, Hornet
-Culture20
*/
#include "ship.h"
REGISTER_FILE

#include "melee/mmain.h"
#include "melee/mcbodies.h"

#include "melee/mshppan.h"

#include "frame.h"

#define BCC 3

class DjinniLancer : public Ship
{
	int          regenWaitFrames;//wait this many frames after using batt before even starting to regenerate shields
	int          regenWaitCount;
	int          regenrateFrames;//wait this many frames between shield regens
	int          regenrateCount;
	int          regenrating;
	int          regenrateAmount;

	int    weaponColor;
	double weaponRange;
	int    weaponFrames;
	int    weaponDamage;

	int    specialColor;
	double specialRange;
	int    specialFrames;
	int    specialDamage;

	double          shield_max;
	double          shield;
	double          shield_old;

	double       crew_old, crew_real;

	SpaceLine *laser1;
	SpaceLine *laser2;
	int        laserSpark;
	double     laserAngle;

	public:
		DjinniLancer(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);

		Color crewPanelColor(int k = 0);

};

DjinniLancer::DjinniLancer(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code),
laser1(NULL),
laser2(NULL),
laserSpark(0),
laserAngle(0)
{
	STACKTRACE;

	weaponColor  = tw_get_config_int("Weapon", "Color", 0);
	weaponRange  = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponFrames = tw_get_config_int("Weapon", "Frames", 0);
	weaponDamage = tw_get_config_int("Weapon", "Damage", 0);

	laserAngle = asin(4 / weaponRange);

	specialColor  = tw_get_config_int("Special", "Color", 0);
	specialRange  = scale_range(tw_get_config_float("Special", "Range", 0));
	specialFrames = tw_get_config_int("Special", "Frames", 0);
	specialDamage = tw_get_config_int("Special", "Damage", 0);

	regenWaitFrames = tw_get_config_int("Extra", "WaitFrames", 0);
	regenWaitCount  = regenWaitFrames;
	regenrateFrames = tw_get_config_int("Extra", "Frames", 0);
	regenrateCount  = regenrateFrames;
	regenrating     = FALSE;
	regenrateAmount = tw_get_config_int("Extra", "RechargeAmount", 0);

	shield_max      = tw_get_config_int("Extra", "Thickness", 0);
	shield          = shield_max;
	shield_old      = 0;

	crew_real       = crew;

	crew = crew_real + shield;

	crew_old        = 0;

	ship->update_panel = true;

}


void DjinniLancer::calculate()
{
	STACKTRACE;
	if (laserSpark > 0) {
		laserSpark-= frame_time;
		if ((laserSpark <= 0) && (laserSpark > -frame_time)) {
			if ((laser1->damage_factor > 0) && (laser2->damage_factor > 0))
				add(new Animation(this,
					laser1->normal_pos() + laser1->edge(),
					meleedata.sparkSprite, 0, SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
			laser1 = NULL;
			laser2 = NULL;
		}
	}

	// this should always occur, it's a bit strange if countdown of wait time only

	// starts after that you take damage (and shield isn't max) ?!

	if (regenWaitCount >= 0)

		regenWaitCount -= frame_time;

	if (regenrating) {
		if ((regenWaitCount) < 0) {
			if (shield < shield_max) {
				if ((regenrateCount -= frame_time) < 0) {
					shield_old = shield;
					shield += regenrateAmount;
					if (shield > shield_max) {
						shield = shield_max;
					}
					regenrateCount = regenrateFrames;
				}
			} else {
				regenrating = FALSE;
			}
		}
	}
	else if (!(regenrating) && (shield < shield_max)) {
		regenrating = TRUE;
		regenrateCount = regenrateFrames;
	}

	crew = crew_real + shield;

	if (shield != shield_old || crew_old != crew) {

		update_panel = true;

		crew_old = crew;

		shield_old = shield;
	}

	Ship::calculate();

}


int DjinniLancer::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	if (normal > 0) {
		if ((normal - shield) <= 0) {
			shield -= normal;
			normal = 0;
		} else {
			normal -= shield;
			shield = 0;
		}
	}

	crew = crew_real + shield;
	int k = Ship::handle_damage(source, normal, direct);

	// below "real" level you have lost real crew
	if (crew < crew_real)
		crew_real = crew;

	// shield is what is remaining.
	shield = crew - crew_real;

	return k;
}


int DjinniLancer::activate_weapon()
{
	STACKTRACE;
	add(laser1 = new Laser(this, angle + laserAngle,
		tw_get_palete_color(weaponColor), weaponRange, weaponDamage, weapon_rate, this,
		Vector2(-4.0, 2.0), true));
	add(laser2 = new Laser(this, angle - laserAngle,
		tw_get_palete_color(weaponColor), weaponRange, weaponDamage, weapon_rate, this,
		Vector2(4.0, 2.0), true));
	laserSpark = weapon_rate;
	regenWaitCount = regenWaitFrames;
	regenrateCount = regenrateFrames;
	return(TRUE);
}


int DjinniLancer::activate_special()
{
	STACKTRACE;
	int fire = FALSE;
	SpaceObject *o;

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
	bit(LAYER_CBODIES), specialRange); a.current; a.next()) {
		o = a.currento;
		if ( (!o->isInvisible()) && !o->sameTeam(this) && (o->collide_flag_anyone & bit(LAYER_LINES))) {
			SpaceLocation *l = new PointLaser(this, tw_get_palete_color(specialColor), 1,
				specialFrames, this, o, Vector2(0.0, 10.0));
			game->add(l);
			if (l->exists()) {
				fire = TRUE;
				l->set_depth(LAYER_EXPLOSIONS);
			}
		}
	}
	if (fire) {

		play_sound((SAMPLE *)(melee[MELEE_BOOM + 0].dat));

		// only wait regenerating, if the weapon is used.
		regenWaitCount = regenWaitFrames;
		regenrateCount = regenrateFrames;

	}
	return(fire);
}


Color DjinniLancer::crewPanelColor(int k)
{
	STACKTRACE;

	Color c1 = {32,164,32};
	Color c2 = {64,64,255};

	if (k < crew_real)			 // it starts counting captain as 0
		return c1;
	else
		return c2;

}


REGISTER_SHIP(DjinniLancer)
