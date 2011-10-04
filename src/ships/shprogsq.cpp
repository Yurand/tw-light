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

#include "ship.h"
REGISTER_FILE

#include "util/aastr.h"

#include <string.h>
#include <stdio.h>

#include "melee/mview.h"

/*
rogsc

Rogue Squadron

  to-do:
	// check if an enemy targets the virtual ship, or a dead fighter:

*/

class PulseLaser;
class RogueFighter;

class RogueSquadron : public Ship
{
	public:

		class Formation_cl
		{
			public:
				Vector2 pos;
		} formationinfo[10*20];	 // max 10 formations and 20 crew

		double  weaponRange, weaponLength;
		int     weaponDamage;
		int     weaponColor;
		double  weaponVelocity;

		double  fighterCrew, fighterVel, fighterVelMax, fighterVelMin, fighterDamage;
		int     fighterAsteroidsKill;
		double  fighterAvoidanceRange, fighterEvadeRotationPerSec;

		int     formation, max_formations;
		int     fire_main;
		int     ileader;
		int     regroup;

		Vector2 player_pos;
		double  player_angle;

		RogueFighter    *fighter[100];

		RogueSquadron(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void animate(Frame *space);
		virtual int handle_damage(SpaceLocation* source, double normal, double direct = 0);
		virtual void materialize();
		virtual void calculate_hotspots();

};

class RogueFighter : public Ship
{
	public:

		RogueSquadron   *mother;
		Vector2         idealpos, idealrelpos;
		double speed_max_default, turn_rate_default;

		RogueFighter(RogueSquadron *creator, SpaceSprite *osprite);

		virtual void calculate();
		virtual int  handle_damage(SpaceLocation* source, double normal, double direct = 0);

		SpaceObject* nearest_location();
		void avoid_location(SpaceObject *s);
};

class PulseLaser : public SpaceLine
{
	protected:
		double frame;
		double frame_count;

		SpaceLocation *lpos;
		Vector2 rel_pos;

	public:
		PulseLaser(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage, int lfcount,
			SpaceLocation *opos, Vector2 rpos = Vector2(0,0), double rvelocity = 0.0);

		void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

int skipitems(char *str, int N)
{
	STACKTRACE;
	int i, nfound;

	// return the start of the N-th item
	i = 0;
								 // skip heading spaces
	while (str[i] == ' ' && str[i] != 0)
		++i;

	nfound = 0;
	while ( nfound < N && str[i] != 0 ) {
		++i;
								 // find space-item transitions
		if ( str[i] != ' ' && str[i-1] == ' ' )
			++nfound;
	}

	return i;
}


RogueSquadron::RogueSquadron(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponColor    = tw_get_config_int("Weapon", "Color", 0);
	weaponLength   = tw_get_config_float("Weapon", "Length", 0);

	fighterCrew     = tw_get_config_float("Fighter", "Crew", 0);
	fighterVel      = scale_velocity(tw_get_config_float("Fighter", "Velocity", 0));
	fighterVelMax   = scale_velocity(tw_get_config_float("Fighter", "VelocityMax", 0));
	fighterVelMin   = scale_velocity(tw_get_config_float("Fighter", "VelocityMin", 0));
	fighterDamage   = tw_get_config_float("Fighter", "SuicideDamage", 1);

	fighterAsteroidsKill = tw_get_config_int("Fighter", "AsteroidsKill", 0);
								 // 0 = disable, 1 = enable
	fighterAvoidanceRange = tw_get_config_int("Fighter", "AvoidanceRange", 0);
								 // in degrees
	fighterEvadeRotationPerSec = tw_get_config_float("Fighter", "EvadeRotationPerSec", 0);

	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;
	mass = 0;

	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);

	// read formation info:

	max_formations = tw_get_config_int("Formations", "Nformations", 0);

	int i, j;

	for ( j = 0; j < max_formations; ++j ) {
		char strdata[1024];

		char idstring[128]={0};
		sprintf(idstring, "Formation%02i", j+1);

		// char **get_config_argv(const char *section, const char *name, int *argc);
		strcpy(strdata, tw_get_config_string("Formations", idstring, 0));

		//		fprintf(outfile, "%s\n", strdata);

		for ( i = 0; i < crew_max; ++i ) {
			int k = iround(j*crew_max + i);
			int ires;
			ires = sscanf(&strdata[skipitems(strdata,i*2)], "%lf", &formationinfo[k].pos.x);
			if ( ires == 0 ) break;
			ires = sscanf(&strdata[skipitems(strdata,i*2+1)], "%lf", &formationinfo[k].pos.y);
			if ( ires == 0 ) break;

			//			fprintf(outfile, "%8.2f %8.2f\n", formationinfo[k].pos.x, formationinfo[k].pos.y);
		}
		if ( i != crew_max )
			{tw_error("mismatch between number of max crew and formation locations");}

	}

