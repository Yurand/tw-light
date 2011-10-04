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

//#define REVERSE_CONNECT
//for debuging

#include <exception>
using std::exception;
#include <typeinfo>				 //

#include <stdio.h>
#include <string.h>
//#include <stdlib.h>//
#include <string.h>
#include <allegro.h>

#include "melee.h"
REGISTER_FILE
#include "scp.h"
#include "util/history.h"
#include "util/sounds.h"
#include "util/get_time.h"
#include "frame.h"

#include "mframe.h"
#include "mgame.h"
#include "mcontrol.h"
#include "mship.h"
#include "mshppan.h"
#include "mview.h"
#include "mlog.h"
#include "mnet1.h"
#include "mfleet.h"

#include <typeinfo>

#include <stdarg.h>

#include "other/twconfig.h"

static char chat_buf[256];
static int chat_len = 0;
static int chat_on = 0;

int random_seed[2];

MeleeData meleedata;

MeleeData::MeleeData()
{
	STACKTRACE;
	panelSprite             = NULL;
	kaboomSprite            = NULL;
	hotspotSprite           = NULL;
	sparkSprite             = NULL;
	asteroidExplosionSprite = NULL;
	asteroidSprite          = NULL;
	planetSprite            = NULL;
	xpl1Sprite              = NULL;

	melee_music             = NULL;

	melee = NULL;
}


void MeleeData::init()
{
	STACKTRACE;
	melee = tw_load_datafile(data_full_path("melee.dat").c_str());
	if (!melee) tw_error("Error loading melee data\n");

	meleedata.panelSprite             = new SpaceSprite(&melee[MELEE_PANEL], PANEL_FRAMES, SpaceSprite::IRREGULAR);
	meleedata.kaboomSprite            = new SpaceSprite(&melee[MELEE_KABOOM], KABOOM_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED | SpaceSprite::MIPMAPED);
	meleedata.hotspotSprite           = new SpaceSprite(&melee[MELEE_HOTSPOT], HOTSPOT_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED | SpaceSprite::MIPMAPED);
	meleedata.sparkSprite             = new SpaceSprite(&melee[MELEE_SPARK], SPARK_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED | SpaceSprite::MIPMAPED | SpaceSprite::MATCH_SCREEN_FORMAT);
	meleedata.asteroidExplosionSprite = new SpaceSprite(&melee[MELEE_ASTEROIDEXPLOSION], ASTEROIDEXPLOSION_FRAMES);
	meleedata.asteroidSprite          = new SpaceSprite(&melee[MELEE_ASTEROID], ASTEROID_FRAMES);
	meleedata.planetSprite            = new SpaceSprite(&melee[MELEE_PLANET], PLANET_FRAMES);
	meleedata.xpl1Sprite              = new SpaceSprite(&melee[MELEE_XPL1], XPL1_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED | SpaceSprite::MIPMAPED);

	melee_music = (Music*) (melee[MELEE_MUSIC].dat);
	if (!melee_music) {
		tw_error("Unable to load melee music!");
	}
}


void MeleeData::deinit()
{
	STACKTRACE;
	panelSprite             = NULL;
	kaboomSprite            = NULL;
	hotspotSprite           = NULL;
	sparkSprite             = NULL;
	asteroidExplosionSprite = NULL;
	asteroidSprite          = NULL;
	planetSprite            = NULL;
	xpl1Sprite              = NULL;

	tw_unload_datafile(melee);

	melee = NULL;
	melee_music = NULL;
}


int interpolate_frames = false;

#define HIST_POWER 4.0

#define CHECKSUM_CHANNEL Game::_channel_buffered
//#define CHECKSUM_CHANNEL 0

#ifdef _MSC_VER
								 //int -> bool, performance warning
#   pragma warning( disable : 4800 )
#endif

int num_games = 0;
char **game_names = NULL;
GameType **games = NULL;

GameType::GameType(const char *name, Game *(*new_game)(), double order)
{
	this->name = strdup(name);
	this->_new_game = new_game;
	this->order = order;
	::num_games += 1;
	::game_names = (char **) realloc(::game_names, sizeof(char*) * (num_games+1));
	::games = (GameType**) realloc(::games, sizeof(GameType*) * (num_games+1));
	games[num_games-1] = this;
	games[num_games] = NULL;
	game_names[num_games-1] = this->name;
	game_names[num_games] = NULL;
}


Game *GameType::new_game()
{
	STACKTRACE;
	Game *tmp = _new_game();
	tmp->preinit();
	tmp->type = this;
	return tmp;
}


GameType *gametype (const char *name)
{
	GameType ** g;
	for (g = games; *g; g ++)
		if (!strcmp((*g)->name, name))
			return *g;
	return NULL;
}


