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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <allegro.h>

#include "melee.h"

#include "scp.h"
#include "frame.h"

#include "melee/mgame.h"
#include "melee/mmain.h"
#include "melee/mview.h"
#include "melee/mcontrol.h"
#include "melee/mcbodies.h"
#include "melee/mshppan.h"
#include "melee/mship.h"
#include "melee/mshot.h"
#include "melee/mlog.h"
#include "melee/manim.h"
#include "melee/mfleet.h"

#include "util/aastr.h"

#include "ggob.h"
//#include "sc1ships.h"
//#include "sc2ships.h"

#include "other/gup.h"
#include "other/dialogs.h"
#include "other/twconfig.h"

#define gobgame ((GobGame*)game)

////////////////////////////////////////////////////////////////////////
//				Gob stuff
////////////////////////////////////////////////////////////////////////

int GobAsteroid::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (!exists())
		return 0;
	int i = Asteroid::handle_damage(source, normal, direct);
	if (!exists()) {
		GobPlayer *p = gobgame->get_player(source);
		if (p)
			p->buckazoids += 1;
	}
	return i;
}


void GobAsteroid::death()
{
	STACKTRACE;
	Animation *a = new Animation(this, pos,
		explosion, 0, explosion->frames(), time_ratio, get_depth());
	a->match_velocity(this);
	game->add(a);

	game->add ( new GobAsteroid() );
	return;
}


void GobGame::preinit()
{
	STACKTRACE;
	Game::preinit();

	gobplayers = 0;
	gobplayer = NULL;
	gobenemies = 0;
	max_enemies = 0;
	gobenemy = NULL;

	int i;
	for (i = 0; i < 3; i += 1)
		stationSprite[i] = NULL;
	for (i = 0; i < 3; i += 1)
		station_pic_name[i] = NULL;
	for (i = 0; i < 3; i += 1)
		station_build_name[i] = NULL;
	defenderSprite = NULL;
}


void GobGame::add_gobplayer(Control *control)
{
	STACKTRACE;
	int i = gobplayers;
	gobplayers += 1;
	gobplayer = (GobPlayer**) realloc(gobplayer, sizeof(GobPlayer*) * gobplayers);
	gobplayer[i] = new GobPlayer();
	gobplayer[i]->init(control, new_team());
	add_focus(control, control->channel);
	return;
}


void GobPlayer::died(SpaceLocation *killer)
{
	STACKTRACE;
	if (upgrade_list[UpgradeIndex::divinefavor]->num && (random()&1)) {
								 //divine favor
		ship->crew = ship->crew_max;
		ship->batt = ship->batt_max;
		ship->translate(random(Vector2(-2048,-2048), Vector2(2048,2048)));
		ship->state = 1;
	}
	else
		ship = NULL;
	return;
}


void GobGame::play_sound (SAMPLE *sample, SpaceLocation *source, int vol, int freq)
{
	double v;
	Vector2 d = source->normal_pos() - space_center;
	d = normalize(d + size/2, size) - size/2;
	v = 1000;
	if (space_zoom > 0.01) v = 500 + space_view_size.x / space_zoom / 4;
	v = 1 + magnitude_sqr(d) / (v*v);
	Game::play_sound(sample, source, iround(vol/v), freq);
}


