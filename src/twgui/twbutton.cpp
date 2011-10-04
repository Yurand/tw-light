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

REGISTER_FILE

#include "twbutton.h"
#include "twwindow.h"
#include "other/twconfig.h"

EmptyButton::EmptyButton(TWindow *menu, const char *identbranch, int asciicode, bool akeepkey)
{
	STACKTRACE;
	prev = 0;
	next = 0;
	selected = false;
	button_event = 0;

	menu->add(this);

	trigger_key = asciicode;
	//trigger_keepkey = akeepkey;
	trigger_keydetection = false;

	passive = false;			 // by default, I assume that a mouse-touch or a key-press can cause some action

	// let the area know to what collection it belongs
	mainwindow = menu;

								 // a data block
	strcpy(ident, mainwindow->ident);
	strcat(ident, "/");
	if (identbranch)
								 // stuff within the data block
		strcat(ident, identbranch);

	//mouse.bmp.init(TWindow->bmp(strmouse));
	// use the default menu-mouse, defined for area-reserve:
	//	mouse.bmp.init(mainwindow->mouse.bmp.newmousebmp);
	// (you can override this with a similar call but using another bitmap later on, if required).

	size = 0;
	pos = 0;
}


EmptyButton::~EmptyButton()
{
	/* NO !! should be handled by the main window.
	// nothing...
	if (prev)
		prev->next = next;
	if (next)
		next->prev = prev;
		*/
}


bool EmptyButton::hasmouse()
{
	STACKTRACE;
	return (mainwindow->mpos.x >= pos.x &&
		mainwindow->mpos.y >= pos.y &&
		mainwindow->mpos.x < (pos+size).x &&
		mainwindow->mpos.y < (pos+size).y);
}


bool EmptyButton::haskey()
{
	STACKTRACE;
	return flag.haskey;
}


bool EmptyButton::haskeypress()
{
	STACKTRACE;
	return flag.haskeypress;
}


// this should be checked only once per iteration; that's the assumption
// made for the detection flag
void EmptyButton::check_key()
{
	STACKTRACE;
	// this is usually the case.
	flag.haskey = false;
	flag.haskeypress = false;

	// first check, if this button has focus, and the general enter-key is
	// pressed to activate it.
	if (flag.focus) {
		// if the button is hit
		//if ( keyhandler.pressed('\n') )
		if ( keyhandler.keyhit[KEY_ENTER]) {
			flag.haskey = true;
			flag.haskeypress = true;
			return;
		}

		// if the button is kept pressed
		if ( keyhandler.keynew[KEY_ENTER]) {
			flag.haskey = true;
			return;
		}
	}

	// check if there's a key-trigger: it's a short-cut key, which overrides normal
	// mouse/tab navigation.
	/*
	bool foundkey = keyhandler.pressed(trigger_key);

	if ( foundkey )
	*/
	// if the short-cut key-combo is pressed, then ...
	if (keyhandler.keyhit[unmapkey1(trigger_key)] &&
	keyhandler.keynew[unmapkey2(trigger_key)]) {

		// either a key-hold, or a key-press:
		//if (trigger_keepkey || ( (!trigger_keepkey) && (!trigger_keydetection) ))
		flag.haskey = true;

		if (!trigger_keydetection)
			flag.haskeypress = true;

								 // the key is being held.
		trigger_keydetection = true;

	} else
	trigger_keydetection = false;// the key isn't being touched.

}


void EmptyButton::handle_focus()
{
	STACKTRACE;
	//mainwindow->setfocus(this);

	//focus = 1;
	flag.focus = true;

	//	if (button_event)
	//		button_event->handle_focus();
}


void EmptyButton::handle_defocus()
{
	STACKTRACE;
	flag.focus = false;

	//	if (button_event)
	//		button_event->handle_defocus();
}


