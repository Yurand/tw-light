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
#include <stdio.h>
#include <string.h>

#include "melee.h"
#include "melee/mview.h"
REGISTER_FILE

#include "twwindow.h"
#include "twbuttontypes.h"

// to implement a Button, you add bitmaps-feedback to the box-area control
// name#default.bmp
// name#focus.bmp
// name#selected.bmp

Button::Button(TWindow *menu, const char *identbranch, int asciicode, bool akeepkey)
:
GraphicButton(menu, identbranch, asciicode, akeepkey)
{
	STACKTRACE;
	init_pos_size(&bmp_default, "default");

	bmp_focus = getbmp("focus");
	bmp_selected = getbmp("selected");
}


Button::~Button()
{
	del_bitmap(&bmp_default);
	del_bitmap(&bmp_focus);
	del_bitmap(&bmp_selected);
}


// this is the default drawing (at rest):
void Button::draw_default()
{
	STACKTRACE;
	draw(bmp_default);
}


void Button::draw_focus()
{
	STACKTRACE;
	if (!draw(bmp_focus)) {
		draw_default();
		draw_boundaries(bmp_default);
	}
}


void Button::draw_selected()
{
	STACKTRACE;
	if (!draw(bmp_selected))
		draw_default();
}


bool Button::hasmouse()
{
	STACKTRACE;
	return GraphicButton::hasmouse(bmp_default);
}


bool Button::isvalid()
{
	STACKTRACE;
	return bmp_default != 0;
};

Area::Area(TWindow *menu, const char *identbranch, int asciicode, bool akeepkey)
:
GraphicButton(menu, identbranch, asciicode, akeepkey)
{
	STACKTRACE;
	init_pos_size(&backgr, "backgr");

	markfordeletion = true;
}


Area::~Area()
{
	if (markfordeletion)
		del_bitmap(&backgr);
}


void Area::changebackgr(char *fname)
{
	STACKTRACE;
	BITMAP *newb;
	//newb = getbmp(fname);
	newb = getbmp_nobutton(fname);

	if (newb) {
		if (markfordeletion)
			del_bitmap(&backgr);

		backgr = newb;
		markfordeletion = true;	 // locally initialized, hence locally destroyed...
	}
}


void Area::changebackgr(BITMAP *newb)
{
	STACKTRACE;
	if (newb) {
		if (markfordeletion)
			del_bitmap(&backgr); // hmm, well, don't do this, leave that to the program that created it !!

		backgr = newb;
		markfordeletion = false; // not locally initialized, hence not locally destroyed...
	}
}


void Area::overwritebackgr(BITMAP *newb, double scale, int col)
{
	STACKTRACE;
	if (newb && backgr) {
		clear_to_color(backgr, col);
		stretch_blit(newb, backgr, 0, 0, newb->w, newb->h,
			0, 0, iround(newb->w * scale), iround(newb->h * scale));
	}
}


void Area::animate()
{
	STACKTRACE;
	draw(backgr);
}


bool Area::hasmouse()
{
	STACKTRACE;
	// the first rough check whether it's in the boxed bitmap area
	return GraphicButton::hasmouse(backgr);
}


bool Area::isvalid()
{
	STACKTRACE;
	return backgr != 0;
};

// an additional class, which has its own background and drawing area, which
// can be used to create custom representations of information (eg., text or smaller
// bitmaps))
// name#backgr.bmp

AreaTablet::AreaTablet(TWindow *menu, const char *identbranch, int asciicode, bool akeepkey)
:
Area(menu, identbranch, asciicode, akeepkey)
{
	STACKTRACE;

	//init_pos_size(&backgr, "backgr");

	if (size.x != 0)
		drawarea = create_bitmap_ex(bitmap_color_depth(mainwindow->drawarea), iround(size.x), iround(size.y));
	else
		drawarea = 0;

}


AreaTablet::~AreaTablet()
{
	//if (backgr)
	//	destroy_bitmap(backgr);
	del_bitmap(&drawarea);
}


void AreaTablet::animate()
{
	STACKTRACE;
	blit(backgr, drawarea, 0, 0, 0, 0, iround(size.x), iround(size.y));

	subanimate();

	draw(drawarea);
	//	draw(backgr);
}