void GobGame::init(Log *_log)
{
	STACKTRACE;
	int i;
	Game::init(_log);

	log_file("server.ini");
	max_enemies = get_config_int("Gob", "MaxEnemies", 32);
	gobenemy = (GobEnemy**) malloc(sizeof(GobEnemy*) * max_enemies);

	size = Vector2(24000, 24000);

	enemy_team = new_team();

	TW_DATAFILE *tmpdata;
	tmpdata = tw_load_datafile_object(data_full_path("gob.dat").c_str(), "station0sprite");
	if (!tmpdata)
		tw_error( "couldn't find gob.dat#station0sprite");
	stationSprite[0] = new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED | SpaceSprite::MIPMAPED, 64);
	tw_unload_datafile_object(tmpdata);
	stationSprite[0]->permanent_phase_shift(8);

	tmpdata = tw_load_datafile_object(data_full_path("gob.dat").c_str(), "station1sprite");
	if (!tmpdata)
		tw_error ("couldn't find gob.dat#station1sprite");
	stationSprite[1] = new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED | SpaceSprite::MIPMAPED, 64);
	tw_unload_datafile_object(tmpdata);
	stationSprite[1]->permanent_phase_shift(8);

	tmpdata = tw_load_datafile_object(data_full_path("gob.dat").c_str(), "station2sprite");
	if (!tmpdata)
		tw_error ("couldn't find gob.dat#station2sprite");
	stationSprite[2] = new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED | SpaceSprite::MIPMAPED, 64);
	tw_unload_datafile_object(tmpdata);
	stationSprite[2]->permanent_phase_shift(8);

	tmpdata = tw_load_datafile_object(data_full_path("gob.dat").c_str(), "defender");
	if (!tmpdata)
		tw_error ("couldn't find gob.dat#defender");
	defenderSprite = new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED | SpaceSprite::MIPMAPED);
	tw_unload_datafile_object(tmpdata);

	station_pic_name[0] = "gob.dat#station0picture.bmp";
	station_pic_name[1] = "gob.dat#station1picture.bmp";
	station_pic_name[2] = "gob.dat#station2picture.bmp";
	station_build_name[0] = "supbl";
	station_build_name[1] = "orzne";
	station_build_name[2] = "kohma";

	prepare();

	add(new Stars());

	num_planets = 0;
	i = 0;
	add_planet_and_station(meleedata.planetSprite, i,
		stationSprite[i], station_build_name[i], station_pic_name[i]);
	i = 1;
	add_planet_and_station(meleedata.planetSprite, i, stationSprite[i], station_build_name[i], station_pic_name[i]);
	i = 2;
	add_planet_and_station(meleedata.planetSprite, i, stationSprite[i], station_build_name[i], station_pic_name[i]);
	i = random() % 3;
	add_planet_and_station(meleedata.planetSprite, i, stationSprite[i], "utwju", station_pic_name[i]);

	for (i = 0; i < 19; i += 1) add(new GobAsteroid());

	int server_players, client_players;
	tw_set_config_file("client.ini");
	server_players = client_players = get_config_int("Gob", "NumPlayers", 1);
	if (!lag_frames) client_players = 0;
	log_int(channel_server, server_players);
	log_int(channel_client, client_players);
	for (i = 0; i < server_players; i += 1) {
		char buffy[256];
		sprintf(buffy, "Config%d", i);
		add_gobplayer(create_control(channel_server, "Human", buffy));
		gobplayer[i]->new_ship(shiptype("supbl"));
		Ship *s = gobplayer[i]->ship;
		s->translate(size/2-s->normal_pos());
		double angle = PI2 * i / (client_players + server_players);
		s->translate(rotate(Vector2(260, 120), angle));
		s->accelerate(s, PI2/3 + angle, 0.17, MAX_SPEED);
	}
	for (i = server_players; i < client_players + server_players; i += 1) {
		char buffy[256];
		sprintf(buffy, "Config%d", i - server_players);
		add_gobplayer(create_control(channel_client, "Human", buffy));
		gobplayer[i]->new_ship(shiptype("supbl"));
		Ship *s = gobplayer[i]->ship;
		s->translate(size/2-s->normal_pos());
		double angle = PI2 * i / (client_players + server_players);
		s->translate(rotate(Vector2(260, 120), angle));
		s->accelerate(s, PI2/3 + angle, 0.17, MAX_SPEED);
	}

	for (i = 0; i < gobplayers; i += 1) add ( new RainbowRift() );

	next_add_new_enemy_time = 1000;
	add_new_enemy();
	this->change_view("Hero");
	view->window->locate(
		0,0,
		0,0,
		0,0.9,
		0,1
		);
	return;
}


GobGame::~GobGame()
{
	delete stationSprite[0];
	delete stationSprite[1];
	delete stationSprite[2];
	delete defenderSprite;
	int i;
	for (i = 0; i < gobplayers; i += 1) {
		delete gobplayer[i];
	}
	free(gobplayer);
	for (i = 0; i < gobenemies; i += 1) {
		delete gobenemy[i];
	}
	free(gobenemy);
	return;
}