void __checksync( const char *fname, int line)
{
	STACKTRACE;
	if (!game) {
		tw_error("request to compare checksums without a valid game\nfrom file %f, line %d", fname, line);
		return;
	}
	#   ifdef LOTS_OF_CHECKSUMS
	game->compare_checksums();
	#   endif
}


void Game::_event(Event *e)
{
	STACKTRACE;
	switch (e->type) {
		case Event::VIDEO:
		{
			if (game_done)
				return;
			if (e->subtype == VideoEvent::REDRAW)
				this->redraw();
		} break;
		case Event::TW_CONFIG:
		{
			ConfigEvent *c = (ConfigEvent*)e;
			switch (c->subtype) {
				case ConfigEvent::SET:
				{
					if (0) ;
					else if (!strcmp(c->name, "server.ini/game/shotrelativity")) {
						double v = atof(c->value);
						this->shot_relativity = v;
					}
					else if (!strcmp(c->name, "server.ini/game/friendlyfire")) {
						int v = atoi(c->value);
						this->friendly_fire = (bool) v;
					}
				} break;
			}
			std::list<BaseClass*>b_presence;
			std::copy(presence.begin(),presence.end(),back_inserter(b_presence));
			issue_event(b_presence, e);
			std::list<BaseClass*> b_item;
			std::copy(item.begin(),item.end(),std::back_inserter(b_item));
			issue_event(b_item, e);
		} break;
	}
}


void Game::add_focus(Presence *new_focus, int channel)
{
	STACKTRACE;
	if ((channel != -1) && !log->playback && !(log->get_direction(channel) & Log::direction_write))
		return;
	num_focuses += 1;
	focus = (Presence **) realloc(focus, sizeof(Presence *) * num_focuses);
	focus[num_focuses - 1] = new_focus;
	new_focus->attributes |= ATTRIB_FOCUS;
	if (num_focuses == 1) new_focus->attributes |= ATTRIB_ACTIVE_FOCUS;
	if (focus_index == -1) focus_index = 0;
}


void Game::prepare()
{
	STACKTRACE;
	#ifdef _MSC_VER
	_asm { finit }
	#elif defined(__GCC__) && defined(__i386__)
	asm("finit");
	#endif
	Physics::prepare();
	::game = this;
	::targets = &gametargets;
	return;
}


void Game::set_resolution(int screen_x, int screen_y)
{
	STACKTRACE;
	int view_x, view_y;
	view_x = screen_x;
	view_y = screen_y;
	redraw();
	return;
}


void Game::redraw()
{
	STACKTRACE;
	if (!window->surface) return;
	scare_mouse();
	window->lock();
	rectfill(window->surface, window->x, window->y, window->x+window->w-1, window->y+window->h-1, pallete_color[8]);
	FULL_REDRAW += 1;
	view->refresh();
	view->animate(this);
	FULL_REDRAW -= 1;
	window->unlock();
	unscare_mouse();
	return;
}


Ship *Game::create_ship(const char *id, Control *c, Vector2 pos, double angle, int team)
{
	STACKTRACE;
	ShipType *type = shiptype(id);
	if (!type)
		{tw_error("Game::create_ship - bad ship id (%s)", id);}
		log_file(type->file);
	if (team == 0) team = new_team();
	Ship *s = type->get_ship(pos, angle, get_code(new_ship(), team));
	if (c)
		c->select_ship(s, id);
	gametargets.add(s);
	s->attributes |= ATTRIB_NOTIFY_ON_DEATH;
	return s;
}


Ship *Game::create_ship(int channel, const char *id, const char *control, Vector2 pos, double angle, int team)
{
	STACKTRACE;
	Control *c = create_control(channel, control);
	if (!c)
		{tw_error("bad Control type!");}
		c->temporary = true;
	Ship *s = create_ship(id, c, pos, angle, team);
	return s;
}


void Game::increase_latency()
{
	STACKTRACE;
	if (CHECKSUM_CHANNEL) {
		log->buffer(channel_server + Game::_channel_buffered, NULL, 2);
		log->buffer(channel_client + Game::_channel_buffered, NULL, 2);
		if (log->playback)
			log->buffer(channel_playback + Game::_channel_buffered, NULL, 1);
		log->flush();
	}
	lag_frames += 1;
}


void Game::decrease_latency()
{
	STACKTRACE;
	if (lag_frames <= 1) {tw_error("latency decreased too far");}
	if (CHECKSUM_CHANNEL) {
		log->unbuffer(channel_server + Game::_channel_buffered, NULL, 2);
		log->unbuffer(channel_client + Game::_channel_buffered, NULL, 2);
		if (log->playback)
			log->unbuffer(channel_playback + Game::_channel_buffered, NULL, 1);
	}
	lag_frames -= 1;
}


int Game::is_local (int channel)
{
	return (log->get_direction (channel) & Log::direction_write);
}


void Game::log_file (const std::string& fname)
{
	log->log_file(fname);
}


