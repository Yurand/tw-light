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
#include "other/twconfig.h"

REGISTER_FILE

#include "twwindow.h"

// this destroys the bitmap (if it exists i.e. isn't set to 0),
// and resets the pointer to 0
void del_bitmap(BITMAP **bmp)
{
	STACKTRACE;
	if (*bmp)
		destroy_bitmap(*bmp);
	*bmp = 0;
}


// make a backup of the old screen
// note, it's scaled to maximum W,H=1.0, 1.0 being the screen width
// default_W is the screen width for which all buttons in the menu are developed, and
// this is used to scale them to other screen resolutions.
TWindow::TWindow(const char *identbase, int dx, int dy, BITMAP *outputscreen, bool vidwin)
{
	STACKTRACE;
	prev = 0;
	next = 0;

	button_first = 0;
	button_last = 0;
	button_focus = 0;

	grabbedmouse = false;
	mpos = 0;

	// from the scaled thing, calculate the actual size on the current screen

	screen = outputscreen;
	if (!screen) {
		tw_error("Something is wrong with the screen!!");
	}

	strcpy(ident, identbase);
	//	strcat(ident, "_DAT");

	// from this, derive certain file names which you (may) need:
	char backgrname[128], mousename[128], infoname[128];
	strcpy(backgrname, ident);

	strcat(backgrname, "/backgr");
	strcpy(mousename, ident);
	strcat(mousename, "/mouse");
	strcpy(infoname, ident);
	strcat(infoname, "/info.txt");

	//	datafile = load_datafile(datafilename);
	//	if (!datafile)
	//	{
	//		tw_error("TWindow: Could not load datafile");
	//	}

	// read the default resolution from the info.txt file :
	//	DATAFILE *infodat = find_datafile_object(datafile, infoname);
	//	set_config_data( (char*) infodat->dat, infodat->size);
	strncpy(configfilename, infoname, sizeof(configfilename));
	tw_set_config_file( configfilename );

	default_W = get_config_int(0, "res", 800);
	autoplace = get_config_int(0, "autoplace", 1);
	//default_W = def_W;
	scale = double(screen->w) / double(default_W);

	disabled = false;
	hidden = false;

	// this is the background, in scaled form
	backgr = bmp(backgrname, vidwin);

	// for "automatic placement" purposes, which can be used if the "default buttons"
	// are an exact copy of the "background" image (you can design a menu and the buttons
	// by drawing (or copy/paste) them on a background. In the game, the separate buttons
	// will have more or less (?) identical size, and overwrite the background image.

	char check_char[128];
	strcpy(check_char, "none");
	backgr_forsearch = 0;
	if (backgr && autoplace != 0) {
		//BITMAP *b;
		strcpy(check_char, backgrname);
		strcat(check_char, ".bmp");

		//b = (BITMAP*) find_datafile_object(datafile, check_char)->dat;
		backgr_forsearch = load_bitmap(check_char, 0);
		//backgr_forsearch = (BITMAP*) create_bitmap_ex(bitmap_color_depth(b), b->w, b->h);
		//blit(b, backgr_forsearch, 0, 0, 0, 0, b->w, b->h);
	}

	if (backgr) {
		x = iround(dx * scale);
		y = iround(dy * scale);
		W = backgr->w;			 // background is already scaled !!
		H = backgr->h;
	} else {
		tw_error("TWindow: No background defined !!");
	}

	// back up ... well, in case of a changing background, this is hardly useful ...
	//originalscreen = create_bitmap_ex(bitmap_color_depth(screen), W, H);
	//blit(screen, originalscreen, x, y, 0, 0, W, H);

	// used for drawing

	if (vidwin)
		drawarea = create_video_bitmap(W, H);

	if (!(vidwin && drawarea))
		drawarea = create_bitmap_ex(bitmap_color_depth(screen), W, H);

	if (is_same_bitmap(backgr, drawarea) || !backgr || !drawarea || !screen) {
		tw_error("oh my!");
	}

	//Nareas = 0;

	// also, initialize default mouse ptr for use inside the menu area, and for
	// all the items:
	//	Tmouse.bmp.init(bmp(mousename));

	//	clear_keys();

	menu_starttime = get_time();
	menu_time = menu_starttime;

	exclusive = false;
	layer = 0;
}


// ok, this provides a working space

