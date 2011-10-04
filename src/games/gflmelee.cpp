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

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "melee.h"
#include "other/radar.h"
#include "melee/mshppan.h"

REGISTER_FILE
#include "melee/mframe.h"
#include "melee/mgame.h"
#include "melee/mmain.h"
#include "melee/mcbodies.h"
#include "melee/mview.h"
#include "other/orbit.h"
#include "melee/mitems.h"
#include "util/aastr.h"
#include "melee/mcontrol.h"

#include "gflmelee.h"

#include "melee/mlog.h"

#define Num_Planet_Pics 7

void copy_sprite2bmp(SpaceSprite* src, BITMAP *bmp)
{
	STACKTRACE;

	src->lock();

	BITMAP *tmp = src->get_bitmap(0);
	aa_stretch_blit(tmp, bmp, 0, 0, tmp->w, tmp->h,
		0, 0, bmp->w, bmp->h);

	src->unlock();
}


// show the direction to some object, by showing a small sprite
// if it;s a location, show only a default thingy ?!
static int ImIndicatorSize = 40;
class ImIndicator : public Presence
{
	public:
		BITMAP            *bmp;
		SpaceLocation *showme;

		ImIndicator(SpaceLocation *o);
		~ImIndicator();
		void newtarget(SpaceLocation *o);
		int colortype(SpaceLocation *o);
		void animate(Frame *frame);
};

extern int PANEL_WIDTH;
extern int PANEL_HEIGHT;
extern int CAPTAIN_X;
extern int CAPTAIN_Y;
extern int PANEL_DEATH_FRAMES;
extern int crew_x;
extern int crew_y;
extern int batt_x;
extern int batt_y;

// most of this is copied from ZRadar.
class YRadar : public Presence
{
	public:
		BITMAP    *ship_f_icon, *ship_e_icon, *cbody_icon, *planet_icon,
			*target_icon1, *target_icon2, *backgr_bmp, *foregr_bmp;
		TeamCode  homeplayer_team;

		Vector2   location;		 // where it's drawn on the screen

		YRadar(Control *ocontroller, double Size, TeamCode hteam, char *datafilename,
			bool rel, int shape);
		~YRadar();

		// the code for painting on the radar screen is in here.
		void PaintItem(BITMAP *Slate, Vector2 T, SpaceLocation *o, double Scale);
		virtual void animate(Frame *space);

		BITMAP        *Painted;
		Control       *controller;
		double size;
		bool active;

		bool  relative_angle;	 // 0=absolute, 1=rotated in orientation direction
		int       display_shape; // 1=square, 2=round.

		//	double shiftscale(double r_center, double v_center, double scale, double n);
		Vector2 shiftscale(Vector2 r_center, Vector2 v_center, double scale, Vector2 n);

		void initbmp(char *datafilename);

		void setTarget(SpaceLocation *target);
		void setSize(double Size);
		void toggleActive();
};

/*! \brief function-object class for painting items */
class PaintYRadar
{
	YRadar* _radar;
	BITMAP * _slate;
	Vector2 _T;
	double _Scale;
	public:
		PaintYRadar(YRadar* radar, BITMAP* slate, Vector2 T):_radar(radar),_slate(slate),_T(T) {
			_Scale = slate->w/(2.*radar->size);
		};
		void operator()(SpaceLocation* o) {
			if ( o != _radar->controller->ship )
				_radar->PaintItem(_slate, _T, o, _Scale);
		}
};

static const int StatsMax_Nships = 128;
static const int StatsMax_Nkilled = 32;

// note, this is not part of the game, really - just accepts neutral info
class StatsManager
{
								 // at most 512 interesting different objects (ships)?
	BITMAP    *list_bmp[StatsMax_Nships];
	int       Nlist;
	unsigned int list_allyflag[StatsMax_Nships];

	public:
		int Nships;
		struct stats_str
		{
			int Nkilled;
			int Npressedfire;
			int Npressedspecial;
			// ID's that are unique to a ship and the weapons it spawns (up to SpaceLocation)
			unsigned int allyflag_owner, allyflag_killed[StatsMax_Nkilled];
		} stats[StatsMax_Nships];// at most 64 ships in the game?

