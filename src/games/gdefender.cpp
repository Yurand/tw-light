
#include <stdio.h>				 //standard io libary (needed for sprintf)

#include "melee.h"				 //used by all TW source files.  well, mostly.
#include "melee/mgame.h"		 //Game stuff
#include "melee/mcbodies.h"		 //asteroids & planets
#include "melee/mship.h"		 //ships
#include "melee/mshot.h"		 //shots, missiles, lasers
#include "melee/mlog.h"			 //networking / demo recording / demo playback
#include "melee/mcontrol.h"		 //controllers & AIs
#include "melee/mview.h"		 //Views & messages
#include "melee/mshppan.h"		 //ship panels...

#include "melee/mitems.h"		 //ship panels...
#include "melee/manim.h"		 //ship panels...
#include "scp.h"
#include "other/dialogs.h"

// Defender of the Starbase

/// define the starbase
class DefenderStation : public Orbiter
{
	public:
		double health, maxhealth;///< current and maximum starbase health
								 ///< rate & phase of ship healing
		int healtime, nexthealtime;
								 ///< rate & phase of self healing
		int regentime, nextregentime;
		/// handle_damage lets the starbase be hurt & die
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
		/// calculate does the continuous things like regeneration
		virtual void calculate ();
		//the constructor.  all game items must have constructors
		//this one just sets up values for health, etc.
		DefenderStation ( SpaceSprite *sprite, SpaceLocation *orbit );
};

/// DefenderGame
///
/// Idea:
///
/// defend a starbase
///
/// Current status:
///
/// Seems to be working, but needs more work perhaps I should add a
/// high-score table?
class DefenderGame : public Game
{
	public:
		TeamCode player_team, enemy_team;
								 ///< this holds the graphics for the space station
		SpaceSprite *stationsprite;

		///initialize all pointers to NULL, so that the destructor won't
		///crash if it's called early for some reason
		virtual void preinit ();

		virtual ~DefenderGame();

		virtual void init ( Log *_log ) ;

		//begin the game
		void restart();

		/// here we check to see if the player lost the game and we add new enemies if it's time for that
		virtual void calculate ( ) ;
		/// this is used to display the starbases current health
		virtual void fps();

		/// a pointer at the player
		Control *player;

		/// a pointer at the starbase
		DefenderStation *starbase;

		/// rate of enemy ships spawning
		int time_for_next_attack;
		/// phase of enemy ships spawning
		int time_between_attacks;

};

#define defendergame ((DefenderGame*)game)

int DefenderStation::handle_damage ( SpaceLocation *source, double normal, double direct)
{
	double old = health;
	health -= normal;
	health -= direct;
	if (health <= 0) {
		add ( new Animation ( this, pos, meleedata.kaboomSprite, 0,
			meleedata.kaboomSprite->frames(), 50, DEPTH_EXPLOSIONS) );
		state = 0;
	}
	return iround(old-health);
}


DefenderStation::DefenderStation( SpaceSprite *sprite, SpaceLocation *orbit)
: Orbiter ( sprite, orbit, 600 )
{
	STACKTRACE;
	health = maxhealth = 30;
	regentime = 6000;
	nextregentime = 0;
	healtime = 2000;
	nexthealtime = 0;
}


void DefenderStation::calculate ( )
{
	Orbiter::calculate();
	if ( (health < maxhealth) && (nextregentime <= game->game_time) ) {
		health += 1;
		nextregentime = game->game_time + regentime;
	}
	Ship *ship = defendergame->player->ship;
	if ( ship && (distance(ship) < 300) && (nexthealtime <= game->game_time) &&
	(ship->crew < ship->crew_max) && (magnitude_sqr(ship->get_vel() - vel) < 0.01) ) {
		ship->handle_damage(this, -1);
		nexthealtime = game->game_time + healtime;
		SpaceLocation *l = new PointLaser ( this, palette_color[10], 0, 150, this, ship, 0);
		game->add ( l );
		l->set_depth(DEPTH_SHIPS + 0.2);
	}
	accelerate(this, trajectory_angle(center), 0.00005, MAX_SPEED);
	return;
}


static int num_ships = 4;
static const char *someships[] = { "thrto", "supbl", "syrpe", "shosc", NULL };

void DefenderGame::calculate ( )
{
	Game::calculate();
	static bool first = true;
	if ((!starbase || !starbase->exists())&& first) {
		first = false;
		starbase = NULL;
		char buffy[1024];
		sprintf(buffy, "You lost after %d seconds", game_time/1000);
		message.print(9999000, 15, buffy);
	}
	else if (game_time >= time_for_next_attack) {
		time_for_next_attack += time_between_attacks + (random(10000)) - 4000;
		time_between_attacks -= 10;
		time_between_attacks = iround(time_between_attacks * 0.975);
		time_between_attacks += 10;
		SpaceObject *whatever = create_ship ( channel_none, someships[random(num_ships)], "WussieBot", Vector2(0, 2000), 0, enemy_team);
		add (whatever);
	}
	return;
}


void DefenderGame::preinit()
{
	STACKTRACE;
	Game::preinit();
	//because the desctructor deals with stationsprite, we have to initialize it here
	//just in case the normal init() function doesn't get called
	stationsprite = NULL;
}


DefenderGame::~DefenderGame()
{
	//we have to manually unload space station graphics since they aren't part of a ship
	if (stationsprite)
		delete stationsprite;
}


void DefenderGame::fps()
{
	STACKTRACE;
	int s = 0;
	if (starbase)
		s = iround(starbase->health);
	message.print((int)msecs_per_fps, 15, "Current Time: %d", game->game_time / 1000);
	message.print((int)msecs_per_fps, 12, "Starbase Health: %d", s);
	int p = 0;
	if (player->ship)
		p = iround(player->ship->getCrew());
	message.print((int)msecs_per_fps, 12, "Your Health: %d", p);
}


void DefenderGame::restart()
{
	STACKTRACE;
	for(std::list<SpaceLocation*>::iterator i = item.begin();
	i != item.end(); i++) {
		if ((*i)->exists())
			(*i)->die();
	}
	game_time = 0;

	Ship *ship = create_ship("supbl", player, Vector2(500, 200), 270);
	add(ship);

	Planet *planet = new Planet ( 0, meleedata.planetSprite, random(3) );
	add ( planet );

	starbase =  new DefenderStation ( stationsprite, planet);
	add ( starbase );
	starbase->change_owner ( ship );
	gametargets.add(starbase);

								 // first attack in 3 seconds from now
	time_for_next_attack = game_time + 3 * 1000;
								 //24 seconds between attacks
	time_between_attacks = 24 * 1000;

	message.out("Defend the starbase!", 30000);

}


void DefenderGame::init( Log * _log)
{
	STACKTRACE;
	Game::init(_log);

	prepare();
	add ( new Stars() );

	player = create_control ( channel_server, "Human" );
	add_focus(player);

	TW_DATAFILE *tmpdata;
	tmpdata = tw_load_datafile_object(data_full_path("gob.dat").c_str(), "station0sprite");
	if (!tmpdata)
		tw_error( "couldn't find gob.dat#station0sprite");
	stationsprite = new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED, 64);
	stationsprite->permanent_phase_shift(8);
	tw_unload_datafile_object(tmpdata);

	this->change_view("Hero");
	restart();

	message.out("An enemy ship will attack it every 20-30 seconds", 20000, 7);
	message.out("The starbase will heal you if you fly close to", 20000, 7);
	message.out("  it and match its velocity", 20000, 7);
	message.out("It can also heal itself, but slowly", 20000, 7);
	return;
}


REGISTER_GAME ( DefenderGame, "Defender" )