void EmptyButton::check_focus()
{
	STACKTRACE;
	flag.lastfocus = flag.focus;

	if (haskey()) {
		if (!flag.focus)
			mainwindow->setfocus(this);
	}

	// if the mouse has moved, then check (new) mouse position to see if
	// the current button has gained or lost focus (so that if the mouse is not
	// moving, the keys can be used to check this).
	else if (Tmouse.vx() != 0 || Tmouse.vy() != 0) {
		if ( hasmouse() ) {
			if (!flag.focus)
				mainwindow->setfocus(this);

		} else {

			if (flag.focus)
				handle_defocus();
		}
	}
}


// This routine is very important, since it can deliver an instant up-date
// when the menu as a whole has suddenly lost focus (mouse can move away in
// an undetected period of time, eg. an independent game loop), and the usual
// calls to calculate aren't made...
void EmptyButton::handle_menu_focus_loss()
{
	STACKTRACE;
	selected = 0;
	//update_key();
	flag.reset();
}


void EmptyButton::calculate()
{
	STACKTRACE;

	// copy information from the TWindow manager
	//	update_mouse();
	//	update_key();

	if (!passive) {
		check_key();

		check_focus();
	}

	subcalculate();				 // users can put stuff in here, which is then calculated just
	// before the mouse handles, but just after the mouse/ key updates.

	// check if button 1 (left) releases -> clicks for this item

	flag.left_mouse_press = false;
	flag.left_mouse_release = false;
	flag.right_mouse_press = false;
	flag.left_mouse_hold = false;

	if (flag.focus) {

		if ( Tmouse.left.press() || haskeypress() ) {
			flag.left_mouse_press = true;
			handle_lpress();
			if (button_event)
				button_event->handle_main_press();
		}

		if (Tmouse.left.hold() || haskey()) {
			flag.left_mouse_hold = true;
			handle_lhold();
			if (button_event)
				button_event->handle_main_hold();
		}

		if (Tmouse.left.release() && flag.focus) {
			flag.left_mouse_release = true;
			handle_lrelease();
			//			if (button_event)
			//				button_event->handle_lrelease();

		}

		if (Tmouse.right.press()) {
			flag.right_mouse_press = true;
			handle_rpress();
			if (button_event)
				button_event->handle_special_press();
		}

		if (Tmouse.right.hold()) {
			flag.right_mouse_hold = true;
			handle_rpress();
			if (button_event)
				button_event->handle_special_hold();
		}

		// change the mouse pointer if needed, if it has the mouse:
		//		mouse.bmp.set();
		// note, "restore" is done by default by the area manager; this sets
		// it back to the custom mouse pointer, if needed.
	}
}


void EmptyButton::animate()
{
	STACKTRACE;
}


EmptyButton::flag_struct::flag_struct()
{
	STACKTRACE;
	// these flags are updated by the area's calculate function.
	//	focus = 0;
	//	left_mouse_press = left_mouse_release = left_mouse_hold = 0;
	//	right_mouse_press = right_mouse_release = right_mouse_hold = 0;
	reset();
}


void EmptyButton::flag_struct::reset()
{
	STACKTRACE;
	// these flags are updated by the area's calculate function.
	focus = 0;
	left_mouse_press = left_mouse_release = left_mouse_hold = 0;
	right_mouse_press = right_mouse_release = right_mouse_hold = 0;
	haskey = haskeypress = 0;
}


bool EmptyButton::isvalid()
{
	STACKTRACE;
	return true;
}


// that was the general part ; now a real rectangular area.
// (similarly, you could define triangular and other types
// of areas).

GraphicButton::GraphicButton(TWindow *menu, const char *identbranch, int asciicode, bool akeepkey)
:
EmptyButton(menu, identbranch, asciicode, akeepkey)
{
	STACKTRACE;
	//	mainwindow->scalepos(&x, &y);
}


void GraphicButton::locate(int ax, int ay)
{
	STACKTRACE;
	pos.x = ax;
	pos.y = ay;
	mainwindow->scalepos(&pos);
}


GraphicButton::~GraphicButton()
{
}


