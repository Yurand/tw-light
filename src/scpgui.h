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
Modifications to the Allegro GUI
*/
//for Allegro's d_text_list_proc
char *shipListboxGetter(int index, int *list_size) ;
char *shippointsListboxGetter(int index, int *list_size) ;

int d_list_proc2(int msg, DIALOG *d, int c);
//for the new d_list_proc2
char *genericListboxGetter(int index, int *list_size, char **list) ;
char *fleetListboxGetter(int index, int *list_size, class Fleet *fleet) ;
char *fleetpointsListboxGetter(int index, int *list_size, class Fleet *fleet) ;

//button proc with standard SC sounds :)
int my_d_button_proc(int msg, DIALOG * d, int c);

// this is a flag for buttons which specifies that its activate sound should be special.
// you should OR this with the button's flags to enable it.
#define D_SPECIAL_BUTTON (D_USER*2)

/*
TimeWarps own custom GUI
*/
