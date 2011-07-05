/*
 *      scp.cpp
 *
 *      Copyright 2001-2004  TimeWarp development team
 *      Copyright 2008 Yura Siamashka <yurand2@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <allegro.h>
#ifdef WIN32
#include <allegro/platform/aintwin.h>
#include <winalleg.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "other/twconfig.h"
#include "other/dialogs.h"

#include "util/helper.h"
#include "util/port.h"

#ifdef ALLEGRO_MSVC
#pragma warning (disable:4786)
#endif

#ifdef ALLEGRO_MSVC
#include <crtdbg.h>
#endif

const char *tw_version()
{
	static char tw_version_string[1024];
	sprintf(tw_version_string, "%s %s version: %s", __TIME__, __DATE__, VERSION);
	return tw_version_string;
}


#include "melee.h"
REGISTER_FILE
#include "scp.h"
#include "gui.h"
#include "frame.h"

#include "util/get_time.h"

#include "melee/mview.h"
#include "melee/mcontrol.h"
#include "melee/mcbodies.h"
#include "melee/mgame.h"

#include "melee/mmain.h"
#include "melee/mnet1.h"
#include "util/aastr.h"
#include "melee/mship.h"		 //remove
#include "melee/mfleet.h"

#include "util/sounds.h"

//deprecated.  This mode of using dat files is terrible, I can't believe
//this technique was ever created.
#define SCPGUI_TITLE   0

TW_DATAFILE *scp = NULL;

FILE *debug_file;

/*! \brief Blits GUI background bitmap on to a video window */
void showTitle(VideoWindow *window = &videosystem.window);

/*! \brief MELEE_EX dialog - selects alternate games as opposed to standard melee.
  From here you can access diagnostics (DIAGNOSTICS dialog) and ship info.
  (SHIPVIEW dialog) You can also test key jamming from this dialog.
 */
void extended_menu(int i = -1);

/*! \brief TEAMS dialog - from here you can select controllers for each player, and access their respective fleets. (FLEET dialog)
 */
void change_teams();

/*! \brief FLEET dialog - manages fleet compositions for an individual player.
  \param player Player index indicating which player's fleet to edit.
 */
void edit_fleet(int player);

/*! \brief SHIPVIEW dialog - displays statistics and text information (if available) about the currently installed ships.
  \param si Ship index. By default 0, the first ship in the fleet.
  \param fleet Pointer to a fleet upon which the ship list is built. When
  this parameter is set to the default value NULL, the reference fleet is
  used to build the ship list.
*/
//void ship_view_dialog(int si = 0, Fleet *fleet = NULL);

/*! \brief DIAGNOSTICS dialog - displays version number and platform data */
void show_diagnostics();

/*! \brief Opens a screen showing which keys are currently pressed. Here the user may test various key combinations for conflicts. */
void keyjamming_tester();

void play_demo(const char *file_name = "demo.dmo") ;
void play_game(const char *_gametype_name, Log *_log = NULL) ;
void play_net1client ( const char * address = NULL, int port = -1 ) ;
void play_net1server ( const char *_gametype_name, int port = -1 ) ;

int getKey();

Log *new_log (int logtype)
{
	union { Log *log; NetLog *netlog; };
	log = NULL;

	switch (logtype) {
		case Log::log_normal:
		{
			log = new Log();
			log->init();
			return log;
		}
		case Log::log_net1server:
		{
			netlog = new NetLog();
			netlog->init();
			netlog->type = Log::log_net1server;
			return netlog;
		}
		default:
		{
			tw_error("that is not a valid log type");
		}
	}
	return NULL;
}


char *detect_gametype( Log *_log )
{
	int ltype;
	_log->unbuffer(Game::channel_init, &ltype, sizeof(int));
	ltype = intel_ordering(ltype);
	int gnamelength;
	_log->unbuffer(Game::channel_init, &gnamelength, sizeof(int));
	gnamelength = intel_ordering(gnamelength);
	if (gnamelength > 1000) {
		tw_error("Game name too long");
		gnamelength = 1000;
	}
	char buffy[1024];
	_log->unbuffer(Game::channel_init, &buffy, gnamelength);
	buffy[gnamelength] = 0;
	_log->reset();
	return strdup(buffy);
}


Music * titleMusic   = NULL;
SAMPLE * menuAccept   = NULL;
SAMPLE * menuFocus    = NULL;
SAMPLE * menuDisabled = NULL;
SAMPLE * menuSpecial  = NULL;
BITMAP * titlePic     = NULL;

/**
  loads up the title screen and music, and starts playing the background menu music.
*/
void prepareTitleScreenAssets()
{
	STACKTRACE;
	scp = tw_load_datafile(data_full_path("scpgui.dat").c_str());
	if (!scp)
		tw_error("Couldnt load title music");

	{
		TW_DATAFILE * data = tw_load_datafile_object(data_full_path("titlescreen.dat").c_str(), "TITLEMUSIC");
		if (data != NULL && data->type==DAT_SAMPLE) {
			titleMusic = (Music *) data->dat;
		}
	}

	if (!titleMusic && sound.is_music_supported())
		tw_error("Couldnt load title music");

	if (titleMusic)
		sound.play_music( titleMusic, TRUE);

	{
		TW_DATAFILE * data = tw_load_datafile_object(data_full_path("titlescreen.dat").c_str(), "MENUACCEPT");
		if (data != NULL && data->type==DAT_SAMPLE) {
			menuAccept = (SAMPLE*) data->dat;
		}
	}

	{
		TW_DATAFILE * data = tw_load_datafile_object(data_full_path("titlescreen.dat").c_str(), "MENUFOCUS");
		if (data != NULL && data->type==DAT_SAMPLE) {
			menuFocus = (SAMPLE*) data->dat;
		}
	}

	{
		TW_DATAFILE * data = tw_load_datafile_object(data_full_path("titlescreen.dat").c_str(), "MENUDISABLED");
		if (data != NULL && data->type==DAT_SAMPLE) {
			menuDisabled = (SAMPLE*) data->dat;
		}
	}

	{
		TW_DATAFILE * data = tw_load_datafile_object(data_full_path("titlescreen.dat").c_str(), "MENUSPECIAL");
		if (data != NULL && data->type==DAT_SAMPLE) {
			menuSpecial = (SAMPLE*) data->dat;
		}
	}
	{
		TW_DATAFILE * data = tw_load_datafile_object(data_full_path("scpgui.dat").c_str(), "SCPTITLE");
		titlePic = create_bitmap(videosystem.window.w, videosystem.window.h);
		aa_stretch_blit((BITMAP*)data->dat, titlePic,
			0,0,((BITMAP*)data->dat)->w,((BITMAP*)data->dat)->h,
			0, 0, titlePic->w, titlePic->h);
		tw_unload_datafile_object(data);

	}
}


