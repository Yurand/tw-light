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

#ifndef __MCBODIES_H__
#define __MCBODIES_H__

#include "melee.h"
#include "mframe.h"

/// \brief Asteroid space object
class Asteroid : public SpaceObject
{
	protected:
		int step;
		int speed;
		SpaceSprite *explosion;
		double armour;

	public:
		/// \brief init various stuff, pic, speed etc
		Asteroid();

		/// \brief rotate
		virtual void calculate();
		/// \brief handle damage
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
		/// \brief Show explosion, add new new asteroid
		virtual void death();
};

/// \brief Planet space object
class Planet : public SpaceObject
{
	public:
		double gravity_force;
		double gravity_mindist;
		double gravity_range;
		double gravity_power;
		double gravity_whip;
		double gravity_whip2;
		/// \brief Init stuff
		///
		/// Set collide flag, set layer, set mass to very huge,
		/// read from server.ini planet attributes
		Planet(Vector2 location, SpaceSprite *sprite, int index);

		/// \brief inflict damage when collide
		///
		/// Kill 1/3 crew for ship, kill objects without mass,
		/// play BOOM sound
		virtual void inflict_damage(SpaceObject *other);
		/// \brief Apply gravitation
		virtual void calculate();
};

/// \brief Background stars
class Stars : public Presence
{
	virtual void _event( Event *e);
	public:
		Stars();
		~Stars();
		SpaceSprite **pic;
		double width;
		double height;
		int num_pics;
		int num_layers;
		int num_stars;			 //300?
		int aa_mode;			 //0 to 5, 1, 2, or 5 recommended
		int field_depth;		 //0 to 255

		Uint64 seed;
		void animate(Frame *space);
		void select_view( View **view);
		View **v;
};

/// \brief helpers for drawing your own starfields
void _draw_starfield_raw (
//surface to draw starfield on
Frame *frame,
//star sprite
SpaceSprite *sprite,
//index into star sprite
int index, int sprites,
//number of stars
int n,
//center of screen for drawing purposes
int cx, int cy,
//scrolly amount
double x, double y,
//size of starfield (usually the same as wrap point, sometimes smaller)
double w, double h,
//wrap point
double mx, double my,
//size of stars
double zoom,
//anti-aliasing mode to use
int aa_mode
);

/// \brief helpers for drawing your own starfields
void _draw_starfield_cached (
//surface to draw starfield on
Frame *frame,
//star sprite
SpaceSprite *sprite,
//index into star sprite
int index,
//number of stars
int n,
//center of screen for drawing purposes
int cx, int cy,
//scrolly amount
double x, double y,
//size of starfield (usually the same as wrap point, sometimes smaller)
double w, double h,
//wrap point
double mx, double my,
//size of stars
double zoom,
//anti-aliasing mode to use
int aa_mode
);
#endif							 // __MCBODIES_H__
