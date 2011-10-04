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

#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

#include <algorithm>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "util/sounds.h"
#include "other/dialogs.h"

/*

Various helpers included in mhelpers.cpp :

Type verication
Byte Ordering (endianness)
	offers invert_ordering, intel_ordering, motorola_ordering,
	normal versions are 32 bit; and short versions of each are 16 bit
File Registration System
	keeps a list of linked files & compile times
VideoSystem
	sets screen resolutions, color formats, color transforms, fonts
	gets screen surface
	records redraw event times
SoundSystem
	inits sound hardware
	plays sounds
	loads sounds

SC2 Unit Conversion
Help Dialog

*/

#define PLATFORM_IS_ALLEGRO

#if defined PLATFORM_IS_ALLEGRO
#include <allegro.h>
#if defined ALLEGRO_MSVC
#include <winalleg.h>
#include <windows.h>
#endif
#else
#error unknown platform (allegro?)
#endif

#include "melee.h"
REGISTER_FILE

#include "scp.h"

#include "mframe.h"
#include "mgame.h"

volatile int debug_value = 0;

/*------------------------------
		File Registration System
------------------------------*/
registered_file_type *registered_files = NULL;
int num_registered_files = 0;
void _register_file (const char *fname, const char *fdate, const char *ftime)
{
	registered_files = (registered_file_type*) realloc(registered_files, sizeof(registered_file_type) * (num_registered_files+1));
	registered_files[num_registered_files].fname = fname;
	registered_files[num_registered_files].fdate = fdate;
	registered_files[num_registered_files].ftime = ftime;
	num_registered_files += 1;
	return;
}


/*------------------------------
		Video mode
------------------------------*/
int  GFX_TIMEWARP_WINDOW =
#if defined DJGPP
GFX_AUTODETECT
#else
//	GFX_OPENGL_WINDOWED
GFX_AUTODETECT_WINDOWED
//GFX_GDI
#endif
;
int GFX_TIMEWARP_FULLSCREEN =
GFX_AUTODETECT_FULLSCREEN
//GFX_AUTODETECT
;

VideoSystem videosystem;

static void tw_display_switch_out()
{
	STACKTRACE;
	//pause_mod();
}


