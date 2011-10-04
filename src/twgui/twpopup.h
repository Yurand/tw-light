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

#ifndef __TWMENUCOMPONENT_H__
#define __TWMENUCOMPONENT_H__

/*

  Notes:

  *	a popup that's exclusive means, all other active menus are (should be) inacessible.
	This can also be achieved on a limited scale by option.## to disable the host window

  *	use option.## to determine where it's displayed on popup: relative to the mouse,
	or to the main window.

*/

#include "twwindow.h"

class Popup : public TWindow
{
	protected:
		bool    inherited;

		bool movingthereserve;	 // in case you want to move the menu area somewhere else

	public:
		int     returnstatus;

		Popup(const char *ident, int xcenter, int ycenter, BITMAP *outputscreen);
		virtual ~Popup();

		virtual void doneinit();

		bool returnvalueready;
		bool ready();

		virtual void calculate();
		virtual void check_end();

		virtual void close(int areturnstatus);
		virtual int getvalue();

		// this allows you to acces the return value, but only once after the submenu was
		// closed; otherwise it returns -1.
		virtual void enable();

		virtual void newscan();
};

// a triggered popup-window: the trigger is a button from another window.
class PopupT : public Popup
{
	public:
		struct option_str
		{
								 // disables all i/o of the host menu (true)
			bool disable_othermenu,
				place_relative2mouse,// places relative to mouse (true), or the host menu (false)
				place_relative2window;
		} option;

		int xshift, yshift;

		EmptyButton     *trigger;// this controls the on/off of the menu

		// origin relative to the creators' position
		PopupT(const char *ident, int xcenter, int ycenter, BITMAP *outputscreen);
		PopupT(EmptyButton *creator, const char *identbranch, int axshift, int ayshift);
		~PopupT();
		virtual void init_components(const char *identbranch);

		// returnstatus can be (usually is) the array index of a selected item in a list.
		virtual void close(int areturnstatus);

		virtual void calculate();
		virtual void handle_focus_loss();

		// this should call "close" ; you should replace this with your own code, to get the
		// submenu-exit behaviour you want. Default is that it returns value -1 when the mouse
		// pointer leaves the submenu area.
		virtual void check_end();

		// this toggles whether the list is auto-closed when focus is lost, or remains
		// focus even if the mouse moves out of sight of the list...
		bool close_on_defocus;
};
#endif							 // __TWMENUCOMPONENT_H__
