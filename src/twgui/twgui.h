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

#ifndef __TWGUI_GUI_H__
#define __TWGUI_GUI_H__

#include "twwindow.h"
#include "twbuttontypes.h"
#include "twhelpers.h"

// something which has a background, and its own drawing-area
class AreaTabletScrolled : public AreaTablet
{
	protected:

		//scrollpos_str	scroll;
		ScrollControl   scroll;

	public:

		AreaTabletScrolled(TWindow *menu, const char *identbranch, int asciicode, bool akeepkey = 0);

		int gety();

		virtual void calculate();
};

// a text on top of a button.
// no interaction with environment, it's purely a message.

class TextButton : public AreaTablet
{
	public:
		TextButton(TWindow *menu, const char *identbranch, FONT *afont);
		virtual ~TextButton();

		FONT        *usefont;

		char *text;
		int text_color;

		virtual void subanimate();
		void set_text(char *txt, int color);
};

// Draws a list of text strings onto a background
// Returns the "item" (or line) number that's selected
// This must support a scroll-bar, in case the list does not fit in the box
class TextList : public AreaTabletScrolled
{
	int Nreserved;

	public:
		TextList(TWindow *menu, const char *identbranch, FONT *afont);
		~TextList();

		//ScrollControl	scroll;

		char    **optionlist;
		int     N;				 // number of options

		FONT    *usefont;
		int     Htxt, Nshow;

		//int		yselected;	// the selected item.

		void clear_optionlist();
								 // overwrite a list.
		void set_optionlist(char **aoptionlist, int color);
		void set_optionlist(char **aoptionlist, int aN, int color);
								 // add one item to an existing list.
		void add_optionlist(char *newstr);

		void set_selected(int iy);

		int     text_color;

		void initbackgr(bool autoplace);

		virtual void subanimate();

		virtual void handle_lpress();
		virtual void handle_rpress();
		virtual void calculate();

		int getk();
};

// Draw some text into a box... text can consist of many lines. No editing possible.
// If there's a lot of text, you could scroll.

class TextInfoArea : public AreaTabletScrolled
{
	protected:
		int maxchars;			 // you can't add chars beyond that
		char *localcopy;
		TextInfo *textinfo;
		//scrollpos_str	*scroll;
	public:
		TextInfoArea(TWindow *menu, const char *identbranch, FONT *afont, char *atext, int amaxtext);
		virtual ~TextInfoArea();

		int     linestart[maxlines];
		int     Nlines;

		FONT    *usefont;
		//int		Htxt, Nshow;

		int     text_color;

		void set_textinfo(char *atextinfo);
		void set_textinfo(char *atextinfo, int Nchars);
		void set_textinfo_unbuffered(char *newtext, int Nchars);

		virtual void subanimate();

};

class TextEditBox : public TextInfoArea
{
	protected:
		int charpos;

		// this is used to record when the last key-press was made (with a resolution
		// of 1 frame at best).
		bool    keypr[KEY_MAX];
		int     lastpressed;
		int     repeattime, lasttime;

	public:
		char *text;				 //[128];	// can hold 1 line of text.
		int     text_color;

		TextEditBox(TWindow *menu, const char *identbranch, FONT *afont, char *atext, int amaxtext);
		virtual ~TextEditBox();

		virtual void calculate();
		virtual void subanimate();

		virtual void handle_lpress();

		//	void clear_text();
		//	void show_text();

		// set the typematic delay to "atime" milliseconds (default = 100)
		//void set_repeattime(int atime);
		void set_textcolor(int c);

		void text_reset();
		void text_reset(char *newtext, int N);

		char *get_text();
};

// implement a matrix of icons
// the box defines the area where the icons are visible;

class MatrixIcons : public AreaTabletScrolled
{
	protected:
		int     Nx, Ny, Nxshow, Nyshow, maxitems;
		int     Wicon, Hicon;
		BITMAP  *overlay, *tmp;
		double  extrascale;
	public:

		enum property {ENABLE=0x01, DISABLE=0x02};

		//	int xselect, yselect;
		//	int xold, yold;
		bool selected;

		int     *itemproperty;

		BITMAP  **listIcon;

		MatrixIcons(TWindow *menu, const char *identbranch, int akey);
		virtual ~MatrixIcons();

		void set_iconinfo(BITMAP **alistIcon, double ascale);

		int getk();

		virtual void subanimate();
		virtual void subcalculate();

		virtual void handle_lpress();
								 // for scrolling
		virtual void handle_rpress();
		//virtual void handle_doubleclick();

};
#endif							 // __TWGUI_GUI_H__