void GraphicButton::animate()
{
	STACKTRACE;
	// check if button 1 (left) releases -> clicks for this item
	// unconditional drawing:
	if (flag.focus) {
		if (flag.left_mouse_hold)
			draw_selected();
		else
			draw_focus();

	} else
	draw_default();

}


void GraphicButton::draw_default()
{
	STACKTRACE;
}


void GraphicButton::draw_focus()
{
	STACKTRACE;
}


void GraphicButton::draw_selected()
{
	STACKTRACE;
}


// check if it's a visible pixel
bool check_visibility(BITMAP *bmp, int x, int y)
{
	STACKTRACE;
	if (!bmp)
		return false;

	if (x < 0 || x >= bmp->w || y < 0 || y >= bmp->h)
		return false;

	int col;
	col = getpixel(bmp, x, y);
	if (col != makecol(255,0,255))
		return true;
	else
		return false;
}


// check a square area to see if it has the mouse on it.
bool GraphicButton::hasmouse(BITMAP *bmpref)
{
	STACKTRACE;
	// first, check the square bitmap area
	if (EmptyButton::hasmouse()) {
		// then, check the bmp if there's a pixel touched...
		return check_visibility(bmpref, mainwindow->mpos.x - pos.x, mainwindow->mpos.y - pos.y);
	} else
	return false;
}


void GraphicButton::draw_rect()
{
	STACKTRACE;
	BITMAP *b = mainwindow->drawarea;
	int x1, y1, x2, y2;

	x1 = pos.x;
	y1 = pos.y;
	x2 = (pos+size).x - 1;
	y2 = (pos+size).y - 1;

	// draw something simple:

	int D = 4;					 // width of the rectangle
	D *= iround(mainwindow->scale - 1);
	if (D < 0)
		D = 0;

	while (D >= 0) {
		if (x2-x1 > 2*D && y2-y1 > 2*D)
			rect(b, x1+D, y1+D, x2-D, y2-D, makecol(200,200,200));
		--D;
	}
}


int rect_fancy_getcolor2(double phase, double L, double Ltot)
{
	STACKTRACE;
	unsigned char r, g, b;

	r = (unsigned char)(128 + 127 * sin(phase + PI2 * L/Ltot));
	g = (unsigned char)(128 + 127 * sin(phase + 0.3*PI + PI2 * L/Ltot));
	b = (unsigned char)(128 + 127 * sin(phase + 0.7*PI + PI2 * L/Ltot));

	return makecol(r, g, b);
}


void GraphicButton::draw_rect_fancy()
{
	STACKTRACE;
	BITMAP *b = mainwindow->drawarea;
	int x1, y1, x2, y2;

	x1 = pos.x;
	y1 = pos.y;
	x2 = (pos+size).x - 1;
	y2 = (pos+size).y - 1;

	int D = 2;					 // width of the rectangle
								 // cause 0 also counts as 1...
	D = iround(D * mainwindow->scale) - 1;
	if (D < 0)
		D = 0;

	int ix, iy;

								 //offset angle
	double phase = mainwindow->menu_time * 1E-3 * PI;

	int L;

	while (D >= 0) {
		double Ltotal = (size.x-2*D) * (size.y-2*D);

		if (Ltotal <= 0)
			break;

		L = 0;

		iy = y1 + D;
		ix = x1 + D - 1;

		while(ix < x2-D ) {
			++ix;
			putpixel(b, ix, iy, rect_fancy_getcolor2(phase, L, Ltotal));
			++L;
		}

		while(iy < y2-D ) {
			++iy;
			putpixel(b, ix, iy, rect_fancy_getcolor2(phase, L, Ltotal));
			++L;
		}

		while(ix > x1+D ) {
			--ix;
			putpixel(b, ix, iy, rect_fancy_getcolor2(phase, L, Ltotal));
			++L;
		}

		while(iy > y1+D+1 ) {
			--iy;
			putpixel(b, ix, iy, rect_fancy_getcolor2(phase, L, Ltotal));
			++L;
		}

		--D;
	}

}