		StatsManager();
		~StatsManager();
								 // add to the stats list
		void addship ( Ship *statship, int ofordisplay );
		void updatestats(SpaceLocation *killer, Ship *victim);
		void showstats(Frame *frame);
		int list_item(unsigned int flag);
};

static const int Nradarmodes = 3;// off, all, and medium

StatsManager::StatsManager()
{
	STACKTRACE;
	Nships = 0;
	// no stats yet :)

	Nlist = 0;
	// no ships have been added (or killed) yet
}


void StatsManager::addship ( Ship *statship, int ofordisplay )
{

	SpaceSprite *spr = statship->data->spriteShip;
	BITMAP *tmp = create_bitmap(40, 40);

	spr->lock();
	BITMAP *src = spr->get_bitmap(0);
	stretch_blit(src, tmp, 0, 0, src->w, src->h, 0, 0, tmp->w, tmp->h);
	spr->unlock();

	// add the bmp to the list now that we're still sure the ship exists:
	list_bmp[Nlist] = tmp;
	list_allyflag[Nlist] = statship->ally_flag;
	++Nlist;
	if ( Nlist >= StatsMax_Nships ) {
		Nlist = StatsMax_Nships-1;
	}

	// only add a stats thingy if the ships needs monitoring (eg. has weapons, belongs to a team)
	if (ofordisplay) {
		stats[Nships].Nkilled = 0;
		stats[Nships].allyflag_owner = statship->ally_flag;

		++Nships;
		if ( Nships >= StatsMax_Nships ) {
			Nships = StatsMax_Nships-1;
		}
	}
}


void StatsManager::updatestats(SpaceLocation *killer, Ship *victim)
{
	STACKTRACE;

	if ( !(killer && victim) )
		return;

	int i;
	// check the list_allyflag to see if it's an important enough target to record

	int k = 0;
	for ( i = 0; i < Nlist; ++i ) {
		if ( list_allyflag[i] == killer->ally_flag )
			++k;
		if ( list_allyflag[i] == victim->ally_flag )
			++k;
	}

	if ( k != 2 )				 // either killer or killee isn't important.
		return;

	// search the owner:
	for ( i = 0; i < Nships; ++i ) {
		if ( stats[i].allyflag_owner == killer->ally_flag ) {
			int k = stats[i].Nkilled;
			stats[i].allyflag_killed[k] = victim->ally_flag;
			++stats[i].Nkilled;
			if ( stats[i].Nkilled >= StatsMax_Nkilled ) {
				stats[i].Nkilled = StatsMax_Nkilled - 1;
			}

		}
	}
}


void StatsManager::showstats(Frame *frame)
{
	STACKTRACE;

	// well ... show the graphics of all the victims ?
	int i;

	Vector2 spos, ssize;
	double H1 = 28;
	double H2 = 30;
	ssize = Vector2 ( H1, H1 );

	BITMAP *dest;
	dest = frame->surface;

	for ( i = 0; i < Nships; ++i ) {
		BITMAP *src;

		int ilist;
		ilist = list_item(stats[i].allyflag_owner);
		src = list_bmp[ilist];

		spos = Vector2( 50, 60+i*H2 );

		aa_stretch_blit(src, dest,
			0, 0, src->w, src->h,
			spos.x, spos.y, ssize.x, ssize.y
			);
		frame->add_box(spos.x, spos.y, ssize.x, ssize.y);

		int k;
		for ( k = 0; k < stats[i].Nkilled; ++k ) {
			ilist = list_item(stats[i].allyflag_killed[k]);
			src = list_bmp[ilist];

			spos = Vector2( 110+k*H2, 60+i*H2 );

			aa_stretch_blit(src, dest,
				0, 0, src->w, src->h,
				spos.x, spos.y, ssize.x, ssize.y
				);
			frame->add_box(spos.x, spos.y, ssize.x, ssize.y);

		}

	}
}