/** clears the screen, and displays a loading message to the user.
 */
void showLoadingScreen()
{
	STACKTRACE;
	static BITMAP * logo = NULL;
	static int depth = bitmap_color_depth(screen);
	std::string path;

	acquire_screen();
	clear_to_color(screen, 0);

	if (NULL == logo || bitmap_color_depth(screen) != depth) {
		depth = bitmap_color_depth(screen);

		path = data_full_path("titlescreen.dat");
		TW_DATAFILE * data = tw_load_datafile_object(path.c_str(),"LOGO");
		if (data != NULL && data->type==DAT_BITMAP) {
			BITMAP * temp = (BITMAP*) data->dat;
			logo = create_bitmap(temp->w, temp->h);
			blit(temp, logo, 0,0, 0,0, temp->w, temp->h);
			tw_unload_datafile_object(data);
		}
	}

	if (logo != NULL ) {
		if (screen->w/2 >= logo->w) {
			draw_sprite(screen, logo, screen->w/2 - logo->w/2, screen->h/2 - logo->h/2);
		} else {
			float ratio = logo->w / logo->h;

			int h = screen->h/4;
			int w = iround(ratio * h);

			stretch_blit(logo, screen,
				0,0,
				logo->w, logo->h,
				screen->w/2 - w/2, screen->h/2 - h/2,
				w, h);
		}
	}

	const char * loadString = "Loading...";
	textout_right(screen, font, loadString,
		screen->w - 1*text_length(font, loadString), screen->h - 4*text_height(font),
		palette_color[15]);
	release_screen();
}


int is_escape_pressed()
{
	STACKTRACE;
	poll_keyboard();
	return key[KEY_ESC];
}


void play_game(const char *_gametype_name, Log *_log)
{
	STACKTRACE;
	bool gui_stuff = false;
	char gametype_name[1024];
	char *c;
	Game *new_game = NULL;

	showLoadingScreen();

	strncpy(gametype_name, _gametype_name, 1000);
	for (c = strchr(gametype_name, '_'); c; c = strchr(c, '_'))
		*c = ' ';

	if (scp) {
		gui_stuff = true;
		sound.stop_music();
		if (scp) tw_unload_datafile(scp);
		scp = NULL;
	}

	try
	{
		if (game) {
			delete game;
			game = NULL;
		}

		if (!_log) {
			_log = new Log();
			_log->init();
		}

		GameType *type = gametype(gametype_name);
		if (type)
			new_game = type->new_game();
		else
			tw_error("wait a sec... I can't find that game type");

		new_game->preinit();
		new_game->window = new VideoWindow;
		new_game->window->preinit();
		new_game->window->init(&videosystem.window);
		new_game->window->locate(0,0,0,0,0,1,0,1);
		new_game->init(_log);
		new_game->play();
		new_game->log->deinit();
		//game = NULL;
		new_game->game_done = true;
	}

	catch (int i) {
		if (i == -1) throw;
		if (__error_flag & 1) throw;
		if (i != 0) caught_error ("%s %s caught int %d", __FILE__, __LINE__, i);
		if (__error_flag & 1) throw;
	}
	catch (const char *str) {
		if (__error_flag & 1) throw;
		caught_error("message: \"%s\"", str);
		if (__error_flag & 1) throw;
	}
	catch (...) {
		if (__error_flag & 1) throw;
		caught_error("Ack(2)!!!\nAn error occured!\nBut I don't know what error!");
		if (__error_flag & 1) throw;
	}

	if (gui_stuff) {
		prepareTitleScreenAssets();
		showTitle();
	}
	return;
}


char dialog_string[20][128];

int MAX_PLAYERS = 1;
int MAX_CONFIGURATIONS = 1;
int MAX_TEAMS = 1;

// list box getter functions
char *playerListboxGetter(int index, int *list_size) ;
const char *controlListboxGetter(int index, int *list_size) ;
char *viewListboxGetter(int index, int *list_size) ;

FONT *TW_font = NULL;

// dialog results
/*int mainRet = 0;
int shipRet = 0;
int keyRet = 0;
int fleetRet = 0;
int optionsRet= 0;*/

const char **player_type = NULL;
int *player_config = NULL;
int *player_team = NULL;

Control *load_player(int i)
{
	char tmp[32];
	Control *r = NULL;

	sprintf (tmp, "Config%d", player_config[i]);
	r = getController(player_type[i], tmp, Game::channel_none);
	if (r) {
		r->load("scp.ini", tmp);
	}
	return r;
}


class MainMenu : public BaseClass
{
	public:
		virtual void _event(Event * e);
		virtual void preinit();
		virtual void deinit();
		virtual void init(VideoWindow *parent);
		virtual void doit();
		virtual void enable();
		virtual void disable();
		int state;
		VideoWindow *window;
} mainmenu;

void MainMenu::_event(Event *e)
{
	STACKTRACE;
	if (e->type == Event::VIDEO) {
		if (e->subtype == VideoEvent::REDRAW) if (state & 1) showTitle();
	}
}


void MainMenu::enable()
{
	STACKTRACE;
	if (!(state & 2)) window->add_callback(this);
	state |= 3;
}


void MainMenu::disable()
{
	STACKTRACE;
	state &=~ 1;
}


void MainMenu::preinit()
{
	STACKTRACE;
	window = NULL;
	state = 0;
}


void MainMenu::init(VideoWindow *parent)
{
	STACKTRACE;
	if (window) window->init(parent);
	else {
		window = new VideoWindow();
		window->preinit();
		window->init(parent);
	}
}


void MainMenu::deinit()
{
	STACKTRACE;
	if (state & 2) {
		window->remove_callback(this);
		window->deinit();
		delete window;
		window = NULL;
	}
}


