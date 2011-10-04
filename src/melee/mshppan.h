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

#ifndef __MSHPPAN_H__
#define __MSHPPAN_H__

#include "melee.h"

extern int PANEL_WIDTH;
extern int PANEL_HEIGHT;

extern int CAPTAIN_X;
extern int CAPTAIN_Y;
extern int PANEL_DEATH_FRAMES;
extern int crew_x;
extern int crew_y;
extern int batt_x;
extern int batt_y;

class ShipPanel : public Presence
{
	protected:
		BITMAP *panel;
		BITMAP *captain;

		int crew_old;
		int batt_old;
		char batt_light;

		char panel_needs_update;
		char captain_needs_update;

		char old_thrust;		 // previous keypresses for ship
		char old_turn_left;
		char old_turn_right;
		char old_fire_weapon;
		char old_fire_special;

		int deathframe;

	public:
		Ship *ship;

		int always_redraw;

		ShipPanel(Ship *oship);
		virtual ~ShipPanel();

		virtual void draw_stuff(int x, int y, int w, int h, int dx, int dy, int m, int value, int max, int color, int bcolor);
		//used for drawing crew & battery bars

		virtual void refresh();
		//causes redraw sometime

		//virtual void hide();
		//remove the panel from the screen
		//virtual void locate(int x, int y, int w = PANEL_WIDTH, int h = PANEL_HEIGHT);
		//move the panel to x, y //add width & height later?
		VideoWindow *window;

		virtual void calculate();
		//called every frame
		virtual void animate(Frame *space);
		//the parameter "space" is not used
};
#endif							 // __MSHPPAN_H__