	//	fclose(outfile);

}


// This is called just before the first calculate(), after the ship phases into battle
void RogueSquadron::materialize()
{
	STACKTRACE;
	int i;

	formation = 0;

	player_pos = pos;
	player_angle = angle;

	for ( i = 0; i < crew_max; ++i ) {
		fighter[i] = new RogueFighter(this, data->spriteWeapon);

		int k;
		k = iround(formation*crew_max + i);

		fighter[i]->idealrelpos = rotate(formationinfo[k].pos, player_angle-PI/2);

		fighter[i]->idealpos = player_pos + fighter[i]->idealrelpos;

		game->add(fighter[i]);
		targets->add(fighter[i]);// make this targetable (isn't done by default?)

		fighter[i]->pos = fighter[i]->idealpos;
		fighter[i]->angle = player_angle;
		fighter[i]->vel = 0;
	}

	fire_main = 0;

	ileader = 0;				 // number of the leader ship ... CHANGE: not needed really
	regroup = 0;

	// remove the virtual ship from the target list:
	// HOW ??

	// copied from tauhu code.
	//	for(i=0; game->target[i] != this; i++);
	//	game->num_targets--;
	//	game->target[i] = game->target[game->num_targets];
	targets->rem(this);

	// not used in queries
	attributes |= ATTRIB_UNDETECTABLE;
}


int RogueSquadron::activate_weapon()
{
	STACKTRACE;
	// all ships fire a pulse laser:
	fire_main = 1;
	return TRUE;
}


int RogueSquadron::activate_special()
{
	STACKTRACE;
	int i;

	// first of all, clean up the old formation.
	if ( regroup ) {
		for ( i = 0; i < crew_max; ++i ) {
			int j, k;
			if ( !fighter[i] ) { // see if you can replace this 0 entry

				// check if there are non-zero ships left:
								 // item i should be removed, higher items shift down
				for ( k = i+1; k < crew_max; ++k )
					if (fighter[k])
						break;

				if ( k == crew_max )
					break;		 // stop, there's nothing anymore

				k -= i;			 // make it relative
								 // item i should be removed, higher items shift down
				for ( j = i; j < crew_max-k; ++j ) {
								 // note, j > 1
					fighter[j] = fighter[j+k];
								 // after moving it, reset it.
					fighter[j+k] = 0;
				}

			}

		}

		ileader = 0;
		regroup = 0;
	} else {
		++ formation;
		if ( formation > max_formations-1 )
			formation = 0;
	}

	return TRUE;
}


void RogueSquadron::calculate()
{
	STACKTRACE;
	fire_main = 0;				 // reset this first.

	int i;

	// find a new leader, if needed
	i = 0;
	while ( i < crew_max && !(fighter[i] && fighter[i]->exists()) )
		++i;

	ileader = i;

	if ( ileader == crew_max ) {
		state = 0;
		return;
	}

	// check if fighters have died ...
	for ( i = 0; i < crew_max; ++i ) {
		if ( !fighter[i] )
			continue;

								 // check if this ship died
		if ( !fighter[i]->exists() ) {
			--crew;
			update_panel = 1;
			fighter[i] = 0;
			regroup = 1;
		}
	}

	if ( crew <= 1E-3 ) {		 // all fighters have gone :(
		state = 0;
		return;
	}

	double scale_speed;
	//	scale_speed = fighter[ileader]->speed_max / fighter[ileader]->speed_max_default;
	scale_speed = 1.0;			 // the grid always moves as fast as possible.

	//	turn_rate = fighter[ileader]->turn_rate;

	this->accel_rate = 0;		 // normally, no thrust
	if ( thrust )
		vel = fighterVel * unit_vector(angle) * scale_speed;
	else
		vel = fighterVelMin * unit_vector(angle) * scale_speed;

	Ship::calculate();

	player_pos = pos;
	player_angle = angle;

	// and thrust is always maximum

	// define ideal positions for the ships in your squadron, based on the
	// formation pattern: but, well, best is to do that still from position 0 ;)

	for ( i = 0; i < crew_max; ++i ) {
		if ( !fighter[i] )
			continue;

		int k;
		k = iround(formation*crew_max + i);

		// slow relaxed deformation of the ideal positions, to match new
		// positions relative to the main ship (mrel=0). Althought the grid
		// is slowly updated, it always has a fixed position around the leader.

		double a;
		a = frame_time*1E-3;

		Vector2 newrelpos;

		// the grid is relative to position (0,0):
		newrelpos = rotate(formationinfo[k].pos, player_angle-PI/2);

		fighter[i]->idealrelpos = (1-a) * fighter[i]->idealrelpos + a * newrelpos;

		fighter[i]->idealpos = player_pos + fighter[i]->idealrelpos;

	}

	// check if an enemy targets the virtual ship, or a dead fighter:
	// (is still to do)

	SpaceObject *o;
	Query a;
	for (a.begin(this, bit(LAYER_SHIPS), 999999.0); a.current; a.next()) {
		o = a.currento;
		if (o->target == this ) {
			for ( i = 0; i < crew_max; ++i ) {
				if ( !fighter[i] )
					continue;

				o->target = fighter[i];
				break;
			}

		}

	}

}


void RogueSquadron::animate(Frame *space)
{
	STACKTRACE;
	int a;
	a = aa_get_trans();
	aa_set_trans(100);
	Ship::animate(space);
	aa_set_trans(a);
	return;
}


int RogueSquadron::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	// the Syreen, and maybe other area-effect weapons, can do damage to
	// the virtual ship, which I don't want. So, I'll disable all damage
	// taking by this ship