void GraphicButton::draw_boundaries(BITMAP *bmpref)
{
	STACKTRACE;
	BITMAP *b = mainwindow->drawarea;

	int i, j;
	int W = 2;
	W = iround(W * mainwindow->scale) - 1;

	int L = 0;
	int Ltotal = 5 * sqrt( (double)bmpref->w * bmpref->h );
								 //offset angle
	double phase = mainwindow->menu_time * 1E-3 * PI;

	for ( j = -W; j < bmpref->h + W; ++j ) {
		for ( i = -W; i < bmpref->w + W; ++i ) {

			if (!check_visibility(bmpref, i, j)) {
				int wx, wy;
				for ( wy = -W; wy <= W; ++wy ) {
					for ( wx = -W; wx <= W; ++wx ) {

						if (check_visibility(bmpref, i+wx, j+wy)) {
								 //makecol(255,255,255));
							putpixel(b, i+pos.x, j+pos.y,
								rect_fancy_getcolor2(phase, L, Ltotal));
						}
					}
				}
				++L;
			}

		}
	}
}


/*
NEED TO REWRITE THIS
(should be completely handled by the main window.
*/
void GraphicButton::locate_by_backgr(const char *strid)
{
	STACKTRACE;
	char stron[128];
	strcpy(stron,  ident);
	strcat(stron,  strid);

	BITMAP *tmp;
	bool k;
	char check_char[128];

	strcpy(check_char, stron);
	strcat(check_char, ".bmp");

	//tmp = (BITMAP*) find_datafile_object(TWindow->datafile, check_char)->dat;
	tmp = load_bitmap(check_char, 0);
	if (!tmp) {
		tw_error("Could not find the comparison bmp in the datafile");
	}

	// first, load the last known position from the ini file
	// set the config file, to guarantee that the correct ini file is used ...
	tw_set_config_file(mainwindow->configfilename);
	char strx[256], stry[256];

	strcpy(strx, stron);
	strcat(strx, "_x");
	strcpy(stry, stron);
	strcat(stry, "_y");

	pos.x = get_config_int(0, strx, 0);
	pos.y = get_config_int(0, stry, 0);
	Vector2 oldpos = pos;

	k = mainwindow->search_bmp_location(tmp, &pos);

	// if the position has changed, then set the new position...
	if (pos != oldpos) {
		set_config_int(0, strx, pos.x);
		set_config_int(0, stry, pos.y);
	}

	mainwindow->scalepos(&pos);	 // is normally called by the other init();

	if (!k && mainwindow->autoplace) {
		tw_error("Could not find the bmp on the background image");
	}
}


// obtain a bitmap, specific to this "object" :
BITMAP *GraphicButton::getbmp(const char *name)
{
	STACKTRACE;
	char streditbox[128];
	strcpy(streditbox,  ident);
	strcat(streditbox,  name);

	// a background image is needed of course.
	return mainwindow->bmp(streditbox);
}


// obtain a bitmap using "absolute" path, so that it can come from anywhere...
BITMAP *GraphicButton::getbmp_nobutton(char *name)
{
	STACKTRACE;
	// a background image is needed of course.
	return mainwindow->bmp(name);
}


void GraphicButton::init_pos_size(BITMAP **bmp_default, const char *idstr)
{
	STACKTRACE;
	*bmp_default = getbmp(idstr);

	if (*bmp_default) {
								 // note: it's already scaled on initialization.
		size.x = (*bmp_default)->w;
		size.y = (*bmp_default)->h;
	} else {
		//tw_error("Could not initialize Button bitmap");
		size = 0;
	}

	if ( *bmp_default )
		locate_by_backgr(idstr);
	//	else
	//	{
	//		tw_error("No default Button defined!");
	//	}
}


bool GraphicButton::draw(BITMAP *b)
{
	STACKTRACE;
	if (b) {
		masked_blit(b, mainwindow->drawarea, 0, 0, pos.x, pos.y, b->w, b->h);
		return true;

	} else
	return false;
}
