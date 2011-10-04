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

#ifndef __TW_DIALOGS_H__
#define __TW_DIALOGS_H__

#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif
extern "C"
{
	#include "libraries/agup/agup.h"
}


#define d_box_proc d_agup_box_proc
#define d_shadow_box_proc d_agup_shadow_box_proc
#define d_button_proc d_agup_button_proc
#define d_push_proc d_agup_push_proc
#define d_check_proc d_agup_check_proc
#define d_radio_proc d_agup_radio_proc
#define d_icon_proc d_agup_icon_proc
#define d_edit_proc d_agup_edit_proc
#define d_list_proc d_agup_list_proc
#define d_text_list_proc d_agup_text_list_proc
#define d_textbox_proc d_agup_textbox_proc
#define d_slider_proc d_agup_slider_proc
#define d_menu_proc d_agup_menu_proc
#define d_window_proc d_agup_window_proc
#define d_text_proc d_agup_text_proc
#define d_ctext_proc d_agup_ctext_proc
#define d_rtext_proc d_agup_rtext_proc
#define d_clear_proc d_agup_clear_proc

enum
{
	JOY_DIALOG_BOX = 0,
	JOY_DIALOG_TITLE,
	JOY_DIALOG_CALIBRATE,
	JOY_DIALOG_SWITCH,
	JOY_DIALOG_DONE,
	JOY_DIALOG_DESCRIPTION,
};

extern DIALOG joyDialog[];

enum
{
	KEY_DIALOG_MODIFY = 0,
	KEY_DIALOG_OK = 17,
	KEY_DIALOG_CANCEL,
	KEY_DIALOG_CALIBRATE,
};

extern DIALOG keyDialog[];

extern DIALOG tw_alert_dialog1[];
extern DIALOG tw_alert_dialog2[];
extern DIALOG tw_alert_dialog3[];
extern DIALOG tw_alert_dialog4[];
extern DIALOG tw_alert_dialog5[];

enum
{
	SELECT_DIALOG_LIST = 0,
	SELECT_DIALOG_TITLE,
	SELECT_DIALOG_SHIP,
	SELECT_DIALOG_RANDOM,
	SELECT_DIALOG_ARANDOM,
	SELECT_DIALOG_INFO,
	SELECT_DIALOG_PIC
};

extern DIALOG selectDialog[];
extern DIALOG help_dialog[];

enum
{
	DIALOG_OPTIONS_BOX = 0,
	DIALOG_OPTIONS_DONE,
	DIALOG_OPTIONS_VIDEO,
	DIALOG_OPTIONS_AUDIO,
	DIALOG_OPTIONS_CONFIG,
	DIALOG_OPTIONS_PHYSICS,
	DIALOG_OPTIONS_DEFAULT
};

extern DIALOG options_dialog[];

enum
{
	DIALOG_CONFIRM_VIDEO_BOX = 0,
	DIALOG_CONFIRM_VIDEO_TEXT,
	DIALOG_CONFIRM_VIDEO_YES,
	DIALOG_CONFIRM_VIDEO_NO
};

extern DIALOG confirmVideoDialog[];

enum
{
	DIALOG_VIDEO_BOX = 0,
	DIALOG_VIDEO_FULLSCREEN,
	DIALOG_VIDEO_RESTEXT,
	DIALOG_VIDEO_RESLIST,
	DIALOG_VIDEO_BPPTEXT,
	DIALOG_VIDEO_BPPLIST,
	DIALOG_VIDEO_EXIT,
	DIALOG_VIDEO_GET_DEFAULT,
	DIALOG_VIDEO_OK,
	DIALOG_VIDEO_GAMMA_TEXT,
	DIALOG_VIDEO_GAMMA_SLIDER,
};
extern DIALOG video_dialog[13];

enum
{
	DIALOG_AUDIO_BOX = 0,
	DIALOG_AUDIO_OK,
	DIALOG_AUDIO_CANCEL,
	DIALOG_AUDIO_SOUND_ON,
	DIALOG_AUDIO_SOUND_VOL,
	DIALOG_AUDIO_MUSIC_ON,
	DIALOG_AUDIO_MUSIC_VOL
};

extern DIALOG audio_dialog[];
enum
{
	OPTIONS_DIALOG_BOX = 0,
	OPTIONS_DIALOG_STARS_TEXT,
	OPTIONS_DIALOG_STARS_SLIDER,
	OPTIONS_DIALOG_RELATIVITY_TEXT,
	OPTIONS_DIALOG_RELATIVITY_SLIDER,
	OPTIONS_DIALOG_FRIENDLY_FIRE,
	OPTIONS_DIALOG_HIDE_CLOAKERS,
	OPTIONS_DIALOG_3DPLANET,
	OPTIONS_DIALOG_VIEW_TEXT,
	OPTIONS_DIALOG_VIEW,
	OPTIONS_DIALOG_OK,
	OPTIONS_DIALOG_CANCEL,

	OPTIONS_DIALOG_QUALITY_TEXT,
	OPTIONS_DIALOG_INTERPOLATION_ON,
	OPTIONS_DIALOG_AA_ON,
	OPTIONS_DIALOG_NOALIGN_ON,
	OPTIONS_DIALOG_BLEND_ON,
	OPTIONS_DIALOG_ALPHA_ON,

	OPTIONS_DIALOG_END
};

extern DIALOG old_optionsDialog[];

