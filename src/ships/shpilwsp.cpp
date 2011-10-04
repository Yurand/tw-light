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

#include "shpilwsp.h"

/*
#define ILWRATH_FIRE_ANIM_RATE           50
#define ILWRATH_MINE_FIRST_WHIRL_INDEX   13
// #define ILWRATH_MINE_LAST_WHIRL_INDEX    22
// it produced some artifacts
#define ILWRATH_MINE_LAST_WHIRL_INDEX    20
#define ILWRATH_MINE_SLOWDOWN_RATE       0.95
#define ILWRATH_SPECIAL_REL_X            0
#define ILWRATH_SPECIAL_REL_Y            (-size.y * 0.3)
*/

class IlwrathSpider : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	double       weaponAngle;

	double       specialVelocity;
	int          specialNumber;
	double       specialSpread;
	int          specialLifeTime;
	double       specialRandomness;
	int          specialStopTime;

	public:
		IlwrathSpider(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:
		virtual void calculate();
		virtual int  activate_weapon();
		virtual int  activate_special();
};

/*
class IlwrathSpiderMine : public SpaceObject {
	int    step;
	int    life;
	int    inc;
	double randomness;
	int    stoptime;

public:
	IlwrathSpiderMine( SpaceLocation *creator, double ox, double oy, double oangle, double v, int olife, double orandomness, int ostoptime, SpaceSprite* osprite );
	void calculate();
	void inflict_damage(SpaceObject *other);
};

class IlwrathStop : public SpaceLocation {
	SpaceObject* victim;
	int          life;
	double       old_v;

public:
	IlwrathStop( SpaceLocation* creator, SpaceObject* ovictim, int olife );
	virtual void calculate();
};
*/

IlwrathSpider::IlwrathSpider(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange       = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity    = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage      = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour      = tw_get_config_int("Weapon", "Armour", 0);
	weaponAngle       = tw_get_config_float("Weapon", "Angle", 0) * ANGLE_RATIO;
	specialVelocity   = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialNumber     = tw_get_config_int("Special", "Number", 0);
	specialSpread     = tw_get_config_float("Special", "Spread", 0) * ANGLE_RATIO;
	specialLifeTime   = tw_get_config_int("Special", "LifeTime", 0);
	specialRandomness = tw_get_config_float("Special", "Randomness", 0);
	specialStopTime   = tw_get_config_int("Special", "StopTime", 0);
}


int IlwrathSpider::activate_weapon()
{
	STACKTRACE;
	game->add( new AnimatedShot( this, Vector2(size.x*0.12, size.y*0.35),
		angle + weaponAngle, weaponVelocity, weaponDamage, weaponRange, weaponArmour, this,
		data->spriteWeapon, data->spriteWeapon->frames(), ILWRATH_FIRE_ANIM_RATE, 1 ));
	game->add( new AnimatedShot( this, Vector2(-size.x*0.12, size.y*0.35),
		angle - weaponAngle, weaponVelocity, weaponDamage, weaponRange, weaponArmour, this,
		data->spriteWeapon, data->spriteWeapon->frames(), ILWRATH_FIRE_ANIM_RATE, 1 ));

	return true;
}


int IlwrathSpider::activate_special()
{
	STACKTRACE;
	double alpha = specialSpread / specialNumber;
	double beta = normalize( angle + PI - 0.5 * specialSpread + random(alpha), PI2 );
	double tx = cos( angle );
	double ty = sin( angle );
	double ox = pos.x + ILWRATH_SPECIAL_REL_Y * tx - ILWRATH_SPECIAL_REL_X * ty;
	double oy = pos.y + ILWRATH_SPECIAL_REL_Y * ty + ILWRATH_SPECIAL_REL_X * tx;

	int i;
	for (i = 0; i < specialNumber; i++) {
		game->add( new IlwrathSpiderMine( this, ox, oy, beta + alpha * i, specialVelocity, specialLifeTime, specialRandomness, specialStopTime, data->spriteSpecial ));
	}

	return i;
}


void IlwrathSpider::calculate()
{
	STACKTRACE;
	Ship::calculate();
}


void IlwrathSpiderMine::calculate()
{
	STACKTRACE;
	double alpha = (-10 + random(21)) * ANGLE_RATIO;
	alpha *= randomness;
	angle += alpha * frame_time;

	double v = magnitude(vel);
	vel = v * unit_vector(angle);

	//depth--;
	set_depth(DEPTH_SHOTS - .01);

	if ( step <= 0 ) {
		vel *= ILWRATH_MINE_SLOWDOWN_RATE;

		step += time_ratio;
		sprite_index += inc;
		if ( sprite_index == ILWRATH_MINE_FIRST_WHIRL_INDEX ) {
			inc = 1;
		}
		if ( sprite_index == ILWRATH_MINE_LAST_WHIRL_INDEX && life > 0 ) {
			inc = -1;
		}
		if ( sprite_index == sprite->frames() ) {
			state = 0;
		}
	}else step -= frame_time;
	//  }else step -= (int)((double)frame_time*(double)(10+random() % 20)/30.0);
	// this would add random animation speed but i don't like it
	life -= frame_time;
}


void IlwrathSpiderMine::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	state = 0;

	int freq;
	if ( other->isShot() ) {
		freq = (int)(magnitude_sqr(other->get_vel())*2000.0);
	} else {
		freq = 2000 - 80*(int)other->mass;
	}
	play_sound2( data->sampleExtra[0], 512, freq<500?500:freq );
	game->add( new FixedAnimation( this, other, data->spriteExtra, 0, data->spriteExtra->frames(), time_ratio, LAYER_EXPLOSIONS ));

	if ( other->vel != 0 ) {
		other->vel = 0;
		if ( other->isShip() ) ((Ship*)other)->nextkeys &= ~(keyflag::thrust | keyflag::left | keyflag::right);

		game->add( new IlwrathStop( this, other, stoptime ));
	}
}


IlwrathSpiderMine::IlwrathSpiderMine( SpaceLocation *creator, double ox, double oy, double oangle, double v, int olife, double orandomness, int ostoptime, SpaceSprite* osprite ):
SpaceObject( creator, Vector2(ox,oy), oangle, osprite ),
step( 0 ), life( olife ), inc( 1 ),
randomness( orandomness ), stoptime( ostoptime )
{
	STACKTRACE;
	layer = LAYER_SPECIAL;
	vel = unit_vector(angle) * v;
}


IlwrathStop::IlwrathStop( SpaceLocation* creator, SpaceObject* ovictim, int olife ):
SpaceLocation( creator, 0, 0 ),
victim( ovictim ), life( olife )
{
	STACKTRACE;
	if ( victim ) {
		if ( !victim->exists() ) {
			victim = 0;
			state = 0;
		} else {
			if ( victim->isShot() ) {
				old_v = ((Shot*)victim)->v;
				((Shot*)victim)->v = 0;
			} else {
				old_v = 0;
			}
		}
	}
}


void IlwrathStop::calculate()
{
	STACKTRACE;
	SpaceLocation::calculate();
	if ( !(victim && victim->exists()) ) {
		victim = 0;
		state = 0;
		return;
	}
	if ( victim->isShip() ) ((Ship*)victim)->nextkeys &= ~(keyflag::thrust | keyflag::left | keyflag::right);
	victim->vel = 0;

	life -= frame_time;
	if ( life <= 0 ) {
		state = 0;
		if ( victim->isShot() ) {
			((Shot*)victim)->v = old_v;
			victim->vel = old_v * unit_vector(victim->get_angle());
		}
	}
}


REGISTER_SHIP(IlwrathSpider)