TWindow::~TWindow()
{
	// remove this window item from the list of windows
	// also, delete other windows that are in the same list.
	if (prev) {
		prev->next = 0;			 // so that "prev" doesn't know "this" exists
		delete prev;
	}
	if (next) {
		next->prev = 0;			 // so that "next" doesn't know "this" exists
		delete next;
	}

	//if (originalscreen)	del_bitmap(originalscreen);
	del_bitmap(&drawarea);
	del_bitmap(&backgr);

	// of course ;) but it's worth checking again, since it's not required to call this,
	// it's optional
	doneinit();

	//del_bitmap(backgr_forsearch);
	//if (datafile)		unload_datafile(datafile);

	// delete associated buttons
	EmptyButton *button;
	button = button_first;
	while (button) {
		EmptyButton *b;
		b = button;
		button = button->next;
		if (button)
			button->prev = 0;	 // to avoid disaster as the next button is deleted...

		delete b;
	}

}


// list functions, to handle a set of related windows. You just have to call one
// of them, and all of them will be handled.

// it's not really a tree, but a list .. oh well.

TWindow *TWindow::tree_root()
{
	STACKTRACE;
	TWindow *current;
	current = this;
	while (current->prev)
		current = current->prev;

	return current;
}


TWindow *TWindow::tree_last()
{
	STACKTRACE;
	TWindow *current;
	current = this;
	while (current->next)
		current = current->next;

	return current;
}


void TWindow::tree_calculate()
{
	STACKTRACE;
	TWindow *current;
	current = tree_root();

	// but first, update the keys and the mouse

	keyhandler.update();
	Tmouse.update();

	// if this window is "exclusive", other windows cannot be accessed while this one has focus
	if (current->exclusive && !current->disabled) {
		current->calculate();
		return;
	}

	while (current) {
		current->calculate();
		current = current->next;
	}

}


void TWindow::tree_animate()
{
	STACKTRACE;
	TWindow *current;
	current = tree_last();

	// the fist one should plot on top, and therefore, should be plotted last ...

	acquire_bitmap(screen);

	while (current) {
		current->animate();
		current = current->prev;
	}

	release_bitmap(screen);
}


void TWindow::focus()
{
	STACKTRACE;
	// no need if this already is the root ... cause then it already has the focus.
	if (!prev)
		return;

	TWindow *root;
	root = tree_root();

	// check layers, you're not allowed to focus in a "lower" layer
	while (root && root->layer < layer)
		root = root->next;

	if (!root || root == this)	 // apparently you're alone/ already focus in your own layer ...
		return;

	// remove "this" from the list
	if (prev)
		prev->next = next;
	if (next)
		next->prev = prev;

	// insert "this" at the front
	// (note that root has no "prev" originally)
	root->prev = this;
	next = root;
	prev = 0;					 //root->prev;
}


void TWindow::tree_setscreen(BITMAP *scr)
{
	STACKTRACE;
	TWindow *current;
	current = tree_root();

	while (current) {
		current->setscreen(scr);
		current = current->next;
	}
}


// add a window at the end of the window list
void TWindow::add(TWindow *newwindow)
{
	STACKTRACE;
	TWindow *current;
	current = this;
	while (current->next)
		current = current->next;

	current->next = newwindow;
	newwindow->prev = current;
}


// call all the clean up routines for all the menus
void TWindow::tree_doneinit()
{
	STACKTRACE;
	TWindow *current;
	current = tree_root();

	while (current) {
		current->doneinit();
		current = current->next;
	}
}


bool TWindow::hasfocus()
{
	STACKTRACE;
	// only the root has primary focus...
	return (prev != 0);
}


// --------------------- DEFINING AREAS -------------------
// areas can be places where special actions can take place (graphically)
// but first, these places need to be defined, and their (functional) link
// to those possible actions needs to be shown.

// divide the screen in different areas, each of which has the basic properties
// of focus, defocus, and mouse status.

// this checks whether a area (usually a box) has focus or not depending on I/O (mouse, keyboard)

// a menu superstructure ... to manage a bunch of related areas ... in a reserved area/ region

