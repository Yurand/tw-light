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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <allegro.h>

#include "melee.h"
REGISTER_FILE
#include "scp.h"
#include "frame.h"

#include "mmain.h"
#include "mview.h"
#include "mcontrol.h"
#include "other/planet3d.h"
#include "mshppan.h"
#include "mship.h"
#include "mlog.h"
//#include "mnet1.h"
#include "mitems.h"
#include "mfleet.h"

#include "games/gflmelee.h"

#include "other/twconfig.h"

int NormalGame::add_player (Control *c, int team_index, const char *name, const char *fleet_section, const char *fleet_file)
{
	int i = num_players;
	num_players += 1;
	player_control = (Control**) realloc(player_control,   sizeof(Control*)   * num_players);
	player_name    =    (char**) realloc(player_name,      sizeof(char*)      * num_players);
	//	player_panel = (ShipPanel**) realloc(player_panel,     sizeof(ShipPanel*) * num_players);
	player_fleet =     (Fleet**) realloc(player_fleet,     sizeof(Fleet *)    * num_players);
	player_team =    (TeamCode*) realloc(player_team,      sizeof(TeamCode)   * num_players);
	player_control[i] = c;
	add_focus(c, c->channel);
	//	player_panel[i] = NULL;
	player_fleet[i] = new Fleet();
	player_fleet[i]->reset();
	player_name[i] = strdup(name);
	if (team_index >= team_table_size) {
		int i = team_table_size;
		team_table_size = team_index + 1;
		team_table = (TeamCode*) realloc(team_table, sizeof(TeamCode) * team_table_size);
		for (; i < team_table_size; i += 1) {
			if (i) team_table[i] = new_team();
			else team_table[i] = 0;
		}
	}
	if (team_index) player_team[i] = team_table[team_index];
	else player_team[i] = new_team();
	char sect[40];
	sprintf(sect, "Player%d", i+1);
	if (c->channel == channel_none) {
		tw_error("channel_none not allowed here");
		//log_file(fleet_file);
		//::fleet->load(NULL, fleet_section);
	}
	else if (log->get_direction(c->channel) & Log::direction_write) {
		player_fleet[i]->load(fleet_file, fleet_section);
		log_fleet(c->channel, player_fleet[i]);
		c->target_sign_color = ((3+i) % 7) + 1;
	} else {
		log_fleet(c->channel, player_fleet[i]);
	}
	tw_set_config_file(home_ini_full_path("tmp.ini"));
	set_config_string(sect, "Name", name);
	set_config_string(sect, "Type", c->getTypeName());
	set_config_int(sect, "Team", team_index);
	set_config_int(sect, "Channel", c->channel);
	set_config_int(sect, "StartingFleetCost", player_fleet[i]->getCost());
	set_config_int(sect, "StartingFleetSize", player_fleet[i]->getSize());
	player_fleet[i]->save(NULL, sect);
	player_fleet[i]->save("fleets.tmp", sect);
	return i;
}


void NormalGame::init_objects()
{
	STACKTRACE;
	int i;
	add(new Stars());
	Planet *planet = create_planet();
	//Planet *planet = new Planet (size/2, planetSprite, random(planetSprite->frames()));
	//add (planet);
	if (view) view->camera.pos = size/2;
	add(new WedgeIndicator(planet, 75, 4));
	for (i = 0; i < num_asteroids; i += 1) add(new Asteroid());
}