void Game::log_fleet(int channel, Fleet *fleet)
{
	STACKTRACE;
	int fl;
	void *tmpdata = fleet->serialize(&fl);
	char buffer[16384];

	if (fl > 16000) {tw_error("blah");}
	memcpy(buffer, tmpdata, fl);
	free(tmpdata);
	log_int(channel, fl);
	if (fl > 16000) {tw_error("blah");}
	log_data(channel, buffer, fl);
	fleet->deserialize(buffer, fl);
}


Control *Game::create_control (int channel, const char *type, char *config, char *file)
{
	if ((channel != channel_none) && !is_local(channel)) {
		type = "VegetableBot";
		config = "Config0";
		file = "scp.ini";
	}

	Control *c = getController(type, "whatever", channel);
	if (!c) {
		tw_error("Game::create_control - bad control type (%s)", type);
		return c;
	}
	c->load(file, config);
	add(c);
	return c;
}


void Game::log_char(int channel, char &data)
{
	STACKTRACE;
	if (!log) return;
	log->log  (channel, &data, 1);
	return;
}


void Game::log_short(int channel, short &data)
{
	STACKTRACE;
	if (!log) return;
	data = intel_ordering_short(data);
	log->log  (channel, &data, sizeof(short));
	data = intel_ordering_short(data);
	return;
}


void Game::log_int(int channel, int &data)
{
	STACKTRACE;
	if (!log) return;
	data = intel_ordering(data);
	log->log  (channel, &data, sizeof(int));
	data = intel_ordering(data);
	return;
}


void Game::log_data(int channel, void *data, int size)
{
	STACKTRACE;
	if (!log) return;
	log->log  (channel, data, size);
	return;
}


void Game::idle(int time)
{
	STACKTRACE;
	if (log->listen()) return;
	::idle(time);
	return;
}


void Game::animate(Frame *frame)
{
	STACKTRACE;
	Physics::animate(frame);
}


void Game::animate()
{
	STACKTRACE;

	double t = get_time();
	paused_time = 0;
	view->animate(this);
	t = get_time() - t - paused_time;
	render_history->add_element(pow(t, HIST_POWER));
	return;
}


bool Game::game_ready()
{
	STACKTRACE;
	if (CHECKSUM_CHANNEL == 0) return 1;
	if (log->playback) {
		return (log->ready(channel_server + Game::_channel_buffered) != 0);
	}
	else switch (log->type) {
		case Log::log_normal:
		{
			return true;
		}
		break;
		case Log::log_net1server:
		case Log::log_net1client:
		{
			if (!log->ready(channel_client + Game::_channel_buffered)) return false;
			if (!log->ready(channel_server + Game::_channel_buffered)) return false;
			return true;
		}
	}
	return true;
}


void Game::handle_desynch(int local_checksum, int server_checksum, int client_checksum)
{
	STACKTRACE;
	tw_error("Game Desynchronized\nTime=%d Frame=%d\nClient=%d Server=%d Local=%d", game_time, frame_number, (int)client_checksum, (int)server_checksum, (int)local_checksum);
}


//static int old_num_items;
//static int old_rng;
//static int old_frame;
//static char old_checksum_buf[200][200];

//if a game is killed due to an error, this may be executed
void handle_game_error ( Game *game )
{
	log_debug("handle_game_error() executed\n");
	if (game->log) {
		game->log->save("error.dmo");
		log_debug("Demo recording saved to error.dmo\n");
	}
	/*
		FILE *f;

		f = fopen("error.log", "a");
		if (!f) return;

		if (exitmessage)
			fprintf(f, "\n\n-------- error report, with message [%s]\n", exitmessage);
		else
			fprintf(f, "\n\n-------- error report, unknown error\n");
		//fprintf(f, "-------- showing in-game objects --------\n");
		fprintf(f, "timewarp version = %s\n", tw_version());

		if (game)
			fprintf(f, "lag_frames = %i", game->lag_frames);
		else
			fprintf(f, "no game defined");

		time_t t;
		tm *td;
		t = ::time(0);
		td = ::localtime(&t);
		// month: 0=januari
		fprintf(f, "local time = %i-%02i-%02i %02i:%02i\n\n", td->tm_mday, td->tm_mon+1, 1900+td->tm_year,
			td->tm_hour, td->tm_min);

		if (physics)
		{
			fprintf(f, "name, pos(x,y), vel(x,y), state, obj-pointer(this), ship-pointer(ship), target pointer(target)\n\n");

			int i;
			for (i = 0; i < physics->num_items; i += 1)
			{
				SpaceLocation *s;
				s = physics->item[i];

				if (!(s && s->exists() && s->detectable()))
					continue;

				int is = s->state;
				Vector2 p = s->pos;
				Vector2 v = s->vel;

				// set "enable run-type information" for this feature
				// (rebuild all after changing that option)
				fprintf(f, "%30s %11.3e %011.3e %11.3e %11.3e %3i 0x%08X 0x%08X 0x%08X\n",
					typeid(*s).name(), p.x, p.y, v.x, v.y, is, (unsigned int)s, (unsigned int)s->ship, (unsigned int)s->target );
			}
		} else {
			fprintf(f, "No physics defined\n");
		}

		#ifdef DO_STACKTRACE
		//char *s = tw_error_str;
	//	fprintf(f, "%s\n", tw_error_str);
		//free(s);	// cause s was allocated with malloc().

		fprintf(f, "\nPROCLIST: level of call, line number in file, file name (top=most recent call)\n\n");

		const char *fname = 0;
		int *linenum = 0, *level = 0;

		int i;
		i = 0;	// start with the most recent one.
		while (	get_stacklist_info(i, &fname, &linenum, &level) )
		{
			++i;	// go on until it's a full circle
			if (fname && linenum && level)
				fprintf(f, "%2i   %4i  %s\n", *level, *linenum, fname);
		}

		fprintf(f, "\nPROCSTACK: level of call, line number in file, file name (top=most recent call)\n\n");

		// also, read the "other" stack info ...
		i = 0;
		while ( get_stacktrace_info( i, &fname, &linenum, &level) )
		{
			++i;
			if (fname && linenum && level)
				fprintf(f, "%2i   %4i  %s\n", *level, *linenum, fname);
		}

		#endif

		fprintf(f, "----------------- end error log ---------------------\n");

		fclose(f);
		*/
}