BITMAP* TWindow::bmp(char *bmpname, bool vidmem)
{
	STACKTRACE;
	BITMAP *bmp, *tmpbmp;
	int bpp;

	bpp = bitmap_color_depth(screen);
	//bpp = 32;
	//bmp = find_datafile_bmp(datafile, bmpname);	// this adds _BMP

	char objname[128];

	if (strlen(ident) > 120) {
		tw_error("string exceeds max length");
	}

	strcpy(objname, bmpname);

	char *tmp;
	tmp = &objname[strlen(objname)-4];
	if (strcmp(tmp, ".bmp"))	 // if the extension isn't already .bmp
		strcat(objname, ".bmp"); // default extension for .bmp files.

	tmpbmp = load_bitmap(objname, 0);
	bmp = clone_bitmap(bpp, tmpbmp, scale, vidmem);
	del_bitmap(&tmpbmp);

	return bmp;
}


void TWindow::doneinit()
{
	STACKTRACE;
	del_bitmap(&backgr_forsearch);
	//if (datafile)			unload_datafile(datafile);
}


void TWindow::setscreen(BITMAP *scr)
{
	STACKTRACE;
	screen = scr;
}


void TWindow::center(int xcenter, int ycenter)
{
	STACKTRACE;
	// put back the background
	//blit(originalscreen, screen, 0, 0, x, y, W, H);

	// move
	x = iround(xcenter*scale) - W / 2;
	y = iround(ycenter*scale) - H / 2;

	// read the new background
	//blit(screen, originalscreen, x, y, 0, 0, W, H);
}


void TWindow::center_abs(int xcenter, int ycenter)
{
	STACKTRACE;
	// put back the background
	//blit(originalscreen, screen, 0, 0, x, y, W, H);

	// move
	x = xcenter - W / 2;
	y = ycenter - H / 2;

	// read the new background
	//blit(screen, originalscreen, x, y, 0, 0, W, H);
}


void TWindow::center()
{
	STACKTRACE;
	ASSERT(screen)
		center(screen->w/2, screen->h/2);
}


// changes the "disabled" flag, and possibly does other stuff as well?
void TWindow::enable()
{
	STACKTRACE;
	disabled = false;
	//	mouse.reset();		// reset the mouse to the newest values.
}


void TWindow::disable()
{
	STACKTRACE;
	if (!disabled) {			 // to prevent possible recursive call (disable can be called within focus loss in some cases...)
		disabled = true;
		handle_focus_loss();	 // disables other flags, also of sub-menus
	}

}


void TWindow::show()
{
	STACKTRACE;
	hidden = false;				 // allow animation
	enable();					 // allow calculation
	focus();					 // bring this window to the front of the list, so that it draws on top of the rest
}


void TWindow::hide()
{
	STACKTRACE;
	hidden = true;
	disable();
	//blit(originalscreen, screen, 0, 0, 0, 0, W, H);
}


void TWindow::add(EmptyButton *newbutton)
{
	STACKTRACE;
	if (!button_first) {
		button_first = newbutton;
		button_last = newbutton;

	} else {
		button_last->next = newbutton;
		newbutton->prev = button_last;
		button_last = newbutton;
	}
}


void TWindow::rem(EmptyButton *newbutton)
{
	STACKTRACE;
	if (! newbutton)
		return;

	EmptyButton *nb;
	nb = newbutton;

	if (button_first == nb)
		button_first = nb->next;

	if (button_last == nb)
		button_last = nb->prev;

	if (nb->prev)
		nb->prev->next = nb->next;

	if (nb->next)
		nb->next->prev = nb->prev;

	//delete *newbutton;
}


bool TWindow::checkmouse()
{
	STACKTRACE;
	if (!(prev && prev->grabbedmouse)) {
		// If the mouse hasn't been "grabbed" by a window above this window

		int xrel, yrel;

		// this test is performed in relative coordinates (with respect to the (0,0) corner
		// of the reserved area).
		xrel = iround(mpos.x);
		yrel = iround(mpos.y);

		if (  xrel >= 0 && xrel < W &&
			yrel >= 0 && yrel < H
			&& getpixel(backgr, xrel, yrel) != makecol(255,0,255) )
			return true;
		else
			return false;

	} else
	return false;

}


void TWindow::setfocus(EmptyButton *newbutton)
{
	STACKTRACE;
	if (button_focus)
		button_focus->handle_defocus();

	button_focus = newbutton;
	if (button_focus)
		button_focus->handle_focus();
	// this is (or can be) called from within a button::handle_focus() subroutine,
	// therefore don't call that subroutine here.
}


