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

#include "melee/mmain.h"
#include "melee/mcbodies.h"

#include "frame.h"

#define BCC 3

class ConfederationHornet : public Ship
{
	int          regenrateFrames;
	int          regenrateCount;
	int          regenrating;
	int          regenrateAmount;

	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       specialRange;
	double       specialVelocity;
	int          specialDamage;
	int          specialDDamage;
	int          specialArmour;
	double       specialTurnRate;

	int          shield_max;
	int          shield;
	int          shield_old;
	int          shield_x;
	int          shield_y;

	public:
		ConfederationHornet(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);

		Color crewPanelColor(int k = 0);
		virtual double getCrew();
};

class TorpedoMissile : public HomingMissile
{
	protected:
		int Direct_Damage;

	public:
		TorpedoMissile(double ox, double oy, double oangle, double ov, int odamage,
			int oddamage, double orange, int oarmour, double otrate, SpaceLocation *creator,
			SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget);
		virtual void inflict_damage(SpaceObject *other);
};

ConfederationHornet::ConfederationHornet(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	regenrateFrames = tw_get_config_int("Extra", "Frames", 0);
	regenrating     = FALSE;
	regenrateAmount = tw_get_config_int("Extra", "RechargeAmount", 0);

	weaponRange     = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity  = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage    = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour    = tw_get_config_int("Weapon", "Armour", 0);

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialDDamage   = tw_get_config_int("Special", "DDamage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));

	shield_max      = tw_get_config_int("Extra", "Thickness", 0);
	shield          = shield_max;
	shield_old      = 0;
	/*
	shield_x        = 8;
	shield_y        = 51;

	if (crew_max > 2) {
	  if  (((int)ceil(crew_max) % 2) == 0) {
		shield_y -= iround(crew_max) - 2;
	  } else {
		shield_y -= iround(crew_max);
	  }
	}
	*/

	crew_max = 1 + shield_max;
}


int ConfederationHornet::activate_weapon()
{
	STACKTRACE;
	add(new Missile(this, Vector2(7,35),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	add(new Missile(this, Vector2(-7,35),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}


int ConfederationHornet::activate_special()
{
	STACKTRACE;
	add(new TorpedoMissile(0.0, (size.y / 2.0),
		angle, specialVelocity, specialDamage, specialDDamage, specialRange, specialArmour, specialTurnRate, this, this, data->spriteSpecial, target));
	return(TRUE);
}


void ConfederationHornet::calculate()
{
	STACKTRACE;

	//   int shield_color = 9; // Blue
	//   int i, bar_x, bar_y, shield_panel;

	if (regenrating) {
		if (shield < shield_max) {
			if ((regenrateCount -= frame_time) < 0) {
				shield_old = shield;
				shield += regenrateAmount;
				if (shield > shield_max) shield = shield_max;
				regenrateCount = regenrateFrames;
			}
		} else regenrating = FALSE;
	}
	else if (!(regenrating) && (shield < shield_max)) {
		regenrating = TRUE;
		regenrateCount = regenrateFrames;
	}
	/*
	if (shield != shield_old) {
	 bar_x = 0;
	 bar_y = 0;
	 shield_panel = ((shield*8)/shield_max) + 8;
		 BITMAP *bmp = spritePanel->get_bitmap(0);
	 blit(ship->spritePanel->get_bitmap(shield_panel), bmp, 0, 0, 13, 14, 50, 39);
	 for(i = 0; i < shield_max; i++) {
		if ((i - shield) < 0) {
		 tw_putpixel(bmp, shield_x + bar_x, shield_y + bar_y, pallete_color[shield_color]);
		 tw_putpixel(bmp, shield_x + bar_x + 1, shield_y + bar_y, pallete_color[shield_color]);
	   } else {
		 tw_putpixel(bmp, shield_x + bar_x, shield_y + bar_y, 0);
		 tw_putpixel(bmp, shield_x + bar_x + 1, shield_y + bar_y, 0);
	   }
	   if ((i % 2) == 0)
		 bar_x = -3;
	   else {
		 bar_x = 0;
		 bar_y -= 2;
	   }
	 }
	 ship->update_panel = TRUE;
	 shield_old = shield;
	}
	*/
	Ship::calculate();

	crew = 1 + shield;
}


int ConfederationHornet::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if ((normal - shield) <= 0) {
		shield -= iround(normal);
		normal = 0;
	} else {
		normal -= shield;
		shield = 0;
	}

	return Ship::handle_damage(source, normal, direct);
}


Color ConfederationHornet::crewPanelColor(int k)
{
	STACKTRACE;
	Color c1 = {0,255,0};
	Color c2 = {				 // blue
		0,0,255
	};

	if ( k == 0 )
		return c1;
	else
		return c2;
}


// returns the "real" live crew (this can fool the Syreen).
double ConfederationHornet::getCrew()
{
	STACKTRACE;
	if (crew)
		return 1;
	else
		return 0;
}


/** Torpedo Definitions ************************************************************/

TorpedoMissile::TorpedoMissile(double ox, double oy, double oangle, double ov,
int odamage, int oddamage, double orange, int oarmour, double otrate, SpaceLocation *creator,
SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget) :
HomingMissile(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otrate, opos, osprite, otarget),
Direct_Damage(oddamage)
{
	STACKTRACE;
}


void TorpedoMissile::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	damage(other, 0, Direct_Damage);
	HomingMissile::inflict_damage(other);
}


/** End Torpedo Definitions ********************************************************/

REGISTER_SHIP(ConfederationHornet)