void Game::compare_checksums()
{
	STACKTRACE;
	unsigned char local_checksum = checksum() & 255;
	unsigned char client_checksum = local_checksum;
	unsigned char server_checksum = local_checksum;
	bool desync = false;

	log_char(channel_server + CHECKSUM_CHANNEL, server_checksum);
	if (lag_frames)
		log_char(channel_client + CHECKSUM_CHANNEL, client_checksum);

	if (server_checksum != client_checksum)
		desync = true;
	if (log->playback) {
		if (lag_frames)
			log_char (channel_playback + CHECKSUM_CHANNEL, local_checksum);
		if (local_checksum != server_checksum) desync = true;
	}

	this->local_checksum = local_checksum;
	this->client_checksum = client_checksum;
	this->server_checksum = server_checksum;

	if (desync) {
		handle_desynch(local_checksum, server_checksum, client_checksum);
	}
}


void Game::do_game_events()
{
	STACKTRACE;
	int i;

	//transmit from server
	if (log->get_direction(channel_server) & Log::direction_write) {
		COMPILE_TIME_ASSERT(sizeof(events_waiting) == sizeof(char));
		log->buffer( channel_server + _channel_buffered, &events_waiting, sizeof(events_waiting) );
		for (i = 0; i < events_waiting; i += 1) {
			log->buffer ( channel_server + _channel_buffered, waiting_events[i], waiting_events[i]->size );
		}
		//deallocate transmitted events
		for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
		events_waiting = 0;
	}

	//transmit from client
	if (log->get_direction(channel_client) & Log::direction_write) {
		COMPILE_TIME_ASSERT(sizeof(events_waiting) == sizeof(char));
		log->buffer( channel_client + _channel_buffered, &events_waiting, sizeof(events_waiting) );
		for (i = 0; i < events_waiting; i += 1) {
			log->buffer ( channel_client + _channel_buffered, waiting_events[i], waiting_events[i]->size );
		}
		//deallocate transmitted events
		for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
		events_waiting = 0;
	}

	//double-check transmission
	if (events_waiting) {
		tw_error("Game::do_game_events - events weren't sent properly");
		for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
		events_waiting = 0;
	}

	//recieve
	char ne;
	COMPILE_TIME_ASSERT(sizeof(events_waiting) == sizeof(ne));
	char buffy[1024];

	//recieve from server
	log->unbuffer(channel_server + _channel_buffered, &ne, sizeof(ne));
	for (i = 0; i < ne; i += 1) {
		char *tmp = buffy;
		log->unbuffer(channel_server + _channel_buffered, &buffy, sizeof(GameEvent));
		int s = ((GameEvent*)tmp)->size;
		if (s > 1024) {
			tmp = (char *)malloc(s);
			memcpy(tmp, buffy, sizeof(GameEvent));
		}
		log->unbuffer(channel_server + _channel_buffered, tmp + sizeof(GameEvent), s - sizeof(GameEvent));
		handle_game_event ( channel_server, ((GameEvent*)tmp));
		if (tmp != buffy) free(tmp);
	}

	//recieve from client
	log->unbuffer(channel_client + _channel_buffered, &ne, sizeof(ne));
	for (i = 0; i < ne; i += 1) {
		char *tmp = buffy;
		log->unbuffer(channel_client + _channel_buffered, &buffy, sizeof(GameEvent));
		int s = ((GameEvent*)tmp)->size;
		if (s > 1024) {
			tmp = (char *)malloc(s);
			memcpy(tmp, buffy, sizeof(GameEvent));
		}
		log->unbuffer(channel_client + _channel_buffered, tmp + sizeof(GameEvent), s - sizeof(GameEvent));
		handle_game_event ( channel_client, ((GameEvent*)tmp));
		if (tmp != buffy) free(tmp);
	}
}


