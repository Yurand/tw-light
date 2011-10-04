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

#ifndef __MSHOT_H__
#define __MSHOT_H__

#include "mframe.h"
#include "mgame.h"

/// \brief A bullet with a range
class Shot : public SpaceObject
{
	public:
		double v;
		double d;
		double range;
		double armour;

		SpaceSprite *explosionSprite;
		SAMPLE      *explosionSample;
		int          explosionFrameCount;
		int          explosionFrameSize;

		Shot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, double odamage,
			double orange, double oarmour, SpaceLocation *opos, SpaceSprite *osprite, double relativity = game->shot_relativity);

		virtual void animate(Frame *space);

		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
		virtual void death();

		virtual void animateExplosion();
		virtual void soundExplosion();

		void stop();
		void destroy();

		virtual void changeDirection(double oangle);
		int isHomingMissile();
};

/// \brief A shot that uses a sequence of images over time
class AnimatedShot : public Shot
{
	protected:
		int frame_count;
		int frame_size;
		int frame_step;

	public:
		AnimatedShot(SpaceLocation *creator, Vector2 rpos, double oangle,
			double ov, double odamage, double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, int ofcount, int ofsize, double relativity = game->shot_relativity);

		virtual void calculate();
};

/// \brief A shot that uses an image depending upon which angle it's pointing
class Missile : public Shot
{
	public:
		Missile(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, double odamage,
			double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, double relativity = game->shot_relativity);

		virtual void changeDirection(double oangle);
};

/// \brief A missile that turns towards its target
class HomingMissile : public Missile
{
	protected:
		double turn_rate;
		double turn_step;

	public:
		HomingMissile(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, double odamage,
			double orange, double oarmour, double otrate, SpaceLocation *opos,
			SpaceSprite *osprite, SpaceObject *target);

		virtual void animate(Frame *space);
		virtual void calculate();
};

class Laser : public SpaceLine
{
	protected:
		double frame;
		double frame_count;

		SpaceLocation *lpos;
		Vector2 rel_pos;
		double relative_angle;
		bool sinc_angle;

	public:
		Laser(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage, int lfcount,
			SpaceLocation *opos, Vector2 rpos = Vector2(0,0), bool osinc_angle=false);

		void calculate();
};

class PointLaser : public Laser
{
	protected:

		SpaceObject *target;

	public:
		PointLaser(SpaceLocation *creator, int lcolor, double ldamage, int lfcount,
			SpaceLocation *lsource, SpaceObject *ltarget, Vector2 rel_pos = Vector2(0,0)) ;

		void calculate();
		int canCollide(SpaceObject *other);
};

/** A shot that expires after a specified time */
class TimedShot : public SpaceObject
{
	public:
	public:
		double armour;
		double existtime, maxtime;

		TimedShot(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
			double ovel, double otime, double oarmour, double odamage);

		virtual void calculate();

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};
#endif							 // __MSHOT_H__
