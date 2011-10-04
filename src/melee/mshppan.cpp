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

#include <allegro.h>

#include "melee.h"
REGISTER_FILE
#include "mgame.h"
#include "mshppan.h"
#include "mship.h"
#include "mview.h"
#include "id.h"
#include "util/aastr.h"

int PANEL_WIDTH = 64;
int PANEL_HEIGHT = 100;

int CAPTAIN_X = 4;
int CAPTAIN_Y = 65;
int PANEL_DEATH_FRAMES = 2500;
int crew_x = 8;
int crew_y = 53;
int batt_x = 56;
int batt_y = 53;

ShipPanel::ShipPanel(Ship *_ship)
{
	STACKTRACE;
	id |= ID_SHIP_PANEL;

	panel   = create_bitmap(64, 100);
	captain = create_bitmap(CAPTAIN_WIDTH, CAPTAIN_HEIGHT);

	crew_old   = -1;
	batt_old   = -1;
	batt_light = FALSE;

	panel_needs_update = false;
	captain_needs_update = false;

	always_redraw = false;

	old_thrust       = FALSE;
	old_turn_left    = FALSE;
	old_turn_right   = FALSE;
	old_fire_weapon  = FALSE;
	old_fire_special = FALSE;

	deathframe = 0;
	ship = _ship;

	ship->spritePanel->draw(0, 0, 0, panel);
	ship->spritePanel->draw(0, 0, 1, captain);
	draw_stuff (crew_x, crew_y, 2, 1, -3, -2, 2, iround_up(ship->crew), iround_up(ship->crew_max), 1, 0);
	draw_stuff (batt_x, batt_y, 2, 1, -3, -2, 2, iround_up(ship->batt), iround_up(ship->batt_max), 2, 0);

	window = new VideoWindow();
	window->preinit();
}


ShipPanel::~ShipPanel()
{
	destroy_bitmap(captain);
	destroy_bitmap(panel);
	window->deinit();
	delete window;
}


void ShipPanel::refresh()
{
	STACKTRACE;
	panel_needs_update = 1;
	captain_needs_update = 1;
	return;
}


void ShipPanel::calculate()
{
	STACKTRACE;
	if (!ship) {
		deathframe -= frame_time;
		if (deathframe <= 0) this->die();
		return;
	}
	if (!ship->exists()) {		 //ship is dieing
		ship->spritePanel->draw(0, 0, 0, panel);
		panel_needs_update = true;
		ship = NULL;
		deathframe = PANEL_DEATH_FRAMES;
		return;
	}
	if (ship->update_panel) {
		ship->update_panel = false;
		panel_needs_update = true;
		captain_needs_update = true;
	}
	if (
		(ship->thrust != old_thrust) ||
		(ship->turn_left != old_turn_left) ||
		(ship->turn_right != old_turn_right) ||
		(ship->fire_weapon != old_fire_weapon) ||
		(ship->fire_special != old_fire_special))
		captain_needs_update = true;
	return;
}


void ShipPanel::animate(Frame *space)
{
	STACKTRACE;

	BITMAP *screen = window->surface;
	if (!screen) return;
	int panel_x, panel_y, panel_width, panel_height;
	panel_x = window->x;
	panel_y = window->y;
	panel_width = window->w;
	panel_height = window->h;
	if (always_redraw || FULL_REDRAW) {
		panel_needs_update = true;
		captain_needs_update = true;
	}
	if (!ship) {
		double w, h;
		if (panel_needs_update) {
			blit (captain, panel, 0, 0, CAPTAIN_X, CAPTAIN_Y, captain->w, captain->h);
			acquire_screen();
			//			blit(panel, screen, 0, 0, panel_x, panel_y, 64, 100);
			//			blit(captain, screen, 0, 0, captain_x, captain_y, CAPTAIN_WIDTH, CAPTAIN_HEIGHT);
			//			int a = aa_get_mode();
			//			aa_set_mode(AA_NO_AA);
			aa_stretch_blit(panel, screen, 0, 0, panel->w, panel->h, panel_x, panel_y, panel_width, panel_height);
			//			aa_set_mode(a);
			release_screen();
		}
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
		acquire_screen();
		//		blit(captain, screen, 0, 0, captain_x, captain_y, CAPTAIN_WIDTH, CAPTAIN_HEIGHT);
		//			int a = aa_get_mode();
		//		aa_set_mode(AA_NO_AA);
		aa_stretch_blit(panel, screen, 0, 0, panel->w, panel->h, panel_x, panel_y, panel_width, panel_height);
		//			aa_set_mode(a);
		release_screen();
		return;
	}

	if (panel_needs_update) {
		ship->spritePanel->draw(0, 0, 0, panel);
		crew_old = -1;
		batt_old = -1;
	}

	if (captain_needs_update) {
		captain_needs_update = false;
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
		panel_needs_update = true;
	}

	if (iround_up(ship->crew) != crew_old || panel_needs_update) {

		crew_old = iround_up(ship->crew);
		draw_stuff (crew_x, crew_y, 2, 1, -3, -2, 2, crew_old, iround_up(ship->crew_max), 1, 0);
		panel_needs_update = true;
	}

	if (iround_up(ship->batt) != batt_old || panel_needs_update) {

		batt_old = iround_up(ship->batt);
		draw_stuff (batt_x, batt_y, 2, 1, -3, -2, 2, batt_old, iround_up(ship->batt_max), 2, 0);
		panel_needs_update = true;
	}

	if ((!batt_light) && (ship->weapon_low || ship->special_low)) {
		meleedata.panelSprite->draw(40, 58, 2, panel);
		batt_light = TRUE;
		panel_needs_update = true;
	}
	else if (batt_light && (!ship->weapon_low) && (!ship->special_low)) {
		meleedata.panelSprite->draw(40, 58, 1, panel);
		batt_light = FALSE;
		panel_needs_update = true;
	}

	if (panel_needs_update) {
		panel_needs_update = false;
		window->lock();
		//		blit(panel, screen, 0, 0, panel_x, panel_y, 64, 100);
		//		blit(captain, screen, 0, 0, captain_x, captain_y, CAPTAIN_WIDTH, CAPTAIN_HEIGHT);
		aa_set_mode(AA_NO_AA);
		aa_stretch_blit(panel, window->surface, 0, 0, panel->w, panel->h, panel_x, panel_y, panel_width, panel_height);
		window->unlock();
	}

	return;
}


