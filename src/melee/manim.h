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

#ifndef __MANIM_H__
#define __MANIM_H__

#include "mframe.h"

/// \brief an item that doesn't interact with things, just is there for graphics, to display a little animation
class Animation : public SpaceObject
{
	protected:
		int frame_count;
		int frame_size;
		int frame_step;
		double scale;

	public:
		Animation(SpaceLocation *creator, Vector2 opos, SpaceSprite *osprite,
			int first_frame, int num_frames, int frame_size, double depth, double scale = 1.0) ;

		virtual void calculate();
		virtual void animate ( Frame * space ) ;
	public:
		float transparency;
};

/// \brief an item that doesn't interact with things, just is there for graphics, to display a little animation
class FixedAnimation : public Animation
{
	public:
		SpaceLocation *follow;

		FixedAnimation(SpaceLocation *creator, SpaceLocation *opos, SpaceSprite *osprite,
			int first_frame, int num_frames, int frame_length, double depth) ;

		virtual void calculate();
};

class PositionedAnimation : public FixedAnimation
{
	protected:
		Vector2 relative_pos;

	public:
		PositionedAnimation(SpaceLocation *creator, SpaceLocation *opos, Vector2 orel_pos,
			SpaceSprite *osprite, int first_frame,
			int num_frames, int frame_length, double depth);
		virtual void calculate();
};
#endif							 // __MANIM_H__