void NormalGame::init_players()
{
	STACKTRACE;
	switch (log->type) {
		case Log::log_normal:
		{
			for (int i = 0; true; i += 1) {
				char buffy[64];
				sprintf(buffy, "Player%d", i + 1);
				tw_set_config_file("scp.ini");
				const char *type = get_config_string(buffy, "Type", NULL);
				if (!type) break;
				if (strcmp(type, "none") == 0) continue;
				const char *name = get_config_string(buffy, "Name", buffy);
				char config[64];
				sprintf(config, "Config%d", get_config_int(buffy, "Config", 0));
				//int channel = channel_server;
				//if (strcmp(type, "WussieBot") == 0) channel = channel_none;
				//if (strcmp(type, "MoronBot") == 0) channel = channel_none;
				int ti = get_config_int(buffy, "Team", 0);
				add_player(create_control(channel_server, type, config), ti, name, buffy);
			}
		}
		break;
		case Log::log_net1client:
		case Log::log_net1server:
		{
			log_file(home_ini_full_path("server.ini"));
			//int use_teams_menu = get_config_int("Network", "NetworkMeleeUseTeams", 0);
			//if (use_teams_menu) {
			if (1) {
				int j;
				for (j = 0; j < 2; j += 1) {
					int ch;
					if (j == 0) ch = channel_server;
					else ch = channel_client;
					if (is_local(ch)) {
								 //each side determines whether they are using manually specified teams
						tw_set_config_file("client.ini");
						int use_teams_menu = get_config_int("Network", "NetworkMeleeUseTeams", 0);
						const char *simple_config =
							"[Player1]\nType=Human\nConfig=0\nTeam=0\n";
						for (int i = 0; true; i += 1) {
							char buffy[64];
							sprintf(buffy, "Player%d", i + 1);
							if (use_teams_menu) tw_set_config_file("scp.ini");
							else set_config_data(simple_config, strlen(simple_config));
							const char *type = get_config_string(buffy, "Type", NULL);
							if (!type) {
								int tmp = 0;
								log_int(ch, tmp);
								break;
							}
							if (strcmp(type, "none") == 0) continue;
							const char *name = get_config_string(buffy, "Name", buffy);
							char config[64];
							sprintf(config, "Config%d", get_config_int(buffy, "Config", 0));
							//int channel = channel_server;
							//if (strcmp(type, "WussieBot") == 0) channel = channel_none;
							//if (strcmp(type, "MoronBot") == 0) channel = channel_none;
							int ti = get_config_int(buffy, "Team", 0);
							{int tmp = 1; log_int(ch, tmp);}
							log_int(ch, ti);
							int name_length = strlen(name);
							log_int(ch, name_length);
							log_data(ch, (char*)name, name_length);
							add_player(create_control(ch, type, config), ti, name, buffy);
						}
					} else {
						for (int i = 0; true; i += 1) {
							int tmp;
							log_int(ch, tmp);
							if (tmp == 0) break;
							int team;
							char *name;
							log_int(ch, team);
							int name_length;
							log_int(ch, name_length);
							name = (char*)malloc((name_length+1)*sizeof(char));
							log_data(ch, name, name_length);
							name[name_length] = 0;
							add_player(create_control(ch, "Whatever"), team, name, NULL);
						}
					}
				}
			}
		}
		break;
	}
	return;
}


void NormalGame::set_resolution(int screen_x, int screen_y)
{
	STACKTRACE;
	int view_x, view_y;
	view_x = screen_x;
	view_y = screen_y;
	int n, m;
	n = 99;						 //int(ceil(double(num_players) / int(view_y / PANEL_HEIGHT)));
	if (n) m = int(ceil(num_players / (double)n));
	else m = 0;
	//view->set_window(screen, 0, 0, view_x - PANEL_WIDTH * m, view_y);
	redraw();
	return;
}


void NormalGame::preinit()
{
	STACKTRACE;
	Game::preinit();
	player_control = NULL;
	player_name = NULL;
	//	player_panel = NULL;
	player_fleet = NULL;

	player_team = NULL;
	team_table = NULL;
	num_kills = 0;
	kills = NULL;
	num_players = 0;
}


void NormalGame::init(Log *_log)
{
	STACKTRACE;
	Game::init(_log);

	team_table_size = 0;

	view->window->locate(0,0,0,0,0,0.9,0,1);

	tw_delete_file(home_ini_full_path("tmp.ini"));
	tw_delete_file(home_ini_full_path("fleets.tmp"));
	tw_set_config_file(home_ini_full_path("tmp.ini"));
	set_config_string (NULL, "Ignorethis", "");
	if (!log->playback) init_players();
	log_file(home_ini_full_path("tmp.ini"));
	if (log->playback) {
		for (int i = 0; true; i += 1) {
			char buffy[64];
			sprintf(buffy, "Player%d", i + 1);
			log_file(home_ini_full_path("tmp.ini"));
			const char *type = get_config_string(buffy, "Type", NULL);
			if (!type) break;
			const char *name = get_config_string(buffy, "Name", buffy);
			int channel = get_config_int(buffy, "Channel", -2);
			int ti = get_config_int(buffy, "Team", 0);
			add_player(create_control(channel, type), ti, name, buffy);
			player_fleet[i]->load(NULL, buffy);
			player_fleet[i]->save("fleets.tmp", buffy);
		}
	}

	prepare();
	init_objects();

	next_choose_new_ships_time = game_time + 200;

	// team and health indicators.
	indteamtoggle = 0;
	indhealthtoggle = 0;

	return;
}


