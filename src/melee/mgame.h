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

#ifndef __MGAME_H__
#define __MGAME_H__

#include "mframe.h"
#include "mtarget.h"

extern int random_seed[2];
extern int interpolate_frames;

class GameType
{
	private:
		Game *(*_new_game)();
	public:
		double order;
		char *name;
		Game *new_game();
		GameType ( const char *name, Game *(*new_game)(), double order = 0);
};
extern int num_games;
extern char **game_names;
extern GameType **games;
#define REGISTER_GAME(b, a) static Game *new_ ## b () {return new b();} static GameType ignoreme_ ## b( a, new_ ## b, 0);
GameType *gametype(const char *name);

// these are default data used in ships and game objects, which in principle, should be
// independent of the (normal) game type.
class MeleeData
{
	public:
		MeleeData();

		void init();
		void deinit();

		SpaceSprite *planetSprite;
		SpaceSprite *asteroidSprite;
		SpaceSprite *asteroidExplosionSprite;
		SpaceSprite *sparkSprite;
		SpaceSprite *hotspotSprite;
		SpaceSprite *kaboomSprite;
		SpaceSprite *panelSprite;
		SpaceSprite *xpl1Sprite;
		Music *melee_music;
};

class Game : public Physics
{
	public:
		GameType *type;

		enum {
			channel_none = -1,	 //not a valid channel
			channel_init = 4,	 // game type, version, length, etc.. things that need to read by a reader independant of a particular game type
			channel_playback = 8,// used for demo playbacks only
			channel_server  = 12,//data originating on the server
			channel_client  = 16,//data originating on the client
			_channel_buffered = 1
		};

		Log *log;				 //logging system for networking, demo recording/playback, etc.
								 //helper for using the logging system
		void log_char (int channel, char &data);
		void log_char (int channel, unsigned char &data) {log_char (channel,*(char*)&data);}
								 //helper for using the logging system
		void log_short(int channel, short &data);
		void log_short(int channel, unsigned short &data) {log_short (channel,*(short*)&data);}
								 //helper for using the logging system
		void log_int  (int channel, int &data);
		void log_int  (int channel, unsigned int &data) {log_int (channel,*(int*)&data);}
								 //helper for using the logging system
		void log_data (int channel, void *data, int length);
		virtual void log_file (const std::string& fname);
		void log_fleet(int channel, class Fleet *fleet);
		int is_local ( int channel ) ;

	protected:

		char events_waiting;
		class GameEvent **waiting_events;
		virtual void do_game_events();
	public:
		void send_game_event ( class GameEvent *event );
		virtual void handle_game_event ( int source, class GameEvent *event );
		enum {
			maximum_events_waiting = 100,
			event_invalid = 0,
			event_change_lag,
			event_message,
			event_last
		};

		virtual void preinit();	 //pre-initialization (enough so that the destructor won't crash)
								 //initialization (loading data, creating a few game objects, whatever)
		virtual void init(Log *log);
		virtual ~Game();		 //deallocating memory etc.

		virtual void init_lag();

		virtual void _event ( Event *e );

		virtual void change_view (View *new_view);
		virtual void change_view (const char * name );

		View *view;
		VideoWindow *window;

		virtual void prepare();	 //called before doing anything with game objects contained within you.
		//This is particularly necessary if you are working with multiple Games simultaneously

		virtual void calculate();
		virtual void play();
								 // stupid C++
		virtual void animate(Frame *frame);
		virtual void animate();
		//	using Physics::animate;

		virtual void object_died(SpaceObject *who, SpaceLocation *source);
		virtual void ship_died(Ship *who, SpaceLocation *source);

		virtual void compare_checksums() ;
		virtual void handle_desynch( int local_checksum, int server_checksum, int client_checksum ) ;

		virtual bool game_ready();

		virtual void set_resolution (int screen_x, int screen_y);
		virtual void redraw();

		bool game_done;
		virtual void quit(const char *message) {game_done = true;}

		// Rob: moved to the physics class
		//	bool friendly_fire;
		//	double shot_relativity;

		int hiccup_margin;

		int lag_frames;
		virtual void increase_latency();
		virtual void decrease_latency();

		bool view_locked;
		bool physics_locked;

		int num_focuses;
		int focus_index;
		Presence **focus;
		virtual void add_focus (Presence *focus, int channel = -1);

		Targets gametargets;

		virtual Control *create_control (int channel, const char *type, char *config = "Config0", char *file = "scp.ini") ;
		virtual Ship    *create_ship (const char *id, Control *c, Vector2 pos, double angle, int team=0);
		virtual Ship    *create_ship (int channel, const char *id, const char *control, Vector2 pos, double angle, int team=0);

		virtual void pause();
		virtual void unpause();
		virtual bool is_paused();
		virtual void idle(int time = 5);

		virtual void play_music();

		Music *music;
		//Music *planet_victory;
		/*
		SpaceSprite *planetSprite;
		SpaceSprite *asteroidSprite;
		SpaceSprite *asteroidExplosionSprite;
		SpaceSprite *sparkSprite;
		SpaceSprite *hotspotSprite;
		SpaceSprite *kaboomSprite;
		SpaceSprite *panelSprite;
		SpaceSprite *xpl1Sprite; //added by Tau
		*/

	public:
		virtual double get_turbo();
		virtual int set_turbo( double t);
		virtual int set_frame_time (int t);
		//	protected:
		int time_paused;
		int paused_time;
		double normal_turbo;
		double f4_turbo;
		double next_tic_time;
		double msecs_per_render;
		double next_render_time;
		Histograph *tic_history;
		Histograph *render_history;
		double msecs_per_fps;
		double next_fps_time;
		virtual void fps();
		virtual bool handle_key(int k);
		virtual void save_screenshot();
		int show_fps;
		unsigned char local_checksum, client_checksum, server_checksum;

};
#endif							 // __MGAME_H__
