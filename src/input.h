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

/*
Key format
*/
typedef short int Key;

int enable_input ( int which = 255) ;
int disable_input ( int which = 255 ) ;
void poll_input ();

int key_pressed(Key key) ;
Key name_to_key ( const char *name);

Key get_key();

int key_to_name( Key key, char *buffer );
int key_to_description( Key key, char *buffer );
//returns at most 63 characters

int joykey_enumerate (Key *keys) ;
//no overflow check, better have plenty of room
//the max possible is about 2000
//typical is 6-12
