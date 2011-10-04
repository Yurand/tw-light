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

#ifndef __MMAIN_H__
#define __MMAIN_H__

#include "melee.h"
#include "mgame.h"

class NormalGame : public Game
{
	public:
		enum {
			num_asteroids = 4
		};

		~NormalGame();

		virtual void calculate();
		virtual void preinit();
		virtual void init (Log *_log = NULL);
		virtual void set_resolution(int screen_x, int screen_y);

		virtual void init_players();
		virtual void init_objects();

		virtual void ship_died(Ship *who, SpaceLocation *source);
		//kill history stuff
		int num_kills;
		struct ShipKill
		{
			int time;
			struct Party
			{
				unsigned int ally_flag;
				ShipData *data;
				ShipType *type;
			};
			Party victim, killer;
		} *kills;
		void display_stats();

		virtual int add_player (Control *c, int team_index, const char *name, const char *fleet, const char *fleet_file = "fleets.ini") ;
		int num_players;
		Control **player_control;
		//	ShipPanel **player_panel;
		Fleet **player_fleet;
		char **player_name;
		char *player_attributes;
		TeamCode *player_team;
		TeamCode *team_table;
		int team_table_size;
		//	virtual void player_said(int who, const char *what);

	protected:
		virtual bool handle_key(int k);

		int next_choose_new_ships_time;
		virtual void choose_new_ships() ;

		int indhealthtoggle, indteamtoggle;

};
#endif							 // __MMAIN_H__