	return 0;
}


void RogueSquadron::calculate_hotspots()
{
	STACKTRACE;
	// do nothing: no hotspots for this virtual thing
}


RogueFighter::RogueFighter(RogueSquadron *creator, SpaceSprite *osprite)
:
Ship(creator, creator->pos, creator->angle, osprite)
{
	STACKTRACE;
	mother = creator;
	pos = mother->pos;

	// settings for this ship are ...
	crew = mother->fighterCrew;
	crew_max = crew;

	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS);

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = 0;

	mass = 4.0;

	// remember this to detect changes applied by other ships ...
	//	recharge_rate_default = recharge_rate;
	speed_max = mother->fighterVelMax;
	turn_rate = creator->turn_rate;
	speed_max_default = speed_max;
	turn_rate_default = turn_rate;

	hotspot_rate = mother->hotspot_rate;
	hotspot_frame = 0;
}


SpaceObject* RogueFighter::nearest_location()
{
	STACKTRACE;
	SpaceObject *p = NULL;
	double r = 99999999;
	Query q;
	q.begin(this, ALL_LAYERS, 1600);
	while (q.current) {
		if ((q.current->isObject() || q.current->isAsteroid() ||  q.current->isShot())
		&& !this->sameShip(q.current)) {
			double t = distance(q.current);
			if (t < r) {
				r = t;
				p = (SpaceObject *) q.current;
			}
		}
		q.next();
	}
	return p;
}


// copy some stuff from Orz-marines:
// should be added to SpaceLocation I think !
void RogueFighter::avoid_location(SpaceObject *o)
{
	STACKTRACE;
	double t_a = trajectory_angle(o);
	double d_a = normalize(t_a - angle, PI2);

	if (d_a > PI)   d_a -= PI2;

	//	if (fabs(d_a)<PI/2)
	//	{
	double dt = frame_time * 1E-3;

	double da_max = mother->fighterEvadeRotationPerSec * ANGLE_RATIO * dt;

	if (d_a > 0)
		angle -= da_max;		 // normalize(t_a - PI/2, PI2);
	else
		angle += da_max;		 // normalize(t_a + PI/2, PI2);
	//	}
}


