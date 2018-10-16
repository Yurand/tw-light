/*
This file is part of "TW-Light"
					https://tw-light.appspot.com/
Copyright (C) 2001-2013  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include <cstdint>
#include <cstdio>
#include <cmath>

#include "mvideosystem.h"
#include "other/gameconf.h"

#include "util/round.h"
#include "util/errors.h"
#include "util/helper.h"

/*------------------------------
		Video mode
------------------------------*/
int GFX_TIMEWARP_WINDOW = GFX_AUTODETECT_WINDOWED;
int GFX_TIMEWARP_FULLSCREEN = GFX_AUTODETECT_FULLSCREEN;

VideoSystem videosystem;

static void tw_display_switch_out()
{
	STACKTRACE;
}

static void tw_display_switch_in()
{
	STACKTRACE;
	if (get_time() > videosystem.last_poll + 1000) {
		videosystem.redraw();
	}
	else {
		videosystem.screen_corrupted = true;
	}
}

static int _gamma = -1;
static unsigned char _gamma_map[256];
int get_gamma()
{
	STACKTRACE;
	return _gamma;
}

void set_gamma(int gamma)
{
	STACKTRACE;
	if (gamma < 0) gamma = 0;
	if (gamma > 255) gamma = 255;
	_gamma = gamma;
	int i;
	for (i = 0; i < 256; i += 1) {
		_gamma_map[i] = iround_down(256 * pow(i / 255.5, 1-gamma/258.));
	}
	return;
}

void gamma_color_effects (Color *c)
{
	if (!c->filler) {
		c->r = _gamma_map[c->r];
		c->g = _gamma_map[c->g];
		c->b = _gamma_map[c->b];
	} else {
		int alpha = (c->filler ^ 255) + 1;
		int r, g, b;
		r = (c->r << 8) / alpha;
		g = (c->g << 8) / alpha;
		b = (c->b << 8) / alpha;
		if ((r | g | b) > 255) {
			tw_error("gamma_color_effects : premultiplied alpha color invalid");
		}
		r = _gamma_map[r];
		g = _gamma_map[g];
		b = _gamma_map[b];
		r = (r * alpha) >> 8;
		g = (g * alpha) >> 8;
		b = (b * alpha) >> 8;
		c->r = r;
		c->g = g;
		c->b = b;
	}
	return;
}

int tw_color (Color c)
{
	videosystem.color_effects(&c);
	return makecol(c.r, c.g, c.b);
}

int tw_color (int r, int g, int b)
{
	Color c = {r,g,b};
	return tw_makecol(c.r, c.g, c.b);
}


int VideoSystem::poll_redraw()
{
	STACKTRACE;
	last_poll = get_time();
	if (screen_corrupted) {
		screen_corrupted = false;
		videosystem.redraw();
		return 1;
	}
	return 0;
}

void VideoSystem::preinit()
{
	STACKTRACE;
	int i;
	surface = NULL;
	width = -1;
	height = -1;
	bpp = -1;
	fullscreen = false;
	font_data = NULL;
	basic_font = NULL;
	palette = (Color*)malloc(sizeof(Color) * 256);
	if (1) {
		FILE *f = tw_fopen(data_full_path("palette").c_str(), "rb");
		if (!f) {
			log_debug("Error reading palette: %s\n", data_full_path("palette").c_str());
		}
		if (f) {
			for (i = 0; i < 256; i += 1) {
				palette[i].r = fgetc(f);
				palette[i].g = fgetc(f);
				palette[i].b = fgetc(f);
				palette[i].filler = 0;
			}
			fclose(f);
		}
	} else {
		enum {
			r_levels = 6,
			g_levels = 6,
			b_levels = 6,
			total_levels = r_levels * g_levels * b_levels
		};
		COMPILE_TIME_ASSERT(total_levels < 256);
		for (i = 0; i < 256; i += 1) {
			if (i < total_levels) {
				int j = 1;
				palette[i].r = ((i/j)%r_levels) * 255 / r_levels;
				j *= r_levels;
				palette[i].g = ((i/j)%g_levels) * 255 / g_levels;
				j *= g_levels;
				palette[i].b = ((i/j)%b_levels) * 255 / b_levels;
				j *= b_levels;
			} else {
				palette[i].r = 0;
				palette[i].g = 0;
				palette[i].b = 0;
				palette[i].filler = 0;
			}
			/*palette[i].r = (((i >> 0) & (bit(red_bits)-1)) * 255) / (bit(red_bits)-1);
			palette[i].g = (((i >> red_bits) & (bit(green_bits)-1)) * 255) / (bit(green_bits)-1);
			palette[i].b = (((i >> (red_bits+green_bits)) & (bit(blue_bits)-1)) * 255) / (bit(blue_bits)-1);*/
		}
	}
	color_effects = gamma_color_effects;

	screen_corrupted = false;
	last_poll = -1;
	window.preinit();
	window.init( &window );
	window.locate(0,0,0,0,  0,1,0,1);
}


FONT *VideoSystem::get_font(int s)
{
	STACKTRACE;
	if (!font_data) {
		if (basic_font) return basic_font;
		if (!font) {
			tw_error_exit("VideoSystem::get_font - something horribly wrong!");
			return font;
		}
	}
	if (s < 0) s = 0;
	if (s > 7) s = 7;
	return (FONT*) font_data[s].dat;
}


void VideoSystem::set_palette(Color *new_palette)
{
	STACKTRACE;
	memcpy(palette, new_palette, sizeof(Color) * 256);
	update_colors();
	return;
}


