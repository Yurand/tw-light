/*
This file is part of "TW-Light"
					http://tw-light.berlios.de/
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "melee.h"
REGISTER_FILE

#include "moptions.h"
#include "scp.h"
#include "gui.h"

#include "util/aastr.h"
#include "other/twconfig.h"
#include "other/dialogs.h"

/*

-	master menu
		client.ini
-			video mode
				exit (exit menu)
				apply (use settings)
				make default (use settings and save)
				screen resolution
				bits per pixel
				fullscreen
				gamma correction?
				custom color filter settings?
-			audio mode
				done (use settings, save, and exit menu)
				cancel (exit menu)
				sound disable
				sound volume
				sound channels?
				music disable
				music volume
				music channels?
				game specific settings?
-			other
-				keyboard configuration
				star depth
				star density
				antialiasing mode
				raw backup images
				alpha blending?
				mouse sensitivity?
-		server.ini
			tic rate
			friendly fire
			shot relativity
			map width
			map height
-		turbo
-		f4turbo

*/

void options_menu (Game *game)
{
	int a;
	while (true) {
		a = tw_popup_dialog(NULL, options_dialog, 0);
		switch (a) {
			default:
			case DIALOG_OPTIONS_DONE:
			{
				return;
			}
			break;
			case DIALOG_OPTIONS_VIDEO:
			{
				video_menu(game);
			}
			break;
			case DIALOG_OPTIONS_AUDIO:
			{
				audio_menu(game);
			}
			break;
			case DIALOG_OPTIONS_CONFIG:
			{
				config_menu(game);
			}
			break;
			case DIALOG_OPTIONS_PHYSICS:
			{
				physics_menu(game);
			}
			break;
			case DIALOG_OPTIONS_DEFAULT:
			{
			}
			break;
		}
	}
	return;
}


bool confirmVideoChanges()
{
	STACKTRACE;
	int choice = -1;
	while (-1 != (choice = tw_popup_dialog(NULL, confirmVideoDialog, 0))) {
		switch (choice) {
			case DIALOG_CONFIRM_VIDEO_YES:
				return true;
			case DIALOG_CONFIRM_VIDEO_NO:
				return false;
		}
	}
	return false;
}


char *resolution[] =
{
	"640x480", "800x600", "1024x768", "1280x1024", "Custom", NULL
};
char *color_depth[] =
{
	"8", "15", "16", "24", "32", NULL
};

int handleGammaSliderChange(void *dp3, int d2)
{
	STACKTRACE;
	set_gamma(d2);
	return d2;
}