enum
{
	MAIN_DIALOG_BOX = 0,
	MAIN_DIALOG_MELEE,
	MAIN_DIALOG_GOB,
	MAIN_DIALOG_MELEE_EXTENDED,
	MAIN_DIALOG_TEAMS,
	MAIN_DIALOG_OPTIONS,
	MAIN_DIALOG_HELP,
	MAIN_DIALOG_EXIT,
};

extern DIALOG mainDialog[];

#define FLEET_TITLE_DIALOG_BOX    0
#define FLEET_TITLE_DIALOG_EDIT   1
#define FLEET_TITLE_DIALOG_OK     2
#define FLEET_TITLE_DIALOG_CANCEL 3

extern char title_str[80];
extern DIALOG fleet_titleDialog[];

extern DIALOG select_game_dialog[];

// MELEE_EX - dialog objects
enum
{
	MELEE_EX_DIALOG_BOX = 0,
	MELEE_EX_DIALOG_PLAY_GAME,
	MELEE_EX_DIALOG_KEYTESTER,
	MELEE_EX_DIALOG_SHIPINFO,
	MELEE_EX_DIALOG_DIAGNOSTICS,
	MELEE_EX_DIALOG_EXIT
};
extern DIALOG melee_ex_dialog[];

// TEAMS - dialog objects
enum
{
	TEAMS_DIALOG_BOX = 0,
	TEAMS_DIALOG_TITLE,
	TEAMS_DIALOG_PLAYERLIST_TEXT,
	TEAMS_DIALOG_PLAYERLIST,
	TEAMS_DIALOG_CONTROLLIST,
	TEAMS_DIALOG_SELECTCONTROL,
	TEAMS_DIALOG_TEAM_NUM,
	TEAMS_DIALOG_CONFIG_NUM,
	TEAMS_DIALOG_SETUP,
	TEAMS_DIALOG_FLEET,
	TEAMS_DIALOG_MAINMENU
};

extern DIALOG teamsDialog[];

// FLEET - dialog objects
enum
{
	FLEET_DIALOG_AVAILABLE_SHIPS_TEXT=0,
	FLEET_DIALOG_SHIP_CATAGORIES_TEXT,
	FLEET_DIALOG_TW_OFFICIAL_TOGGLE,
	FLEET_DIALOG_TW_EXP_TOGGLE,
	FLEET_DIALOG_TW_SPECIAL_TOGGLE,
	FLEET_DIALOG_TWA_TOGGLE,
	FLEET_DIALOG_SORTBY_TEXT1,
	FLEET_DIALOG_SORTBY_BUTTON1,
	FLEET_DIALOG_SORTBY_ASCENDING1,
	FLEET_DIALOG_AVAILABLE_SHIPS_LIST,
	FLEET_DIALOG_FLEET_SHIPS_LIST,
	FLEET_DIALOG_PLAYER_FLEET_BUTTON,
	FLEET_DIALOG_PLAYER_FLEET_TITLE,
	FLEET_DIALOG_SAVE_BUTTON,
	FLEET_DIALOG_LOAD_BUTTON,
	FLEET_DIALOG_POINT_LIMIT_TEXT,
	FLEET_DIALOG_POINT_LIMIT_BUTTON,
	FLEET_DIALOG_CURRENT_POINTS_TEXT,
	FLEET_DIALOG_CURRENT_POINTS_VALUE,
	FLEET_DIALOG_SORTBY_TEXT2,
	FLEET_DIALOG_SORTBY_BUTTON2,
	FLEET_DIALOG_SORTBY_ASCENDING2,
	FLEET_DIALOG_ADD_BUTTON,
	FLEET_DIALOG_ADD_ALL_BUTTON,
	FLEET_DIALOG_CLEAR,
	FLEET_DIALOG_CLEARALL,
	FLEET_DIALOG_SHIP_PICTURE_BITMAP,
	FLEET_DIALOG_SHIP_SUMMARY_TEXT,
	FLEET_DIALOG_BACK_BUTTON,
	FLEET_DIALOG_HELP_TEXT		 /**/
};

extern DIALOG fleetDialog[];

// SHIPVIEW - dialog objects
enum
{
	SHIPVIEW_DIALOG_DONE = 1,
	SHIPVIEW_DIALOG_FONT,
	SHIPVIEW_DIALOG_SORT,
	SHIPVIEW_DIALOG_LIST,
	SHIPVIEW_DIALOG_DESCRIPTION,
	SHIPVIEW_DIALOG_TXTFILE,
	SHIPVIEW_DIALOG_PICTURES,
	SHIPVIEW_DIALOG_TWYIELD,
};

// list of sort algorithm names
extern const char *sorttypes[];
enum
{
	SHIPVIEW_DIALOG_SORT_BYNAME,
	SHIPVIEW_DIALOG_SORT_BYCOST,
	SHIPVIEW_DIALOG_SORT_BYORIGIN,
	SHIPVIEW_DIALOG_SORT_BYCODERS,
};

extern DIALOG shipviewDialog[];

// DIAGNOSTICS - dialog objects
enum
{
	DIAGNOSTICS_DIALOG_EXIT=1,
	DIAGNOSTICS_DIALOG_MAIN=3,
	DIAGNOSTICS_DIALOG_FILES=5,
	DIAGNOSTICS_DIALOG_VERSION_TXT=7,
};

extern DIALOG diagnostics_dialog[];
#endif
