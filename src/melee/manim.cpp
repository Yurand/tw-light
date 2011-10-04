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

#include "ship.h"
#include "mgame.h"
#include "manim.h"

#include "util/aastr.h"

Animation::Animation(SpaceLocation *creator, Vector2 opos,
SpaceSprite *osprite, int first_frame, int num_frames, int frame_length,
double depth, double _scale)
:
SpaceObject(creator, opos, 0.0, osprite),
frame_count(num_frames),
frame_size(frame_length),
frame_step(frame_length),
scale(_scale),
transparency(0)
{
	STACKTRACE;
	if (frame_size <= 0) {
		tw_error("Animation::Animation - frame_size == %d", frame_size);
		frame_size = 1;
	}
	sprite_index = first_frame;
	layer = LAYER_HOTSPOTS;
	set_depth(depth);
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = 0;
	mass = 0;

	// it's got no physical interaction with the rest of the world, so, remove it
	// from the query list:
	attributes |= ATTRIB_UNDETECTABLE;
}


void Animation::calculate()
{
	STACKTRACE;

	frame_step -= frame_time;
	while (frame_step < 0) {
		frame_step += frame_size;
		sprite_index += 1;
		if (sprite_index == sprite->frames())
			sprite_index = 0;
		frame_count -= 1;
		if (!frame_count)
			state = 0;
	}
	SpaceObject::calculate();
}


void Animation::animate(Frame *space)
{
	STACKTRACE;
	if (transparency != 0) {
		int old = aa_get_trans();
		aa_set_trans( iround(old * (1 - transparency) + 255 * transparency) );
		sprite->animate(pos, sprite_index, space, scale);
		aa_set_trans( old );

	}
	else
		sprite->animate(pos, sprite_index, space, scale);
}


FixedAnimation::FixedAnimation(SpaceLocation *creator, SpaceLocation *opos,
SpaceSprite *osprite, int first_frame, int num_frames, int frame_length,
double depth)
:
Animation(creator, opos->normal_pos(), osprite, first_frame, num_frames, frame_length, depth),
follow(opos)
{
	STACKTRACE;
	if (!follow->exists()) {
		state = 0;
		follow = NULL;
	}
}


void FixedAnimation::calculate()
{
	STACKTRACE;
	if (follow->exists()) {
		pos = follow->normal_pos();
		Animation::calculate();
	} else {
		state = 0;
		follow = NULL;
	}
}


PositionedAnimation::PositionedAnimation(SpaceLocation *creator,
SpaceLocation *opos, Vector2 rel_pos, SpaceSprite *osprite, int first_frame,
int num_frames, int frame_length, double depth)
:
FixedAnimation(creator, opos, osprite, first_frame, num_frames, frame_length, depth),
relative_pos(rel_pos)
{
	STACKTRACE;
	if (!follow || !follow->exists()) {
		state = 0;
		return;
	}
	pos += rotate(relative_pos, follow->get_angle());
}


void PositionedAnimation::calculate()
{
	STACKTRACE;
	FixedAnimation::calculate();
	if (!exists())
		return;
	pos += rotate(relative_pos, follow->get_angle());
}
