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
//#include "melee/mview.h"

REGISTER_FILE

#include "twbutton.h"
#include "twpopup.h"

// here, the background is initialized (by mainwindow)
Popup::Popup(const char *ident, int xcenter, int ycenter, BITMAP *outputscreen)
:
TWindow(ident, xcenter, ycenter, outputscreen)
{
	STACKTRACE;
	movingthereserve = false;

	center(xcenter, ycenter);	 // center around this position, in relative coordinates

	returnvalueready = false;
}


Popup::~Popup()
{
}


void Popup::doneinit()
{
	STACKTRACE;
	TWindow::doneinit();

	hide();
}


// this calls close with return value
// (the general close is ok, it closes and hides the menu).
void Popup::check_end()
{
	STACKTRACE;
	// nothing
}


void Popup::calculate()
{
	STACKTRACE;
	// must be called before the "return", otherwise focus-loss isn't called
	TWindow::calculate();

	if (disabled)
		return;

	// also, check if the mouse is clicked, and dragged - in that case, move the menu:
	// but, only if it's clicked in a part that is not a button ;)

	if ( Tmouse.left.press() && grabbedmouse ) {
		EmptyButton *current;
		current = button_first;
		while (current) {
								 //flag.focus)
			if (current->hasmouse())
				break;

			current = current->next;
		}

		// if there are no buttons that have the focus of the mouse, then movement is allowed
		if ( !current )
			movingthereserve = true;
	}

	if (!(Tmouse.left.hold() || Tmouse.left.press()) )
		movingthereserve = false;

	if ( movingthereserve ) {
		int dx, dy;
		dx = Tmouse.vx();
		dy = Tmouse.vy();
		x += dx;
		y += dy;

		// update the relative mouse position also
		mpos.x = Tmouse.pos.x - x;
		mpos.y = Tmouse.pos.y - y;

	}

	check_end();
}


void Popup::close(int areturnstatus)
{
	STACKTRACE;
	hide();

	returnstatus = areturnstatus;
	returnvalueready = true;
}


// call this once (true) to probe if the popup has completed its function; it
// resets its "ready" status as well.
bool Popup::ready()
{
	STACKTRACE;
	if (returnvalueready) {
		returnvalueready = false;
		return true;
	} else
	return false;
}


// call this to get a int-value from some selection mechanism (eg a list)
int Popup::getvalue()
{
	STACKTRACE;
	//	if (!returnvalueready)
	//		return -1;				// otherwise it's undefined

	ready();
	return returnstatus;
}


// restore the ability to do calculations and actions
void Popup::enable()
{
	STACKTRACE;
	TWindow::enable();
	returnvalueready = false;
}


void Popup::newscan()
{
	STACKTRACE;
	show();						 // (more general than enable)
	ready();
}


// This popup is invoked by a push-button somewhere. I'm not going to create some
// special push button type for that, I can just as well let this (new) class
// access the state of that general button type.

PopupT::PopupT(const char *identbranch, int axshift, int ayshift, BITMAP *outputscreen)
:
Popup(identbranch, axshift, ayshift, outputscreen)
{
	STACKTRACE;
	trigger = 0;

	init_components(identbranch);

	xshift = iround(axshift * scale);
	yshift = iround(ayshift * scale);
}


PopupT::PopupT(EmptyButton *atrigger, const char *identbranch, int axshift, int ayshift)
:
Popup(identbranch,
axshift, ayshift,
atrigger->mainwindow->screen)
{
	STACKTRACE;
	trigger = atrigger;

	init_components(identbranch);

	xshift = iround(axshift * scale);
	yshift = iround(ayshift * scale);
}


void PopupT::init_components(const char *id)
{
	STACKTRACE;
	returnvalueready = false;

	// and default option settings:
	option.disable_othermenu = true;
	option.place_relative2mouse = true;
	option.place_relative2window = false;

	close_on_defocus = true;
}


PopupT::~PopupT()
{
	/*
	if (left)
		delete left;
	if (right)
		delete right;
	if (up)
		delete up;
	if (down)
		delete down;

	if (scrollhor)
		delete scrollhor;
	if (scrollvert)
		delete scrollvert;
	if (scroll_control)
		delete scroll_control;
		*/

}


void PopupT::calculate()
{
	STACKTRACE;
	// must be called before the "return", otherwise focus-loss isn't called
	Popup::calculate();

	// first, do some tests for activation, that should always be done.
	// if you press the trigger, this menu is activated
	// (if a trigger isn't needed, but some more forcefull external control,
	// trigger==0)
	if ( trigger && disabled && trigger->flag.left_mouse_hold ) {
		if (option.disable_othermenu) {
			trigger->mainwindow->disable();
			trigger->mainwindow->handle_focus_loss();
		}
		this->show();			 // this also resets the mouse position and old position, so that it has
		// the most recent value (since it hasn't been updated by the
		// calculate funtion for a while).
		this->focus();

		// but where exactly : near the mouse !!
		if (option.place_relative2mouse) {
			x = iround(trigger->mainwindow->x + trigger->mainwindow->mpos.x + xshift);
			y = iround(trigger->mainwindow->y + trigger->mainwindow->mpos.y + yshift);
		}
		else if (option.place_relative2window) {
			x = trigger->mainwindow->x + xshift;
			y = trigger->mainwindow->y + yshift;
		}
		// otherwise don't do anything, just leave the window there.

		// don't let the menu go off-screen:

		if ( x+W > screen->w )
			x = screen->w - W;
		if ( x < 0 )
			x = 0;

		if ( y+H > screen->h )
			y = screen->h - H;
		if ( y < 0 )
			y = 0;

	}

	if (disabled)
		return;

	// must be called before the "return", otherwise focus-loss isn't called
	//	mainwindow::calculate();

	check_end();
}


// if you leave the area, it's auto-hiding -- but only if you're not keeping the
// left-key pressed, which means you're moving the thing - when you move *fast*, the
// mouse pointer can exit the area before the area follows the mouse ...

void PopupT::handle_focus_loss()
{
	STACKTRACE;
	/*
	if (!close_on_defocus)
		return;

	if (Tmouse.left.hold())
	{
		hasfocus = true;
		return;
	}
	*/

	//	check_end();
}


void PopupT::close(int areturnstatus)
{
	STACKTRACE;
	if (trigger) {
		// give back focus/control to the window that called this window.
		trigger->mainwindow->show();
		trigger->mainwindow->focus();
	}

	Popup::handle_focus_loss();
	this->hide();

	returnstatus = areturnstatus;
	returnvalueready = true;
}


// you can (and should) replace this routine with one suited for your own purposes
void PopupT::check_end()
{
	STACKTRACE;
	//	if (!hidden && !hasfocus && close_on_defocus)	// to prevent this from being called twice, due to recursive call to handle_focus() in hide()
	//	{
	//tw_error("PopupList : Losing focus !!");

	//		close(-1);		// nothing ;)
	//	}
}