void MainMenu::doit()
{
	STACKTRACE;
	int i;
	char tmp[32];

	tw_set_config_file("scp.ini");
	if (!player_type) {
		MAX_PLAYERS        = get_config_int("Limits", "MaxPlayers", 12);
		MAX_CONFIGURATIONS = get_config_int("Limits", "MaxConfigurations", 4);
		MAX_TEAMS          = get_config_int("Limits", "MaxTeams", 6);
		player_type = new const char*[MAX_PLAYERS];
		player_config = new int[MAX_PLAYERS];
		player_team   = new int[MAX_PLAYERS];
	}
	for (i = 0; i < MAX_PLAYERS; i += 1) {
		sprintf(tmp, "Player%d", i+1);
		player_type[i] = strdup(get_config_string(tmp, "Type", "Human"));
		player_config[i] = get_config_int (tmp, "Config", i % MAX_CONFIGURATIONS);
		player_team[i] = get_config_int (tmp, "Team", 0);
	}

	prepareTitleScreenAssets();
	showTitle();
	enable();

	int mainRet;
	do {
		//mainRet = popup_dialog(mainDialog, MAIN_DIALOG_MELEE);
		mainRet = tw_do_dialog(window, mainDialog, MAIN_DIALOG_MELEE);
		switch (mainRet) {
			case MAIN_DIALOG_MELEE:
				disable();
				play_game("Melee");
				enable();
				break;
			case MAIN_DIALOG_GOB:
				disable();
				play_game("GOB");
				enable();
				break;
			case MAIN_DIALOG_MELEE_EXTENDED:
				disable();
				extended_menu();
				enable();
				break;
			case MAIN_DIALOG_OPTIONS:
				showTitle();
				options_menu(NULL);
				break;
			case MAIN_DIALOG_HELP:
				show_file(data_full_path("ingame.txt").c_str());
				break;
			case MAIN_DIALOG_TEAMS:
				change_teams();
				showTitle();
				break;
		}
	} while((mainRet != MAIN_DIALOG_EXIT) && (mainRet != -1));

}


int tw_main(int argc, char *argv[]);

#ifndef UNITTEST
int main(int argc, char *argv[])
{
	STACKTRACE;
	int r;
	r = tw_main(argc, argv);
	return r;
}


END_OF_MAIN();
#endif

int tw_main(int argc, char *argv[])
{
	STACKTRACE;
	#ifdef WIN32
	char szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, sizeof(szPath));
	if (strrchr(szPath, '\\')) *strrchr(szPath, '\\') = '\0';
	SetCurrentDirectory(szPath);
	#endif
	int i;
	int auto_port = -1;
	const char *auto_play = NULL, *auto_param = NULL;

	#ifdef __BEOS__
	// set cwd to path of exe
	// to allow running from icon
	char datapath[256];
	for (i=strlen(argv[0]) ; argv[0][i]!='/' ; i--);
	strncpy(datapath, argv[0], i);
	chdir(datapath);
	#endif

	#if (defined _MSC_VER) && (defined _DEBUG)
	_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG|_CRTDBG_LEAK_CHECK_DF);
	#endif

	for (i = 1; i < argc; i += 1) {
		if (strcmp(argv[i], "-log") == 0) {
			log_debug(NULL);
			break;
		}
	}

	time_t start_time = time(NULL);
	log_debug("Log started at %s\n", asctime(localtime(&start_time)));
	if (allegro_init() < 0)
		tw_error_exit("Allegro initialization failed");
	create_user_ini();
	videosystem.preinit();

	register_ships();

	try
	{
		init_time();
		init_error();

		set_window_title("TW-Light");
		tw_set_config_file("client.ini");

		int screen_width = 640, screen_height = 480, screen_bpp = 32;
		int fullscreen = 0;

		auto_unload = get_config_int("System", "AutoUnload", 0);

		const AGUP_THEME *theme = agup_theme_by_name(get_config_string("Video", "GuiTheme", "Photon"));

		if (get_config_int("Video", "NativeResolution", 1)) {
			if (tw_get_desktop_resolution(&screen_width, &screen_height)) {
				tw_error("Unable to get desktop resolution");
			}
		} else {
			screen_width     = get_config_int("Video", "ScreenWidth", 640);
			screen_height    = get_config_int("Video", "ScreenHeight", 480);
		}
		if (get_config_int("Video", "NativeBpp", 1)) {
			screen_bpp = tw_desktop_color_depth();
		} else {
			screen_bpp       = get_config_int("Video", "BitsPerPixel", 16);
		}
		fullscreen       = get_config_int("Video", "FullScreen", false);

		SpaceSprite::mip_bias = get_config_int ("View", "Mip_bias", 0);
		SpaceSprite::mip_min = get_config_int ("View", "Mip_min", 0);
		SpaceSprite::mip_max = get_config_int ("View", "Mip_max", 0);

		interpolate_frames = get_config_int("View", "InterpolateFrames", 0);
		set_tw_aa_mode(get_config_int("Rendering", "AA_Mode", 0));
		int gamma   = get_config_int("Video", "Gamma", 128);
		set_gamma( gamma );

		int inputs = 7;

		// parse command-line arguments
		for (i = 1; i < argc; i += 1) {
			if (false) ;
			else if (!strcmp(argv[i], "-res") && (argc > i + 2)) {
				log_debug("command-line argument -res\n");
				screen_width = atoi(argv[i+1]);
				screen_height = atoi(argv[i+2]);
				i += 2;
			}
			else if (!strcmp(argv[i], "-bpp") && (argc > i + 1)) {
				log_debug("command-line argument -bpp\n");
				screen_bpp = atoi(argv[i+1]);
				i += 1;
			}
			else if (!strcmp(argv[i], "-fullscreen") && (argc > i + 0)) {
				log_debug("command-line argument -fullscreen\n");
				fullscreen = true;
			}
			else if (!strcmp(argv[i], "-window") && (argc > i + 0)) {
				log_debug("command-line argument -window\n");
				fullscreen = false;
			}
			else if (!strcmp(argv[i], "-nosound") && (argc > i + 0)) {
				log_debug("command-line argument -nosound\n");
				sound.disable();
			}
			else if (!strcmp(argv[i], "-nokeyboard") && (argc > i + 0)) {
				log_debug("command-line argument -nokeyboard\n");
				inputs &= ~1;
			}
			else if (!strcmp(argv[i], "-nomouse") && (argc > i + 0)) {
				log_debug("command-line argument -nomouse\n");
				inputs &= ~2;
			}
			else if (!strcmp(argv[i], "-nojoystick") && (argc > i + 0)) {
				log_debug("command-line argument -nojoystick\n");
				inputs &= ~4;
			}
			else if (!strcmp(argv[i], "-noidle") && (argc > i + 0)) {
				log_debug("command-line argument -noidle\n");
				_no_idle = 1;
			}
			else if (!strcmp(argv[i], "-play") && (argc > i + 2)) {
				log_debug("command-line argument -play\n");
				auto_play = argv[i+1];
				auto_param = argv[i+2];
				i += 2;
				if ((argc > i + 0) && (argv[i][0] != '-')) {
					auto_port = atoi(argv[i]);
					i += 1;
				}
			} else {
				log_debug("unrecognized command-line argument\n");
			}
		}

		log_debug("command-line arguments parsed:\n"
			"\tscreen_width = %d\n"
			"\tscreen_height = %d\n"
			"\tscreen_bpp = %d\n"
			"\tfullscreen = %d\n", screen_width, screen_height, screen_bpp, fullscreen);

		srand(time(NULL));
		set_color_conversion(COLORCONV_KEEP_TRANS);

		if (!videosystem.set_resolution(screen_width, screen_height, screen_bpp, fullscreen)) {
			// try safest defaults
			screen_width = 640;
			screen_height = 480;
			screen_bpp = tw_desktop_color_depth();
			fullscreen = 0;
			if (!videosystem.set_resolution(screen_width, screen_height, screen_bpp, fullscreen)) {
				tw_error_exit("Unable to init screen!!!");
			}
		}
		if (!theme)
			theme = agup_theme_by_name("Photon");
		agup_init(theme);
		gui_shadow_box_proc = d_agup_shadow_box_proc;
		gui_ctext_proc = d_agup_ctext_proc;
		gui_button_proc = d_agup_button_proc;
		gui_edit_proc = d_agup_edit_proc;
		gui_list_proc = d_agup_list_proc;
		gui_text_list_proc = d_agup_text_list_proc;

		enable_input(inputs);
		sound.init();
		sound.load();

		showLoadingScreen();

		View *v = NULL;
		v = get_view ( get_config_string("View", "View", NULL) , NULL );
		if (!v) v = get_view ( "Hero", NULL );
		set_view(v);

		init_fleet();
		meleedata.init();		 //mainmain

		if (auto_play) {		 // FIX ME
			if (!strcmp(auto_play, "game")) play_game(auto_param, NULL);
		} else {
			mainmenu.preinit();
			mainmenu.init(&videosystem.window);
			mainmenu.doit();
			mainmenu.deinit();
		}

		if (game) {
			delete game;
			game = NULL;
		}
		meleedata.deinit();
		sound.disable();
		disable_input();
	}

	catch (int i) {
		if (i == -1) throw;
		if (__error_flag & 1) throw;
		if (i != 0) caught_error("%s %s caught int %d", __FILE__, __LINE__, i);
		if (__error_flag & 1) throw;
	}
	catch (const char *str) {
		if (__error_flag & 1) throw;
		caught_error("message: \"%s\"", str);
		if (__error_flag & 1) throw;
	}
	catch (...) {
		if (__error_flag & 1) throw;
		caught_error("Ack!!!\nAn error occured on startup!\nBut I don't know what error!");
		if (__error_flag & 1) throw;
	}

	tw_exit(0);
	return 0;
}


