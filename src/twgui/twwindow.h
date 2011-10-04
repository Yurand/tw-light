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

#ifndef __TWWINDOW_H__
#define __TWWINDOW_H__

/*

TAB = navigates between buttons.
LSHIFT+TAB = navigates in the opposite direction.
LCONTROL+TAB = navigates between windows.

  exclusive = when focused, all other windows are disabled. Default=false.
  layer = you can only change focus within your own layer. Default=0. In this way you can cause a set of windows to always appear above another set of windows.
  disabled = that window does no calculate (use ... to do this ?)
  hidden = that window does not animate (use hide() to stop activity, or show() to restore activity)

*/

#include "utils.h"

#include "twbutton.h"

void del_bitmap(BITMAP **bmp);

class EmptyButton;

// a menu superstructure ... to manage a bunch of related areas ... in a reserved area/ region

const int max_area = 128;

extern normalmouse Tmouse;
extern TKeyHandler keyhandler;

class TWindow
{
	public:

		int autoplace;
		// true: then the auto-search is used to place a bitmap somewhere, and also
		// there's always a check to see if this is needed;
		// false: then the ini positions are used and neither checked nor updated.

		// managing a list of (related) windows
		TWindow *prev, *next;

		TWindow *tree_root();
		TWindow *tree_last();

		void tree_calculate();
		void tree_animate();
		void tree_setscreen(BITMAP *scr);
		void tree_doneinit();
		void focus();

		void add(TWindow *newwindow);

		// status of the window (ready to handle input, or not ...)

		bool        disabled;
		bool        hidden;		 // hide is "stronger" than disabled, since it also prevents drawing.

		void enable();
		void disable();			 // changes the "disabled" flag, and possibly does other stuff as well?
		void hide();
		void show();

		bool hasfocus();		 // so that items can check if the menu is active or not
		// (in case the menu calculations fail, occasionally happens)
		// eg., mouse can move out of the menu, in a separate iteration
		// so that incremental updates don't work.

		double      menu_time, menu_starttime;
		void        update_time();

		//	int			keybuff[128];
		//	int			keybuff_count;
		//	void		add_key(int akey);
		//	void		clear_keys();
		//	void		add_keys();
		//	normalmouse		mouse;

		int             x, y, W, H;

		bool checkmouse();
		bool grabbedmouse;
		Vector2 mpos;

		char            ident[128];

		int     default_W;
		double  scale;
		//BITMAP *originalscreen;	// to backup the original screen content
		BITMAP *screen;			 // pointer to (modifiable) original screen
		BITMAP *drawarea;		 // intermediately used for drawing ... so that menu items won't draw off this area
		BITMAP *backgr_forsearch;// waste of memory perhaps ?!

		BITMAP *backgr;

		//	DATAFILE *datafile;		// this file should contain info for all related areas
		// (used for initialization).

		EmptyButton *button_first, *button_last;
		void add(EmptyButton *newbutton);
		void rem(EmptyButton *newbutton);

		// the last button that has the focus!
		EmptyButton *button_focus;
		void setfocus(EmptyButton *newbutton);

		// vidwin: places the bitmaps used by the menu in video-memory, which is faster.
		TWindow(const char *identbase, int dx, int dy, BITMAP *outputscreen, bool vidwin = false);
		virtual ~TWindow();

		void setscreen(BITMAP *scr);

		virtual void doneinit();

		// can be changed
		virtual void calculate();// calls all menu items
		virtual void animate();

		// return a bmp from the data file
		BITMAP* bmp(char *bmpname, bool vidmem = false);

		// this centers the bitmap on this position.
		void center_abs(int xcenter, int ycenter);
		void center(int xcenter, int ycenter);
		void center();

		void handle_focus();
		void handle_focus_loss();

		void scalepos(int *ax, int *ay);
		void scalepos(Vector2 *apos);

		char configfilename[256];
		bool search_bmp_location(BITMAP *bmp_default, Vector2 *apos);

		bool exclusive;
		int layer;				 // you can only switch focus within a layer...
};
#endif							 // __TWWINDOW_H__
