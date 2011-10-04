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
#include "melee/mview.h"
#include "other/nullphas.h"

#define BIPOLE_STABILITY 0.4

class BipoleShip;

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class BipoleKatamaran : public Ship
{
	// abstract class that is instanciated at start and is coordinating movement afterwards

	double       specialIncreaseRate;
	double       specialDecreaseRate;
	double       specialDamage;
	int          specialColor;
	double       specialMinRange;
	double       specialMaxRange;

	double       extraBackThrust;// the ships accelerate backwards with this ratio
	double       extraMaxSpin;	 // a value attempting to regulate spinning

	double       range;
	double       residualDamage;

	double       od, oangle;

	BipoleShip*  left;
	BipoleShip*  right;

	public:
		BipoleKatamaran(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

								 // this ship should not be percievable in any way
		virtual double  isInvisible() const;
								 // fires the weapons of the ships
		virtual int  activate_weapon();
								 // increases range
		virtual int  activate_special();
								 // does not appear
		virtual void animate(Frame* space);
		virtual void calculate();// coordinates ship movement and places laser beam
								 // accelerates both ships
		virtual void calculate_thrust();
								 // accelerates right ship and deccelerates left
		virtual void calculate_turn_left();
								 // accelerates left ship and deccelerates right
		virtual void calculate_turn_right();
								 // does not leave hotspots
		virtual void calculate_hotspots();
								 // remove itself as a target
		virtual void materialize();
		//virtual int  exists();               // returns if at least one ship exists
		//note from orz : exists() cannot be overriden
		virtual void death();	 // kill ships too

								 // returns a BipolePhaser
		virtual SpaceLocation *get_ship_phaser();
};

class BipoleShip : public Ship
{
	// one of the two sub-ships making up the Katamaran

	friend class BipoleKatamaran;

	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       specialBrakes;

	protected:
		bool         shooting;
		Ship         *parent;

	public:
		Ship *sibling;

		BipoleShip(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code, Ship* oparent, SpaceSprite* osprite);

								 // shoots with every second call
		virtual int activate_weapon();
								 // deccelerates
		virtual int activate_special();
								 // works if on our own
		virtual void calculate_fire_weapon();
								 // works if on our own
		virtual void calculate_fire_special();
								 // works if on our own
		virtual void calculate_thrust();
								 // works if on our own
		virtual void calculate_turn_left();
								 // works if on our own
		virtual void calculate_turn_right();
		//  virtual void calculate_hotspots();
		virtual int handle_damage( SpaceLocation* source, double normal, double direct );
		virtual void destroyed( SpaceLocation* source );

		virtual ShipType *get_shiptype();
};

SpaceLocation* BipoleKatamaran::get_ship_phaser()
{
	STACKTRACE;
	return new NullPhaser( this );
}


BipoleKatamaran::BipoleKatamaran(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;

	specialIncreaseRate = tw_get_config_float("Special", "IncreaseRate", 0);
	specialDecreaseRate = tw_get_config_float("Special", "DecreaseRate", 0);
	specialMinRange = tw_get_config_int("Special", "MinRange", 0);
	specialMaxRange = tw_get_config_int("Special", "MaxRange", 0);
	specialDamage = tw_get_config_float("Special", "Damage", 0);
	specialColor = tw_get_config_int("Special", "Color", 0);

	extraBackThrust = tw_get_config_float("Extra", "BackThrust", 0);
	extraMaxSpin = scale_turning(tw_get_config_float("Extra", "MaxSpin", 0)) * ANGLE_RATIO;

	crew_max *= 2;
	batt_max *= 2;
	//  weapon_drain *= 2;
	special_drain *= 2;

	range = specialMinRange;
	residualDamage = 0;
	od = range;
	oangle = angle;

	left = new BipoleShip( pos + (range/2)*unit_vector(angle-90),
	/*pos.x+(range/2)*sin( angle ),
		 pos.y-(range/2)*cos( angle ),*/
		angle, data, code, this, sprite );
	right = new BipoleShip(  pos + (range/2)*unit_vector(angle+90),
	/*pos.x-(range/2)*sin( angle ),
		 pos.y+(range/2)*cos( angle ),*/
		angle, data, code, this, data->spriteSpecial );
	left->change_owner( this );
	right->change_owner( this );
	targets->add( left );
	targets->add( right );
	left->shooting = true;
	id = 0;						 /* this is nothing */

	left->sibling = right;
	right->sibling = left;
}


double BipoleKatamaran::isInvisible() const
{
	return 2;
}


int BipoleKatamaran::activate_weapon()
{
	STACKTRACE;
	if ( !left || !right ) return FALSE;
	weapon_sample = random(11);
	int left_w = left->activate_weapon();
	int right_w = right->activate_weapon();
	return left_w || right_w;
}


int BipoleKatamaran::activate_special()
{
	STACKTRACE;
	if ( !left || !right ) return FALSE;
	if ( range >= specialMaxRange ) {
		range = specialMaxRange;
		return FALSE;
	}
	range += specialIncreaseRate;
	return TRUE;
}


void BipoleKatamaran::animate(Frame* space)
{
	STACKTRACE;
	/* we do not show */
	return;
}


void BipoleKatamaran::calculate()
{
	STACKTRACE;
	if ( left ) if ( !left->exists() ) {
		left = NULL;
		if ( right ) {
			right->sibling = NULL;
			BITMAP *bmp = spritePanel->get_bitmap(0);
			tw_blit( spritePanel->get_bitmap(8), bmp, 0, 0, 0, 0, 63, 99);
			update_panel = TRUE;
		}
	}
	if ( right ) if ( !right->exists() ) {
		right = NULL;
		if ( left ) {
			left->sibling = NULL;
			Surface *bmp = spritePanel->get_bitmap(0);
			tw_blit( spritePanel->get_bitmap(7), bmp, 0, 0, 0, 0, 63, 99);
			update_panel = TRUE;
		}
	}
	if ( !left && !right ) {
		state = 0;
		game->ship_died( this, control->target );
		//    Ship::calculate();
		return;
	}
	if ( !right ) {
		batt = left->batt;
		crew = left->crew;
		pos = left->normal_pos();
		angle = left->angle;
		Ship::calculate();
		return;
	}
	if ( !left ) {
		batt = right->batt;
		crew = right->crew;
		pos = right->normal_pos();
		angle = right->angle;
		Ship::calculate();
		return;
	}
	crew = left->crew + right->crew;
	batt = left->batt + right->batt;

	oangle = angle;
	angle = normalize( left->trajectory_angle( right ) - PI/2, PI2 );
	left->angle = angle;
	right->angle = angle;
	double d = left->distance( right );
	//  x = left->normal_x() + 0.5*d*cos( angle+PI/2 );
	//  y = left->normal_y() + 0.5*d*sin( angle+PI/2 );
	pos = left->normal_pos() + 0.5 * d * unit_vector(angle+PI/2);

	int o_recharge_step = recharge_step;
	recharge_step = frame_time;
	Ship::calculate();
	recharge_step = o_recharge_step;

	//  left->nextkeys &= keyflag::thrust | keyflag::turn_left | keyflag::turn_right;
	//  right->nextkeys &= keyflag::thrust | keyflag::turn_left | keyflag::turn_right;

	left->batt = batt / 2;
	right->batt = batt - left->batt;

	if ( range > specialMinRange ) range -= specialDecreaseRate * frame_time;
	if ( range < specialMinRange ) range = specialMinRange;
	if ( range > specialMaxRange ) range = specialMaxRange;

	/* calculate frame_time indepentent damage */
	int dmg = (int)(specialDamage * frame_time);
	residualDamage += specialDamage * (double)frame_time - (int)(specialDamage * frame_time);
	if ( residualDamage >= 1.0 ) {
		dmg += 1;
		residualDamage -= 1.0;
	}
	/* place the laser */
	game->add(new Laser( this, angle + PI/2,
		tw_get_palete_color(specialColor), d, dmg, 1,
		left, Vector2(0,0), true ));
	/* it goes the other way too -- without the damage */
	game->add(new Laser( this, angle - PI/2,
		tw_get_palete_color(specialColor), d, 0, 1,
		right, Vector2(0,0), true ));

	/* place the laser */
	game->add(new Laser( this, angle + PI/2,
		tw_get_palete_color(specialColor), d, dmg, 1,
		left, Vector2(0,10), true ));
	/* it goes the other way too -- without the damage */
	game->add(new Laser( this, angle - PI/2,
		tw_get_palete_color(specialColor), d, 0, 1,
		right, Vector2(0,10), true ));

	/* we try to do something against spinning wildly */
	double dangle = angle - oangle;
	if ( dangle < -PI ) dangle += PI2;
	if ( dangle > PI ) dangle -= PI2;

	if (( !turn_left && !turn_right )||( dangle > extraMaxSpin*frame_time || dangle < -extraMaxSpin*frame_time )) {
		left->accelerate(this, left->get_angle(), -dangle*(180/PI)*0.1*accel_rate * frame_time, 0.5*speed_max);
		right->accelerate(this, right->get_angle(), dangle*(180/PI)*0.1*accel_rate * frame_time, 0.5*speed_max);
	}

	/*double decay = 0.001;
	if (game->frame_number & 1) {
		  left->vx = (decay * -frame_time + 1) * ( left->vx - right->vx ) + right->vx;
		  left->vy = (decay * -frame_time + 1) * ( left->vy - right->vy ) + right->vy;
	} else {
		  right->vx = (decay * -frame_time + 1) * ( right->vx - left->vx ) + left->vx;
		  right->vy = (decay * -frame_time + 1) * ( right->vy - left->vy ) + left->vy;
	}*/

	// we try to stabilize the distance of the two ships around _range_
	d = left->distance( right ) - range;
	double dd = d - od;
	od = d;
	if ( d > 0.0 ) {
		if ( dd < d ) {
			left->accelerate( this, left->trajectory_angle( right ), BIPOLE_STABILITY*d*accel_rate*frame_time, speed_max );
			right->accelerate( this, right->trajectory_angle( left ), BIPOLE_STABILITY*d*accel_rate*frame_time, speed_max );
		}
		else if ( dd > d ) {
			left->accelerate( this, right->trajectory_angle( left ), BIPOLE_STABILITY*dd*accel_rate*frame_time, speed_max );
			right->accelerate( this, left->trajectory_angle( right ), BIPOLE_STABILITY*dd*accel_rate*frame_time, speed_max );
		}
	}
	else if ( d < 0.0 ) {
		if ( dd < d ) {
			left->accelerate( this, right->trajectory_angle( left ), BIPOLE_STABILITY*d*accel_rate*frame_time, MAX_SPEED );
			right->accelerate( this, left->trajectory_angle( right ), BIPOLE_STABILITY*d*accel_rate*frame_time, MAX_SPEED );
		}
		else if ( dd > d ) {
			left->accelerate( this, left->trajectory_angle( right ), BIPOLE_STABILITY*dd*accel_rate*frame_time, MAX_SPEED );
			right->accelerate( this, right->trajectory_angle( left ), BIPOLE_STABILITY*dd*accel_rate*frame_time, MAX_SPEED );
		}
	}

}


void BipoleKatamaran::calculate_thrust()
{
	STACKTRACE;
	if ( !left || !right ) return;
	if ( left->thrust && !left->turn_left && !left->turn_right ) {
		left->accelerate(this, angle, accel_rate * frame_time, speed_max);
	}
	if ( right->thrust && !right->turn_left && !right->turn_right ) {
		right->accelerate(this, angle, accel_rate * frame_time, speed_max);
	}
}


void BipoleKatamaran::calculate_turn_left()
{
	STACKTRACE;
	if ( !left || !right ) return;
	if ( left->turn_left ) {
		left->nextkeys &= ~keyflag::thrust;
		left->accelerate(this, left->get_angle(), -extraBackThrust * accel_rate * frame_time, speed_max);
	}
	if ( right->turn_left ) {
		right->nextkeys |= keyflag::thrust;
		right->accelerate(this, right->get_angle(), accel_rate * frame_time, speed_max);
	}
}


void BipoleKatamaran::calculate_turn_right()
{
	STACKTRACE;
	if ( !left || !right ) return;
	if ( left->turn_right ) {
		left->nextkeys |= keyflag::thrust;
		left->accelerate(this, left->get_angle(), accel_rate * frame_time, speed_max);
	}
	if ( right->turn_right ) {
		right->nextkeys &= ~keyflag::thrust;
		right->accelerate(this, right->get_angle(), -extraBackThrust * accel_rate * frame_time, speed_max);
	}
}


void BipoleKatamaran::calculate_hotspots()
{
	STACKTRACE;
	return;
}


void BipoleKatamaran::materialize()
{
	STACKTRACE;

	//  int i;
	//  for( i = 0; game->target[i] != this; i++ );
	//  game->num_targets--;
	//  game->target[i] = game->target[game->num_targets];
	targets->rem(this);
	left->control = control;
	right->control = control;
	game->add( left->get_ship_phaser() );
	game->add( right->get_ship_phaser() );
}


void BipoleKatamaran::death()
{
	STACKTRACE;
	// bug fix Geo.
	// added exists(), because I *think* that the "invisible" ships can be destroyed
	// by some field weapon ? At the same time, this field weapon can destroy the
	// "left" or "right". Then, it's possible that both ships die without each knowing
	// it...
	if ( left && left->exists() ) left->die();
	if ( right && right->exists() ) right->die();
}


BipoleShip::BipoleShip(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code, Ship* oparent, SpaceSprite* osprite) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	parent = oparent;
	sprite = osprite;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	double extraBackThrust = tw_get_config_float("Extra", "BackThrust", 0);
	specialBrakes = accel_rate * extraBackThrust;

	shooting = false;
}