static void tw_display_switch_in()
{
	STACKTRACE;
	if (get_time() > videosystem.last_poll + 1000) {
		videosystem.redraw();
	}
	else videosystem.screen_corrupted = true;

	//	resume_mod();
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
		FILE *f = fopen(data_full_path("palette").c_str(), "rb");
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
	if (!font_data) font_data = tw_load_datafile("fonts.dat");
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


struct VW_lock_data
{
	short int x, y, w, h;
};

void VideoWindow::lock ( )
{
	//int i = lock_level;
	lock_level += 1;
	//if (!surface) return;
	if (lock_level == 1) {
		/*		if (!lock_data) lock_data = (VW_lock_data*)malloc(sizeof(VW_lock_data));
				if (w && h) {
					lock_data[i].x = surface->cl;
					lock_data[i].y = surface->ct;
					lock_data[i].w = surface->cr - surface->cl;
					lock_data[i].h = surface->cb - surface->ct;
					set_clip(surface, x, y, x+w-1, y+h-1);
				} else {
					lock_data[i].x = 0;
					lock_data[i].y = 0;
					lock_data[i].w = 0;
					lock_data[i].h = 0;
				}*/
		set_clip(surface, x, y, x+w-1, y+h-1);
		acquire_bitmap(surface);
	}
}


void VideoWindow::unlock ( )
{
	//int i = lock_level - 1;
	if (lock_level == 0) {
		tw_error("VideoWindow unlocked too many times");
		return;
	}
	lock_level -= 1;
	//if (!surface) return;
	/*	if (w && h) {
			set_clip(surface, lock_data[i].x, lock_data[i].y,
				lock_data[i].x + lock_data[i].w - 1,
				lock_data[i].y + lock_data[i].h - 1
				);
		}*/
	if (lock_level == 0) {
		release_bitmap(surface);
		set_clip(surface, 0, 0, surface->w-1, surface->h-1);
	}
}


#define VideoWindow_callbacklist_units 4
void VideoWindow::match ( VideoWindow *old )
{
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	if (!parent && old->parent) init( old->parent );
	locate (
		old->const_x, old->propr_x,
		old->const_y, old->propr_y,
		old->const_w, old->propr_w,
		old->const_h, old->propr_h
		);
	return;
}


void VideoWindow::hide()
{
	STACKTRACE;
	locate(0,0,0,0,0,0,0,0);
	return;
}


void VideoWindow::add_callback( BaseClass *callee )
{
	STACKTRACE;
	//int i;
	//	for (i = 0; i < num_callbacks; i += 1) {
	//	if (callback_list[i] == callee) {tw_error("adding VideoWindow callback twice");}
	//}
	std::list<BaseClass*>::iterator cb = std::find(callback_list.begin(),callback_list.end(), callee);
	if (cb != callback_list.end()) {
		tw_error("adding VideoWindow callback twice");
	}
	callback_list.push_back(callee);
	//	if (!(num_callbacks & (VideoWindow_callbacklist_units-1))) {
	//	callback_list = (BaseClass**) realloc(callback_list, sizeof(BaseClass*) *
	//		(num_callbacks + VideoWindow_callbacklist_units) );
	//}
	//callback_list[num_callbacks] = callee;
	//num_callbacks += 1;
	return;
}


void VideoWindow::remove_callback( BaseClass *callee )
{
	STACKTRACE;
	//int i;
	//	for (i = 0; i < num_callbacks; i += 1) {
	//	if (callback_list[i] == callee) {
	//		callback_list[i] = callback_list[num_callbacks-1];
	//		num_callbacks -= 1;
	//		if (!(num_callbacks & (VideoWindow_callbacklist_units-1))) callback_list = (BaseClass**)
	//			realloc(callback_list, sizeof(BaseClass*) * (num_callbacks) );
	//		return;
	//	}
	//}
	callback_list.remove(callee);
	return;
}


void VideoWindow::event(int subtype)
{
	STACKTRACE;
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	VideoEvent ve;
	ve.type = Event::VIDEO;
	ve.subtype = subtype;
	ve.window = this;
	issue_event( callback_list, &ve);
	return;
}


void VideoWindow::update_pos()
{
	STACKTRACE;
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	int nx = 0, ny = 0, nw = 0, nh = 0;
	if (parent == this) {
		surface = videosystem.surface;
		if (surface) {
			nx = 0;
			ny = 0;
			nw = videosystem.width;
			nh = videosystem.height;
		}
	} else {
		if (parent) surface = parent->surface;
		else surface = NULL;
		if (surface) {
			nx = parent->x;
			ny = parent->y;
			nw = parent->w;
			nh = parent->h;
		}
	}

	x = nx + iround_up(const_x + propr_x * nw - 0.05);
	y = ny + iround_up(const_y + propr_y * nh - 0.05);
	w = iround_down(const_w + propr_w * nw + 0.05);
	h = iround_down(const_h + propr_h * nh + 0.05);

	if ((w <= 0) || (h <= 0)) surface = NULL;
	return;
}


void VideoWindow::_event( Event *e )
{
	STACKTRACE;
	if (e->type == Event::VIDEO) {
		const VideoEvent *ve = (const VideoEvent *) e;
		if (ve->window != parent) {tw_error ("VideoWindow event not from parent?");}
		VideoEvent nve;
		nve.type = Event::VIDEO;
		nve.window = this;
		nve.subtype = ve->subtype;
		switch (ve->subtype) {
			case VideoEvent::RESIZE :
			{
				update_pos();
			} break;
			case VideoEvent::INVALID:
			{
				x = y = w = h = 0;
				surface = NULL;
			} break;
			case VideoEvent::VALID:
			{
				update_pos();
			} break;
			case VideoEvent::REDRAW:
			{
				//update_pos();
			} break;
		}
		issue_event( callback_list, &nve);
	} else {
		tw_error ( "VideoWindow got non-video event" );
	}
	return;
}


void VideoWindow::preinit ()
{
	parent = NULL;
	surface = NULL;
	callback_list.clear();

	lock_level = 0;

	const_x = const_y = const_w = const_h = 0;
	propr_x = propr_y = 0;
	propr_w = propr_h = 1;
	x = y = w = h = 0;
	return;
}


void VideoWindow::init ( VideoWindow *parent_window)
{
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	if (parent) parent->remove_callback( this );
	parent = parent_window;
	//if (parent == this) {tw_error("VideoWindow - incest");}
	if (parent && (parent != this)) parent->add_callback ( this );
	update_pos();
	event(VideoEvent::RESIZE);
	event(VideoEvent::REDRAW);
	return;
}


void VideoWindow::locate ( double x1, double x2, double y1, double y2, double w1, double w2, double h1, double h2)
{
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	const_x = x1;
	propr_x = x2;
	const_y = y1;
	propr_y = y2;
	const_w = w1;
	propr_w = w2;
	const_h = h1;
	propr_h = h2;
	update_pos();
	event(VideoEvent::RESIZE);
	event(VideoEvent::REDRAW);
	return;
}


void VideoWindow::deinit()
{
	STACKTRACE;
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	if (parent) {
		parent->remove_callback( this );
		parent = NULL;
	}
	if (callback_list.size())
		{tw_error("VideowWindow - deinit illegal while child windows remain");}
		/*for (int i = 0; i < num_callbacks; i += 1) {
			Event e;
			e.type =
			callback_list[i]->_event(e);
		}*/

		//free(callback_list);
		//callback_list = NULL;
		//num_callbacks = 0;
		callback_list.clear();

	//	free(lock_data);
	//	lock_data = NULL;
}


VideoWindow::~VideoWindow()
{
	deinit();
}


/*------------------------------
		SC2 Unit Conversion
------------------------------*/
int time_ratio;					 //1000 milliseconds / SC2 framerate
double distance_ratio;
void init_sc2_unit_conversion()
{
	STACKTRACE;
}


int scale_frames(double value)
{
	STACKTRACE;
	return (int)((value + 1) * time_ratio);
}


double scale_turning (double turn_rate)
{
	//  turn_rate = 20.0 / ((turn_rate + 1.0) * 5.0);
	return (PI2 / 16) / (turn_rate + 1.0) / time_ratio;
}


double scale_velocity (double velocity)
{
	//  velocity = velocity / 7.5;
	//velocity = x sc2pixels / sc2frame
	//velocity = y twpixels / twframe
	//velocity = x (1. / 1600) / (50ms)
	//velocity = y (1. / 3840) / (1ms)
	// y = x * (3840 / 1600) / 50
	// WTF????
	return velocity * distance_ratio / time_ratio;
}


double scale_acceleration (double acceleration, double raw_hotspot_rate)
{
	//  accel_rate = accel_rate / 100.0;
	return acceleration * distance_ratio / (1 + raw_hotspot_rate) / time_ratio / time_ratio;
}


double scale_range (double range)
{
	return range * 40;
}


void show_file(const char *file)
{
	STACKTRACE;
	int i;
	char *willy;
	PACKFILE *f;
	f = pack_fopen (file, F_READ);
	if (!f) {
		willy = (char*) malloc(strlen(file)+1);
		sprintf(willy, "Failed to load file \"%s\"", file);
	} else {
		i = file_size(file);
		willy = (char*)malloc(i+1);
		i = pack_fread (willy, i, f);
		pack_fclose(f);
		willy[i] = 0;
	}
	show_text(willy);
	free(willy);
	return;
}


void show_text(const char *text)
{
	STACKTRACE;
	help_dialog[2].dp = (void *) text;
	help_dialog[2].d1 = 0;
	help_dialog[2].d2 = 0;
	tw_popup_dialog(&videosystem.window, help_dialog, 1);
	return;
}
