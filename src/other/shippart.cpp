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
#include "melee/mview.h"
REGISTER_FILE

#include <stdio.h>

#include "frame.h"

#include "shippart.h"

BigShip::BigShip(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	// remove this from the physics interaction
	//mass = 0;
	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	attributes |= ATTRIB_UNDETECTABLE;
}


void BigShip::calculate()
{
	STACKTRACE;

	// make sure the "ship" is not a real target
	if (targets->isintargetlist(this))
		//removefromtargetlist(this);
		targets->rem(this);

	Ship::calculate();

	// check if all the parts are still alive.
	int i;
	for ( i = 0; i < Nparts; ++i ) {
		if (!parts[i])
			continue;

		if (!parts[i]->exists())
			parts[i] = 0;
	}
}


// change velocities of the ship and all its parts
void BigShip::change_vel(Vector2 dvel)
{
	STACKTRACE;

	int i;

	vel += dvel;

	for ( i = 0; i < Nparts; ++i ) {
		if (!parts[i])
			continue;

		parts[i]->vel += dvel;
	}
}


// change positions of the ship and all its parts
void BigShip::change_pos(Vector2 dpos)
{
	STACKTRACE;

	int i;

	pos = normalize(pos + dpos);

	for ( i = 0; i < Nparts; ++i ) {
		if (!parts[i])
			continue;

		parts[i]->pos = normalize(parts[i]->pos + dpos);
	}
}


void BigShip::animate(Frame *space)
{
	STACKTRACE;
	return;
}


BigShipPart::BigShipPart(Ship *aowner, Vector2 orelpos, double orelangle,
SpaceSprite *spr, SpaceSprite *spr_uncrewed)
:
Ship(aowner, 0, 0, spr)
{
	STACKTRACE;
	owner = aowner;
	relpos = orelpos;
	relangle = orelangle;

	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS-0.1);	 // the ship should come first in calculations.
	mass = owner->mass;

	oldpos = pos;
	oldvel = vel;

	// these values aren't really used ... I think ...
	crew = owner->crew;
	crew_max = owner->crew_max;
	batt = owner->batt;
	batt_max = owner->batt_max;
	recharge_rate = owner->recharge_rate;

	recharge_step = recharge_rate;
	recharge_amount = owner->recharge_amount;
	turn_rate = owner->turn_rate;
	accel_rate = owner->accel_rate;

	sprite_uncrewed = spr_uncrewed;

	relposrot = rotate(relpos, angle + owner->turn_step);

	collide_flag_sameship = 0;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_anyone = ALL_LAYERS;

	// check if you've collided with something.
	collider = 0;

	targets->add(this);
}


// this is in a separate routine, so that it can be called (again) by
// the owner if necessary (collision)

void BigShipPart::syncpos()
{
	STACKTRACE;

	// maintain (relative) position wrt the ship
								 // this is the discrete angle (64 values).
	angle = owner->angle + relangle;
	sprite_index = get_index(angle);

	// for positioning, use the accurate angle.
	oldpos = pos;

	relposrot = rotate(relpos, owner->angle + owner->turn_step);
	pos = owner->pos + relposrot;

	// the overall ship velocity
	oldvel = vel;
	vel = owner->vel;

	// THIS SHOULD BE REPLACED BY ADDING ROTATIONAL VELOCITY ...
}


void BigShipPart::calculate()
{
	STACKTRACE;

	if ( !(owner && owner->exists()) ) {
		owner = 0;
		state = 0;
		return;
	}

	if (!collider)
		syncpos();
	else
		collider = 0;
	// otherwise, you shouldn't touch the current ship-part settings,
	// instead let the owner handle this case. The vel/ pos are needed
	// to update the ship and all the other parts velocities ...

	/*
	// rotational velocity
	double angrate;

	angrate = 0;
	if (owner->turn_left)
		angrate = -owner->turn_rate;
	if (owner->turn_right)
		angrate = owner->turn_rate;

	double v;
	v = relpos.length() * angrate;

	Vector2 norm;
	norm = Vector2(-relposrot.y, relposrot.x);

	if (norm.length() > 0)
		vel += v * unit_vector(norm);
	*/

	Ship::calculate();

}


int BigShipPart::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	// by default, transmit damage to the ship owner ...
	return owner->handle_damage(source, normal, direct);
}


void BigShipPart::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

	Ship::inflict_damage(other);

	// this routine is called because there's a collision going on.

	collider = other;
}


bool BigShipPart::isdisabled()
{
	STACKTRACE;
	return false;
}


// change velocities of the ship and all its parts
void BigShipPart::change_vel(Vector2 dvel)
{
	STACKTRACE;

	owner->change_vel(dvel);
}


// change positions of the ship and all its parts
void BigShipPart::change_pos(Vector2 dpos)
{
	STACKTRACE;

	owner->change_pos(dpos);
}


BigShipPartDevice::BigShipPartDevice(BigShipPart *aownerpart, SpaceSprite *ospr)
:
SpaceObject(aownerpart, aownerpart->pos, aownerpart->angle, ospr)
{
	STACKTRACE;
	ownerpart = aownerpart;

	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS+0.01); // for plotting?

	// such a device shouldn't block things - impacts and damage should be handled
	// by the ship-part that owns this device.

	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	mass = 0;

}


void BigShipPartDevice::calculate()
{
	STACKTRACE;

	if ( !(ownerpart && ownerpart->exists()) ) {
		state = 0;
		return;
	}

	if (ownerpart->isdisabled())
		return;

	angle = ownerpart->angle;
	pos = ownerpart->pos;
	vel = ownerpart->vel;

	SpaceObject::calculate();
}


void BigShipPartDevice::animate(Frame *space)
{
	STACKTRACE;

	if (ownerpart->isdisabled())
		return;

	SpaceObject::animate(space);
}


int BigShipPart::handle_fuel_sap(SpaceLocation *source, double normal)
{
	STACKTRACE;

	return owner->handle_fuel_sap(source, normal);
}


ShipType *BigShipPart::get_shiptype()
{
	STACKTRACE;
	// this is necessary, otherwise the Kat Poly ship crashes.
	// also, "type" cannot be redirected inside the constructor, cause the
	// parents' type is declared outside of its constructor (bad?).
	return owner->type;
};