void showTitle(VideoWindow *window)
{
	STACKTRACE;
	BITMAP *src = titlePic;
	if (!src) {
		return;
		tw_error("Unable to open title pic");
	}

	if (!window->surface)
		return;
	window->lock();
	aa_stretch_blit(src, window->surface,
		0,0,src->w,src->h,
		window->x, window->y, window->w, window->h);
	textout_right(screen, font, tw_version(),
		screen->w, screen->h - text_height(font),
		palette_color[15]);
	window->unlock();
	return;
}


const char *select_game_menu ()
{
	select_game_dialog[2].dp3 = game_names;
	tw_set_config_file(home_ini_full_path("client.ini"));
	select_game_dialog[2].d1 = get_config_int("Menu", "SelectGame", 0);
	int i = tw_popup_dialog(NULL, select_game_dialog, 2);
	if (i == -1) return NULL;
	else {
		set_config_int("Menu", "SelectGame", select_game_dialog[2].d1);
		return game_names[select_game_dialog[2].d1];
	}
}


// MELEE_EX - dialog function
void extended_menu(int i)
{
	STACKTRACE;

	showTitle();
	if (i == -1)
		i = tw_popup_dialog(NULL, melee_ex_dialog, MELEE_EX_DIALOG_PLAY_GAME);
	switch (i) {
		case -1:
		case MELEE_EX_DIALOG_EXIT:
		{
			return;
		}
		break;
		case MELEE_EX_DIALOG_PLAY_GAME:
		{
			const char *gname = select_game_menu();
			if (gname) play_game(gname);
			//			play_game(game_names[melee_ex_dialog[MELEE_EX_DIALOG_GAMELIST].d1], log_types[melee_ex_dialog[MELEE_EX_DIALOG_LOGLIST].d1]);
		}
		break;
		case MELEE_EX_DIALOG_DIAGNOSTICS:
		{
			show_diagnostics();
		}
		break;
		case MELEE_EX_DIALOG_SHIPINFO:
		{
			ship_view_dialog(0, reference_fleet);
		}
		break;
		case MELEE_EX_DIALOG_KEYTESTER:
		{
			keyjamming_tester();
		}
		break;
	}
	showTitle();
	return;
}


// TEAMS - dialog function
void change_teams()
{
	STACKTRACE;
	int a, i;

	tw_set_config_file("scp.ini");

	teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1 = 0;

	while (1) {
		dialog_string[0][0] = 0;
		sprintf(dialog_string[0], "Config #");

		a = tw_do_dialog(NULL, teamsDialog, 0);
		if ((a == TEAMS_DIALOG_SELECTCONTROL) || (a == TEAMS_DIALOG_CONTROLLIST)) {
			player_type[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] =
				control_name[teamsDialog[TEAMS_DIALOG_CONTROLLIST].d1];
			teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1 += 1;
		}
		else if ((a == TEAMS_DIALOG_SETUP) || (a == TEAMS_DIALOG_PLAYERLIST)) {
			Control *tmpc = load_player(teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1);
			if (tmpc) {
				showTitle();
				tmpc->setup();
				delete tmpc;
				showTitle();
			}
		}
		else if (a == TEAMS_DIALOG_TEAM_NUM) {
			player_team[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] += 1;
			player_team[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] %= MAX_TEAMS;
		}
		else if (a == TEAMS_DIALOG_CONFIG_NUM) {
			player_config[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] += 1;
			player_config[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] %= MAX_CONFIGURATIONS;
		}
		else if (a == TEAMS_DIALOG_FLEET) {
			edit_fleet(teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1);
			showTitle();
		}
		else break;
	}

	tw_set_config_file("scp.ini");
	for (i = 0; i < MAX_PLAYERS; i += 1) {
		sprintf(dialog_string[0], "Player%d", i+1);
		set_config_string (dialog_string[0], "Type", player_type[i]);
		set_config_int (dialog_string[0], "Config", player_config[i]);
		set_config_int (dialog_string[0], "Team", player_team[i]);
	}

	return;
}