void video_menu (Game *game)
{
	int choice = -1;
	bool done = false;

	while ( (choice != DIALOG_VIDEO_EXIT) && (!done) ) {
		sprintf(dialog_string[3], "%dx%d", videosystem.width, videosystem.height);

		//set index for resolution
		int x=-1, y=-1, x2, y2, i, bpp, bpp2, fs;
		x2 = videosystem.width;
		y2 = videosystem.height;
		for (i = 0; resolution[i+1]; i += 1) {
			x = strtol(resolution[i], NULL, 10);
			y = strtol(strchr(resolution[i], 'x') + 1, NULL, 10);
			if ((x == x2) && (y == y2)) break;
		}
		if (!resolution[0]) { tw_error("Resolution error"); }

		video_dialog[DIALOG_VIDEO_RESLIST].d1 = i;
		//set index for bpp
		bpp = videosystem.bpp;
		for (i = 0; true; i += 1) {
			if (!color_depth[i]) { tw_error("video_menu - current bpp invalid?"); }
			if (strtol(color_depth[i], NULL, 10) == bpp) break;
		}
		video_dialog[DIALOG_VIDEO_BPPLIST].d1 = i;

		//set button for fullscreen
		video_dialog[DIALOG_VIDEO_FULLSCREEN].flags = videosystem.fullscreen ? D_SELECTED : 0;

		int startfs = video_dialog[DIALOG_VIDEO_FULLSCREEN].flags;

		//set gamma correction
		video_dialog[DIALOG_VIDEO_GAMMA_SLIDER].d2 = get_gamma();

		//do the dialog
		choice = tw_popup_dialog(NULL, video_dialog, 0);
		if (choice == -1) choice = DIALOG_VIDEO_EXIT;

		//set resolution
		i = video_dialog[DIALOG_VIDEO_RESLIST].d1;
		char *tmp = resolution[i];
		if (!resolution[i+1]) tmp = dialog_string[3];
		x2 = strtol(tmp, NULL, 10);
		y2 = strtol(strchr(tmp, 'x') + 1, NULL, 10);

		//set bpp from menu
		i = video_dialog[DIALOG_VIDEO_BPPLIST].d1;
		bpp2 = strtol(color_depth[i], NULL, 10);

		//set fullscreen from menu
		fs = video_dialog[DIALOG_VIDEO_FULLSCREEN].flags & D_SELECTED;

		switch (choice) {
			case DIALOG_VIDEO_GET_DEFAULT:
				tw_set_config_file("client.ini");
				bpp2   = get_config_int("Video", "BitsPerPixel", 16);
				x2     = get_config_int("Video", "ScreenWidth", 640);
				y2     = get_config_int("Video", "ScreenHeight", 480);
				fs     = get_config_int("Video", "FullScreen", false);
				set_gamma(get_config_int("Video", "Gamma", 128));

				videosystem.set_resolution(x2, y2, bpp2, fs);
				break;

			case DIALOG_VIDEO_SET_DEFAULT:
				if ((bpp2 != bpp) && game) {
					tw_alert ("Color depths cannot be changed in\nthe middle of a game\nin this version", "Okay");
				}
				else {
					done = true;
				}
				break;

			case DIALOG_VIDEO_BPPLIST:
			case DIALOG_VIDEO_RESLIST:
				break;

			case DIALOG_VIDEO_GAMMA_SLIDER:
				set_gamma(video_dialog[DIALOG_VIDEO_GAMMA_SLIDER].d2);
				break;

			case DIALOG_VIDEO_EXIT:
			case -1:
				return;
				break;
		}

		if ( (x2 != x) ||
			(y2 != y) ||
			(bpp != bpp2) ||
		(startfs != fs) ) {
			set_gamma(video_dialog[DIALOG_VIDEO_GAMMA_SLIDER].d2);

			//try to set video mode.  If it does not work, back out, and do not confirm
			//the changes.
			if ( ! videosystem.set_resolution(x2, y2, bpp2, fs)) {
				done = false;
			}
			else {
				//if the video mode was actually set, confirm the changes
				if (confirmVideoChanges()) {
					tw_set_config_file("client.ini");
					set_config_int("Video", "BitsPerPixel", bpp2);
					set_config_int("Video", "ScreenWidth", x2);
					set_config_int("Video", "ScreenHeight", y2);
					set_config_int("Video", "FullScreen", fs);
					set_config_int("Video", "Gamma", get_gamma());
					return;
				}
				else {
					tw_set_config_file("client.ini");
					bpp2   = get_config_int("Video", "BitsPerPixel", 16);
					x2     = get_config_int("Video", "ScreenWidth", 640);
					y2     = get_config_int("Video", "ScreenHeight", 480);
					fs     = get_config_int("Video", "FullScreen", false);
					set_gamma(get_config_int("Video", "Gamma", 128));

					i = videosystem.set_resolution(x2, y2, bpp2, fs);
					done = false;
				}
			}
		}

	}
	return;
}


