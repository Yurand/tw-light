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
#include "melee/mgame.h"
#include "nullphas.h"

NullPhaser::NullPhaser( Ship* oship ):
Phaser( oship, oship->normal_pos() - unit_vector( oship->get_angle()) * PHASE_MAX *
oship->size, unit_vector( oship->get_angle()) * PHASE_MAX * oship->size,
oship, oship->get_sprite(), oship->get_sprite_index(), hot_color, HOT_COLORS,
PHASE_DELAY, PHASE_MAX, PHASE_DELAY )
{
	STACKTRACE;
}


void NullPhaser::animate( Frame* space )
{
	STACKTRACE;
}


void NullPhaser::calculate()
{
	STACKTRACE;

	if ( state > 0 ) {
		game->add( ship );
		ship->materialize();
		ship = NULL;
		state = 0;
	}
}