int StatsManager::list_item(unsigned int flag)
{
	STACKTRACE;

	int i;

	for ( i = 0; i < Nlist; ++i )
		if ( list_allyflag[i] == flag )
			return i;

	return 0;					 // default value in case nothing was found.
}


StatsManager::~StatsManager()
{
	int i;

	for ( i = 0; i < Nlist; ++i ) {
		BITMAP *b = list_bmp[i];
		if (b)
			destroy_bitmap(b);
	}
}


BITMAP* copybmp( BITMAP* src )
{

	BITMAP *dest;

	// copied from vanguard
								 //Create a new bitmap
	dest = create_bitmap_ex(bitmap_color_depth(screen), src->w, src->h);
	if (!dest) return NULL;		 //If failed, return NULL

								 //Copy bitmap from datafile
	blit(src, dest, 0, 0, 0, 0, src->w, src->h);

	return dest;
}


Vector2 YRadar::shiftscale(Vector2 r_center, Vector2 v_center, double scale, Vector2 n)
{
	STACKTRACE;

	//Used to scale game coordinates onto RADAR screen coordinates
	return  scale * min_delta(n - r_center, map_size) + v_center;
}


void YRadar::PaintItem(BITMAP *Slate, Vector2 T, SpaceLocation *o, double Scale)
{
	STACKTRACE;

	Vector2 pos;

	pos = shiftscale(T, Vector2(Slate->w/2,Slate->h/2), Scale, o->pos);

	if ( display_shape == 2 )
		if ( (pos - Vector2(Slate->w/2,Slate->w/2)).magnitude() > Slate->w/2 - 6)
			return;

	if ( relative_angle ) {
		Ship *s = controller->ship;
		if (s)
			rotate(pos, -s->angle );
	}

	BITMAP *bmp = 0;

	if (o->isShip()) {
		if ( o->get_team() == homeplayer_team )
			bmp = ship_f_icon;
		else
			bmp = ship_e_icon;
	}

	if (o->isAsteroid())
		bmp = cbody_icon;

	if (o->isPlanet())
		bmp = planet_icon;

	if (o == controller->target) {
		if ( game->game_time & 512 )
			bmp = target_icon1;
		else
			bmp = target_icon2;
	}

	if ( bmp ) {
		pos -= Vector2(bmp->w/2, bmp->h/2);

		masked_blit(bmp, Slate,
			0, 0, (int)pos.x, (int)pos.y, bmp->w, bmp->h);
	} else {
		putpixel(Slate, (int)pos.x, (int)pos.y, makecol(200,200,200));
	}

}


void YRadar::initbmp(char *datafilename)
{
	STACKTRACE;

	TW_DATAFILE *dat;
	dat = tw_load_datafile(datafilename);

	if (!ship_f_icon) destroy_bitmap(ship_f_icon);
	if (!ship_e_icon) destroy_bitmap(ship_e_icon);
	if (!cbody_icon)  destroy_bitmap(cbody_icon);
	if (!planet_icon) destroy_bitmap(planet_icon);
	if (!target_icon1)    destroy_bitmap(target_icon1);
	if (!target_icon2)    destroy_bitmap(target_icon2);
	if (!backgr_bmp)  destroy_bitmap(backgr_bmp);
	if (!foregr_bmp)  destroy_bitmap(foregr_bmp);
	if (!Painted)     destroy_bitmap(Painted);

	ship_f_icon = copybmp( (BITMAP*) tw_find_datafile_object(dat, "radar_ship_f")->dat );
	ship_e_icon = copybmp( (BITMAP*) tw_find_datafile_object(dat, "radar_ship_e")->dat );
	cbody_icon  = copybmp( (BITMAP*) tw_find_datafile_object(dat, "radar_cbody")->dat );
	planet_icon = copybmp( (BITMAP*) tw_find_datafile_object(dat, "radar_planet")->dat );
	target_icon1 = copybmp( (BITMAP*) tw_find_datafile_object(dat, "radar_target1")->dat );
	target_icon2 = copybmp( (BITMAP*) tw_find_datafile_object(dat, "radar_target2")->dat );

	backgr_bmp = copybmp( (BITMAP*) tw_find_datafile_object(dat, "radar_backgr")->dat );
	foregr_bmp = copybmp( (BITMAP*) tw_find_datafile_object(dat, "radar_foregr")->dat );

	tw_unload_datafile(dat);

	Painted = create_bitmap_ex(bitmap_color_depth(screen),backgr_bmp->w,backgr_bmp->h);

	if ( !(ship_f_icon && ship_e_icon && cbody_icon && planet_icon &&
	target_icon1 && target_icon2 && backgr_bmp && foregr_bmp ) ) {
		tw_error("Failed to load one of the radar icons");
	}
}