void audio_menu (Game *game)
{
	int i;

	//set dialog values
	audio_dialog[DIALOG_AUDIO_SOUND_ON].flags = sound.sound_on ? D_SELECTED : 0;
	audio_dialog[DIALOG_AUDIO_MUSIC_ON].flags = sound.music_on ? D_SELECTED : 0;
	audio_dialog[DIALOG_AUDIO_SOUND_VOL].d2 = sound.sound_volume;
	audio_dialog[DIALOG_AUDIO_MUSIC_VOL].d2 = sound.music_volume;

	//do the dialog
	i = tw_popup_dialog(NULL, audio_dialog, 0);
	if (i != DIALOG_AUDIO_OK) return;

	//set actual values
	sound.set_volumes(
		audio_dialog[DIALOG_AUDIO_SOUND_VOL].d2,
		audio_dialog[DIALOG_AUDIO_MUSIC_VOL].d2,
		audio_dialog[DIALOG_AUDIO_SOUND_ON].flags & D_SELECTED,
		audio_dialog[DIALOG_AUDIO_MUSIC_ON].flags & D_SELECTED
		);
	tw_set_config_file("client.ini");
	sound.save();
};

int handleSoundSliderChange(void *dp3, int d2)
{
	STACKTRACE;
	sound.set_volumes(
		audio_dialog[DIALOG_AUDIO_SOUND_VOL].d2,
		audio_dialog[DIALOG_AUDIO_MUSIC_VOL].d2,
		audio_dialog[DIALOG_AUDIO_SOUND_ON].flags & D_SELECTED,
		audio_dialog[DIALOG_AUDIO_MUSIC_ON].flags & D_SELECTED
		);
	return 0;
}


int handleMusicSliderChange(void *dp3, int d2)
{
	STACKTRACE;
	sound.set_volumes(
		audio_dialog[DIALOG_AUDIO_SOUND_VOL].d2,
		audio_dialog[DIALOG_AUDIO_MUSIC_VOL].d2,
		audio_dialog[DIALOG_AUDIO_SOUND_ON].flags & D_SELECTED,
		audio_dialog[DIALOG_AUDIO_MUSIC_ON].flags & D_SELECTED
		);
	return 0;
}


#include "mview.h"
#include "mgame.h"

char *viewListboxGetter(int index, int *list_size)
{
	static char tmp[40];
	tmp[0] = 0;
	if (index < 0) {
		*list_size = num_views;
		return NULL;
	}
	else {
		return(view_name[index]);
	}
}