NormalGame::~NormalGame()
{
	if (player_control) free (player_control);
	int i;
	if (player_name) {
		for (i = 0; i < num_players; i += 1) {
			free(player_name[i]);
		}
		free(player_name);
	}
	//	if (player_panel) free (player_panel);

	if (player_team) free(player_team);
	if (kills) free(kills);
}


static int kill_all_delay_counter = 0;
void NormalGame::calculate()
{
	STACKTRACE;
	Game::calculate();
	if (next_choose_new_ships_time <= game_time) {
		choose_new_ships();
		next_choose_new_ships_time = game_time + 24*60*60*1000;
	}

	// specially for play-testers:
	// kill all ships and ship-objects in the melee-game
	if (kill_all_delay_counter > 0) {
		kill_all_delay_counter -= frame_time;
	} else {

		if (key[KEY_LCONTROL] && key[KEY_ALT] && key[KEY_K]) {
								 // 1 second delay
			kill_all_delay_counter += 1000;

			for(std::list<SpaceLocation*>::iterator i=physics->item.begin();i!=physics->item.end();i++) {
				SpaceLocation *o;
				o = *i;
				if (!(o && o->exists()))
					continue;
				if (o->isPlanet() || o->isAsteroid())
					continue;
				o->die();
			}
		}
	}

	return;
}


void NormalGame::ship_died(Ship *who, SpaceLocation *source)
{
	STACKTRACE;
	int n = game_time + 4000;
	if (next_choose_new_ships_time > n) next_choose_new_ships_time = n;
	Game::ship_died(who, source);
	return;
}


void NormalGame::display_stats()
{
	STACKTRACE;
	pause();
	int i;
	for (i = 0; i < num_players; i += 1) {
		Fleet *fleet = player_fleet[i];
		switch (log->type) {
			case Log::log_net1client:
			case Log::log_net1server:
			{
				//				if (log->get_direction(player_control[i]->channel) & Log::direction_write)
				message.print(6000, 15, "%s status: : %d / ?? Ships, %d / ??? points", player_name[i], fleet->getSize(), fleet->getCost());
				//				else
				//					message.print(6000, 15, "%s status: : %d / %d points", buffy, fleet->cost, player_total_fleet[i]);
			}
			break;
			default:
			{
				message.print(6000, 15, "%s status: : %d / ?? Ships, %d / ??? points", player_name[i], fleet->getSize(), fleet->getCost());
			}
			break;
		}
	}
	unpause();
	return;
}


bool NormalGame::handle_key(int k)
{
	STACKTRACE;
	switch (k >> 8) {
		default:
		{
			return Game::handle_key(k);
		}
		break;
		case KEY_F5:
		{
			display_stats();
			return true;
		}
		break;
		case KEY_F7:
		{
			if (log->type == Log::log_normal) Game::handle_key(k);
			return true;
		}
		break;
		case KEY_F9:
		{
			if (log->type != Log::log_normal) return false;
			message.out("MUHAHAHAHAHA!!!!", 5000, 12);
			add(new Planet(random(size), meleedata.planetSprite, random(meleedata.planetSprite->frames())));
			return true;
		}
		break;
		//don't use hardwired normal keys
		case KEY_H:
			if ((k & 255) == 'H'-'A'+1) indhealthtoggle = ~indhealthtoggle;
			break;
		case KEY_T:
			if ((k & 255) == 'T'-'A'+1) indteamtoggle = ~indteamtoggle;
			break;
	}
	return false;
}


// added ROB
class TeamIndicator : public Presence
{
	public:
		int     *indtoggle;
		Ship    *mother;
		TeamIndicator(Ship *creator, int *toggle);

		virtual void calculate();
		virtual void animate(Frame *space);
};