YRadar::YRadar(Control *ocontroller, double Size, TeamCode hteam, char *datafilename, bool rel, int shape)
{
	STACKTRACE;
	relative_angle = rel;
	display_shape = shape;

	controller = ocontroller;
	size = Size;
	active = TRUE;
	set_depth(DEPTH_STARS + 0.1);

	ship_f_icon = 0;
	ship_e_icon = 0;
	cbody_icon  = 0;
	planet_icon = 0;
	target_icon1 = 0;
	target_icon2 = 0;
	backgr_bmp = 0;
	foregr_bmp = 0;

	Painted = 0;

	initbmp(datafilename);

	homeplayer_team = hteam;

	location = 0;
}


void YRadar::animate(Frame *space)
{
	STACKTRACE;

	//If the radar is disabled, don't do anything.
	if (active==FALSE) return;

	clear_to_color(Painted, 0x0FF00FF);

	//Copy the blank slate onto the temporary bitmap Painted
	masked_blit(backgr_bmp, Painted, 0,0,0,0,backgr_bmp->w, backgr_bmp->h);

	SpaceLocation *l = controller->get_focus();
	if (l) {
		PaintYRadar paint(this, Painted, l->pos);
		std::for_each(physics->item.begin(), physics->item.end(), paint);
		//Paint(Painted,l->pos);
	}

	// also, blit the radar foreground:
	masked_blit(foregr_bmp, Painted, 0,0,0,0,backgr_bmp->w, backgr_bmp->h);

	int ix, iy;
	ix = (int)location.x;
	iy = (int)location.y;

	//Copy Painted onto space->frame, which will then paint it on the screen.
	masked_blit(Painted,space->surface,0,0,ix,iy,Painted->w,Painted->h);

	//Tell the frame to redraw this space
	space->add_box(ix,iy,Painted->w,Painted->h);

}


void YRadar::toggleActive()
{
	STACKTRACE;
	active^=1;
}


void YRadar::setSize(double Size)
{
	STACKTRACE;
	size=Size;
}


YRadar::~YRadar()
{
	destroy_bitmap(Painted);
	destroy_bitmap(ship_f_icon);
	destroy_bitmap(ship_e_icon);
	destroy_bitmap(cbody_icon);
	destroy_bitmap(planet_icon);
	destroy_bitmap(target_icon1);
	destroy_bitmap(target_icon2);
	destroy_bitmap(backgr_bmp);
	destroy_bitmap(foregr_bmp);
}


/*
Same as mshppan.cpp, except the panel is NOT drawn onto a new window
(gives me more control), but only the (panel) bitmap is being updated.
*/