void Game::handle_game_event ( int source, class GameEvent *event )
{
	if ((event->type <= event_invalid) || (event->type >= event_last)) {
		tw_error("Game::handle_game_event - Bad event type: %d", event->type);
	}
	switch (event->type) {
		case event_change_lag: ((GameEventChangeLag*)event)->execute(source);
		break;
		case event_message: ((GameEventMessage*)event)->execute(source);
		break;
	}
}


void Game::send_game_event ( class GameEvent *event )
{
	if (events_waiting == maximum_events_waiting) {
		tw_error("too many GameEvents");
		return;
	}
	if (!waiting_events) waiting_events = new GameEvent*[maximum_events_waiting];
	waiting_events[events_waiting] = event;
	events_waiting += 1;
}


void Game::calculate()
{
	STACKTRACE;
	int i;
	double t = get_time();
	int active_focus_destroyed = false;

	paused_time = 0;
	compare_checksums();
	do_game_events();

	for (i = 0; i < num_focuses; i += 1) {
		if (!focus[i]->exists()) {
			num_focuses -= 1;
			if (focus_index == i) {
				focus[i]->attributes &= ~ATTRIB_ACTIVE_FOCUS;
				active_focus_destroyed = 1;
				focus_index -= 1;
				if (num_focuses && (focus_index < 0))
					focus_index += 1;
			}
			focus[i] = focus[num_focuses];
			i -= 1;
		}
	}

	if (active_focus_destroyed && (focus_index >= 0))
		focus[focus_index]->attributes |= ATTRIB_ACTIVE_FOCUS;

	Physics::calculate();

	gametargets.calculate();

	view->calculate(this);

	t = get_time() - t - paused_time;
	tic_history->add_element(pow(t, HIST_POWER));
	return;
}


void Game::play()
{
	STACKTRACE;
	set_resolution(window->w, window->h);
	prepare();
	if (is_paused()) unpause();
	try
	{
		while(!game_done) {
			unsigned int time = get_time();
			poll_input();
			videosystem.poll_redraw();
			if (!sound.is_music_playing()) {
				play_music();
			}
			if ((next_tic_time <= time) && (next_render_time > game_time) && game_ready()) {
				_STACKTRACE("Game::play - Game physics")
					calculate();
				if (auto_unload) unload_unused_ship_data();
				log->flush();
				log->listen();
				if (key[KEY_F4])
					turbo = f4_turbo;
				else
					turbo = normal_turbo;
				next_tic_time += (frame_time / turbo);
				if ((hiccup_margin >= 0) && (next_tic_time + hiccup_margin < get_time()))
					next_tic_time = get_time();
				if (next_fps_time <= game_time) {
					next_fps_time += msecs_per_fps;
					fps();
				}
			}
			else if (interpolate_frames || (game_time > next_render_time - msecs_per_render)) {
				_STACKTRACE("Game::play - Game rendering")
					animate();
				next_render_time = game_time + msecs_per_render;
			}
			else idle();
			while (keypressed())
				handle_key(readkey());
		}
	}
	catch (int i) {
		if (i == -1) throw;
		if (__error_flag & 1) throw;
		handle_game_error(this);
		if (i != 0) {
			caught_error ("%s %s caught int %d", __FILE__, __LINE__, i);
		}
		if (__error_flag & 1) throw;
	}
	catch (const char *str) {
		if (__error_flag & 1) throw;
		handle_game_error(this);
		caught_error("message: \"%s\"", str);
		if (__error_flag & 1) throw;
	}
	//ArrayIndexOutOfBounds NullPointerException
	catch (exception &e) {
		if (__error_flag & 1) throw;
		handle_game_error(this);
		caught_error ("A standard exception occured\n%s", e.what());
		if (__error_flag & 1) throw;
	}
	catch (...) {
		if (__error_flag & 1) throw;
		handle_game_error(this);
		caught_error("Ack(1)!!!\nAn error occured in the game!\nBut I don't know what error (check error log)!");
		if (__error_flag & 1) throw;
	}
	return;
}


void Game::ship_died(Ship *who, SpaceLocation *source)
{
	STACKTRACE;

	if (source && source->data) {
		Music *tmp = NULL;
		//if (source && source->ship && source->ship->data) tmp = source->ship->data->moduleVictory;
		// note: it's not guaranteed that a ship exists longer than its weapon, while data keep existing, right ?
		if (source && source->exists()) tmp = source->data->moduleVictory;
		if (tmp) sound.play_music(tmp);
	}
	return;
}