void AreaTablet::subanimate()
{
	STACKTRACE;
	// nothing; you can put extra drawing commands here, stuff that's drawn onto
	// the background before being blitted onto the reserved area.
}


bool AreaTablet::hasmouse()
{
	STACKTRACE;
	// the first rough check whether it's in the boxed bitmap area
	return GraphicButton::hasmouse(backgr);
}


bool AreaTablet::isvalid()
{
	STACKTRACE;
	return backgr != 0;
};

// A button which can be in 2 states (on/off)
// name#on.bmp
// name#off.bmp

SwitchButton::SwitchButton(TWindow *menu, const char *identbranch, int asciicode, bool initialstate)
:
GraphicButton(menu, identbranch, asciicode)
{
	STACKTRACE;
	init_pos_size(&bmp_on, "on");
	bmp_off = getbmp("off");

	state = initialstate;
}


SwitchButton::~SwitchButton()
{
	del_bitmap(&bmp_on);
	del_bitmap(&bmp_off);
}


void SwitchButton::draw_default()
{
	STACKTRACE;
	if (state)
		draw(bmp_on);
	else
		draw(bmp_off);
}


void SwitchButton::draw_focus()
{
	STACKTRACE;
	draw_default();
	draw_rect_fancy();
}


// is the same as focus, cause a switch cannot be selected all the time !!
void SwitchButton::draw_selected()
{
	STACKTRACE;
	draw_focus();
}


void SwitchButton::calculate()
{
	STACKTRACE;
	GraphicButton::calculate();

	// determine if the state of the button is being changed by (some) interaction:
	if (flag.left_mouse_press)
		state = !state;			 // switch state.
}


bool SwitchButton::hasmouse()
{
	STACKTRACE;
	// the first rough check whether it's in the boxed bitmap area
	return GraphicButton::hasmouse(bmp_on);
}


bool SwitchButton::isvalid()
{
	STACKTRACE;
	return bmp_on != 0;
};

GhostButton::GhostButton(TWindow *menu)
:
EmptyButton(menu)
{
	STACKTRACE;
	passive = true;
}


GhostButton::~GhostButton()
{
}


ScrollBar::ScrollBar(TWindow *menu, char *identbranch)
:
AreaTablet(menu, identbranch, 255)
{
	STACKTRACE;
	relpos = 0.0;				 // between 0 and 1

	button = getbmp("button");

	if (button) {
		bwhalf = button->w/2;
		bhhalf = button->h/2;
	} else {
		bwhalf = 0;
		bhhalf = 0;
	}

	if (size.y >= size.x)
		direction = ver;		 // vertically oriented
	else
		direction = hor;		 // horizontally oriented

	if (direction == ver) {
		pmin = bhhalf;
		pmax = iround(size.y - bhhalf);
	} else {
		pmin = bwhalf;
		pmax = iround(size.x - bwhalf);
	}

	pbutton = pmin;
}


ScrollBar::~ScrollBar()
{
	del_bitmap(&button);
}


void ScrollBar::handle_lhold()
{
	STACKTRACE;
	if (direction == ver)
								 // mouse pos relative in the little bar area
		pbutton = iround(mainwindow->mpos.y - pos.y);
	else
		pbutton = iround(mainwindow->mpos.x - pos.x);

	if (pbutton < pmin)
		pbutton = pmin;

	if (pbutton > pmax)
		pbutton = pmax;

	relpos = double(pbutton - pmin) / double(pmax - pmin);
}


void ScrollBar::subanimate()
{
	STACKTRACE;
	AreaTablet::subanimate();

	if (direction == ver)
		masked_blit(button, drawarea, 0, 0, iround(size.x/2 - bwhalf), iround(pbutton-bhhalf), iround(button->w), iround(button->h));
	else
		masked_blit(button, drawarea, 0, 0, iround(pbutton-bwhalf), iround(size.y/2 - bhhalf), button->w, button->h);
}


void ScrollBar::setrelpos(double arelpos)
{
	STACKTRACE;
	if (relpos == arelpos)
		return;

	relpos = arelpos;

	// also update the button position to reflect this change
	pbutton = pmin + iround(relpos * (pmax - pmin));
}


void ScrollBar::calculate()
{
	STACKTRACE;
	AreaTablet::calculate();
}
