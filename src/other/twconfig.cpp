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

#include <stdio.h>
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

#include "twconfig.h"
#include "scp.h"

#include "melee.h"
#include "melee/mgame.h"
#include "melee/mfleet.h"
#include "util/aastr.h"

void tw_set_config_file(const std::string& filename)
{
	STACKTRACE;
	std::string file = home_ini_full_path(filename);
	//if (exists(file.c_str()))
	set_config_file(file.c_str());
	//  else
	//  tw_error("File %s does not exist", file.c_str());
}


int tw_delete_file(const std::string& filename)
{
	STACKTRACE;
	std::string file = home_ini_full_path(filename);
	return delete_file(file.c_str());
}


/*
Configuration string format

a/b/c/d == current config option a/b/c/d
/ini/a/b/c/d == ini option d in section c in file b in directory a
/dev/a/b == option b in device a
/cfg/a/b/c/d == read from ini, write to ini & config if they match

*/
const char *_get_ini_string ( const char *name )
{
	char *item;
	char *section;
	char file[1024];
	if (strlen(name) > 1000) tw_error("_get_ini_string - name too long");
	strncpy (file, name, 1000);
	char *_slash[2];
	_slash[0] = strchr(file, '/');
	if (!_slash[0]) {
		tw_error("_get_ini_string - bad name (%s)", name);
		return NULL;
	}
	_slash[1] = strchr(_slash[0] + 1, '/');
	if (!_slash[1]) {
		tw_error("_get_ini_string - bad name (%s)", name);
		return NULL;
	}

	int i = 1;

	char *tmp = strchr(_slash[i] + 1, '/');
	while ( tmp ) {
		i ^= 1;
		_slash[i] = tmp;
		tmp = strchr(_slash[i] + 1, '/');
	}

	item = _slash[i] + 1;
	item[-1] = 0;
	section = _slash[i^1] + 1;
	section[-1] = 0;
	tw_set_config_file(file);
	return get_config_string(section, item, NULL);
}


void _set_ini_string ( const char *name, const char *value )
{
	char *item;
	char *section;
	char file[1024];
	if (strlen(name) > 1000) tw_error("_set_ini_string - name too long");
	strncpy (file, name, 1000);
	char *_slash[2];
	_slash[0] = strchr(file, '/');
	if (!_slash[0]) {
		tw_error("_set_ini_string - bad name (%s)", name);
		return;
	}
	_slash[1] = strchr(_slash[0]+1, '/');
	if (!_slash[1] ) {
		tw_error("_set_ini_string - bad name (%s)", name);
		return;
	}

	int i = 1;
	char *tmp = strchr(_slash[i] + 1, '/');
	while (tmp ) {
		i ^= 1;
		_slash[i] = tmp;
		tmp = strchr(_slash[i] + 1, '/');
	}

	item = _slash[i] + 1;
	item[-1] = 0;
	section = _slash[i^1] + 1;
	section[-1] = 0;
	tw_set_config_file(file);
	set_config_string(section, item, value);
	return;
}


const char * twconfig_get_string (const char *item)
{
	char buffy[256];
	static char result[512];
	enum { INI, CFG, DEV, NORMAL};
	int type = -1;
	strcpy(buffy, item);
	if (*item == '/') {
		if (0) ;
		//else if (!strncmp(item, "/dev/", 5)) type = DEV;
		else if (!strncmp(item, "/ini/", 5)) type = INI;
		else if (!strncmp(item, "/cfg/", 5)) type = CFG;
		else tw_error("twconfig_get_string - unknown prefix");
	}
	else type = NORMAL;
	ConfigEvent ce;

	ce.value = NULL;
	ce.subtype = ConfigEvent::GET;

	ASSERT(type!=-1);
	switch (type) {
		case NORMAL:
		{
			ce.name = buffy;
			if (game) game->_event(&ce);
		} break;
		case INI:
		{
			ce.value = strdup(_get_ini_string(&buffy[5]));
		} break;
		case CFG:
		{
			ce.name = &buffy[5];
			if (game) game->_event(&ce);
			if (!ce.value) ce.value = strdup(_get_ini_string(&buffy[5]));
		} break;
		case DEV:
		{
			//ce.name = buffy;
			//if (game) game->_event(&ce);
		} break;
	}
	if (ce.value) {
		strncpy(result, ce.value, 500);
		free(ce.value);
	}
	return result;
}


void twconfig_set_string (const char *item, const char *value)
{
	char buffy[256];
	char buffy2[256];
	enum { INI, CFG, DEV, NORMAL};
	int type = -1;
	strcpy(buffy, item);
	if (*item == '/') {
		if (0) ;
		//else if (!strncmp(item, "/dev/", 5)) type = DEV;
		else if (!strncmp(item, "/ini/", 5)) type = INI;
		else if (!strncmp(item, "/cfg/", 5)) type = CFG;
		else tw_error("twconfig_get_string - unknown prefix");
	}
	else type = NORMAL;
	ConfigEvent ce;

	ce.name = buffy2;
	ce.value = (char*) value;
	ce.type = Event::TW_CONFIG;
	ce.subtype = ConfigEvent::SET;

	ASSERT(type!=-1);
	switch (type) {
		case NORMAL:
		{
			ce.name = buffy;
			if (game) game->_event(&ce);
		} break;
		case INI:
		{
			_set_ini_string(&buffy[5], value);
		} break;
		case CFG:
		{
			_set_ini_string(&buffy[5], value);
			ce.name = &buffy[5];
			if (game) game->_event(&ce);
		} break;
		case DEV:
		{
		} break;
	}
	return;
}


int twconfig_get_int (const char *item)
{
	const char *v = twconfig_get_string(item);
	if (!v) return 0;
	return atoi(v);;
}


void twconfig_set_int (const char *item, int value)
{
	char buffy[32];
	sprintf(buffy, "%d", value);
	twconfig_set_string(item, buffy);
	return;
}


double twconfig_get_float ( const char *item )
{
	const char *v = twconfig_get_string(item);
	if (!v) return 0;
	return atof(v);
}


void twconfig_set_float ( const char *item, double value )
{
	char buffy[64];
	sprintf(buffy, "%f", value);
	twconfig_set_string(item, buffy);
	return;
}
