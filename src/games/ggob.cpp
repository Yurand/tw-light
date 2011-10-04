
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <allegro.h>

#include "melee.h"
REGISTER_FILE
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

#include "ships/shpshosc.h"
#include "ships/shpzfpst.h"
#include "ships/shpsyrpe.h"
#include "ships/shpdruma.h"
#include "ships/shpkzedr.h"
#include "ships/shpearcr.h"
#include "ships/shpchmav.h"
#include "ships/shpandgu.h"

//#include "../sc1ships.h"
//#include "../sc2ships.h"

#include "../other/gup.h"

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
		if (p) p->buckazoids += 1;
	}
	return i;
}


void GobAsteroid::death ()
{
	STACKTRACE;

	Animation *a = new Animation(this, pos,
		explosion, 0, explosion->frames(), time_ratio, get_depth());
	a->match_velocity(this);
	game->add(a);

	game->add ( new GobAsteroid() );
	return;
}


void GobPlanet::calculate ()
{
	STACKTRACE;
	SpaceObject::calculate();
	SpaceObject *o;
	Query a;
	a.begin(this, OBJECT_LAYERS, gravity_range);
	for (;a.currento;a.next()) {
		o = a.currento;
		if (o->mass > 0) {
			bool roswell = false;
			for (int i = 0; i < gobgame->gobplayers; i++) {
				if (o->ship == gobgame->gobplayer[i]->ship && gobgame->gobplayer[i]->upgrade_list[UpgradeIndex::roswelldevice]->num)
					roswell = true;
			}
			if (roswell) continue;
			double r = distance(o);
			if (r < gravity_mindist) r = gravity_mindist;
			double sr = 1;
			//gravity_power rounded up here
			if (gravity_power < 0) {
				r /= 40 * 5;
				for (int i = 0; i < -gravity_power; i += 1) sr *= r;
				o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force / sr, MAX_SPEED);
			} else {
				r = 1 - r/gravity_range;
				for (int i = 0; i < gravity_power; i += 1) sr *= r;
				o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force * sr, MAX_SPEED);
			}
		}
	}
}


void GobPlanet::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	bool roswell = false;
	for (int i = 0; i < gobgame->gobplayers; i++) {
		if (other->ship == gobgame->gobplayer[i]->ship && gobgame->gobplayer[i]->upgrade_list[UpgradeIndex::roswelldevice]->num)
			roswell = true;
	}
	if (roswell) return;
	Planet::inflict_damage(other);
}


void GobGame::preinit()
{
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
	//divine favor
	if (upgrade_list[UpgradeIndex::divinefavor]->num && (random()&1)) {
		//	if (true) {
		ship->crew = ship->crew_max;
		ship->batt = ship->batt_max;
		double angle = tw_random(PI2);
		ship->translate(unit_vector(angle) * random(1.0) * random(1.0) * 8192);
		ship->state = 1;
		ship->death_counter = -1;
		ship->collide_flag_anyone = ALL_LAYERS;
		ship->collide_flag_sameteam = ALL_LAYERS;
		game->remove(ship);
		game->add(ship->get_ship_phaser());
		ship->attributes |= ATTRIB_NOTIFY_ON_DEATH;
		ship->vel = 0;
	}
	else ship = NULL;
	return;
}