void Game::object_died(SpaceObject *who, SpaceLocation *source)
{
	STACKTRACE;
	if (who && who->isShip()) {
		ship_died((Ship*)who, source);
	}
}


void Game::fps()
{
	STACKTRACE;
	if ((!log->playback) && ((log->type == Log::log_net1server) || (log->type == Log::log_net1client))) {
		int ping = ((NetLog*)log)->ping;
		char *tt = "good";
		if (ping > 100) tt = "okay";
		if (ping > 200) tt = "bad";
		if (ping > 400) tt = "BAD!";
		if (ping > 800) tt = "VERY BAD!";
		message.print((int)msecs_per_fps, 12, "ping: %dms (that's %s)", ping, tt);
	}

	if (this->show_fps) {
		/*			double a = 1.0;
					double b = 1.0;
		//			double *c = (double *)(((int)&a - (int)&b) & 0x80000000);
		//			message.print(1000, 15, "inf = %f", a + *c);
						{
						SpaceLocation *frog = getFirstItem(LAYER_CBODIES);
		//				const type_info *d = &typeid(*frog);
						message.print(msecs_per_fps, 15, "%s %d %d %d",
								d->name(), sizeof(*frog), sizeof(Asteroid),
								0);
						}*/
		double tt = pow(tic_history->get_average(0, 1000/frame_time), 1/HIST_POWER);
		double rt = pow(render_history->get_average(0, 1000/frame_time), 1/HIST_POWER);
		char *tmp;

		if (tt*8 < frame_time)
			tmp = "good";
		else if (tt*2 < frame_time)
			tmp = "ok";
		else if (tt < frame_time)
			tmp = "bad";
		else
			tmp = "BAD!";
		message.print((int)msecs_per_fps, 12, "tic time: %.3fms (that's %s)", tt, tmp);

		if (rt < 2)
			tmp = "good";
		else if (rt < 20)
			tmp = "ok";
		else if (rt < 50)
			tmp = "bad";
		else
			tmp = "BAD!";
		message.print((int)msecs_per_fps, 12, "render time: %.3fms (that's %s)", rt, tmp);
		message.print((int)msecs_per_fps, 12, "debug: %d", debug_value);
	}

	if (chat_on)
		message.print((int)msecs_per_fps, 15, "say: %s", chat_buf);
}


void Game::preinit()
{
	STACKTRACE;
	Physics::preinit();
	//	meleedata.planetSprite = meleedata.asteroidSprite = meleedata.asteroidExplosionSprite = meleedata.hotspotSprite = meleedata.kaboomSprite = meleedata.panelSprite = meleedata.sparkSprite = meleedata.xpl1Sprite = NULL;
	// you should reset it here (again), cause there can be subgames of this type.
	log = NULL;
	tic_history = render_history = NULL;

	events_waiting = 0;
	waiting_events = NULL;
	num_focuses = 0;
	focus_index = 0;
	focus = NULL;
	//	num_targets = 0;
	//	target = NULL;
	gametargets.reset();
	view = NULL;
	window = NULL;
	music = NULL;
	time_paused = -1;
}