void VideoSystem::update_colors()
{
	STACKTRACE;
	Color tmp[256];
	if (!palette) return;
	memcpy(tmp, palette, sizeof(Color) * 256);
	int i;
	for (i = 1; i < 256; i += 1) {
		color_effects(&tmp[i]);
		tmp[i].r = ((unsigned int)(tmp[i].r) * 63) / 255;
		tmp[i].g = ((unsigned int)(tmp[i].g) * 63) / 255;
		tmp[i].b = ((unsigned int)(tmp[i].b) * 63) / 255;
	}
	if (rgb_map) create_rgb_table ( rgb_map, (RGB*)tmp, NULL);
	::set_palette((RGB*)tmp);
	return;
}


void VideoSystem::redraw()
{
	STACKTRACE;
	VideoEvent ve;
	ve.type = Event::VIDEO;
	ve.subtype = VideoEvent::REDRAW;
	ve.window = &window;
	window._event(&ve);
	//clear_to_color(surface, palette_color[4]);
}


int VideoSystem::set_resolution (int width, int height, int bpp, int fullscreen)
{
	VideoEvent ve;
	ve.type = Event::VIDEO;
	ve.window = &window;
	if (width == 0) width = this->width;
	if (height == 0) height = this->height;
	if (bpp == 0) bpp = this->bpp;
	if (!basic_font) basic_font = font;
	if (!font_data) font_data = tw_load_datafile(data_full_path("fonts.dat").c_str());
	if ((bpp == this->bpp) && (width == this->width) && (height == this->height) && (fullscreen == this->fullscreen)) return true;
	if ((width < 300) || (height < 200)) {
		char buffy[512];
		sprintf(buffy, "Error switching to graphics mode\n(%dx%d @ %d bit)\nresolution too low", width, height, bpp);
		if (this->bpp == -1) {
			tw_error_exit(buffy);
		}
		tw_alert (buffy, "Continue");
		return false;
	}
	ve.subtype = VideoEvent::INVALID;
	window._event(&ve);
	surface = NULL;
	set_color_depth(bpp);
	if (set_gfx_mode((fullscreen ? GFX_TIMEWARP_FULLSCREEN : GFX_TIMEWARP_WINDOW), width, height, 0, 0)) {
		// sometimes I get "Can not grab keyboard error" when I run from GNOME menu.
		// Try again.
		rest(1000);
		if (set_gfx_mode((fullscreen ? GFX_TIMEWARP_FULLSCREEN : GFX_TIMEWARP_WINDOW), width, height, 0, 0)) {
			const char *part1 = "Error switching to graphics mode";
			char part2[256];
			sprintf (part2, "(%dx%d @ %d bit)", width, height, bpp);
			const char *part3 = allegro_error;
			if (this->bpp == -1) {
				char buffy[1024];
				sprintf(buffy, "%s\n%s\n%s", part1, part2, part3);
				log_debug(buffy);
				return false;
			}
			set_color_depth(this->bpp);
			set_gfx_mode((this->fullscreen ? GFX_TIMEWARP_FULLSCREEN : GFX_TIMEWARP_WINDOW), this->width, this->height, 0, 0);
			alert (part1, part2, part3, "Continue", NULL, ' ', '\n');
			surface = screen;
			ve.subtype = VideoEvent::VALID;
			window._event(&ve);
			surface = NULL;
			redraw();
			return false;
		}
	}
	surface = screen;
	if (set_display_switch_mode(SWITCH_BACKAMNESIA) == -1)
		set_display_switch_mode(SWITCH_BACKGROUND);
	set_display_switch_callback(SWITCH_IN, tw_display_switch_in);
	set_display_switch_callback(SWITCH_OUT, tw_display_switch_out);

	int owidth, oheight, obpp, ogamma, ofullscreen;
	owidth = this->width; oheight = this->height; obpp = this->bpp;
	ogamma = this->gamma; ofullscreen = this->fullscreen;

	this->width = width;
	this->height = height;
	this->bpp = bpp;
	//this->gamma = gamma;
	this->fullscreen = fullscreen;
	update_colors();
	if (font_data) font = (FONT *)(font_data[15].dat);
	else font = basic_font;
	if (bpp == 8) {
		if (!rgb_map) rgb_map = (RGB_MAP*)malloc(1<<15);
		Color tmp[256];
		memcpy(tmp, palette, sizeof(Color) * 256);
		int i;
		for (i = 0; i < 256; i += 1) {
			tmp[i].r = ((unsigned int)(tmp[i].r) * 63) / 255;
			tmp[i].g = ((unsigned int)(tmp[i].g) * 63) / 255;
			tmp[i].b = ((unsigned int)(tmp[i].b) * 63) / 255;
		}
		create_rgb_table ( rgb_map, (RGB*)tmp, NULL);
	}
	if (obpp != bpp) {
		ve.subtype = VideoEvent::CHANGE_BPP;
		window._event(&ve);
	}
	if ((owidth != width) || (oheight != height)) {
		ve.subtype = VideoEvent::RESIZE;
		window._event(&ve);
	}

	ve.subtype = VideoEvent::VALID;
	window._event(&ve);

	redraw();
	return true;
}

BITMAP* VideoSystem::load_bitmap(const std::string& path)
{
	BITMAP* tmp = ::load_bitmap(path.c_str(), NULL);
	BITMAP* tmp2 = create_bitmap_ex(bpp, tmp->w, tmp->h);
	blit(tmp, tmp2, 0, 0, 0, 0, tmp->w, tmp->h);
	destroy_bitmap(tmp);
	return tmp2;
}

BITMAP *VideoSystem::create_bitmap(BITMAP *src)
{
	BITMAP* ret = create_bitmap_ex(bpp, src->w, src->h);
	blit(src, ret, 0, 0, 0, 0, src->w, src->h);
	return ret;
}