void NormalGame::choose_new_ships()
{
	STACKTRACE;
	char tmp[40];
	int i;
	pause();
	message.out("Selecting ships...", 1000);
	int *slot = new int[num_players];
	//choose ships and send them across network
	for (i = 0; i < num_players; i += 1) {
		slot[i] = -2;
		if (player_control[i]->ship) {
		} else {
			//			if (player_panel[i]) player_panel[i]->window->hide();
			//			player_panel[i] = NULL;
			sprintf (tmp, "Player%d", i+1);
			Fleet *fleet = player_fleet[i];
			if (fleet->getSize() == 0) continue;
			char buffy[512];

			if (strlen(fleet->getTitle()) != 0)
				sprintf(buffy, "%s\n%s\n", player_name[i], fleet->getTitle());
			else
				sprintf(buffy, "%s\n", player_name[i]);

			slot[i] = player_control[i]->choose_ship(window, buffy, fleet);
			if (player_control[i]->channel != channel_none) {
				slot[i] = intel_ordering(slot[i]);
				log->buffer(player_control[i]->channel, &slot[i], sizeof(int));
				log->flush();
				//slot[i] = intel_ordering(slot[i]);
			}
		}
	}
	//recieve the ships that were chosen
	log->listen();
	for (i = 0; i < num_players; i += 1) {
		if (slot[i] == -2) continue;
		if (player_control[i]->channel != channel_none) {
			log->unbuffer(player_control[i]->channel, &slot[i], sizeof(int));
			slot[i] = intel_ordering(slot[i]);
		}
	}
	//create the ships that were chosen
	for (i = 0; i < num_players; i += 1) {
		if (slot[i] == -2) continue;
		sprintf (tmp, "Player%d", i+1);
		//fleet->load("./fleets.tmp", tmp);
		Fleet *fleet = player_fleet[i];
		if (slot[i] == -1) slot[i] = random() % fleet->getSize();
		if (slot[i] < 0 || slot[i] >= fleet->getSize()) {tw_error("trying to load invalid ship");}
		Ship *s = create_ship(fleet->getShipType(slot[i])->id, player_control[i], random(size), random(PI2), player_team[i]);
		if (!s) {tw_error("unable to create ship");}
		fleet->clear_slot(slot[i]);
		fleet->Sort();
		//fleet->save("./fleets.tmp", tmp);
		s->locate();
		add ( new WedgeIndicator ( s, 30, i+1 ) );
		ShipPanel *panel = new ShipPanel(s);
		panel->window->init(window);
		panel->window->locate(
			0, 0.9,
			0, i * (100.0/480),
			0, 0.1,
			0, (100.0/480)
			);
		add(panel);
		add(s->get_ship_phaser());

		// add a healthbar for the ship, and also a team indicator.
		add(new HealthBar(s, &indhealthtoggle));
		add(new TeamIndicator(s, &indteamtoggle));

		// CHECK FILE SIZES !! to intercept desynch before they happen.
		int myfsize, otherfsize;

		myfsize = file_size(s->type->data->file);
		otherfsize = myfsize;
		if (player_control[i]->channel != channel_none) {
			log_int(player_control[i]->channel, otherfsize);
		}

		if (otherfsize != myfsize) {
			// the player who loads the ship doesn't get this message, cause his own file is identical by default
			tw_error("DAT files have different size! This may cause a desynch. Press Retry to continue");
		}

	}
	delete[] slot;
	message.out("Finished selecting ships...", 1500);
	unpause();
	return;
}


// this should be places elsewhere I think ...
TeamIndicator::TeamIndicator(Ship *s, int *atoggle)
{
	STACKTRACE;
	indtoggle = atoggle;
	mother = s;
}


void TeamIndicator::calculate()
{
	STACKTRACE;
	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;
		return;
	}
}


void TeamIndicator::animate(Frame *space)
{
	STACKTRACE;
	if (!*indtoggle)
		return;

	if (mother->isInvisible())
		return;

	Vector2i co1, co2;

	co1 = corner(mother->pos - 0.5 * mother->size).round();
	co2 = corner(mother->pos + 0.5 * mother->size).round();

	if (co2.x < 0) return;
	if (co2.y < 0) return;
	if (co1.x >= space->surface->w) return;
	if (co1.y >= space->surface->h) return;

	int col;
								 // team 0 is black ...
	col = palette_color[mother->get_team() + 1];

	rect(space->surface, co1.x, co1.y, co2.x, co2.y, col);
	space->add_box(co1.x, co1.y, co2.x, co2.y);

}


REGISTER_GAME(NormalGame, "Melee")
