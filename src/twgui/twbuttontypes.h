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

#ifndef __TWBUTTONTYPES_H__
#define __TWBUTTONTYPES_H__

#include "twbutton.h"

// to implement a button, you add bitmaps-feedback to the box-area control

/** \brief A single button.

 */

class Button : public GraphicButton
{
	protected:

	public:
		BITMAP *bmp_default, *bmp_focus, *bmp_selected;

		Button(TWindow *menu, const char *identbranch, int asciicode = 0, bool keepkey = 0);
		virtual ~Button();

		virtual void draw_default();
		virtual void draw_focus();
		virtual void draw_selected();

		virtual bool hasmouse();
		virtual bool isvalid();
};

/** \brief A general area for displaying graphics, which can be changed if needed.

 */

class Area : public GraphicButton
{
	protected:

	public:
		BITMAP *backgr;
		bool markfordeletion;

		Area(TWindow *menu, const char *identbranch, int asciicode = 0, bool akeepkey = 0);
		virtual ~Area();

		virtual void changebackgr(char *fname);
		virtual void changebackgr(BITMAP *newb);
		virtual void overwritebackgr(BITMAP *newb, double scale, int col);

		virtual void animate();	 // shouldn't be changed.

		virtual bool hasmouse();
		virtual bool isvalid();
};

/** \brief A more specialized area for displaying graphics. It allocates an extra
drawing bitmap (i.e. adds an extra drawing layer), so that you can draw over
the background without changing the background itself.

*/
// something which has a background, and its own drawing-area
class AreaTablet : public Area
{
	protected:

	public:
		BITMAP *drawarea;

		AreaTablet(TWindow *menu, const char *identbranch, int asciicode = 0, bool akeepkey = 0);
		virtual ~AreaTablet();

		virtual void animate();	 // shouldn't be changed.
		virtual void subanimate();

		virtual bool hasmouse();
		virtual bool isvalid();
};

/** \brief A button that toggles between two states, on or off, whenever it's pushed.

 */

class SwitchButton : public GraphicButton
{
	protected:
	public:
		BITMAP  *bmp_on, *bmp_off;

		bool    state;			 // true=on, false=off
		// x, y, W, H are inside the draw area
		SwitchButton(TWindow *menu, const char *identbranch, int asciicode = 0, bool initialstate = false);
		virtual ~SwitchButton();

		virtual void calculate();

		virtual void draw_default();
		virtual void draw_focus();
								 // is the same as focus, cause a switch cannot be selected all the time !!
		virtual void draw_selected();

		virtual bool hasmouse();
		//void locate_by_backgr(char *stron);
		virtual bool isvalid();
};

/** \brief An invisible button ; can be used to fake a trigger-button for a popup menu.

 */

class GhostButton : public EmptyButton
{
	public:
		GhostButton(TWindow *menu);
		virtual ~GhostButton();
};

/** \brief Manages a horizontal or vertical (auto-determined based on bitmap
dimension) scrollbar. It has a background, and animates the bar-handle on top of
that. This can be used together with a "manager" class, the ScrollControl.

*/

class ScrollBar : public AreaTablet
{
	protected:
	public:
		BITMAP  *button;

		enum {hor = 1, ver= 2}
		direction ;

		double relpos;

								 // p = position (can be x or y)
		int pmin, pmax, bwhalf, bhhalf;
		int pbutton;

		// x, y, W, H are inside the draw area
		ScrollBar(TWindow *menu, char *identbranch);
		virtual ~ScrollBar();

		virtual void handle_lhold();

		virtual void subanimate();

		void setrelpos(double arelpos);

		virtual void calculate();
};
#endif							 // __TWBUTTONTYPES_H__
