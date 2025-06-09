/*
This file is part of "TW-Light"
					https://tw-light.xyz
Copyright (C) 2001-2018 TimeWarp development team

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
#include <cstring>
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

#include <algorithm>

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
	if (registered_files) {
		registered_files[num_registered_files].fname = fname;
		registered_files[num_registered_files].fdate = fdate;
		registered_files[num_registered_files].ftime = ftime;
		num_registered_files += 1;
	}
	else {
		tw_error("Memory error");
	}
	return;
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
	char *willy = NULL;
	PACKFILE *f;
	f = pack_fopen (file, F_READ);
	if (!f) {
		willy = (char*) malloc(strlen(file) + strlen("Failed to load file \"\"") + 1);
		if (willy)
			sprintf(willy, "Failed to load file \"%s\"", file);
		else
			tw_error("Memory error");
	} else {
		i = file_size_ex(file);
		willy = (char*)malloc(i+1);
		if (willy) {
			i = pack_fread(willy, i, f);
			willy[i] = 0;
		} else
			tw_error("Memory error");
	}
	show_text(willy);
	if (willy)
		free(willy);
	if (f)
		pack_fclose(f);
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
