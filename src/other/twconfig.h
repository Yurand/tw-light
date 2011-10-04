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

#ifndef TWCONFIG_H

#include <string>

void tw_set_config_file(const std::string& filename);
int tw_delete_file(const std::string& filename);

const char *twconfig_get_string (const char *item);
//string destroyed by next call to twconfig_get_*
void   twconfig_set_string (const char *item, const char *value);
int    twconfig_get_int ( const char *item );
void   twconfig_set_int ( const char *item, int value );
void   twconfig_set_float ( const char *item, double value );
double twconfig_get_float ( const char *item );
#endif							 // TWCONFIG_H