void TWindow::calculate()
{
	STACKTRACE;

	if (disabled) {
		grabbedmouse = false;	 // just to be sure, that if a window is "closed", its value is reset
		return;
	}

	//	mouse.bmp.restore();	// resets the mouse pointer.
	//	mouse.update();			// stores current mouse settings (x,y,wheel,buttons)

	//	add_keys();

	update_time();

	// position relative in the reserve box area: 0,0 = top left of the draw_area

	mpos.x = Tmouse.pos.x - x;
	mpos.y = Tmouse.pos.y - y;

	bool oldgrab = grabbedmouse;
	grabbedmouse = checkmouse();

	if (grabbedmouse && !oldgrab)
		handle_focus();

	else if (!grabbedmouse && oldgrab)
		handle_focus_loss();

	// if there's no contact with the mouse, then you don't have to calculate anything
	// either, I suppose ... also, the keys should only apply to the focused window,
	// that would be logical or not ?
	// Except the shortcut key for a particular button perhaps ... hmm, difficult.

	if (prev && !grabbedmouse)	 // neither focused nor a mouse on top of it.
		return;

	// check the focused button
	if (keyhandler.keyhit[KEY_TAB]) {
		EmptyButton *lastfocus;
		lastfocus = button_focus;

		if (!keyhandler.keynew[KEY_LSHIFT]) {
			if (!lastfocus)
				lastfocus = button_last;

			if (!button_focus)
				button_focus = button_first;

			while (button_focus) {
				button_focus = button_focus->next;
				if (!button_focus)
					button_focus = button_first;

				if (button_focus == lastfocus || !button_focus->passive)
					break;
			}

		} else {

			if (!lastfocus)
				lastfocus = button_first;

			if (!button_focus)
				button_focus = button_last;

			while (button_focus) {
				button_focus = button_focus->prev;
				if (!button_focus)
					button_focus = button_last;

				if (button_focus == lastfocus || !button_focus->passive)
					break;
			}

		}

		// if you've changed buttons by tabbing, then let the old button
		// lose its focus, and let the new button gain focus.
		if (lastfocus != button_focus) {
			if (lastfocus)
				lastfocus->handle_defocus();

			if (button_focus)
				button_focus->handle_focus();
		}

		// you can also focus to another window, if you press control
		if (keyhandler.keynew[KEY_LCONTROL]) {
			if (next) {
				next->focus();
				keyhandler.update();
				Tmouse.update();
				// otherwise the next in line of calculations brings the following one to
				// focus as well ... so better to refreshen all keys and the mouse, and
				// start completely fresh for the new window.
			}
		}

	}

	// calculate all the buttons
	EmptyButton *button;
	button = button_first;
	while (button) {
		button->calculate();
		button = button->next;
	}

	return;
}


void TWindow::animate()
{
	STACKTRACE;

	if (hidden)
		return;

	scare_mouse();				 // otherwise it'll leave artifacts when things are drawn.

	// release for in-game drawing
	acquire_bitmap(drawarea);

	if (!disabled) {

		// draw the background
		// also copy transparent color!

		// this takes about 1 ms on geo's comp if it's in video mem
		blit(backgr, drawarea, 0, 0, 0, 0, W, H);

		// draw the buttons
		EmptyButton *button;
		button = button_first;
		while (button) {
			button->animate();
			button = button->next;
		}

		// update the main screen
		// ignore transparent color
		if (screen) {
			// this takes about 2 ms on geo's comp if it's in video mem
			masked_blit(drawarea, screen, 0, 0, x, y, W, H);
		}

	} else
	//draw_lit_sprite(drawarea, screen, x, y, makecol(100,100,100));
	masked_blit(drawarea, screen, 0, 0, x, y, W, H);

	release_bitmap(drawarea);

	unscare_mouse();
}


void TWindow::handle_focus()
{
	STACKTRACE;
	// bring it first in the list ...
	focus();
}


void TWindow::handle_focus_loss()
{
	STACKTRACE;
	//for (int i = 0; i < Nareas; ++i )
	//	area[i]->handle_menu_focus_loss();
	EmptyButton *button;
	button = button_first;
	while (button) {
		button->handle_menu_focus_loss();
		button = button->next;
	}
}


void TWindow::scalepos(int *ax, int *ay)
{
	STACKTRACE;
	(*ax) = iround( (*ax) * scale );
	(*ay) = iround( (*ay) * scale );
}


void TWindow::scalepos(Vector2 *apos)
{
	STACKTRACE;
	(apos->x) = iround( (apos->x) * scale );
	(apos->y) = iround( (apos->y) * scale );
}


