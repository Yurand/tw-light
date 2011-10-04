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

#ifndef __TWHELPERS_H__
#define __TWHELPERS_H__

#include "twbuttontypes.h"

/** \brief Manages 2 scrollbars and 4 buttons to determine a position in 2D. This
is used to determine item number, rather than pixels; the number of items is entered
in initialization.

*/

class ScrollControl
{
	public:
		EmptyButton *left, *right, *up, *down;
		ScrollBar *scrollhor, *scrollvert;

		ScrollControl();

		int x, y;				 // which Icon is visible top left.
		int Nx, Ny;				 // number of Icons that are present.
		int xselect, yselect;
		int Nxshow, Nyshow;

		void set(int xscroll, int yscroll, int Nxscroll, int Nyscroll, int Nx_show, int Ny_show);

		void set_pos(int xnew, int ynew);
		void set_percent_pos_x(double alpha);
		void set_percent_pos_y(double alpha);
		double get_relpos();
		void add(int dx, int dy);
		void check_pos();
		void calculate();

		void set_sel(int xsel, int ysel);
		void check_sel();

		void bind(EmptyButton *left, EmptyButton *right, EmptyButton *up, EmptyButton *down,
			ScrollBar *scrollhor, ScrollBar *scrollvert);

								 //, ScrollControl *scr);
		void setup_hor(TWindow *A, const char *id);
								 //, ScrollControl *scr);
		void setup_ver(TWindow *A, const char *id);
								 //, ScrollControl *scr);
		void setup_hor(EmptyButton *A, const char *id);
								 //, ScrollControl *scr);
		void setup_ver(EmptyButton *A, const char *id);
		// set up both the horizontal and vertical bar (provided the graphics exist)
								 //, ScrollControl *scr);
		void setup(TWindow *A, const char *id);

};

/** \brief Manage a string with enter-characters, generating info about the lines
of text that are present in there.

*/

const int maxlines = 1024;

class TextInfo
{
	public:
		char    *textinfo;

		int     linestart[maxlines];
		int     Nlines, Nchars, maxchars;

		FONT    *usefont;
		int     W, Htxt, Nshow;

		int     text_color;

		BITMAP  *bmp;

		TextInfo(FONT *afont, BITMAP *abmp, char *atextinfo, int Nchars);
		~TextInfo();

		//void set_textinfo(char *atextinfo, int Nchars);

		int getcharpos(char *txt, int x, int max);
		void getxy(int charpos, int *x, int *y);

		int getcharpos(int x, int y);
		void changeline(int *charpos, int line1, int line2);

		void reset(ScrollControl *scroll);
};
#endif							 // __TWHELPERS_H__
