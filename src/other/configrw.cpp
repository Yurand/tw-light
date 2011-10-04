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
#include <string.h>
#include <stdio.h>

#include "configrw.h"
#include "other/twconfig.h"

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "util/errors.h"

bool config_read = true;

const bool CONFIG_READ = true;

const bool CONFIG_WRITE = false;

char *section = 0;

void conf(char *id, int &x, int def)
{
	STACKTRACE;
	if (config_read)
		x = get_config_int(section, id, def);
	else
		set_config_int(section, id, x);
}


void conf(char *id, double &x, double def)
{
	STACKTRACE;
	if (config_read)
		x = get_config_float(section, id, def);
	else
		set_config_float(section, id, x);
}


void conf(char *id, char *x, char *def)
{
	STACKTRACE;
	if (config_read)
		strcpy(x, get_config_string(section, id, def));
	else
		set_config_string(section, id, x);
}


void confnum(char *id0, int i, int &x)
{
	STACKTRACE;
	char id[128];
	sprintf(id, "%s%i", id0, i);
	conf(id, x);
}


void confnum(char *id0, int i, double &x)
{
	STACKTRACE;
	char id[128];
	sprintf(id, "%s%i", id0, i);
	conf(id, x);
}


void confnum(char *id0, int i, char *x)
{
	STACKTRACE;
	char id[128];
	sprintf(id, "%s%i", id0, i);
	conf(id, x);
}


char *init_dir = "data";
char *source_dir = "save/save01";
char *target_dir = "save/save01";

void set_conf(char *f)
{
	STACKTRACE;
	char s[512];

	if (config_read)
		strcpy(s, source_dir);
	else
		strcpy(s, target_dir);

	// to-do:
	// check if the target-directory exists, if not, create it.
	//	if (!file_exists(s, FA_DIREC, 0))		// this is too limited, "f" can also contain directory info...
	//		mkdir(s);

	strcat(s, "/");
	strcat(s, f);

	tw_set_config_file(s);
}