int BipoleShip::activate_weapon()
{
	STACKTRACE;
	shooting = !shooting;
								 // if we would return TRUE single ship would fire slower
	if ( !shooting ) return FALSE;

	weapon_sample = random(11);
	game->add( new Missile( this, Vector2(0.0,size.y/2.0), angle, weaponVelocity, weaponDamage,
		weaponRange, weaponArmour, this, data->spriteWeapon ));

	return TRUE;
}


int BipoleShip::activate_special()
{
	STACKTRACE;

	if ( vel != 0 ) {
		Vector2 oldvel = vel;
		double a = vel.atan();
		accelerate( this, a, -specialBrakes * frame_time, speed_max );
								 // this detects overshoot.
		if ( vel.dot(oldvel) < 0 )
			vel = 0;
	}

	return TRUE;
}


void BipoleShip::calculate_fire_weapon()
{
	STACKTRACE;
	if ( !sibling ) Ship::calculate_fire_weapon();
}


void BipoleShip::calculate_fire_special()
{
	STACKTRACE;
	if ( !sibling ) Ship::calculate_fire_special();
}


void BipoleShip::calculate_thrust()
{
	STACKTRACE;
	if ( !sibling ) Ship::calculate_thrust();
}


void BipoleShip::calculate_turn_left()
{
	STACKTRACE;
	if ( !sibling ) Ship::calculate_turn_left();
}