void RogueFighter::calculate()
{
	STACKTRACE;
	if ( !(mother && mother->exists()) ) {
		state = 0;
		mother = 0;
		return;
	}

	double dt;
	dt = frame_time*1E-3;

	double scale_angle, scale_speed;
	scale_speed = speed_max / speed_max_default;
	scale_angle = turn_rate / turn_rate_default;

	// specials can change the angle, recharge, and other behaviour ...
	// what to do in those cases ?

	// position the ship according to the required formation pattern.

	//	if ( idealrelpos != 0 )	// 0 applies to the leader
	//	{
	// CHANGED: there is no leader, the player controls the grid directly.
	Vector2 D;
	D = min_delta(idealpos - pos, map_size);

	double a, R;
	a = D.atan();
	R = D.magnitude();

	double da, damax;

	da = a - angle;
	while (da >  PI)    da -= PI2;
	while (da < -PI)    da += PI2;

								 // movement per second.
	damax = 1.0*PI2 * dt * scale_angle;
	if ( da >  damax)   da =  damax;
	if ( da < -damax)   da = -damax;

	angle += da;

	double v;
	v = R / frame_time;
								 // leader travels at 0.5; so they can catch up
	if ( v > mother->fighterVelMax )
		v = mother->fighterVelMax;

	vel = v * unit_vector(angle) * scale_speed;
	//	}

	// avoid collision ?!
	if ( mother->fighterAvoidanceRange > 0 ) {
		SpaceObject *o;
		o = nearest_location();	 // with short range
		if ( o && distance(o) < mother->fighterAvoidanceRange ) {
			//			{tw_error("wow, found something!");}
			avoid_location(o);
		}
	}

	// not ship?! Well, that crashes :(
	SpaceObject::calculate();

	sprite_index = get_index(angle);

	// if you get a shoot-order:
	if ( mother->fire_main )
		game->add( new PulseLaser(this, 0.0, mother->weaponColor,
			mother->weaponLength, mother->weaponDamage,
			iround(mother->weaponRange), this, Vector2(0.0, get_size().y/2),
			mother->weaponVelocity) );

	if ( mother->thrust ) {
		thrust = TRUE;
	}
	Ship::calculate_hotspots();
	thrust = FALSE;

}


int RogueFighter::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	if (!state)
		return 0;				 // to avoid an infinite feedback with enemy rogue.

	int total = iround(normal + direct);

	if (source->sameTeam(this))
		return 0;				 // do nothing if it's friendly ones

	// note, you've got damage = either by collision, or some special thing
	// handle the damage in the normal way, if there's no actual collision
	if ( !(source->canCollide(this)) )
								 // handle the special damage
		Ship::handle_damage(source, normal, direct);
	else {
								 // handle the damage
		Ship::handle_damage(source, normal, direct);

		if ( crew <= 0 ||
			(source->isAsteroid() && mother->fighterAsteroidsKill) ||
		source->isShip() ) {
			state = 0;
			// it also inflicts some damage on it's target :)
			damage(source, mother->fighterDamage, 0);
		}
	}

	if ( !state ) {
		// this ship dies :(
		play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
		game->add(new Animation(this, pos, meleedata.kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
	}

	return total;
}


PulseLaser::PulseLaser(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage,
int lfcount, SpaceLocation *opos, Vector2 rpos, double rvelocity)
:
SpaceLine(creator, opos->normal_pos(), opos->angle+langle, lrange, lcolor),
frame(0),
frame_count(lfcount),
lpos(opos),
rel_pos(rpos)
{
	STACKTRACE;

	// angle conventions fucked up??
	rel_pos.x *= -1;
	pos = normalize(pos + rotate(rel_pos, -PI/2+opos->get_angle()));

	id |= SPACE_LASER;
	damage_factor = ldamage;

	vel = /*lpos->get_vel() +*/ rvelocity * unit_vector(angle);

	if (!(lpos && lpos->exists())) {
		lpos = 0;
		state = 0;
	}
}


void PulseLaser::calculate()
{
	STACKTRACE;
	if ((frame < frame_count) && (lpos->exists())) {
		//		pos = lpos->normal_pos() + rotate(rel_pos, lpos->get_angle() - PI/2);
		//		vel = lpos->get_vel();
		SpaceLine::calculate();
		frame += frame_time;
	}
	else state = 0;
	return;
}


void PulseLaser::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	// copied from space_line:
	int i;
	if (damage_factor >= 0) {
		i = iround_down(damage_factor / 2);
		if (i >= BOOM_SAMPLES)
			i = BOOM_SAMPLES - 1;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
		damage(other, damage_factor);
	}
	state = 0;					 // this is different from space_line :)
	return;
}


REGISTER_SHIP(RogueSquadron)
