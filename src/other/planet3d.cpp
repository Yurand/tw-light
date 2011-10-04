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

#include "util/pmask.h"
#include "util/aastr.h"

#include <stdio.h>
#include <string.h>

#include "planet3d.h"

SpaceSprite::SpaceSprite(BITMAP *image, int _attributes)
{
	STACKTRACE;
	if (_attributes == -1)
		_attributes = string_to_sprite_attributes(NULL);
	general_attributes = _attributes;

	int i;
	BITMAP *bmp;
	count = 1;

	references = 0;
	highest_mip = 0;
	for (i = 1; i < MAX_MIP_LEVELS; i += 1) {
		b[i] = NULL;
	}

	bpp = 32;					 //videosystem.bpp;

	m = new PMASK* [count];
	b[0] = new BITMAP*[count];
	attributes = new char [count];

	w = image->w;
	h = image->h;

	bmp = create_bitmap_ex ( bpp, w, h);
	convert_bitmap(image, bmp, general_attributes & MASKED);

	i = 0;
	color_correct_bitmap(bmp, general_attributes & MASKED);
	m[i] = create_allegro_pmask(bmp);
	b[0][i] = bmp;
	attributes[i] = DEALLOCATE_IMAGE | DEALLOCATE_MASK;
}


Planet *create_planet( Vector2 position )
{

	// create a 2D planet
	Planet *planet = new Planet (position,
		meleedata.planetSprite,
		random(meleedata.planetSprite->frames()));
	game->add (planet);
	return planet;
}
