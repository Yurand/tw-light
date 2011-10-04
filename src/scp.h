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

#ifndef __SCP_H__
#define __SCP_H__

class VideoWindow;
#include "allegro.h"
//class SAMPLE;

#include <string>

const char *tw_version();

void change_teams();
void edit_fleet(int player) ;
//void change_options() ;
int connect_menu(VideoWindow *window, char **address, int *port) ;
int is_escape_pressed() ;
void ship_view_dialog(int si = 0, class Fleet *fleet = NULL);

int scp_fleet_dialog_text_list_proc(int msg, DIALOG* d, int c);
int scp_fleet_dialog_bitmap_proc(int msg, DIALOG* d, int c);

int d_check_proc_fleeteditor(int msg, DIALOG *d, int c);
// list box getter functions
char *playerListboxGetter(int index, int *list_size) ;
const char *controlListboxGetter(int index, int *list_size) ;
char *viewListboxGetter(int index, int *list_size) ;

int handleSoundSliderChange(void *dp3, int d2);
int handleMusicSliderChange(void *dp3, int d2);

extern SAMPLE * menuAccept;
extern SAMPLE * menuFocus;
extern SAMPLE * menuDisabled;
extern SAMPLE * menuSpecial;

int create_user_ini();
std::string data_full_path(std::string path);
std::string home_ini_full_path(std::string path);
#endif							 // __SCP_H__