void GobGame::add_planet_and_station ( SpaceSprite *planet_sprite,
int planet_index, SpaceSprite *station_sprite,
const char *builds, const char *background)
{
	Planet *p = new Planet (size/2, planet_sprite, planet_index);
	if (num_planets) while (true) {
		SpaceLocation *n;
		n = p->nearest_planet();
		if (!n || (p->distance(n) > 1500)) break;
		p->translate(random(size));
	}
	add ( p );

	GobStation *gs = new GobStation(station_sprite, p, builds, background);
	gs->collide_flag_sameship = ALL_LAYERS;
	gs->collide_flag_sameteam = ALL_LAYERS;
	gs->collide_flag_anyone = ALL_LAYERS;
	add ( gs );

	gobgame->planet[gobgame->num_planets] = p;
	gobgame->station[gobgame->num_planets] = gs;
	gobgame->num_planets += 1;
}


void GobGame::fps()
{
	STACKTRACE;
	Game::fps();

	message.print((int)msecs_per_fps, 15, "enemies: %d", (int)gobenemies);
	message.print((int)msecs_per_fps, 15, "time: %d", (int)(game_time / 1000));

	int i = 0;
	for (i = 0; i < gobplayers; i += 1) {
		if (!is_local(gobplayer[i]->channel))
			continue;

		if (gobplayer[i]->ship) {
			message.print((int)msecs_per_fps, 15-i, "coordinates: %d x %d",
				iround(gobplayer[i]->ship->normal_pos().x),
				iround(gobplayer[i]->ship->normal_pos().y));
		}
		message.print((int)msecs_per_fps, 15-i, "starbucks: %d", gobplayer[i]->starbucks);
		message.print((int)msecs_per_fps, 15-i, "buckazoids: %d", gobplayer[i]->buckazoids);
		message.print((int)msecs_per_fps, 15-i, "kills: %d", gobplayer[i]->kills);
	}
	return;
}


void GobGame::calculate()
{
	STACKTRACE;

	if (next_add_new_enemy_time <= game_time) {
		next_add_new_enemy_time = game_time;
		int t = 28;
		if ((random() % t) < 4) add_new_enemy();
		int e = gobenemies;
		e -= random() % (1 + game_time / (250 * 1000));
		if (0) ;
		else if (e >=12) next_add_new_enemy_time += 15000;
		else if (e >= 7) next_add_new_enemy_time += 7000;
		else if (e >= 4) next_add_new_enemy_time += 5000;
		else if (e >= 2) next_add_new_enemy_time += 3000;
		else if (e >= 1) next_add_new_enemy_time += 2000;
		else next_add_new_enemy_time += 1000;
	}
	Game::calculate();
	return;
}


int GobGame::get_enemy_index(SpaceLocation *what)
{
	STACKTRACE;
	int i;
	Ship *s = what->ship;
	if (!s) return -1;
	for (i = 0; i < gobenemies; i += 1) {
		if (gobenemy[i]->ship == s) return i;
	}
	return -1;
}


void GobGame::ship_died(Ship *who, SpaceLocation *source)
{
	STACKTRACE;

	GobPlayer *p = this->get_player(who);
	if (p && (p->ship == who)) { //Player died
		p->died(source);
	}
	int i = get_enemy_index(who);
	if ((i != -1) && (gobenemy[i]->ship == who)) {
		GobEnemy *e = gobenemy[i];
		e->died(source);
		gobenemies -= 1;
		GobEnemy *tmp = gobenemy[gobenemies];
		gobenemy[i] = tmp;
		p = get_player(source);
	}

	Game::ship_died(who, source);
	return;
}


GobPlayer *GobGame::get_player(SpaceLocation *what)
{
	STACKTRACE;
	int i;
	for (i = 0; i < gobplayers; i += 1) {
		if (what->get_team() == gobplayer[i]->team) return gobplayer[i];
	}
	return NULL;
}