/*
 *** TEAMS dialog section - end
 */

/*
 *** FLEET dialog section - begin
 */

char *numeric_string[] =
{
	"Zero", "One", "Two", "Three", "Four",
	"Five", "Six", "Seven", "Eight", "Nine", "Ten", "Eleven",
	"Twelve"
};

char fleetPlayer[18];
char fleetTitleString[100];

int scp_fleet_dialog_text_list_proc(int msg, DIALOG* d, int c);
int scp_fleet_dialog_bitmap_proc(int msg, DIALOG* d, int c);

int d_check_proc_fleeteditor(int msg, DIALOG *d, int c)
{
	STACKTRACE;
	if (msg == MSG_CLICK) {

		/* track the mouse until it is released */
		while (gui_mouse_b()) {
			//			state2 = ((gui_mouse_x() >= d->x) && (gui_mouse_y() >= d->y) &&
			//				(gui_mouse_x() < d->x + d->w) && (gui_mouse_y() < d->y + d->h));

			/* let other objects continue to animate */
			broadcast_dialog_message(MSG_IDLE, 0);
		}

		/* should we close the dialog? */
		// imo the following mucho better/ simplere than that messy stuff in the allegro routine
		// ... check d_button_proc in guiproc.c in the allegro sources...

		if (d->flags & D_SELECTED)
			d->flags &= ~D_SELECTED;
		else
			d->flags |= D_SELECTED;

		if ( d->flags & D_EXIT)
			return D_CLOSE;

		return D_O_K;
	}

	return d_agup_check_proc(msg, d, 0);
}


bool safeToDrawPreview = false;

