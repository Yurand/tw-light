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

#include "melee.h"
REGISTER_FILE

#include "objanim.h"

ObjectAnimation::ObjectAnimation( SpaceLocation *creator, Vector2 opos,
Vector2 ovel, double oangle, SpaceSprite *osprite, int first_frame,
int num_frames, int frame_size, double depth ):
Animation( creator, opos, osprite, first_frame, num_frames, frame_size, depth ),
angle(oangle)
{
	STACKTRACE;
	vel = ovel;

	sprite_index = get_index(angle);
	sprite_index += first_frame * 64;
}


void ObjectAnimation::calculate()
{
	STACKTRACE;

	sprite_index >>= 6;
	Animation::calculate();
	sprite_index <<= 6;
	int i = get_index(angle);
	sprite_index += i;
}