void Game::init(Log *_log)
{
	STACKTRACE;
	int i;

	game_done = false;
	log = _log;
	if (!log) {
		log = new Log();
		log->init();
	}

	lag_frames = 0;
	show_fps = 0;
	game_time = 0;
	frame_time = 1;
	frame_number = 0;
	hiccup_margin = 100;
	next_tic_time = get_time();
	next_render_time = game_time;
	next_fps_time = game_time;
	view_locked = false;
	physics_locked = false;
	if (log->type != Log::log_normal || log->playback) physics_locked = true;
	local_checksum = client_checksum = server_checksum = 0;

	Physics::init();
	prepare();

	if (!window) {
		window = new VideoWindow();
		window->preinit();
	}

	std::string client_ini = home_ini_full_path("client.ini");
	std::string server_ini = home_ini_full_path("server.ini");

	tw_set_config_file(client_ini.c_str());
	change_view(get_config_string("View", "View", "Hero"));

	window->add_callback(this);

	if (!log->playback) {
		switch (log->type) {
			case Log::log_normal:
			{
			}
			break;
			case Log::log_net1server:
			{
			}
			break;
			case Log::log_net1client:
			{
			}
			break;
			default:
			{
				tw_error("Knee!");
			}
			break;
		}
	}

	/* CONTENTS OF CHANNEL channel_init :

	offset	size	format		data
	0		4		int			log type number
	4		4		int			size of game type name
	8		?		char[]		game type name
	?		4		int			lag frames

	*/
	int tmp = log->type;
	log_int(channel_init, tmp);
	if (log->playback) log->type = tmp;

	char buffy[128];
	i = strlen(type->name);
	memcpy(buffy, type->name, i);
	if (i > 127) {tw_error("long gamename1");}
	log_int (channel_init, i);
	if (i > 127) {tw_error("long gamename2");}
	log_data(channel_init, buffy, i);
	buffy[i] = 0;
	if (strcmp(buffy, type->name)) {tw_error("wrong game type");}

	i = rand();
	//	i = 9223;
	log_int(channel_server, i);
	random_seed[0] = i;
	rng.seed(i);
	i = rand();
	//	i = 7386;
	log_int(channel_server, i);
	random_seed[1] = i;
	rng.seed_more(i);

	if (!is_paused()) pause();

	text_mode(-1);

	tw_set_config_file(client_ini.c_str());
	msecs_per_fps = get_config_int("View", "FPS_Time", 200);
	msecs_per_render = (int)(1000. / get_config_float("View", "MinimumFrameRate", 10) + 0.5);

	log_file(server_ini);
	camera_hides_cloakers = get_config_int("View", "CameraHidesCloakers", 1);
	time_ratio = (int)(1000. / get_config_float ("Game", "SC2FrameRate", 20));
	distance_ratio = (3840. / get_config_float ("Game", "SC2TotalDistance", 8000));
	frame_time = (int)(1000. / get_config_float ("Game", "TicRate", 40) + 0.5);
	normal_turbo = get_config_float("Game", "Turbo", 1.0);
	f4_turbo = get_config_float("Game", "F4Turbo", 10.0);
	turbo = normal_turbo;
								 //MSVC sucks also
	friendly_fire = get_config_int("Game", "FriendlyFire", 0) == 0 ? 0 : 1;
	shot_relativity = get_config_float("Game", "ShotRelativity", 0);
	size = Vector2 (
		get_config_float("Game", "MapWidth", 0),
		get_config_float("Game", "MapHeight", 0)
		);

	init_lag();
	log_int(channel_server, lag_frames);
	log_int(channel_init, lag_frames);

	tic_history = new Histograph(128);
	render_history = new Histograph(128);

	prepare();

	return;
}


void Game::init_lag()
{
	STACKTRACE;
	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) {
		int lag_time = 0;		 //get_config_int("Network", "Lag", 200);
		char blah = 0;
		log_char(channel_server, blah);
		log->flush();
		log_char(channel_client, blah);
		log->flush();
		log_char(channel_server, blah);
		log->flush();
		log_char(channel_client, blah);
		log->flush();
		log_char(channel_server, blah);
		log->flush();
		lag_time = ((NetLog*)log)->ping;
		log_int(channel_server, lag_time);
		int lag_frames = (int) (1.5 + lag_time * normal_turbo / (double) frame_time );
		#       ifdef _DEBUG
		//			lag_frames += 5;
		#       endif
		//			lag_frames += 5;
		message.print(15000, 15, "target ping set to: %d ms (pessimistically: %d ms)", lag_time, iround(lag_frames * frame_time / normal_turbo));
		for (int i = 0; i < lag_frames; i += 1)
			increase_latency();
	} else {
		int lag_frames = 0;		 //10;//0;
		for (int i = 0; i < lag_frames; i += 1)
			increase_latency();	 //*/
	}
}


								 //this function looks wrong to me
void Game::change_view(View *new_view)
{
	STACKTRACE;
	View *v = new_view;
	v->preinit();
	v->init(view);
	if (view)
		v->replace(view);
	else {
		v->window->init(window);
		v->window->locate(
			0, 0,
			0, 0,
			0, 1,
			0, 1
			);
	}
	view = v;
	return;
}


void Game::change_view(const char * name)
{
	STACKTRACE;
	View *v = get_view(name, view);
	if (!v) {tw_error("Game::change_view - invalid view name");}
	if (view)
		v->replace(view);
	else {
		v->window->init(window);
		v->window->locate(
			0, 0,
			0, 0,
			0, 1,
			0, 1
			);
	}
	view = v;
	return;
}


Game::~Game()
{
	message.out("deleteing GameEvents");
	int i;
	for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
	delete[] waiting_events;

	message.out("deleteing histographs");
	delete tic_history; tic_history = NULL;
	delete render_history; render_history = NULL;

	if (music && (music != (Music*)-1)) {
		sound.stop_music();
		//sound.unload_music(music);
	}

	message.out("deleteing game objects");
	destroy_all();

	message.out("other shit");
	message.flush();

	delete view;
	window->remove_callback(this);
	delete window;
}


bool Game::is_paused()
{
	STACKTRACE;
	if (time_paused != -1) return true;
	return false;
}


void Game::pause()
{
	STACKTRACE;
	if (time_paused != -1) tw_error ("can't pause -- already paused");
	time_paused = get_time();
}


