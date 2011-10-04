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

#ifndef __FONT_MORPH__
#define __FONT_MORPH__

#include <allegro.h>
#include "allegro/internal/aintern.h"

FONT_COLOR_DATA* font_upgrade_to_color_data(FONT_MONO_DATA* mf);

void font_upgrade_to_color(FONT* f);

void morph_font(FONT *f);
#endif