void GobGame::add_new_enemy()
{
	STACKTRACE;

	static char *enemy_types[] = {
		"thrto", "zfpst", "shosc", "dragr",
		"kahbo", "ilwsp",
		"syrpe", "kzedr", "mmrxf",
		"druma", "earcr",
		"yehte", "chmav"
	};
	const int num_enemy_types = sizeof(enemy_types)/sizeof(enemy_types[0]);
	if (gobenemies == max_enemies)
		return;
	GobEnemy *ge = new GobEnemy();

	int base = game_time / 30 / 1000;
	if (gobenemies >= 4)
		base += (gobenemies*gobenemies - 10) / 5;

	gobenemy[gobenemies] = ge;
	gobenemies += 1;
	base = iround(base / 1.5);
	int e = 99999;
	while (e >= num_enemy_types) {
		/*
		base	time	low		high

		  1		.5		-0.1	3.7
		  10	5		2.62	7.47
		  50	25		5.89	14.24
		 100	50		8.1		17.3
		 200	100		11.01	26.49

		*/
		e = base;
		e = random() % (e + 2);
		e = random() % (e + 3);
		if (e < pow(2.5*base,0.4) - 1)
			e = random() % num_enemy_types;
		if (e > sqrt(3*base) + 2)
			e = random() % (e + 1);
		//if (e > num_enemy_types * 2) e = e % num_enemy_types;
		e = e;
	}
	Ship *ship = create_ship(channel_server, enemy_types[e], "WussieBot", random(size), random(PI2), enemy_team);
	//  if (!strcmp(enemy_types[e], "shosc")) ((ShofixtiScout*)ship)->specialDamage /= 4;
	//  if (!strcmp(enemy_types[e], "zfpst")) ((ZoqFotPikStinger*)ship)->specialDamage /= 2;
	//  if (!strcmp(enemy_types[e], "syrpe")) ((SyreenPenetrator*)ship)->specialDamage /= 2;
	//  if (!strcmp(enemy_types[e], "dragr")) ship->special_drain *= 2;
	//  if (!strcmp(enemy_types[e], "chmav")) {
	//    ((ChmmrAvatar*)ship)->weaponDamage += 1;
	//    ((ChmmrAvatar*)ship)->weaponDamage /= 2;
	//    ((ChmmrAvatar*)ship)->specialForce *= 2;
	//    ((ChmmrAvatar*)ship)->specialRange *= 2;
	//  }
	int sb, bz;
	sb = 1 + e / 4;
	if (sb > 2) sb -= 1;
	bz = (e - 9) / 2;
	if (bz > 1) bz -= 1;
	if (sb < 0) sb = 0;
	if (bz < 0) bz = 0;
	ge->init(ship, sb, bz);
	add(ship->get_ship_phaser());
	//add(ship);
	return;
}


void GobEnemy::init(Ship *ship, int kill_starbucks, int kill_buckazoids)
{
	STACKTRACE;
	this->ship = ship;
	this->starbucks = kill_starbucks;
	this->buckazoids = kill_buckazoids;
	return;
}


void GobEnemy::died(SpaceLocation *what)
{
	STACKTRACE;
	GobPlayer *p = gobgame->get_player(what);
	if (p) {
		p->starbucks += starbucks;
		p->buckazoids += buckazoids;
		p->kills += 1;
	}
	return;
}


GobPlayer::~GobPlayer()
{
	free (pair_list);
}


void GobPlayer::init(Control *c, TeamCode team)
{
	STACKTRACE;
	channel = c->channel;
	starbucks = 0;
	buckazoids = 0;
	kills = 0;
	value_starbucks = 0;
	value_buckazoids = 0;
	num_pairs = 0;
	pair_list = NULL;
	ship = NULL;
	panel = NULL;
	control = c;
	total = 0;
	this->team = team;
	int i, j;
	for (i = 0; ::upgrade_list[i]; i += 1) ::upgrade_list[i]->index = i;
	upgrade_list = new Upgrade*[i+1];
	upgrade_list[i] = NULL;
	for (j = 0; j < i; j += 1) {
		upgrade_list[j] = ::upgrade_list[j]->duplicate();
		upgrade_list[j]->clear(NULL, NULL, this);
	}
	return;
}


GobPlayer::pair *GobPlayer::_get_pair(const char *id)
{
	STACKTRACE;
	if (!pair_list) return NULL;
	int i;
	for (i = 0; i < num_pairs; i += 1) {
		if (!strcmp(pair_list[i].id, id))
			return &pair_list[i];
	}
	return NULL;
}


void GobPlayer::_add_pair(const char *id, int value)
{
	STACKTRACE;
	if (_get_pair(id)) {
		tw_error("GobPlayer::_add_pair - \"%s\" already exists", id);
		return;
	}
	pair_list = (pair*)realloc(pair_list, sizeof(pair) * (num_pairs+1));
	pair_list[num_pairs].id = strdup(id);
	pair_list[num_pairs].value = value;
	num_pairs += 1;
	return;
}