void BipoleShip::calculate_turn_right()
{
	STACKTRACE;
	if ( !sibling ) Ship::calculate_turn_right();
}


/*void BipoleShip::calculate_hotspots() {
	STACKTRACE;
  STACKTRACE;
  if ( thrust &&( hotspot_frame <= 0 )){
	game->addItem(new Animation( this,
	  normal_x() - (cos(angle ) * w / 2.5),
	  normal_y() - (sin(angle ) * h / 2.5),
	  game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, LAYER_HOTSPOTS));
	hotspot_frame += hotspot_rate;
  }
  if (hotspot_frame > 0) hotspot_frame -= frame_time;
}*/

int BipoleShip::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int s = exists();
	int i = Ship::handle_damage(source, normal, direct);
	if (s && !exists()) destroyed( source );
	return i;
}


void BipoleShip::destroyed( SpaceLocation* source )
{
	STACKTRACE;
	play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
	game->add(new Animation(this, pos,
		meleedata.kaboomSprite, 0, KABOOM_FRAMES, time_ratio, LAYER_EXPLOSIONS));

	// we do not report ship_died events -- the parent does
	//  game->ship_died(this, source);
}


ShipType *BipoleShip::get_shiptype()
{
	STACKTRACE;
	// this is necessary, otherwise the Kat Poly ship crashes.
	// also, "type" cannot be redirected inside the constructor, cause the
	// parents' type is declared outside of its constructor (bad?).
	return parent->type;
};

REGISTER_SHIP(BipoleKatamaran)
