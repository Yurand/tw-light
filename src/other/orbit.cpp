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

#include <stdio.h>
#include <allegro.h>
#include "melee.h"
REGISTER_FILE
#include "id.h"
#include "scp.h"
#include "frame.h"
#include "ship.h"

#include "melee/mgame.h"
#include "melee/mview.h"
#include "melee/mcbodies.h"
#include "melee/manim.h"
#include "melee/mship.h"

#include "orbit.h"

#define MaxDam 10.
#define Range 120.
#define z (MaxDam/(Range*Range))

void iMessage(char *cmdstr, int num)
{
	STACKTRACE;
	char buf[200];
	sprintf(buf, cmdstr,num);
	message.out(buf);
}


int SpaceStation::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	Crew-=(int)(normal+direct);
	if (Crew<=0) state=0;
	return SpaceObject::handle_damage(source, normal, direct);
}


void SpaceStation::calculate()
{
	STACKTRACE;

	//Healing beam code

	Query a;
	SpaceObject *o;
	for (a.begin(this, bit(LAYER_SHIPS), 200.); a.current; a.next()) {
		o = a.currento;
		if (!o->isInvisible()) {
			game->add(new PointLaser(this, 0x0000ffff, -1,
				10, this, o, Vector2(0.0, 10.0)));
		}
	}
}


SpaceStation::SpaceStation(SpaceLocation *creator, Vector2 opos,
SpaceSprite *oSprite):SpaceObject(creator,opos,0.,oSprite)
{
	STACKTRACE;
	//	layer=LAYER_SPECIAL;
	mass=10.;
	Crew=2000;

}


OrbitHandler::OrbitHandler(SpaceLocation *creator, Vector2 lpos,
double langle, SpaceLocation *p_sun, SpaceLocation *p_planet,
double lrad, double lspeed, int iLock):SpaceLocation(creator,
lpos, 0.)
{
	STACKTRACE;
	id=ORBIT_ID;
	ipos=lpos;
	Lock=iLock;
	sun=p_sun;
	plan=p_planet;
	angle=langle;
	Radius = lrad;
	Vel=lspeed;

	plan->pos=sun->pos+Radius*unit_vector(angle);

	vel=0;

}


void OrbitHandler::calculate()
{
	STACKTRACE;

	if ((sun==NULL)||(plan==NULL))
		return;

	//If they don't both exist, then destroy the OrbitHandler
	if ((!sun->exists())||(!plan->exists())) {
		sun=NULL;
		plan=NULL;
		state=0;
		return;
	}

	if (Lock==1) {
		sun->pos=ipos;
		sun->vel=0;
	} else {
		pos=sun->pos;
		vel=sun->vel;
	}

	double theta = (frame_time*Vel)/Radius;

	angle+= theta;

	if (angle>PI2) angle-=PI2;

	plan->pos=sun->pos+Radius*unit_vector(angle);
	plan->vel=sun->vel+theta*unit_vector(angle+PI/2);

	SpaceLocation::calculate();
}


int OrbitHandler::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	return FALSE;
}


Sun::Sun(Vector2 opos, SpaceSprite *sprite, int index)
:
SpaceObject(NULL, opos, 0.0, sprite)
{
	STACKTRACE;

	layer = LAYER_SHOTS;
	set_depth(LAYER_EXPLOSIONS);
	//	collide_flag_sameship = 0;

	//	collide_flag_sameship = ALL_LAYERS;
	//	layer = LAYER_CBODIES;
	//	id         |= ID_PLANET;
	id=SUN_ID;
	//	mass        = 9999999.0;
	damage_factor=6;

	//use remote .ini file
	game->log_file ("server.ini");
	sprite_index = index;
	gravity_mindist = scale_range(get_config_float("Sun", "GravityMinDist", 0));
	gravity_range = scale_range(get_config_float("Sun", "GravityRange", 0));
	gravity_power = get_config_float("Sun", "GravityPower", 0);
	gravity_force =
		scale_acceleration(get_config_float("Sun", "GravityForce", 0), 0);
	gravity_whip = get_config_float("Sun", "GravityWhip", 0);

}


int Sun::canCollide(SpaceLocation *other)
{
	STACKTRACE;

	if (other->id==COMET_ID) return FALSE;
	return(!other->isPlanet());
}


void Sun::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

	double d = distance(other);
	if (d >= Range) return;
	damage_factor = (int)(MaxDam - z*d*d);

	other->handle_damage(this, (damage_factor+1)/2, damage_factor/2);

	return;
}


void Sun::calculate()
{
	STACKTRACE;

	SpaceObject::calculate();
	SpaceObject *o;
	Query a;
	a.begin(this, OBJECT_LAYERS, gravity_range);
	for (;a.currento;a.next()) {
		o = a.currento;
		if (o->mass > 0) {
			double r = distance(o);
			if (r < gravity_mindist) r = gravity_mindist;
			double sr = 1;
			//gravity_power truncated here
			if (gravity_power > 0) {
				r /= 40 * 4;
				for (int i = 0; i < gravity_power; i += 1) sr *= r;
				o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force / sr, MAX_SPEED);
			} else {
				r = 1 - r/gravity_range;
				for (int i = 0; i < -gravity_power; i += 1) sr *= r;
				o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force * sr, MAX_SPEED);
			}
		}
	}
	/*for (;a.currento;a.next()) {
		o = a.currento;
		if (o->mass > 0) {
			double r = distance(o);
			if (r < gravity_mindist) r = gravity_mindist;
			r = 1 - r / gravity_range;
			o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force * r / 40000., GLOBAL_MAXSPEED);
			}
		}*/
	//	return;

}
