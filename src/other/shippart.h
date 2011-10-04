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

#ifndef __SHIPPART__
#define __SHIPPART__

#include "ship.h"

//void removefromtargetlist(SpaceObject *o);
//bool isintargetlist(SpaceObject *o);

class BigShipPart;

class BigShip : public Ship
{
	protected:

		int Nparts;
		BigShipPart **parts;

	public:

		BigShip(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual void animate(Frame *space);
		virtual void calculate();

		virtual void change_vel(Vector2 dvel);
		virtual void change_pos(Vector2 dpos);

};

class BigShipPart : public Ship
{

	protected:

		Vector2 oldpos;
		Vector2 oldvel;

		Ship *owner;
		SpaceSprite *sprite_uncrewed;

	public:

		Vector2 relpos;
		double  relangle;

		Vector2 relposrot;
		SpaceObject *collider;

		BigShipPart(Ship *aowner, Vector2 orelpos, double orelangle,
			SpaceSprite *spr, SpaceSprite *spr_uncrewed);

		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct=0);
		virtual void inflict_damage(SpaceObject *other);

		virtual void syncpos();
		virtual bool isdisabled();

		virtual void change_vel(Vector2 dvel);
		virtual void change_pos(Vector2 dpos);

		virtual int handle_fuel_sap(SpaceLocation *source, double normal);

		virtual ShipType *get_shiptype();
};

class BigShipPartDevice : public SpaceObject
{
	protected:
		BigShipPart *ownerpart;

	public:

		BigShipPartDevice(BigShipPart *aownerpart, SpaceSprite *ospr);
		virtual void calculate();
		virtual void animate(Frame *space);

};
#endif							 // __SHIPPART__