int GobPlayer::read_pair(const char *id)
{
	STACKTRACE;
	pair *p = _get_pair(id);
	if (p) return p->value;
	return -1;
}


void GobPlayer::write_pair(const char *id, int value)
{
	STACKTRACE;
	pair *p = _get_pair(id);
	if (p) p->value = value;
	else _add_pair(id, value);
	return;
}


int GobPlayer::charge (char *name, int price_starbucks, int price_buckazoids)
{
	char buffy1[512];
	sprintf(buffy1, "Price: %d starbucks plus %d buckazoids", price_starbucks, price_buckazoids);
	if ((starbucks < price_starbucks) || (buckazoids < price_buckazoids)) {
		if (game->is_local(channel))
			alert("You don't have enough.", name, buffy1, "Cancel", NULL, 0, 0);
		return 0;
	}
	int r = 0;
	if (game->is_local(channel))
		r = alert ("Do you wish to make this purchase?", name, buffy1, "&No", "&Yes", 'n', 'y');
	game->log_int(channel, r);
	if (r == 2) {
		starbucks -= price_starbucks;
		buckazoids -= price_buckazoids;
		return 1;
	}
	return 0;
}


void GobPlayer::new_ship(ShipType *type)
{
	STACKTRACE;
	Ship *old = ship;
	Vector2 pos = 0;
	double a = 0;
	int i;
	if (old) {
		pos = old->normal_pos();
		a = old->get_angle();
	}

	ship = game->create_ship ( type->id, control, pos, a, team);

	if (panel) panel->die();
	panel = NULL;
	panel = new ShipPanel(ship);
	panel->always_redraw = true;
	panel->window->init(game->window);
	if (game->is_local(control->channel)) {
		panel->window->locate(
			0,0.9,
			0,0,
			0,0.1,
			0,0.25
			);
	}
	else {
		panel->window->locate(
			0,0.9,
			0,0.25,
			0,0.1,
			0,0.25
			);
	}
	panel->set_depth(10);
	game->add(panel);

	for (i = 0; upgrade_list[i]; i += 1) {
		upgrade_list[i]->clear(old, ship, this);
	}
	if (old) {
		old->die();
		game->add(ship);
	}
	else game->add(ship->get_ship_phaser());
	return;
}


void GobStation::buy_new_ship_menu(GobPlayer *s)
{
	STACKTRACE;

	char buffy1[512], buffy2[512];
	ShipType *otype = s->ship->type;
	ShipType *ntype = shiptype(build_type);
	if (otype == ntype) {
		sprintf (buffy1, "You already have a %s", ntype->name);
		if (game->is_local(s->channel))
			alert(buffy1, NULL, NULL, "&Cancel", NULL, 'c', 0);
		return;
	}
	int ossb = (s->value_starbucks*3) / 4 + (s->ship->type->cost*1)/1;
	int osbz = (s->value_buckazoids*3) / 4 + (s->ship->type->cost*1)/1;
	int nssb = ntype->cost;
	int nsbz = ntype->cost;
	sprintf (buffy1, "You have a %s worth %d s$ / %d b$", otype->name, ossb, osbz);
	sprintf (buffy2, "A %s costs %d s$ / %d b$", ntype->name, nssb, nsbz);
	if ((nssb <= (ossb + s->starbucks)) && (nsbz <= (osbz + s->buckazoids))) {
		int i = 0;
		if (game->is_local(s->channel))
			i = alert(buffy1, buffy2, "Do you wish to buy it?", "Yeah!", "No", 'y', 'n');
		game->log_int(s->channel, i);
		if (i == 1) {
			s->starbucks -= nssb - ossb;
			s->buckazoids -= nsbz - osbz;
			s->new_ship(ntype);
		}
	}
	else {
		if (game->is_local(s->channel))
			alert (buffy1, buffy2, "You don't have enough to buy it", "Cancel", NULL, 0, 0);
	}
	return;
}


GobStation::GobStation ( SpaceSprite *pic, SpaceLocation *orbit_me, const char *ship, const char *background) :
Orbiter(pic, orbit_me, random() % 200 + 500)
{
	build_type = ship;
	background_pic = background;
	layer = LAYER_CBODIES;
	mass = 99;
}