// FLEET - dialog function
void edit_fleet(int player)
{
	STACKTRACE;
	char tmp[40];
	char path[80];
	char fleetCostString[80] = "";
	char maxFleetCostString[80] = "";
	bool availableFleetDirty = true;
	Fleet ref_fleet;

	static Fleet::SortingMethod sortMethod1 = (Fleet::SortingMethod) Fleet::SORTING_METHOD_DEFAULT,
		sortMethod2 = (Fleet::SortingMethod) Fleet::SORTING_METHOD_DEFAULT;
	static bool sortAscending1 = false,
		sortAscending2 = false;

	sprintf (tmp, "Player%d", player+1);
	Fleet* fleet = new Fleet();
	fleet->load("fleets.ini", tmp);

	if (player + 1 <= 12)
		sprintf(fleetPlayer, "Player %s Fleet", numeric_string[player+1]);
	else sprintf(fleetPlayer, "Player%d Fleet", player+1);
	showTitle();

	int fleetRet;
	int selectedSlot;

	fleetDialog[FLEET_DIALOG_CURRENT_POINTS_VALUE].dp = fleetCostString;
	fleetDialog[FLEET_DIALOG_POINT_LIMIT_BUTTON].dp = maxFleetCostString;

	//	// the reference_fleet is used in the list in a hardcoded way, so over"load" it
	Fleet *old_reference_fleet = reference_fleet;
	reference_fleet = &ref_fleet;

	do {
		sprintf(title_str, fleet->getTitle());
		sprintf(fleetTitleString, "%s\n%d points", fleet->getTitle(), fleet->getCost());

		fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].dp3 = fleet;
		fleetDialog[FLEET_DIALOG_SORTBY_BUTTON1].dp = (void*)Fleet::getSortingMethodName(sortMethod1);
		fleetDialog[FLEET_DIALOG_SORTBY_BUTTON2].dp = (void*)Fleet::getSortingMethodName(sortMethod2);

		sprintf(fleetCostString,"%d", fleet->getCost());
		if (fleet->getCost() > fleet->getMaxCost())
			fleetDialog[FLEET_DIALOG_CURRENT_POINTS_VALUE].bg = makecol8(255,0,0);
		else
			fleetDialog[FLEET_DIALOG_CURRENT_POINTS_VALUE].bg = 0;

		sprintf(maxFleetCostString,"%d %s", fleet->getMaxCost(),
			Fleet::getFleetCostName(fleet->getMaxCost()));

		if (sortAscending1)
			fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"^";
		else
			fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"v";

		if (sortAscending2)
			fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"^";
		else
			fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"v";

		//if the user has selected a different choice of available ships, regenerate the
		//list of available ships
		if (availableFleetDirty) {
			availableFleetDirty = false;

			ref_fleet.reset();
			//clear out the fleet
			for (int c=0; c<num_shiptypes; c++) {
				switch (shiptypes[c].origin) {
					case SHIP_ORIGIN_TW:
						if (fleetDialog[FLEET_DIALOG_TW_OFFICIAL_TOGGLE].flags & D_SELECTED)
							ref_fleet.addShipType(&shiptypes[c]);
						break;

					case SHIP_ORIGIN_UQM:
						if (fleetDialog[FLEET_DIALOG_TW_EXP_TOGGLE].flags & D_SELECTED)
							ref_fleet.addShipType(&shiptypes[c]);
						break;

					case SHIP_ORIGIN_TW_SPECIAL:
						if (fleetDialog[FLEET_DIALOG_TW_SPECIAL_TOGGLE].flags & D_SELECTED)
							ref_fleet.addShipType(&shiptypes[c]);
						break;

					case SHIP_ORIGIN_TWA:
						if (fleetDialog[FLEET_DIALOG_TWA_TOGGLE].flags & D_SELECTED)
							ref_fleet.addShipType(&shiptypes[c]);
						break;
				}
			}
			ref_fleet.Sort( sortMethod1, sortAscending1 );
			fleetDialog[FLEET_DIALOG_AVAILABLE_SHIPS_LIST].flags |= D_DIRTY;
		}

		fleetRet = tw_do_dialog(NULL, fleetDialog, -1);

		switch( fleetRet ) {
			case FLEET_DIALOG_AVAILABLE_SHIPS_TEXT: break;
			case FLEET_DIALOG_SHIP_CATAGORIES_TEXT: break;

			case FLEET_DIALOG_TW_OFFICIAL_TOGGLE:
			case FLEET_DIALOG_TW_EXP_TOGGLE:
			case FLEET_DIALOG_TW_SPECIAL_TOGGLE:
			case FLEET_DIALOG_TWA_TOGGLE:
				availableFleetDirty = true;
				break;

			case FLEET_DIALOG_SORTBY_TEXT1: break;
			case FLEET_DIALOG_SORTBY_BUTTON1:
				sortMethod1 = Fleet::cycleSortingMethod(sortMethod1);
				ref_fleet.Sort( sortMethod1, sortAscending1 );
				fleetDialog[FLEET_DIALOG_SORTBY_BUTTON1].dp = (void*)Fleet::getSortingMethodName(sortMethod1);
				break;

			case FLEET_DIALOG_SORTBY_ASCENDING1:
				sortAscending1 = 1 - sortAscending1;
				ref_fleet.Sort( sortMethod1, sortAscending1 );
				if (sortAscending1)
					fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"^";
				else
					fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"v";
				break;

			case FLEET_DIALOG_AVAILABLE_SHIPS_LIST:
			case FLEET_DIALOG_ADD_BUTTON:
				int k;
				k = fleetDialog[FLEET_DIALOG_AVAILABLE_SHIPS_LIST].d1;
				if (k < 0 || k >= ref_fleet.getSize()) {tw_error("invalid ship choice - bug");}

				selectedSlot = fleet->addShipType(ref_fleet.getShipType(k));
				if (selectedSlot != -1)
					fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1 = selectedSlot;

				break;

			case FLEET_DIALOG_PLAYER_FLEET_BUTTON: break;

			case FLEET_DIALOG_PLAYER_FLEET_TITLE:
				if (do_dialog(fleet_titleDialog, FLEET_TITLE_DIALOG_BOX) == FLEET_TITLE_DIALOG_OK)
					sprintf(fleet->getTitle(), title_str);
				showTitle();
				break;

			case FLEET_DIALOG_SAVE_BUTTON:
				sprintf(path, "fleets/");
				if (file_select("Save Fleet", path, "scf")) fleet->save(path, "Fleet");
				showTitle();
				break;

			case FLEET_DIALOG_LOAD_BUTTON:
				sprintf(path, "fleets/");
				if (file_select("Load Fleet", path, "scf")) fleet->load(path, "Fleet");
				sprintf(title_str, fleet->getTitle());
				sprintf(fleetTitleString, "%s\n%d points", fleet->getTitle(), fleet->getCost());
				showTitle();
				break;

			case FLEET_DIALOG_POINT_LIMIT_TEXT: break;

			case FLEET_DIALOG_POINT_LIMIT_BUTTON:
				fleet->cycleMaxFleetCost();
				break;

			case FLEET_DIALOG_CURRENT_POINTS_TEXT: break;
			case FLEET_DIALOG_CURRENT_POINTS_VALUE: break;
			case FLEET_DIALOG_SORTBY_TEXT2: break;

			case FLEET_DIALOG_SORTBY_BUTTON2:
				sortMethod2 = Fleet::cycleSortingMethod(sortMethod2);
				fleet->Sort( sortMethod2, sortAscending2 );
				fleetDialog[FLEET_DIALOG_SORTBY_BUTTON2].dp = (void*)Fleet::getSortingMethodName(sortMethod2);
				break;

			case FLEET_DIALOG_SORTBY_ASCENDING2:
				sortAscending2 = 1 - sortAscending2;
				fleet->Sort( sortMethod2, sortAscending2 );
				if (sortAscending2)
					fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"^";
				else
					fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"v";
				break;

			case FLEET_DIALOG_ADD_ALL_BUTTON:
				fleet->addFleet(&ref_fleet);
				break;

			case FLEET_DIALOG_CLEAR:
			case FLEET_DIALOG_FLEET_SHIPS_LIST:
				fleet->clear_slot(fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1);
				if (fleet->getSize() <= 0)
					fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1 = 0;
				break;

			case FLEET_DIALOG_CLEARALL:
				fleet->reset();
				fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1 = 0;
				break;

			case FLEET_DIALOG_SHIP_PICTURE_BITMAP: break;

			case FLEET_DIALOG_SHIP_SUMMARY_TEXT: break;
			case FLEET_DIALOG_BACK_BUTTON: break;
			case FLEET_DIALOG_HELP_TEXT:
			default:
				;
		}
		/*if (fleetRet == FLEET_DIALOG_INFO) {
			ship_view_dialog(fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1, reference_fleet);
			showTitle();
		}*/

	} while((fleetRet != FLEET_DIALOG_BACK_BUTTON) && (fleetRet != -1));

	reference_fleet = old_reference_fleet;

	fleet->save("fleets.ini", tmp);
	delete fleet;
	showTitle();
}


int scp_fleet_dialog_text_list_proc(int msg, DIALOG* d, int c)
{
	STACKTRACE;

	static int next_anim_time = get_time();
	int old_d1 = d->d1;

	int ret = 0;

	// allow user to select the ships based on keystrokes:
	// select based on the ship's name
	bool shouldConsumeChar = false;
	if (msg == MSG_CHAR) {
		char typed = (char)(0xff & c);
		if (isalnum (typed)) {
			d->d1 = reference_fleet->getNextFleetEntryByCharacter( d->d1, typed);
			shouldConsumeChar = true;
			if (d->d1 != old_d1) {

				int size = reference_fleet->getSize();
				int height = (d->h-4) / text_height(font);

				ret = D_USED_CHAR;
				d->flags |= D_DIRTY;

				//scroll such that the selection is shown.
				//only change the scroll if the selection is not already shown,
				//and the number of ships in the list is greater than the number
				//of slots that can be shown simultaneously.
				if ( (size > height) &&
					( (d->d1 < d->d2) ||
				(d->d1 >= d->d2 + height))) {
					if (d->d1 <= (height/2))
						d->d2 = 0;
					else {

						if (d->d1 >= (size - height))
							d->d2 = (size - height);
						else {
							d->d2 = d->d1 - height/2;
						}
					}
				}
			}
		}
	}
	ret = d_agup_text_list_proc( msg, d, c );

	if (shouldConsumeChar)
		ret = D_USED_CHAR;

	static BITMAP* panel = create_bitmap(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
		fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);
	fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].dp = panel;

	static BITMAP * sprite = create_bitmap(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
		fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);
	static int rotationFrame = 0;

	//selection has changed
	if (d->d1 != old_d1) {
		safeToDrawPreview = false;
		float fractionRotated = 0;

		{
			ShipType* type = reference_fleet->getShipType(old_d1);

			if (type && type->data) {
				if (type->data->spriteShip) {

					fractionRotated = (float)((float)rotationFrame / (float)(type->data->spriteShip->frames()));
				}
				type->data->unlock();
			}
		}

		rotationFrame = 0;

		{
			ShipType* type = reference_fleet->getShipType(d->d1);
			if (type && type->data) {
				type->data->lock();
				if (type->data->spriteShip)
					rotationFrame = (int)(fractionRotated * type->data->spriteShip->frames());
			}
		}
	}

	if ( ( d->d1 != old_d1 || msg == MSG_START) ||
	(msg == MSG_IDLE && next_anim_time < get_time()) ) {
		safeToDrawPreview = false;

		//next_anim_time = get_time() + 50 + rand() % 200;
		next_anim_time = get_time() + 20;

		ShipType* type = reference_fleet->getShipType(d->d1);

		clear_to_color(sprite, 0);

		if (type && type->data && type->data->spriteShip) {

			type->data->spriteShip->draw(
				Vector2(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w/2,
				fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h/2) - type->data->spriteShip->size()/2,
				type->data->spriteShip->size(),
				rotationFrame, sprite
				);

			rotationFrame++;
			if (rotationFrame >= type->data->spriteShip->frames())
				rotationFrame = 0;
		}
		stretch_blit(sprite, panel, 0, 0, sprite->w, sprite->h, 0, 0, panel->w, panel->h);
		safeToDrawPreview = true;

		//TODO decide if these next 3 lines should be here
		scare_mouse();
		SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP], MSG_DRAW, 0);
		unscare_mouse();
	}

	return ret;
}


