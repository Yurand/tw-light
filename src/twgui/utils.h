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

#ifndef __TWGUI_UTILS_H__
#define __TWGUI_UTILS_H__

int mapkey(int scancode_key, int scancode_ctrl = 0);
int unmapkey1(int k);
int unmapkey2(int k);

BITMAP *find_datafile_bmp(TW_DATAFILE *datafile, char *identif);

BITMAP *clone_bitmap(int bpp, BITMAP *src, double scale, bool vidmem);

class normalmouse
{
	protected:

	public:

		struct posstr
		{
			int x, y, wheel;	 // x,y = position, z = wheel position
			void set(int xnew, int ynew, int znew);
								 // so that you can make those values relative?!
			void move(int dx, int dy);
		} pos, oldpos, pos2;

		class mousebutton
		{
			struct buttonstr
			{
				double time;
				bool status;	 // button status
				bool change;
			} button, oldbutton;

			public:
				void update(bool newstatus);

				// mouse button is suddenly pressed
				bool press();
				// mouse button is suddenly released
				bool release();
				// left mouse button is double clicked
				bool dclick();
				// left mouse button is held
				bool hold();

		} left, mid, right;

		normalmouse();

		void reset();

		void update();
		void copyinfo(normalmouse *othermouse);

		int xpos() { return pos.x; };
		int ypos() { return pos.y; };
		int wheelpos() { return pos.wheel; };

		void move(int dx, int dy);
		int vx();
		int vy();

		class bmpstr
		{
			public:
				BITMAP *original_mouse_sprite, *newmousebmp;
				void init(BITMAP *newbmp);
				void set();
				void restore();
		} bmp;

};

static const int buffmax = 128;

class TKeyHandler
{
	private:
		int Nbackbuf;
		int keybackbuf[buffmax];

	public:
		int keynew[KEY_MAX];
		int keyold[KEY_MAX];
		int keyhit[KEY_MAX];
		int keyreleased[KEY_MAX];

		int Nbuf;
		int keybuf[buffmax];

		TKeyHandler();
		void update();
		void add(int key);
		void clear();
		void clearbuf();

		bool pressed(char key);
};
#endif							 // __TWGUI_UTILS_H__