#define STATION_DIALOG_DEPART  0
#define STATION_DIALOG_UPGRADE 1
#define STATION_DIALOG_NEWSHIP 2
#define STATION_DIALOG_REPAIR  3
static DIALOG station_dialog[] =
{								 // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)
	{ d_agup_button_proc,     385,  50,   150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Depart Station" , NULL, NULL },
	{ d_agup_button_proc,     385,  90,   150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Upgrade Ship" , NULL, NULL },
	{ d_agup_button_proc,     385,  130,  150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Buy New Ship" , NULL, NULL },
	{ d_agup_button_proc,     385,  170,  150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Repair Ship" , NULL, NULL },
	{ d_agup_text_proc,       185,  420,  270,  30,   255,  0,    0,    0,          0,    0,    dialog_string[0], NULL, NULL },
	{ d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL }
};
void GobStation::station_screen(GobPlayer *s)
{
	STACKTRACE;
	BITMAP *background = load_bitmap(data_full_path(background_pic).c_str(), NULL);
	if (!background) {
		message.print(1000, 15, "%s", background_pic);
		tw_error ("couldn't load station background");
	}
	game->window->lock();
	aa_set_mode(AA_DITHER);
	aa_stretch_blit(background, game->window->surface,
		0,0,background->w,background->h,
		game->window->x,game->window->y,game->window->w, game->window->h);
	game->window->unlock();
	while (true) {
		sprintf(dialog_string[0], "%03d Starbucks  %03d Buckazoids", s->starbucks, s->buckazoids);
		int r = 0;
		if (game->is_local(s->channel))
			r = tw_do_dialog(game->window, station_dialog, STATION_DIALOG_DEPART);
		game->log_int(s->channel, r);
		switch (r) {
			case STATION_DIALOG_UPGRADE:
			{
				upgrade_menu(this, s);
				aa_set_mode(AA_DITHER);
				aa_stretch_blit(background, game->window->surface,
					0,0,background->w,background->h,
					game->window->x,game->window->y,
					game->window->w, game->window->h);
			}
			break;
			case STATION_DIALOG_NEWSHIP:
			{
				buy_new_ship_menu(s);
			}
			break;
			case STATION_DIALOG_REPAIR:
			{
				if (s->ship->crew == s->ship->crew_max) {
					if (game->is_local(s->channel))
						alert("You don't need repairs", "", "", "Oh, okay", "I knew that", 0, 0);

					break;
				}
				int p = 0;
				if (game->is_local(s->channel))
					p = alert3("Which would you prefer", "to pay for your repairs", "", "1 &Starbuck", "1 &Buckazoid", "&Nothing!", 's', 'b', 'n');
				game->log_int(s->channel, p);
				switch (p) {
					case 1:
					{
						if (s->starbucks) {
							s->starbucks -= 1;
							s->ship->crew = s->ship->crew_max;
						}
						else {
							if (game->is_local(s->channel))
								alert("You don't have enough!", NULL, NULL, "&Shit", NULL, 's', 0);
						}
					}
					break;
					case 2:
					{
						if (s->buckazoids) {
							s->buckazoids -= 1;
							s->ship->crew = s->ship->crew_max;
						}
						else {
							if (game->is_local(s->channel))
								alert("You don't have enough!", NULL, NULL, "&Shit", NULL, 's', 0);
						}
					}
					break;
					case 3:
					{
						r = STATION_DIALOG_DEPART;
					}
					break;
				}
			}
			break;
		}
		if (r == STATION_DIALOG_DEPART) break;
	}
	return;
}


void GobStation::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	SpaceObject::inflict_damage(other);
	if (!other->isShip()) return;
	GobPlayer *p = gobgame->get_player(other);
	if (!p) return;
	gobgame->pause();
	char buffy[256];
	int a;
	sprintf(buffy, "First visited station %s at time", build_type);
	a = p->read_pair(buffy);
	if (a == -1) p->write_pair(buffy, game->game_time);
	sprintf(buffy, "Visited station %s N times", build_type);
	a = p->read_pair(buffy);
	if (a == -1) a = 0;
	p->write_pair(buffy, a+1);
	station_screen(p);
	gobgame->unpause();
	return;
}


int num_upgrade_indexes;
int upgrade_index[999];
GobPlayer *upgrade_list_for;
char *upgradeListboxGetter(int index, int *list_size)
{
	static char tmp[150];
	if (index < 0) {
		*list_size = num_upgrade_indexes;
		return NULL;
	}
	int i = upgrade_index[index];
	sprintf(tmp, "%1d %3d s$ / %3d b$  :  %s", upgrade_list_for->upgrade_list[i]->num, upgrade_list_for->upgrade_list[i]->starbucks, upgrade_list_for->upgrade_list[i]->buckazoids, upgrade_list_for->upgrade_list[i]->name);
	return tmp;
}


#define UPGRADE_DIALOG_EXIT 0
#define UPGRADE_DIALOG_LIST 3
static DIALOG upgrade_dialog[] =
{								 // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)
	{ d_agup_button_proc,     10,  415,  170,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Station menu" , NULL, NULL },
	{ d_agup_textbox_proc,    20,  40,   250,  40,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Upgrade Menu", NULL, NULL },
	{ d_agup_text_proc,       10,  100,  540,  20,   255,  0,    0,    D_EXIT,     0,    0,    (void *)" # Starbucks Buckazoids Description                     ", NULL, NULL },
	{ d_agup_list_proc,       10,  120,  540,  280,  255,  0,    0,    D_EXIT,     0,    0,    (void *) upgradeListboxGetter, NULL, NULL },
	{ d_agup_text_proc,       185, 420,  270,  30,   255,  0,    0,    0,          0,    0,    dialog_string[0], NULL, NULL },
	{ d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL }
};

void GobStation::upgrade_menu(GobStation *station, GobPlayer *gs)
{
	STACKTRACE;
	int i;
	upgrade_list_for = gs;
	clear_to_color(screen, palette_color[8]);
	while (true) {
		sprintf(dialog_string[0], "%03d Starbucks  %03d Buckazoids", gs->starbucks, gs->buckazoids);
		int j = 0;
		for (i = 0; gs->upgrade_list[i]; i += 1) {
			if (gs->upgrade_list[i]->update(gs->ship, station, gs)) {
				upgrade_index[j] = i;
				j += 1;
			}
		}
		num_upgrade_indexes = j;
		int m = 0;
		if (game->is_local(gs->channel))
			m = tw_do_dialog(game->window, upgrade_dialog, UPGRADE_DIALOG_EXIT);
		game->log_int(gs->channel, m);
		if (m == UPGRADE_DIALOG_EXIT) return;
		if (m == UPGRADE_DIALOG_LIST) {
			int i = 0;
			if (game->is_local(gs->channel))
				i = upgrade_dialog[UPGRADE_DIALOG_LIST].d1;
			game->log_int(gs->channel, i);
			i = upgrade_index[i];
			Upgrade *u = gs->upgrade_list[i];
			if (gs->charge(u->name, u->starbucks, u->buckazoids)) {
				u->execute(gs->ship, station, gs);
				u->charge(gs);
			}
		}
	}
	return;
}


GobDefender::GobDefender ( Ship *ship)
: SpaceObject (ship, ship->normal_pos(), 0, gobgame->defenderSprite)
{
	base_phase = 0;
	next_shoot_time = 0;
	collide_flag_anyone = 0;
}


void GobDefender::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();
	if (!ship) {
		die();
		return;
	}
	if (next_shoot_time < gobgame->game_time) {
		SpaceObject *target = NULL;
		Query q;
		q.begin(this, OBJECT_LAYERS &~ bit(LAYER_SHIPS), 300);
		while (q.currento && !target) {
			if (!q.currento->sameTeam(ship)) {
				SpaceLine *l = new PointLaser (
					this, palette_color[4], 2, 150,
					this, q.currento
					);
				add(l);
				if (l->exists()) target = q.currento;
			}
			q.next();
		}
		if (target) {
			next_shoot_time = gobgame->game_time + 400;
		}
	}
	double a = base_phase + (gobgame->game_time % 120000) * ( PI2 / 1000.0) / 6;
	angle = normalize(a,PI2);
	pos = normalize(ship->normal_pos() + 270 * unit_vector ( angle ));
	return;
}


RainbowRift::RainbowRift ()
//: SpaceLocation ( NULL, 12800, 12800, 0)
: SpaceLocation ( NULL, random(map_size), 0)
{
	int i;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	collide_flag_anyone = 0;
	for (i = n*6-6; i < n*6+2; i += 1) {
		p[i] = 75 + random(150.0);
	}
	for (i = 0; i < n; i += 1) {
		squiggle();
	}
	next_time = game->game_time;
	next_time2 = game->game_time;
}


void RainbowRift::animate( Frame *frame )
{
	STACKTRACE;
	Vector2 s;
	s = corner(pos, Vector2(300,300));
	if ((s.x < -500) || (s.x > space_view_size.x + 500) ||
		(s.y < -500) || (s.y > space_view_size.y + 500))
		return;
	int b[n*6+2];
	int i;
	for (i = 0; i < n*6+2; i += 2) {
		b[i] = iround(s.x + p[i] * space_zoom);
		b[i+1] = iround(s.y + p[i+1] * space_zoom);
	}
	for (i = 0; i < n; i += 1) {
		RGB tc = c[n-i-1];
		int a = tw_color(tc.r, tc.g, tc.b);
		spline ( frame->surface, &b[i*6], a );
	}
	frame->add_box (
		iround(s.x - 2), iround(s.y -2),
		iround(300 * space_zoom+5), iround(300 * space_zoom+5)
		);
	return;
}


void RainbowRift::squiggle()
{
	STACKTRACE;
	int i;
	int m = n*6+2;
	for (i = 0; i < m - 6; i += 1) {
		p[i] = p[i+6];
	}
	p[m-6] = p[m-8] * 2 - p[m-10];
	p[m-5] = p[m-7] * 2 - p[m-9];
	p[m-4] = 75 + random(150.0);
	p[m-3] = 75 + random(150.0);
	p[m-2] = 75 + random(150.0);
	p[m-1] = 75 + random(150.0);
	for (i = 0; i < n-1; i += 1) {
		c[i] = c[i+1];
	}
	int r, g, b;
	r = int(game->game_time * 0.5) % 360;
	hsv_to_rgb( r, 1.0, 1.0, &r, &g, &b );
	c[n-1].r = r;
	c[n-1].g = g;
	c[n-1].b = b;
	return;
}


void RainbowRift::calculate()
{
	STACKTRACE;
	while (game->game_time > next_time) {
		next_time += 25;
		squiggle();
	}
	while (game->game_time > next_time2) {
		next_time2 += random() % 10000;
		Query q;
		for (q.begin(this, bit(LAYER_SHIPS), 40); q.current; q.next()) {
			GobPlayer *p = gobgame->get_player(q.currento);
			if (q.currento == p->ship) {
				int i = 0;
				i = p->control->choose_ship(game->window, "You found the Rainbow Rift!", reference_fleet);
				game->log_int(p->channel, i);
				if (i == -1)
					i = random(reference_fleet->getSize());
				game->redraw();
				if (reference_fleet->getShipType(i) == p->ship->type) {
					p->starbucks += random() % 80;
					p->buckazoids += random() % 80;
					game->add(new RainbowRift());
				}
				else {
					p->starbucks += random() % (1+p->value_starbucks);
					p->buckazoids += random() % (1+p->value_buckazoids);
					p->new_ship(reference_fleet->getShipType(i));
				}
				die();
			}
		}
	}
	return;
}


REGISTER_GAME(GobGame, "GOB")

/* intended upgrades:

faster marines       == faster Orz Marines, cost 4s
upgrade battle armor == tougher Orz Marines, cost 4s/4b
improve range        == long range Orz cannons, cost 3s
regeneration         == crew regeneration for Orz, cost 10s/25b, only purchasable once
sharper shurikens    == +1 damage for Kohr-Ah blades, cost 5s
faster shurikens     == higher velocity for Kohr-Ah blades, cost 4s
larger corona        == longer range for Kohr-Ah FRIED, cost 15s, only purchasable once
hotter corona        == double damage for Kohr-Ah FRIED, cost 10s, only purchasable once
divine favor         == pkunk respawn, only available from one base, cost 48s/0b, only purchasable once, kept when ship is sold
sentinel system      == Chmmr ZapSats, only available from one base, cost 30s/30b

long range scanners  == can zoom farther out, gives radar, only available from one base, cost 8s/20b

*/