bool matchimage(BITMAP *backgr, BITMAP *foregr, int i, int j)
{
	STACKTRACE;
	if (!(backgr && foregr))
		return false;

	if (!(backgr->w && foregr->w))
		return false;

	if (!(backgr->h && foregr->h))
		return false;

	// check if the image block at this position, matches that of the background
	int c1 = 0, c2 = 0;
	int m = 0, n;

	for (n = 0; n < foregr->h; ++n) {
		for (m = 0; m < foregr->w; ++m) {
			c1 = getpixel(foregr, m, n);
								 // only compare the non-masked part
			if (getr(c1) == 255 && getg(c1) == 0 && getb(c1) == 255) {
				c2 = c1;
				continue;
			}

			c2 = getpixel(backgr, i+m, j+n);
			if (c2 != c1)
				break;
		}
		if (c2 != c1)
			break;
	}

	if (m == foregr->w && n == foregr->h)
		return true;
	else
		return false;
}


bool TWindow::search_bmp_location(BITMAP *bmp_default, Vector2 *apos)
{
	STACKTRACE;
	ASSERT(bmp_default);
	ASSERT(apos);

	if (!backgr_forsearch)
		return false;

	// first, test the "apos" position for a match.
	if (matchimage(backgr_forsearch, bmp_default, iround(apos->x), iround(apos->y)))
		return true;			 // return without changing the position...

	// search by comparing pixels :
	int i, j;
	for (j = 0; j < backgr_forsearch->h; ++j) {
		for (i = 0; i < backgr_forsearch->w; ++i) {
			if (matchimage(backgr_forsearch, bmp_default, i, j)) {
				(apos->x) = i;
				(apos->y) = j;
				return true;
			}

			/*
			// check if the image block at this position, matches that of the background
			int c1 = 0, c2 = 0;
			int m = 0, n;

			ASSERT( bmp_default->h <= 0 );
			ASSERT( bmp_default->w <= 0 );

			for (n = 0; n < bmp_default->h; ++n)
			{
				for (m = 0; m < bmp_default->w; ++m)
				{
					c1 = getpixel(bmp_default, m, n);
					if (getr(c1) == 255 && getg(c1) == 0 && getb(c1) == 255)	// only compare the non-masked part
					{
						c2 = c1;
						continue;
					}

					c2 = getpixel(backgr_forsearch, i+m, j+n);
					if (c2 != c1)
						break;
				}
				if (c2 != c1)
					break;
			}

			if (m == bmp_default->w && n == bmp_default->h)
			{
				(apos->x) = i;
				(apos->y) = j;
				return true;
			}
			*/

		}
	}

	return false;
}


/*
void TWindow::focus2other(TWindow *other)
{
	STACKTRACE;
  STACKTRACE;
	disable();			// this menu won't generate signals now, but it'll be drawn still
	other->show();
	other->prev = this;
}

void TWindow::back2other()
{
	STACKTRACE;
  STACKTRACE;
	prev->enable();			// switch control back to the meleemenu
	hide();					// this menu is disabled, and won't be drawn
	prev = 0;
}
*/

void TWindow::update_time()
{
	STACKTRACE;
	menu_time = get_time() - menu_starttime;
}


/*
void TWindow::clear_keys()
{
	STACKTRACE;
  STACKTRACE;
	keybuff_count = 0;
}

void TWindow::add_key(int akey)
{
	STACKTRACE;
  STACKTRACE;
	if (disabled)
		return;

	if (keybuff_count < 128)
	{
		keybuff[keybuff_count] = akey;
		++keybuff_count;
	}

}

void TWindow::add_keys()
{
	STACKTRACE;
  STACKTRACE;
	if (keyboard_needs_poll())
		poll_keyboard();

	clear_keys();

	// check all the "keys"
	int i;		// i = the scancode ;)
	for ( i = 0; i < KEY_MAX; ++i )
	{
		if (key[i])
		{
			// control overrules alt and shft
			if (key[KEY_LCONTROL] || key[KEY_RCONTROL])
				add_key( mapkey(i, KEY_LCONTROL) );

			// alt overrules shft
			else if (key[KEY_ALT] || key[KEY_ALTGR])
				add_key( mapkey(i, KEY_ALT) );

			// shift overrules a normal press
			else if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
				add_key( mapkey(i, KEY_LSHIFT) );

			else
				add_key( mapkey(i, 0) );

		}
	}
}

*/
