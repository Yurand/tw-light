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

#ifndef __TW_VIDEOSYSTEM_H__
#define __TW_VIDEOSYSTEM_H__

#include <string>
#include <allegro.h>

#include "util/port.h"
#include "util/base.h"

#include "melee/mvideowindow.h"

int get_gamma();
void set_gamma(int gamma);
void gamma_color_effects (Color *color) ;

class VideoSystem : public BaseClass
{
	public:
		int width, height, bpp, gamma;
		int fullscreen;
		TW_DATAFILE *font_data;	 //fonts
		FONT *basic_font;		 //font to use if no other is available
		Color *palette;
		volatile bool screen_corrupted;
		int last_poll;
		Surface *surface;
		VideoWindow window;

		FONT *get_font(int size);

		void preinit() ;
		int poll_redraw();
								 //returns 0 on failure
		int set_resolution (int width, int height, int bpp, int fullscreen) ;
		void set_palette(Color *pal);
		void (*color_effects)(Color *color);
		void update_colors();
		void redraw();

		BITMAP *load_bitmap(const std::string& path);
} extern videosystem;

#endif
