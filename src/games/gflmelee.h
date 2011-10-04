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

#ifndef __GFLMELEE_H__
#define __GFLMELEE_H__

class ShipPanelBmp : public Presence
{
	public:
		BITMAP *panel;			 // this is the whole panel, including the captain pic.
		BITMAP *captain;

		int crew_old;
		int batt_old;
		char batt_light;

		char old_thrust;		 // previous keypresses for ship
		char old_turn_left;
		char old_turn_right;
		char old_fire_weapon;
		char old_fire_special;

		int deathframe;

		Ship *ship;

		ShipPanelBmp(Ship *oship);
		virtual ~ShipPanelBmp();

		virtual void draw_stuff(int x, int y, int w, int h, int dx, int dy, int m, int value, int max, int color, int bcolor);
		//used for drawing crew & battery bars

		virtual void animate_panel();
		//the parameter "space" is not used
};
#endif							 //__GFLMELEE_H__