ShipPanelBmp::ShipPanelBmp(Ship *_ship)
{
	STACKTRACE;
	id |= ID_SHIP_PANEL;

	panel   = create_bitmap_ex(bitmap_color_depth(screen), 64, 100);
	captain = create_bitmap_ex(bitmap_color_depth(screen), CAPTAIN_WIDTH, CAPTAIN_HEIGHT);

	crew_old   = -1;
	batt_old   = -1;
	batt_light = FALSE;

	old_thrust       = FALSE;
	old_turn_left    = FALSE;
	old_turn_right   = FALSE;
	old_fire_weapon  = FALSE;
	old_fire_special = FALSE;

	deathframe = 0;
	ship = _ship;

	ship->spritePanel->draw(0, 0, 0, panel);
	ship->spritePanel->draw(0, 0, 1, captain);
	draw_stuff (crew_x, crew_y, 2, 1, -3, -2, 2, iround_up(ship->crew), iround_up(ship->crew_max), tw_color(ship->crewPanelColor()), 0);
	draw_stuff (batt_x, batt_y, 2, 1, -3, -2, 2, iround_up(ship->batt), iround_up(ship->batt_max), tw_color(ship->battPanelColor()), 0);

}


ShipPanelBmp::~ShipPanelBmp()
{
	destroy_bitmap(captain);
	destroy_bitmap(panel);
}


void ShipPanelBmp::animate_panel()
{
	STACKTRACE;
	if (!ship) {
		double w, h;

		if (deathframe < 0) return;
		rectfill(captain, 0, 0, CAPTAIN_WIDTH, CAPTAIN_HEIGHT, 0);
		w = CAPTAIN_WIDTH * deathframe / (double)PANEL_DEATH_FRAMES;
		h = CAPTAIN_HEIGHT * deathframe / (double)PANEL_DEATH_FRAMES;
		if (w < 1.0) w = 1.0;
		if (h < 1.0) h = 1.0;
		rectfill(captain,
			(int)(((double)(CAPTAIN_WIDTH) - w) / 2.0),
			(int)(((double)(CAPTAIN_HEIGHT) - h) / 2.0),
			(int)((double)(CAPTAIN_WIDTH) - (((double)(CAPTAIN_WIDTH) - w) / 2.0)),
			(int)((double)(CAPTAIN_HEIGHT) - (((double)(CAPTAIN_HEIGHT) - h) / 2.0)),
			pallete_color[128 - (7 * deathframe / PANEL_DEATH_FRAMES)]);
		blit (captain, panel, 0, 0, CAPTAIN_X, CAPTAIN_Y, captain->w, captain->h);
		return;
	}

	// ship->spritePanel = 0 !! ... this->ship = RogueFigther.
	// can happen, if ships are spawned in-game. You should set spritepanel yourself
	// in that ship, then?
	if (!ship->spritePanel)
		return;

	//	if (panel_needs_update) {
	ship->spritePanel->draw(0, 0, 0, panel);
	crew_old = -1;
	batt_old = -1;
	old_thrust       = ship->thrust;
	old_turn_left    = ship->turn_left;
	old_turn_right   = ship->turn_right;
	old_fire_weapon  = ship->fire_weapon;
	old_fire_special = ship->fire_special;

	ship->spritePanel->draw(0, 0, 1, captain);
	if (ship->thrust)       ship->spritePanel->overlay(1, 2, captain);
	if (ship->turn_right)   ship->spritePanel->overlay(1, 3, captain);
	if (ship->turn_left)    ship->spritePanel->overlay(1, 4, captain);
	if (ship->fire_weapon)  ship->spritePanel->overlay(1, 5, captain);
	if (ship->fire_special) ship->spritePanel->overlay(1, 6, captain);
	blit (captain, panel, 0, 0, CAPTAIN_X, CAPTAIN_Y, captain->w, captain->h);

	// BUG: pkunk can have inf. crew ?!
	if (iround_up(ship->crew) != crew_old) {
		crew_old = iround_up(ship->crew);
		draw_stuff (crew_x, crew_y, 2, 1, -3, -2, 2, crew_old, iround_up(ship->crew_max), tw_color(ship->crewPanelColor()), 0);
		//		panel_needs_update = true;
	}

	if (iround_up(ship->batt) != batt_old) {
		batt_old = iround_up(ship->batt);
		draw_stuff (batt_x, batt_y, 2, 1, -3, -2, 2, batt_old, iround_up(ship->batt_max), tw_color(ship->battPanelColor()), 0);
	}

	return;
}