int scp_fleet_dialog_bitmap_proc(int msg, DIALOG* d, int c)
{
	STACKTRACE;
	//TODO address this: bitmap has to be deleted, but MSG_END does not mean the dialog isn't coming back
	/*if (msg == MSG_END && d->dp) {
		destroy_bitmap( (BITMAP*)d->dp );
		d->dp = NULL;
	}*/

	if ((msg != MSG_DRAW || d->dp) && (safeToDrawPreview) )
		return d_bitmap_proc(msg, d, c);
	return D_O_K;
}


/*
 *** FLEET dialog section - end
 */

/*! This is the maximum size of text file the program is willing to read.  This is for security concerns (buffer overflows) */
enum { MAX_SHIP_TEXT_FILE_SIZE=6000 };

/*! This is the maximum size of ini file the program is willing to read.  This is for security concerns (buffer overflows) */
enum { MAX_SHIP_INI_FILE_SIZE=6000 };

/*
 *** SHIPVIEW dialog section - begin
 */
void ship_view_dialog(int si, Fleet *fleet)
{
	STACKTRACE;
	int i;
	int r = 0;					 // result of tw_do_dialog
	const char *sname;
								 // ship description contents
	char textFile[MAX_SHIP_TEXT_FILE_SIZE] = "";
	BITMAP *sprite = NULL;

	showLoadingScreen();

	shipviewDialog[SHIPVIEW_DIALOG_LIST].d1 = si;
	shipviewDialog[SHIPVIEW_DIALOG_LIST].dp3 = fleet;
	shipviewDialog[SHIPVIEW_DIALOG_TXTFILE].dp = (char*)"";
	//this is set later
	shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp = (char*)"";

	// main dialog loop - begin
	while ((r >= 0) && (r != SHIPVIEW_DIALOG_DONE)) {

		// update ship selection - begin
		if ((r == 0) || (r == SHIPVIEW_DIALOG_LIST)) {
			si = shipviewDialog[SHIPVIEW_DIALOG_LIST].d1;
			sname = shipListboxGetter(si, NULL);
			if (!sname) {
				sprintf(dialog_string[0], "(Null)");
				sprintf(dialog_string[1], "(Null)");
			} else {
				ShipType *type = fleet->getShipType(si);

				if (sprite)
					destroy_bitmap(sprite);
				sprite = NULL;
				type->data->lock();
				if (type->data->spriteShip) {
					sprite = create_bitmap(180, 180);
					clear_to_color(sprite, 0);
					type->data->spriteShip->draw(
						Vector2(90,90) - type->data->spriteShip->size()/2,
						type->data->spriteShip->size(),
						0, sprite
						);
				}
				type->data->unlock();

				// read ship text file contents
				shipviewDialog[SHIPVIEW_DIALOG_TXTFILE].dp = (char*)textFile;
				{
					PACKFILE * f = pack_fopen(type->text, F_READ);
					if (!f) {
						sprintf(textFile, "Failed to load file \"%s\"", type->text);
					} else {
						unsigned long textFileSize = file_size(type->text);
						if (textFileSize > MAX_SHIP_TEXT_FILE_SIZE)
							textFileSize = MAX_SHIP_TEXT_FILE_SIZE;
						i = pack_fread(textFile, textFileSize, f);
						pack_fclose(f);
						textFile[i] = '\0';
					}
				}

				// read ship ini file contents
								 // ship ini file contents
				char * inifile = "";
				{
					PACKFILE * f = pack_fopen(type->file, F_READ);
					if (!f) {
						inifile = (char*) malloc(strlen("Failed to load file \"\"") + strlen(type->file) + 1);
						sprintf(inifile, "Failed to load file \"%s\"", type->file);
					} else {
						unsigned long iniFileSize = file_size(type->file);
						inifile = (char*) malloc(iniFileSize +1 );
						i = pack_fread(inifile, iniFileSize, f);
						pack_fclose(f);
						inifile[i] = '\0';
					}
				}

				// display ship description contents
				char *c = (char*)malloc( strlen("Name: \n") + strlen(type->name) +
					strlen("ID: \n")+strlen(type->id) +
					strlen("Cost: \n")+3+
					strlen("\n\n\n\nINI file: ()\n")+strlen(type->file) +
					strlen("-------------------------\n")+strlen(inifile) );
				char * description = c;

				c += sprintf(c, "Name: %s\n", type->name);
				c += sprintf(c, "ID: %s\n", type->id);
				c += sprintf(c, "Cost: %d\n", type->cost);
				c += sprintf(c, "\n\n\n\nINI file: (%s)\n", type->file);
				c += sprintf(c, "-------------------------\n%s", inifile);

				if (shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp &&
				strlen((char*)(shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp))>0 ) {
					free(shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp);
					shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp = (char*)"";
				}
				shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp = description;

				if (strlen(inifile) >0)
					free(inifile);
			}
			shipviewDialog[SHIPVIEW_DIALOG_PICTURES+0].dp = sprite;
		}

		// change font size - begin
		if (r == SHIPVIEW_DIALOG_FONT) {
			i = shipviewDialog[SHIPVIEW_DIALOG_TWYIELD+1].d1;
			i = (((i/2) + 2) % 3) - 1;
			shipviewDialog[SHIPVIEW_DIALOG_TWYIELD+1].d1 = i*2;
		}
		// change font size - end

		r = tw_do_dialog(NULL, shipviewDialog, SHIPVIEW_DIALOG_LIST);
	}
	// main dialog loop - end

	if (sprite)
		destroy_bitmap(sprite);
	if (shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp &&
	strlen((char*)(shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp))>0 ) {
		shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp = NULL;
		free(shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp);
	}

	videosystem.redraw();
	return;
}