void Game::unpause()
{
	STACKTRACE;
	if (time_paused == -1) tw_error ("can't unpause -- not paused");
	redraw();
	paused_time += get_time() - time_paused;
	time_paused = -1;
	return;
}


void Game::save_screenshot()
{
	STACKTRACE;
	static int shot_index = 0;
	char path[80];

	while (true) {
		sprintf(path, "./scrshots/shot%04d.pcx", shot_index);
		if (!exists(path)) break;
		shot_index += 1;
		if (shot_index > 10000) {
			sprintf(path, "./scrshots/");
			break;
		}
	}

	if (file_select("Save screen shot", path, "BMP;PCX;TGA")) {
		BITMAP *bmp;
		PALETTE pal;
		get_palette(pal);
		bmp = create_sub_bitmap(screen, 0, 0, SCREEN_W, SCREEN_H);
		save_bitmap(path, bmp, pal);
		destroy_bitmap(bmp);
	}

	return;
}


bool Game::handle_key(int k)
{
	STACKTRACE;
	switch (k >> 8) {
		#if !defined _DEBUG
		case KEY_F11:
		{
			pause();
			save_screenshot();
			unpause();
			return true;
		}
		break;
		case KEY_F10:
		#endif

		case KEY_ESC:
		{
			//(*((int*)NULL)) = 0;
			pause();
			if (tw_alert("Game is paused", "&Abort game", "&Resume playing") == 1) {
				game->quit("quit - Game aborted from keyboard");
			}
			unpause();
			return true;
		}
		break;
		case KEY_F1:			 // help
		{
			pause();
			show_file(data_full_path("ingame.txt").c_str());
			unpause();
			return true;
		}
		break;
		case KEY_F3:			 // switch hero
		{
			if (num_focuses) focus_index = (focus_index + 1) % num_focuses;
			message.print(1000, 15, "Camera focus %d (of %d)", focus_index+1, num_focuses);
			return true;
		}
		break;
		case KEY_F6:			 // send message
		{
			chat_len = 0;
			chat_buf[0] = '\0';
			chat_on = 1;
			return true;
		}
		break;
		case KEY_F7:
		{
			if (physics_locked) return false;
			if (frame_time < 7) {
				frame_time = 50;
			}
			else if (frame_time < 15) {
				frame_time = 5;
			}
			else if (frame_time < 30) {
				frame_time = 10;
			} else {
				frame_time = 25;
			}
			message.print(1000, 15, "Game Tic rate set to %f / second", 1000./frame_time);
			return true;
		}
		break;
		case KEY_F8:
		{
			if (view_locked) return false;
			if (!view || !view->type) return false;
			if (!strcmp(view->type->name, "Hero")) {
				game->change_view("Enemy");
				message.print(2500, 15, "View mode changed to 'Enemy'");
				return true;
			}
			if (!strcmp(view->type->name, "Enemy")) {
				game->change_view("Hero");
				message.print(2500, 15, "View mode changed to 'Hero'");
				return true;
			}
			//redraw();
			return true;
		}
		break;
		default:
		{
			if (chat_on) {
				if ((k >> 8) == KEY_ENTER) {
					send_game_event(new GameEventMessage(chat_buf));
					//					player_said(0, chat_buf);
					chat_on = 0;
				}
				else if (k & 255) {
					if ((k & 255) == 8) {
						if (chat_len > 0) {
							chat_len -= 1;
							chat_buf[chat_len] = 0;
						}
					} else {
						chat_buf[chat_len] = k & 255;
						if (chat_len < 255) chat_len += 1;
						chat_buf[chat_len] = 0;
					}
				}
				return true;
			}
		}
		break;
		case KEY_F2:
		{
			pause();
			options_menu(this);
			unpause();
			return true;
		}
		break;
		#       ifdef _DEBUG
		case KEY_B:				 //supposed to be F12, but the debugger isn't fond of that
		#       endif
		case KEY_F12:
		{
			show_fps = !show_fps;
			return true;
		}
		break;
	}
	return false;
}


int Game::set_frame_time(int t)
{
	STACKTRACE;
	this->frame_time = t;
	prepare();
	return 1;
}


int Game::set_turbo(double t)
{
	STACKTRACE;
	this->normal_turbo = t;
	prepare();
	return 1;
}


double Game::get_turbo()
{
	STACKTRACE;
	return this->normal_turbo;
}


void Game::play_music()
{
	STACKTRACE;
	if (-1 == (long)music)
		return;
	if (!music) {
		music = meleedata.melee_music;
		//		music = sound.load_music(data_full_path("melee.dat#MELEEMUS_MOD").c_str());
		if (!music)
			music = (Music*) -1;
	}
	if (music) {
		sound.play_music(music, TRUE);
	} else {
		tw_error("Unable to load melee music!");
	}
	return;
}