void ShipPanel::draw_stuff (int x, int y, int w, int h, int dx, int dy, int m, int value, int max, int display_type, int bcolor)
{

	// custom update by the ship ...
	if (ship->custom_panel_update(panel, display_type))
		return;

	// erase the background...

	int x1, x2, y1, y2, xl, xr, ymid, erasecolor, max2;

	if (display_type == 1) {
		// crew offset
		xl = 3;
		xr = 11;
	} else {
		// batt offset
		xl = 51;
		xr = 59;
	}

	max2 = max;
	if (max2 > 42)
		max2 = 42;

								 // the +(m-1) makes sure it rounds up
	ymid = y + dy * ((max2+m-1) / m);

	// erase to default greyish panel color
	erasecolor = tw_color(100,100,100);
	x1 = xl;
	y1 = 12;
	x2 = xr;
	y2 = ymid;
	rectfill(panel, x1, y1, x2, y2, erasecolor);

	// erase the black background part
	erasecolor = bcolor;
	x1 = xl+1;
	y1 = ymid+1;
	x2 = xr-1;
	y2 = 55;
	rectfill(panel, x1, y1, x2, y2, erasecolor);

	// the borders of the crew/batt panel
	erasecolor = makecol(140,140,140);
	x1 = xl;
	y1 = ymid;
	x2 = xl;
	y2 = 55;
	rectfill(panel, x1, y1, x2, y2, erasecolor);

	x1 = xl;
	y1 = ymid;
	x2 = xr;
	y2 = ymid;
	rectfill(panel, x1, y1, x2, y2, erasecolor);

	erasecolor = makecol(60,60,60);
	x1 = xr;
	y1 = ymid;
	x2 = xr;
	y2 = 56;
	rectfill(panel, x1, y1, x2, y2, erasecolor);

	x1 = xl;
	y1 = 55;
	x2 = xr;
	y2 = 56;
	rectfill(panel, x1, y1, x2, y2, erasecolor);

	int i, color;
	w -= 1;
	h -= 1;
	if (value > max) value = max;

	if (value <= 42) {

		// normal crew/batt bar

		for (i = 0; i < value; i += 1) {
			int _x = x + dx * (i % m);
			int _y = y + dy * (i / m);

			if (display_type == 1)
				color = tw_color(ship->crewPanelColor(i));
			else
				color = tw_color(ship->battPanelColor(i));

			rectfill(panel, _x, _y, _x+w, _y+h, color);
		}

	} else {

		// percentile crew/batt bar

		w = xr - xl - 1;

		int ymin, ymax;
		ymin = ymid;
		ymax = 55;

		int dy;
		dy = (ymax - ymin);
		ymin += iround(dy * (max - value) / double(max));

		for ( i = ymin; i < ymax; ++i ) {
			int _x, _y;
			int k;
			k = iround(value * double(ymax - i) / double(ymax - ymin));

			_x = xl + 1;
			_y = i;

			if (display_type == 1)
				color = tw_color(ship->crewPanelColor(k));
			else
				color = tw_color(ship->battPanelColor(k));

			rectfill(panel, _x, _y, _x+w, _y, color);
		}

	}

	/* already erased
	for (i = value; i < max; i += 1) {
		int _x = x + dx * (i % m);
		int _y = y + dy * (i / m);

		if (display_type == 1)

			color = tw_color(ship->crewPanelColor(i));

		else

			color = tw_color(ship->battPanelColor(i));

		rectfill(panel, _x, _y, _x+w, _y+h, bcolor);
		}
	*/
	return;
}