void GobGame::play_sound (SAMPLE *sample, SpaceLocation *source, int vol, int freq)
{
	STACKTRACE;
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
	/*	switch(_log->type) {
	//		case Log::log_net1server:
	//		case Log::log_net1client: {
	//			error("unsupported game/log type");
	//		}
	//		break;
			default: {
			}
			break;
		}/**/
	Game::init(_log);

	log_file(home_ini_full_path("server.ini"));
	max_enemies = get_config_int("Gob", "MaxEnemies", 32);
	int starting_starbucks, starting_buckazoids;
	starting_starbucks = get_config_int("Gob", "StartingStarbucks", 0);
	starting_buckazoids = get_config_int("Gob", "StartingBuckazoids", 0);
	gobenemy = (GobEnemy**) malloc(sizeof(GobEnemy*) * max_enemies);

	size = Vector2(24000, 24000);

	enemy_team = new_team();
	station_team = new_team();

	//	set_resolution(videosystem.width, videosystem.height);

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
	add_planet_and_station(meleedata.planetSprite, i, stationSprite[i], station_build_name[i], station_pic_name[i]);
	i = 1;
	add_planet_and_station(meleedata.planetSprite, i, stationSprite[i], station_build_name[i], station_pic_name[i]);
	i = 2;
	add_planet_and_station(meleedata.planetSprite, i, stationSprite[i], station_build_name[i], station_pic_name[i]);
	i = random() % 3;
	add_planet_and_station(meleedata.planetSprite, i, stationSprite[i], "utwju", station_pic_name[i]);

	for (i = 0; i < 42; i += 1) add(new GobAsteroid());

	int server_players, client_players;
	set_config_file("client.ini");
	server_players = client_players = get_config_int("Gob", "NumPlayers", 1);
	if (!lag_frames) client_players = 0;
	log_int(channel_server, server_players);
	log_int(channel_client, client_players);
	for (i = 0; i < server_players; i += 1) {
		char buffy[256];
		sprintf(buffy, "Config%d", i);
		add_gobplayer(create_control(channel_server, "Human", buffy));
		gobplayer[i]->new_ship(shiptype("supbl"));
		gobplayer[i]->starbucks = starting_starbucks+0;
		gobplayer[i]->buckazoids = starting_buckazoids+0;
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
		gobplayer[i]->starbucks = starting_starbucks;
		gobplayer[i]->buckazoids = starting_buckazoids;
		Ship *s = gobplayer[i]->ship;
		s->translate(size/2-s->normal_pos());
		double angle = PI2 * i / (client_players + server_players);
		s->translate(rotate(Vector2(260, 120), angle));
		s->accelerate(s, PI2/3 + angle, 0.17, MAX_SPEED);
	}

	for (i = 0; i < gobplayers+0; i += 1) add ( new RainbowRift() );

	next_add_new_enemy_time = 1000;
	this->change_view("Hero");
	//view = get_view ( "Hero", NULL );
	view_locked = true;
	view->window->locate(
		0,0,
		0,0,
		0,0.9,
		0,1
		);

	add_new_enemy();
	add_new_enemy();
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


void GobGame::add_planet_and_station ( SpaceSprite *planet_sprite, int planet_index, SpaceSprite *station_sprite, const char *builds, const char *background)
{
	STACKTRACE;

	Planet *p = new GobPlanet (size/2, planet_sprite, planet_index);
	if (num_planets) while (true) {
		SpaceLocation *n;
		n = p->nearest_planet();
		if (!n || (p->distance(n) > 1900)) break;
		p->translate(random(size));
	}
	add ( p );

	GobStation *gs = new GobStation(station_sprite, p, builds, background);
	gs->collide_flag_sameship = ALL_LAYERS;
	gs->collide_flag_sameteam = ALL_LAYERS;
	gs->collide_flag_anyone = ALL_LAYERS;
	gs->change_owner(station_team);
	add ( gs );

	gobgame->planet[gobgame->num_planets] = p;
	gobgame->station[gobgame->num_planets] = gs;
	gobgame->num_planets += 1;
}


void GobGame::fps()
{
	STACKTRACE;
	Game::fps();

	message.print(msecs_per_fps, 15, "enemies: %d", gobenemies);
	message.print(msecs_per_fps, 15, "time: %d", game_time / 1000);

	int i = 0;
	for (i = 0; i < gobplayers; i += 1) {
		if (!is_local(gobplayer[i]->channel)) continue;

		if (gobplayer[i]->ship) {
			message.print(msecs_per_fps, 15-i, "coordinates: %d x %d",
				iround(gobplayer[i]->ship->normal_pos().x),
				iround(gobplayer[i]->ship->normal_pos().y));
		}
		message.print(msecs_per_fps, 15-i, "starbucks: %d", gobplayer[i]->starbucks);
		message.print(msecs_per_fps, 15-i, "buckazoids: %d", gobplayer[i]->buckazoids);
		message.print(msecs_per_fps, 15-i, "kills: %d", gobplayer[i]->kills);
		//		message.print(msecs_per_fps, 15-i, "debug: %d", debug_value);
	}
	return;
}


double GobGame::get_max_viewable_area ( const Presence *loc ) const
{
	STACKTRACE;
	for (int i = 0; i < gobplayers; i++) {
		if (gobplayer[i]->control == loc) {
			int n = gobplayer[i]->upgrade_list[UpgradeIndex::sensor]->num;
			if (!gobplayer[i]->ship) return 65536. * 65536.;
			if (strcmp(gobplayer[i]->ship->type->id, "supbl")) n += 1;
			double area = (2048. + 512 * n) * (1536. + 384 * n);
			return area;
		}
	}
	return 65536. * 65536.;
}


void GobGame::calculate()
{
	STACKTRACE;

	if (next_add_new_enemy_time <= game_time) {
		next_add_new_enemy_time = game_time;
		if ((random() & 255) < 35) add_new_enemy();
		double e = gobenemies;
		e -= pow(game_time / (320. * 1000), 0.75);
		if (e < 0) e = 0;
		e = pow(e, 1.25);
		next_add_new_enemy_time += iround((random(1.0) + random(1.0) + 1.0) * 250 * (e + 2));
	}
	Game::calculate();
	return;
}


int GobGame::get_enemy_index(SpaceLocation *what)
{
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


static int pick_enemy_type ( int time, int current_enemies, int num_enemy_types )
{
	bool buff = false;
	double fbase = time;
	enum { REGIONS = 7 };
	double region_end_time [REGIONS-1] = {    150,  450,  900,  1500, 2100, 2700,  };
	double region_scales   [REGIONS  ] = { .020, .027, .033, .042, .055, .075, .125};
	double old_fbase = fbase;
	int which_region;
	for (which_region = 0; which_region < REGIONS; which_region++) {
		if (which_region == 0) {
			if (REGIONS == 1) fbase *= region_scales[which_region];
			else if ( old_fbase <= region_end_time[which_region]) {
				fbase += (old_fbase - 0) * (region_scales[which_region] - 1);
			}
			else if (old_fbase >= region_end_time[which_region]) {
				fbase += (region_end_time[which_region] - 0) * (region_scales[which_region] - 1);
			}
		}
		else if (which_region < REGIONS-1) {
			if ( old_fbase <= region_end_time[which_region-1] ) {
			}
			else if ( old_fbase <= region_end_time[which_region]) {
				fbase += (old_fbase - region_end_time[which_region-1]) * (region_scales[which_region] - 1);
			}
			else if ( old_fbase >= region_end_time[which_region]) {
				fbase += (region_end_time[which_region] - region_end_time[which_region-1]) * (region_scales[which_region] - 1);
			}
		} else {
			if ( old_fbase <= region_end_time[which_region-1] ) {
			} else {
				fbase += (old_fbase - region_end_time[which_region-1]) * (region_scales[which_region] - 1);
			}
		}
	}
	double threshold_enemies = 1 + 30 / (2 + time / 60) + pow(time / 320., 0.75);
	if (current_enemies > threshold_enemies)
		fbase += (current_enemies - threshold_enemies) * 1.5;
	int e = 99999;
	while (e >= num_enemy_types) {
		/*
		base	min		sec		low		high	high2	high3	high4		enemies

		1		.5		30		-1.5	2.8		4.1		5.6		7.1			16.17
		5		2.5		150		-0.44	4.9		6.9		9.3		11.97		8.57
		10		5		300		0.32	6.49	9.04	12.45	16.27		5.95
		20		10		600		1.35	8.72	12.22	17.16	23.03		4.60
		40		20		1200	2.76	11.87	16.87	24.31	33.64		4.69
		80		40		2400	4.68	16.33	23.67	35.15	50.28		5.53
		120		60		3600	6.12	19.75	29.04	49.92	64.11		7.14
		160		80		4800	7.31	22.64	33.63	51.57	76.39		8.62
		200		100		6000	8.35	25.18	37.74	58.49	87.63		10.01
		*/
		e = floor(random(fbase + 1));
		e = random() % (e + 2);

		double thresh_low   = pow(1.0*fbase,0.45) - 2.5;
		double thresh_high  = pow(2.9*fbase,0.50) + 1.0;
		double thresh_high2 = pow(3.3*fbase,0.56) + 1.8;
		double thresh_high3 = pow(4.0*fbase,0.63) + 2.6;
		double thresh_high4 = pow(4.5*fbase,0.71) + 3.4;
		if (e < thresh_low) {
			e = random() % num_enemy_types;
			if (e > fbase) e = random() % (e + 1);
		}
		if (e > thresh_high4) e = random() % (e + 1);
		if (e > thresh_high3) e = random() % (e + 1);
		if (e > thresh_high2) e = random() % (e + 1);
		if (e > thresh_high) e = random() % (e + 1);
		if (e >= num_enemy_types) {
			if ((random() & 255) < (fbase - num_enemy_types/2.0)) {
				buff = true;
				while (e >= num_enemy_types) e -= num_enemy_types;
			}
			else e = random() % num_enemy_types;
		}
		if (e < 0) e = 0;
	}
	if (buff) e += num_enemy_types;
	return e;
}


static void examine_distribution(int time, int current_enemies, int num_enemy_types)
{
	double dist2[120] = {0};
	int dist[120] = {0};
	int buffed = 0;
	if (!rand()) {
		printf("fooey!\n");
	}
	for (int i = 0; i < 1000000; i++)
		dist2[pick_enemy_type(time, current_enemies, num_enemy_types)] += 0.001;
	for (int i = 0; i < 120; i++) dist[i] = floor(dist2[i] * 100);
	for (int i = num_enemy_types; i < 120; i++) buffed += dist[i];
	if (!rand()) {
		printf("fooey!\n");
	}
}


void GobGame::add_new_enemy()
{
	STACKTRACE;
	struct EnemyType
	{
		const char *code;
		int starbucks;
		int buckazoids;
	};
	static EnemyType enemy_types[] = {
		{						 //0
			"thrto", 1, 0
		},
		{						 //1
			"zfpst", 1, 0
		},
		{						 //2
			"shosc", 1, 0
		},
		{						 //3
			"ilwsp", 1, 0
		},
		{						 //4
			"dragr", 1, 1
		},
		{						 //5
			"kahbo", 2, 0
		},
		{						 //6
			"ktesa", 2, 0
		},
		{						 //7
			"syrpe", 2, 0
		},
		{						 //8
			"kzedr", 2, 1
		},
		{						 //9
			"mmrxf", 2, 0
		},
		/*		{						 //10
					"lk_sa", 2, 0
				},
		*/
		{						 //11
			"druma", 2, 1
		},
		{						 //12
			"earcr", 3, 1
		},
		{						 //13
			"virli", 3, 1
		},
		{						 //14
			"yehte", 3, 2
		},
		/*		{						 //15
					"herex", 3, 2
				},
		*/
		{						 //16
			"narlu", 4, 2
		},
		{						 //17
			"vuxin", 3, 1
		},
		{						 //18
			"arisk", 3, 2
		},
		{						 //19
			"chmav", 4, 2
		},
		/*		{						 //20
					"plopl", 4, 2
				},
		*/
		{						 //21
			"alabc", 4, 3
		},
	};
	const int num_enemy_types = sizeof(enemy_types) / sizeof(enemy_types[0]);
	/*	examine_distribution( 30, 0, num_enemy_types);
		examine_distribution( 30, 1, num_enemy_types);
		examine_distribution( 30, 2, num_enemy_types);
		examine_distribution( 30, 3, num_enemy_types);
		examine_distribution( 150, 0, num_enemy_types);
		examine_distribution( 150, 2, num_enemy_types);
		examine_distribution( 150, 4, num_enemy_types);
		examine_distribution( 150, 6, num_enemy_types);
		examine_distribution( 300, 0, num_enemy_types);
		examine_distribution( 300, 3, num_enemy_types);
		examine_distribution( 300, 6, num_enemy_types);
		examine_distribution( 300, 9, num_enemy_types);
		examine_distribution( 600, 0, num_enemy_types);
		examine_distribution( 600, 4, num_enemy_types);
		examine_distribution( 600, 8, num_enemy_types);
		examine_distribution( 600, 12, num_enemy_types);
		examine_distribution( 1200, 0, num_enemy_types);
		examine_distribution( 2400, 0, num_enemy_types);
		examine_distribution( 3600, 0, num_enemy_types);
		examine_distribution( 4800, 0, num_enemy_types);
		examine_distribution( 6000, 0, num_enemy_types);//*/
	/*	const int num_enemy_types = 10;
		static char *enemy_types[num_enemy_types] = {
			"thrto", "zfpst", "shosc",
			"syrpe", "druma", "mmrxf",
			"kzedr", "earcr", "chmav",
			"yehte"
			};*/
	if (gobenemies == max_enemies) return;
	GobEnemy *ge = new GobEnemy();
	gobenemy[gobenemies] = ge;
	gobenemies += 1;
	int e = pick_enemy_type(game_time/1000, gobenemies, num_enemy_types);
	bool buff = e >= num_enemy_types;
	if (buff) e %= num_enemy_types;
	Ship *ship = create_ship(channel_server, enemy_types[e].code, "WussieBot", random(size), random(PI2), enemy_team);
	if (!strcmp(enemy_types[e].code, "shosc")) {
		if (random() & 3) {
			((ShofixtiScout*)ship)->special_drain = 999;
		}
		if (!buff) {
			((ShofixtiScout*)ship)->specialDamage /= 3;
		} else {
			((ShofixtiScout*)ship)->weaponDamage += 1;
			((ShofixtiScout*)ship)->weaponArmour += 1;
			((ShofixtiScout*)ship)->weaponRange *= 2;
		}
	}
	if (!strcmp(enemy_types[e].code, "zfpst")) {
		if (!buff) {
			((ZoqFotPikStinger*)ship)->specialDamage /= 2;
			if (random() & 1) ((ZoqFotPikStinger*)ship)->special_drain = 999;
		} else {
			((ZoqFotPikStinger*)ship)->weaponDamage += 1;
			((ZoqFotPikStinger*)ship)->weaponArmour += 1;
			((ZoqFotPikStinger*)ship)->weaponRange *= 2.5;
		}
	}
	if (!strcmp(enemy_types[e].code, "syrpe")) {
		if (!buff) ((SyreenPenetrator*)ship)->specialDamage /= 1.5;
		else {
			((SyreenPenetrator*)ship)->weaponDamage += 2;
			((SyreenPenetrator*)ship)->weaponArmour += 2;
			((SyreenPenetrator*)ship)->weaponRange *= 1.5;
			((SyreenPenetrator*)ship)->specialRange *= 1.5;
		}
	}
	if (!strcmp(enemy_types[e].code, "dragr")) {
		ship->special_rate *= 4;
	}
	if (!strcmp(enemy_types[e].code, "druma")) {
		if (buff) {
			((DruugeMauler*)ship)->weaponRange *= 1.6;
			((DruugeMauler*)ship)->weaponArmour += 3;
			((DruugeMauler*)ship)->weaponVelocity *= 1.1;
			((DruugeMauler*)ship)->recharge_rate /= 3;
		}
		else ((DruugeMauler*)ship)->recharge_rate *= 3;
	}
	if (!strcmp(enemy_types[e].code, "kzedr")) {
		if (buff) {
			if (random() & 3) {
				((KzerZaDreadnought*)ship)->weaponRange *= 2;
				((KzerZaDreadnought*)ship)->weaponVelocity *= 1.3;
				((KzerZaDreadnought*)ship)->weaponArmour += 3;
				ship->special_rate *= 3;
			} else {
				((KzerZaDreadnought*)ship)->specialVelocity *= 3;
				((KzerZaDreadnought*)ship)->specialRange *= 3;
				((KzerZaDreadnought*)ship)->specialLaserRange *= 1.25;
			}
		}
		ship->special_rate *= 3;
	}
	if (!strcmp(enemy_types[e].code, "earcr")) {
		if (buff) {
			((EarthlingCruiser*)ship)->weaponArmour += 3;
			((EarthlingCruiser*)ship)->weaponVelocity *= 1.3;
		}
	}
	if (!strcmp(enemy_types[e].code, "chmav")) {
		if (buff) {
			((ChmmrAvatar*)ship)->specialForce *= 2.2;
			((ChmmrAvatar*)ship)->specialRange *= 2.4;
			((ChmmrAvatar*)ship)->weaponRange *= 1.25;
			((ChmmrAvatar*)ship)->extraDamage *= 2;
			((ChmmrAvatar*)ship)->extraRange  *= 1.25;
			((ChmmrAvatar*)ship)->extraArmour *= 10;
		} else {
			((ChmmrAvatar*)ship)->weaponDamage += 1;
			((ChmmrAvatar*)ship)->weaponDamage /= 2;
			((ChmmrAvatar*)ship)->specialForce *= 1.8;
			((ChmmrAvatar*)ship)->specialRange *= 1.6;
		}
	}
	if (buff) {
		ship->crew_max *= 2.0;
		ship->crew *= 2.0;
		ship->crew_max += 6;
		ship->crew += 6;
		ship->speed_max  *= 1.5;
		ship->accel_rate *= 1.35;
		ship->turn_rate  *= 1.35;
		RepairSystem *rs = new RepairSystem ( ship );
		rs->rate = 0.025 * (tw_random(2.0) * tw_random(2.0) * tw_random(2.0) + 0.1);
		rs->efficiency = 0.5 + tw_random(0.5);
		rs->reset();
		game->add(rs);
	}
	ge->init(ship, enemy_types[e].starbucks * (buff ? 2 : 1), enemy_types[e].buckazoids * (buff ? 2 : 1));
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
	STACKTRACE;
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


int GobPlayer::charge (const char *name, int price_starbucks, int price_buckazoids)
{
	STACKTRACE;
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
	if (!strcmp(type->id, "andgu")) ((AndrosynthGuardian*)ship)->weapon_rate *= 8;

	/*load_ship_data(type);
	char buffy[256];
	sprintf (buffy, "./ships/%s.ini", type->id);
	gobgame->log_file(buffy);
	ship = type->getShip(x, y, a, random());
	game->add_target(ship);

	control->select_ship(ship, type->id);*/

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
	} else {
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
	/*	game->panel[index] = new ShipPanel(ship);
		game->panel[index]->locate(screen_x - PANEL_WIDTH, 0);
		game->control[index]->select_ship(ship, "Captain Kablooey");*/
	//	for (i = gobgame->first_gob_enemy; i < gobgame->first_gob_enemy + gobgame->max_gob_enemies; i += 1) {
	//		if (gobgame->control[i]) gobgame->control[i]->add_target(ship);
	//		}
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
	STACKTRACE;
	last_activate_time = game->game_time - 90000;
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
	{ d_button_proc,     385,  50,   150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Depart Station" , NULL, NULL },
	{ d_button_proc,     385,  90,   150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Upgrade Ship" , NULL, NULL },
	{ d_button_proc,     385,  130,  150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Buy New Ship" , NULL, NULL },
	{ d_button_proc,     385,  170,  150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Repair Ship" , NULL, NULL },
	{ d_text_proc,       185,  420,  270,  30,   255,  0,    0,    0,          0,    0,    dialog_string[0], NULL, NULL },
	{ d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
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
		if (r == STATION_DIALOG_DEPART || r == -1) break;
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
	if (((game->game_time - last_activate_time) & 0x7fffffff) < 250) return;

	last_activate_time = game->game_time;
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
	{ d_button_proc,     10,  415,  170,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Station menu" , NULL, NULL },
	{ d_textbox_proc,    20,  40,   250,  40,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Upgrade Menu", NULL, NULL },
	{ d_text_proc,       10,  100,  540,  20,   255,  0,    0,    D_EXIT,     0,    0,    (void *)" # Starbucks Buckazoids Description                     ", NULL, NULL },
	{ d_list_proc,       10,  120,  540,  280,  255,  0,    0,    D_EXIT,     0,    0,    (void *) upgradeListboxGetter, NULL, NULL },
	{ d_text_proc,       185, 420,  270,  30,   255,  0,    0,    0,          0,    0,    dialog_string[0], NULL, NULL },
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


RepairSystem::RepairSystem ( Ship *ship)
: SpaceLocation ( ship, ship->normal_pos(), ship->get_angle() )
{
	STACKTRACE;
	efficiency = 1.0;
	rate = 1.0;
	reset();
}


void RepairSystem::reset ( )
{
	STACKTRACE;
	accum = 0;
	accum2 = 0;
	old_crew = ship->crew;
}


void RepairSystem::calculate ( )
{
	STACKTRACE;
	double c = ship->crew;
	if (c < old_crew) accum += (old_crew - c) * efficiency;
	old_crew = c;
	accum2 += sqrt(accum * rate) * (frame_time * 0.001);
	if (accum2 >= 1) {
		if (accum < 1) {
			reset();
			return;
		}
		double delta = floor(accum2);
		accum2 -= delta;
		accum -= delta;
		double f = c + delta;
		double m = ship->crew_max;
		if (f > m) f = m;
		ship->crew = f;
	}
}


GobDefender::GobDefender ( Ship *ship)
: SpaceObject (ship, ship->normal_pos(), 0, gobgame->defenderSprite)
{
	STACKTRACE;
	base_phase = 0;
	next_shoot_time = 0;
	collide_flag_anyone = 0;
	advanced = 0;
}


void GobDefender::animate(Frame *space)
{
	STACKTRACE;
	double s = advanced ? 0.6 : 0.35;
	sprite->animate(pos, sprite_index, space, s);
	//	enum { PULSE_TIME = 300 };
	//	double s = 0.4 + 0.025 * cos ( (game->game_time % PULSE_TIME) * PI2 / PULSE_TIME );
	//	sprite->animate_character(pos, sprite_index, pallete_color[1], space, s);
}


void GobDefender::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();
	if (!ship) {
		die();
		return;
	}
	if (!(random(3))) {
		if (next_shoot_time < gobgame->game_time) {
			SpaceObject *target = NULL;
			Query q;
			if (advanced)
				q.begin(this, OBJECT_LAYERS, 330 );
			else
				q.begin(this, OBJECT_LAYERS &~ bit(LAYER_SHIPS), 290 );
			while (q.currento && !target) {
				if (!q.currento->sameTeam(ship) && (q.currento->get_team() != gobgame->station_team) && !q.currento->isPlanet()) {
					SpaceLine *l = new PointLaser (
						this, palette_color[7], 2 + advanced, 40,
						this, q.currento
						);
					add(l);
					if (l->exists()) target = q.currento;
				}
				q.next();
			}
			q.end();
			if (target) {
				if (advanced)
					next_shoot_time = gobgame->game_time + 360;
				else
					next_shoot_time = gobgame->game_time + 560;
			}
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
	STACKTRACE;
	spawn_counter = random(90000);
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
	times_found = 0;
}


void RainbowRift::animate( Frame *frame )
{
	STACKTRACE;
	if (spawn_counter > 0) return;
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
	if (spawn_counter > 0) {
		spawn_counter -= frame_time;
		return;
	}
	while (game->game_time > next_time2) {
		STACKTRACE;
		next_time2 += random() % 3000;
		Query q;
		for (q.begin(this, bit(LAYER_SHIPS), 48); q.current; q.next()) {
			GobPlayer *p = gobgame->get_player(q.currento);
			if (!p) continue;
			if (q.currento == p->ship) {
				STACKTRACE;
				int i = 0;
				gobgame->pause();
				i = p->control->choose_ship(game->window, "You found the Rainbow Rift!\n(select your current ship type to hunt for resources instead of a new ship)", reference_fleet);
				game->log_int(p->channel, i);
				if (i == -1) i = random(reference_fleet->getSize());
				if (reference_fleet->getShipType(i) == p->ship->type) {
					times_found += 1;
					p->starbucks += random((times_found+2) * 9);
					p->buckazoids += random((times_found+2) * 6);
					pos = random(map_size);
					//game->add(new RainbowRift());
				}
				else {
					p->starbucks += random() % (1+p->value_starbucks);
					p->buckazoids += random() % (1+p->value_buckazoids);
					p->new_ship(reference_fleet->getShipType(i));
					pos = random(map_size);
					gobgame->station[0]->station_screen(p);
				}
				gobgame->unpause();
				game->redraw();
				spawn_counter = random(90000 + times_found * 15000);
			}
		}
		Planet *planet = nearest_planet();
		while (planet && distance(planet) < planet->gravity_range) {
			pos = random(map_size);
			planet = nearest_planet();
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