void change_options()
{
	STACKTRACE;
	int optionsRet, i;

	tw_set_config_file("client.ini");
	//	old_optionsDialog[OPTIONS_DIALOG_AA].d1 = get_aa_mode();

	old_optionsDialog[OPTIONS_DIALOG_INTERPOLATION_ON].flags =
		interpolate_frames ? D_SELECTED : 0;
	int aa = get_config_int("Rendering", "AA_Mode", AA_NO_AA);
	old_optionsDialog[OPTIONS_DIALOG_AA_ON].flags =
		(aa&AA_NO_AA) ? 0 : D_SELECTED;
	old_optionsDialog[OPTIONS_DIALOG_NOALIGN_ON].flags =
		(aa&AA_NO_ALIGN) ? D_SELECTED : 0;
	old_optionsDialog[OPTIONS_DIALOG_BLEND_ON].flags =
		(aa&AA_BLEND) ? D_SELECTED : 0;
	old_optionsDialog[OPTIONS_DIALOG_ALPHA_ON].flags =
		(aa&AA_ALPHA) ? D_SELECTED : 0;

	tw_set_config_file("server.ini");
	old_optionsDialog[OPTIONS_DIALOG_STARS_SLIDER].d2 =
		get_config_int("Stars", "Depth", 192);
	old_optionsDialog[OPTIONS_DIALOG_RELATIVITY_SLIDER].d2 =
		iround(get_config_float("Game", "ShotRelativity", 0.5) * 1000);
	if (get_config_int("Game", "FriendlyFire", 1))
		old_optionsDialog[OPTIONS_DIALOG_FRIENDLY_FIRE].flags = D_SELECTED;
	else
		old_optionsDialog[OPTIONS_DIALOG_FRIENDLY_FIRE].flags = 0;

	if (get_config_int("View", "CameraHidesCloakers", 1))
		old_optionsDialog[OPTIONS_DIALOG_HIDE_CLOAKERS].flags = D_SELECTED;
	else
		old_optionsDialog[OPTIONS_DIALOG_HIDE_CLOAKERS].flags = 0;

	tw_set_config_file("client.ini");

	if (get_config_int("Planet", "PlanetDimension", 2) == 3)
		old_optionsDialog[OPTIONS_DIALOG_3DPLANET].flags = D_SELECTED;
	else
		old_optionsDialog[OPTIONS_DIALOG_3DPLANET].flags = 0;

	i = get_view_num ( get_config_string ( "View", "View", NULL ) );
	if (i == -1) i = 0;
	old_optionsDialog[OPTIONS_DIALOG_VIEW].d1 = i;

	optionsRet = tw_popup_dialog(NULL, old_optionsDialog, OPTIONS_DIALOG_OK);

	if (optionsRet == OPTIONS_DIALOG_CANCEL) return;

	//	set_aa_mode(old_optionsDialog[OPTIONS_DIALOG_AA].d1);

	tw_set_config_file("client.ini");
	//	set_config_int("View", "Anti-Aliasing", get_aa_mode());
	interpolate_frames =
		(old_optionsDialog[OPTIONS_DIALOG_INTERPOLATION_ON].flags & D_SELECTED) ? 1 : 0;
	set_config_int("View", "InterpolateFrames", interpolate_frames);
	aa&=~AA_NO_AA;aa|= (old_optionsDialog[OPTIONS_DIALOG_AA_ON].flags) ?
		0 : AA_NO_AA;
	aa&=~AA_NO_ALIGN;aa|= (old_optionsDialog[OPTIONS_DIALOG_NOALIGN_ON].flags) ?
		AA_NO_ALIGN : 0;
	aa&=~AA_BLEND;aa|= (old_optionsDialog[OPTIONS_DIALOG_BLEND_ON].flags) ?
		AA_BLEND : 0;
	aa&=~AA_ALPHA;aa|= (old_optionsDialog[OPTIONS_DIALOG_ALPHA_ON].flags) ?
		AA_ALPHA : 0;
	set_config_int("Rendering", "AA_Mode", aa);
	set_tw_aa_mode(aa);

	if (old_optionsDialog[OPTIONS_DIALOG_3DPLANET].flags == D_SELECTED)
		i = 3;
	else
		i = 2;
	set_config_int("Planet", "PlanetDimension", i);

	View *v = get_view(
		view_name[old_optionsDialog[OPTIONS_DIALOG_VIEW].d1],
		NULL
		);
	set_view(v);
	twconfig_set_string("/cfg/client.ini/view/view",
		view_name[old_optionsDialog[OPTIONS_DIALOG_VIEW].d1]);
	if (game && !game->view_locked) game->change_view(
			view_name[old_optionsDialog[OPTIONS_DIALOG_VIEW].d1]);

	twconfig_set_int("/cfg/server.ini/stars/depth",
		old_optionsDialog[OPTIONS_DIALOG_STARS_SLIDER].d2);

	twconfig_set_float("/cfg/server.ini/game/shotrelativity",
		old_optionsDialog[OPTIONS_DIALOG_RELATIVITY_SLIDER].d2 / 1000.0);

	if (old_optionsDialog[OPTIONS_DIALOG_FRIENDLY_FIRE].flags & D_SELECTED)
		i = 1;
	else i = 0;
	twconfig_set_int("/cfg/server.ini/game/friendlyfire", i);

	if (old_optionsDialog[OPTIONS_DIALOG_HIDE_CLOAKERS].flags & D_SELECTED)
		i = 1;
	else i = 0;
	twconfig_set_int("/cfg/server.ini/view/camerahidescloakers", i);

	return;
}


void config_menu (Game *game)
{
	change_options();
};
void physics_menu (Game *game)
{
};