/*
 * DIAGNOSTICS dialog section - begin
 */

								 //returns length of string
int get_diagnostics_string ( char *dest )
{
	char * tmp = dest;

	#   if defined _DEBUG
	tmp += sprintf(tmp, "DEBUGGING BUILD!\n");
	#   endif
	tmp += sprintf(tmp, "ALLEGRO (.h) version   = Allegro %s, %s\n",
		ALLEGRO_VERSION_STR, ALLEGRO_PLATFORM_STR);
	tmp += sprintf(tmp, "ALLEGRO (.dll) version = %s\n", allegro_id);
	tmp += sprintf(tmp, "Compiler = ");
	#   if defined __MINGW32__
	tmp += sprintf(tmp, "MINGW (gcc)\n");
	#   elif defined __BORLANDC__
	tmp += sprintf(tmp, "Borland\n");
	#   elif defined _MSC_VER
	tmp += sprintf(tmp, "Microsoft Visual C++\n");
	#   elif defined DJGPP
	tmp += sprintf(tmp, "DJGPP (gcc)\n");
	#   elif defined __GNUC__
	tmp += sprintf(tmp, "gcc\n");
	#   else
	tmp += sprintf(tmp, "???\n");
	#   endif
	tmp += sprintf(tmp, "Blah = %s\n", tw_version());
	#   if defined DO_STACKTRACE
	tmp += sprintf(tmp, "DO_STACKTRACE Enabled\n");
	#   else
	tmp += sprintf(tmp, "DO_STACKTRACE Disabled\n");
	#   endif
	return tmp - dest;
}


// DIAGNOSTICS - dialog function
void show_diagnostics()
{
	STACKTRACE;
	int i;
	char buffy [16000];			 //fix sometime
	char buffy2[100000];		 //fix sometime
	char buffy3[16000];			 //yeah right
	char *tmp;
	PACKFILE *f;

	f = pack_fopen (data_full_path("version.txt").c_str(), F_READ);
	if (!f)
		strcpy(buffy, "Failed to load version.txt");
	else {
		i = pack_fread (buffy, 99999, f);
		pack_fclose(f);
		buffy[i] = 0;
	}

	diagnostics_dialog[DIAGNOSTICS_DIALOG_VERSION_TXT].dp = (void *) buffy;
	diagnostics_dialog[DIAGNOSTICS_DIALOG_FILES].dp = (void *) buffy2;
	tmp = buffy2;

	for (i = 0; i < num_registered_files; i += 1) {
		tmp += sprintf(tmp, "%s %s %s\n", registered_files[i].ftime, registered_files[i].fdate, registered_files[i].fname);
	}
	diagnostics_dialog[DIAGNOSTICS_DIALOG_MAIN].dp = (void *) buffy3;
	tmp = buffy3;
	tmp += get_diagnostics_string( tmp );

	/*	diagnostics_dialog[DIAGNOSTICS_DIALOG_SHIPS].dp = (void *) buffy4;
		tmp = buffy4;
		sprintf(tmp, "Ships datafiles missing: ");
		int j = 0;
		for (i = 0; i < num_shiptypes; i += 1) {
			if (
		}*/

	tw_popup_dialog(NULL, diagnostics_dialog, 1);
	return;
}


/*
 * DIAGNOSTICS dialog section - end
 */

void keyjamming_tester()
{
	STACKTRACE;
	int i, j = 0;
	char blah[256];

	scare_mouse();
	videosystem.window.lock();
	clear_to_color(videosystem.window.surface, 0);
	textprintf(screen, font, 40, 20, palette_color[15], "Press the keys combinations you wish to test");
	textprintf(screen, font, 40, 40, palette_color[15], "When you're finished, press ESCAPE or F10");
	videosystem.window.unlock();
	unscare_mouse();

	while (!key[KEY_F10] && !key[KEY_ESC]) {
		if (videosystem.poll_redraw()) {
			scare_mouse();
			videosystem.window.lock();
			clear_to_color(videosystem.window.surface, 0);
			textprintf(screen, font, 40, 20, palette_color[15], "Press the keys combinations you wish to test");
			textprintf(screen, font, 40, 40, palette_color[15], "When you're finished, press ESCAPE or F10");
			videosystem.window.unlock();
			unscare_mouse();
		}
		rectfill(screen, 50, 60, 500, 60 + 20 * j, palette_color[0]);
		j = 0;
		poll_input();
		for (i = 0; (i < 32767) && (j < 16); i += 1) {
			if (key_pressed(i)) {
				key_to_description(i, blah);
				scare_mouse();
				acquire_screen();
				textprintf(screen, font, 50, 60+j*20, palette_color[15], "%s", blah);
				release_screen();
				unscare_mouse();
				j += 1;
			}
		}
		idle(20);
	}
	showTitle();
	while (key[KEY_F10])
		poll_keyboard();
	clear_keybuf();
	return;
}


char *playerListboxGetter(int index, int *list_size)
{
	static char buf[160];
	char *tmp = buf;

	tmp[0] = 0;
	if (index < 0) {
		*list_size = MAX_PLAYERS;
		return NULL;
	}
	else {
		tmp += sprintf(tmp, "Player%d", index + 1);
		if (index + 1 < 10) tmp += sprintf(tmp, " ");
		tmp += sprintf(tmp, "   %d   %d   %s", player_team[index], player_config[index], player_type[index]);
		if ((strlen(buf) >= 80)) tw_error("playerListboxGetter string too long");
		return buf;
	}
}


const char *controlListboxGetter(int index, int *list_size)
{
	static char tmp[40];

	tmp[0] = 0;
	if (index < 0) {
		*list_size = num_controls;
		return NULL;
	}
	else {
		return(control_name[index]);
	}
}