void ShipPanelBmp::draw_stuff (int x, int y, int w, int h, int dx, int dy, int m, int value, int max, int color, int bcolor)
{
	int i;
	w -= 1;
	h -= 1;
	if (value > max) value = max;
	for (i = 0; i < value; i += 1) {
		int _x = x + dx * (i % m);
		int _y = y + dy * (i / m);
		rectfill(panel, _x, _y, _x+w, _y+h, color);
	}
	for (i = value; i < max; i += 1) {
		int _x = x + dx * (i % m);
		int _y = y + dy * (i / m);
		rectfill(panel, _x, _y, _x+w, _y+h, bcolor);
	}
	return;
}


ImIndicator::ImIndicator(SpaceLocation *o)
{
	STACKTRACE;
	showme = o;

	bmp = create_bitmap_ex( bitmap_color_depth(screen), ImIndicatorSize, ImIndicatorSize);

	clear_to_color(bmp, makecol(255,0,255));
	// also, turn this into some "character" of the sprite ?!

	if (o->isObject()) {
		SpaceSprite *src;
		src = ((SpaceObject*)o)->get_sprite();
		src->draw_character(0, 0, bmp->w, bmp->h, 0, colortype(o), bmp);
	} else {
		clear_to_color(bmp, 0x0408090);
	}
}


ImIndicator::~ImIndicator()
{
	destroy_bitmap(bmp);
}


int ImIndicator::colortype(SpaceLocation *o)
{
	STACKTRACE;

	if (o && o->isPlanet())
		return makecol(150,100,25);

	else if (o && o->isShip())
		return makecol(100,150,25);

	else
		return makecol(150,50,50);
}


void ImIndicator::newtarget(SpaceLocation *o)
{
	STACKTRACE;

	showme = o;

	if (o && o->isObject()) {
		SpaceSprite *src;
		src = ((SpaceObject*)o)->get_sprite();
		src->draw_character(0, 0, bmp->w, bmp->h, 0, colortype(o), bmp);
	} else {
		clear_to_color(bmp, 0x0408090);
	}
}


void ImIndicator::animate(Frame *frame)
{
	STACKTRACE;

	if (!showme)
		return;

	Vector2 C, P, S;

	C = space_view_size / 2;	 // center of the map
	P = corner ( showme->pos );

	Vector2 ssize = ((SpaceObject*)showme)->get_sprite()->size() * space_zoom / 2;

	if (   P.y + ssize.y > 0
		&& P.y - ssize.y < space_view_size.y
		&& P.x + ssize.x > 0
		&& P.x - ssize.x < space_view_size.x )
		return;

	S = P;

	if ( S.y < 0 ) {
		S.y = 0;
		S.x = C.x + (S.y - C.y) * (P.x - C.x) / (P.y - C.y);
	}

	if ( S.y > space_view_size.y ) {
		S.y = space_view_size.y;
		S.x = C.x + (S.y - C.y) * (P.x - C.x) / (P.y - C.y);
	}

	if ( S.x < 0 ) {
		S.x = 0;
		S.y = C.y + (S.x - C.x) * (P.y - C.y) / (P.x - C.x);
	}

	if ( S.x > space_view_size.x ) {
		S.x = space_view_size.x;
		S.y = C.y + (S.x - C.x) * (P.y - C.y) / (P.x - C.x);
	}

	// now, S is the position of the indicator. We need to know the distance, for
	// scaling purposes

	double    scale;
	scale = sqrt( frame->surface->w / min_delta(showme->pos - space_center, map_size).magnitude() );
	if ( scale > 1.0 )    scale = 1.0;
	if ( scale < 0.2 )    scale = 0.2;

	// now, plot the indicator ?!

	int iw, ih;
	iw = bmp->w*(int)scale;
	ih = bmp->h*(int)scale;

	S -= Vector2(iw/2, ih/2);

	masked_stretch_blit(bmp, frame->surface, 0, 0, bmp->w, bmp->h,
		(int)S.x, (int)S.y, iw, ih);
	frame->add_box(S.x, S.y, iw, ih);
}
