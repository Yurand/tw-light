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

#include "shpchebr.h"

#define MAX_DOGIS 4

ChenjesuBroodhome::ChenjesuBroodhome(Vector2 opos, double angle, ShipData *data, unsigned int code) :
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponDamage        = tw_get_config_int("Weapon", "Damage", 0);
	shardRange          = scale_range(tw_get_config_int("Weapon", "ShardRange", 0));
	shardDamage         = tw_get_config_int("Weapon", "ShardDamage", 0);
	shardArmour         = tw_get_config_int("Weapon", "ShardArmour", 0);
	shardRelativity     = tw_get_config_float("Weapon", "ShardRelativity", 0);
	shardRotation       = (tw_get_config_int("Weapon", "ShardRotation", 0) > 0);
	weaponArmour        = tw_get_config_int("Weapon", "Armour", 0);
	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponFired         = FALSE;
	weaponObject        = NULL;

	specialVelocity     = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage       = tw_get_config_int("Special", "Damage", 0);
	specialFuelSap      = tw_get_config_int("Special", "FuelSap", 0);
	specialArmour       = tw_get_config_int("Special", "Armour", 0);
	specialAccelRate    = scale_acceleration(tw_get_config_float("Special", "AccelRate", 0), 1);
	specialMass         = tw_get_config_float("Special", "Mass", 0);
	specialNumDOGIs     = 0;
	specialAvoidanceAngle   = tw_get_config_float("Special", "AvoidanceAngle", 0) * ANGLE_RATIO;
	specialAvoidanceFactor  = tw_get_config_float("Special", "AvoidanceFactor", 0);
}


int ChenjesuBroodhome::activate_weapon()
{
	STACKTRACE;
	if (weaponFired)
		return(FALSE);
	weaponObject = new ChenjesuShot(Vector2(0.0, (size.y / 2.0)), angle,
		weaponVelocity, iround(weaponDamage), iround(weaponArmour), this, data->spriteWeapon);
	game->add(weaponObject);
	weaponFired = TRUE;
	return(TRUE);
}


int ChenjesuBroodhome::activate_special()
{
	STACKTRACE;
	if (specialNumDOGIs >= MAX_DOGIS)
		return(FALSE);
	ChenjesuDOGI* DOGI;			 //added by Varith
	DOGI = (new ChenjesuDOGI( Vector2(0.0, -size.y / 1.5),
		angle + PI, specialVelocity, iround(specialFuelSap), iround(specialArmour),
		specialAccelRate, specialMass, this, data->spriteSpecial,
		&specialNumDOGIs));
	DOGI->avoidanceFactor = specialAvoidanceFactor;
	DOGI->avoidanceAngle = specialAvoidanceAngle;
	game->add(DOGI);
	return(TRUE);
}


void ChenjesuBroodhome::calculate()
{
	STACKTRACE;
	int i;

	Ship::calculate();

	if ((weaponObject != NULL) && (!weaponObject->exists()))
		weaponObject = NULL;
	if (weaponFired && (!fire_weapon))
		weaponFired = FALSE;
	if ((weaponObject != NULL) && (!fire_weapon)) {
		for(i = 0; i < 8; i++) {
			Shot *shot = new Missile(weaponObject, Vector2(0.0, 0.0),
				(shardRotation?weaponObject->get_angle():0) +   PI/4 * (double)(i),
				weaponVelocity, shardDamage, shardRange, shardArmour,
				weaponObject, data->spriteExtra, shardRelativity);
			shot->collide_flag_sameship = bit(LAYER_SPECIAL);
			game->add(shot);
		}
		//		weaponObject->play_sound2(data->sampleExtra[0]);
		weaponObject->play_sound(data->sampleExtra[0]);
		weaponObject->destroy();
		weaponObject = NULL;
	}
	return;
}


ChenjesuShot::ChenjesuShot(Vector2 opos, double oangle, double ov,
int odamage, int oarmour, SpaceLocation *creator, SpaceSprite *osprite) :
Missile(creator, opos, oangle, ov, odamage, -1.0, oarmour, creator, osprite)
{
	STACKTRACE;
	collide_flag_sameship = bit(LAYER_SPECIAL);
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 20;
	explosionFrameSize  = 50;
}


void ChenjesuShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	Shot::inflict_damage(other);
	if ((other->mass > 0) && !exists())
		if (other-exists())
			game->add(new FixedAnimation(this, other,
				explosionSprite, 0, explosionFrameCount,
				explosionFrameSize, DEPTH_EXPLOSIONS));
	else
		game->add(new Animation(this, pos,
			explosionSprite, 0, explosionFrameCount,
			explosionFrameSize, DEPTH_EXPLOSIONS));
}


void ChenjesuShot::animateExplosion()
{
	STACKTRACE;
	return;
}


ChenjesuDOGI::ChenjesuDOGI(Vector2 opos, double ov, double oangle,
int fuel_sap, int oarmour, double accel, double omass, Ship *oship,
SpaceSprite *osprite, int *onum_dogis) :
AnimatedShot(oship, opos, ov, oangle, ((ChenjesuBroodhome*)oship)->specialDamage, -1.0, oarmour, oship, osprite, 64, 50),
sap_factor(fuel_sap), accel_rate(accel), num_dogis(onum_dogis)
{
	STACKTRACE;
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);
	mass = omass;
	collide_flag_sameship = ALL_LAYERS;
	(*num_dogis)++;
}


void ChenjesuDOGI::calculate()
{
	STACKTRACE;
	AnimatedShot::calculate();

	if (ship && ship->exists()) {
		target = ship->target;
	} else {
		state = 0;
		return;
	}

	if (target && !target->isInvisible()) {
		angle = trajectory_angle(target);

		double ra = normalize(target->get_angle() - (angle - PI), PI2);
		if (ra > PI) ra -= PI2;

		if (fabs(ra) < avoidanceAngle)
			angle += PI/2 * sign(ra) * pow((1 - fabs(ra)/avoidanceAngle), 1/avoidanceFactor);

		normalize(angle, PI2);
	}

	accelerate (this, angle, accel_rate * frame_time, v);
}


void ChenjesuDOGI::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	play_sound(data->sampleExtra[1]);

	if (sameTeam(other)) return;
	if (!other->isShip())
		return;

	other->handle_fuel_sap(this, sap_factor);
	double a = trajectory_angle(other);
	accelerate(this, a, -v, MAX_SPEED);
	if (other->mass > mass/100.0)
		other->accelerate(this, a, v*3*mass/other->mass, v);
	return;
}


void ChenjesuDOGI::death()
{
	STACKTRACE;
	if (num_dogis) (*num_dogis)--;
}


void ChenjesuDOGI::ship_died()
{
	STACKTRACE;
	num_dogis = NULL;
	SpaceObject::ship_died();
	state = 0;
	return;
}


void ChenjesuDOGI::animateExplosion()
{
	STACKTRACE;
	game->add(new Animation(this, pos,
		data->spriteSpecialExplosion, 0, 20, 25, DEPTH_EXPLOSIONS));
}


void ChenjesuDOGI::soundExplosion()
{
	STACKTRACE;
	play_sound2(data->sampleExtra[2]);
	return;
}


REGISTER_SHIP(ChenjesuBroodhome)
