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

#include "utils.h"
//#include "area.h"
//#include "twgui.h"

#include "twwindow.h"

normalmouse Tmouse;

TKeyHandler keyhandler;

// ----------------- SOME GENERALLY USEFUL STRUCTURES / FUNCTIONS ------------------

// map a key to some ascii code, and map unmappable stuff to 128+scan code:
// this is useful for the menus
// note, separated are the "key" and the extra "control" key (ctrl, alt)
int mapkey(int scancode_key, int scancode_ctrl)
{
	int k;

	k = scancode_key;

	k |= scancode_ctrl << 8;

	// thus we have 2 scan codes, no more needed.
	return k;
}


int unmapkey1(int k)
{
	// return the 1st scancode
	return k & 0x0FF;
}


int unmapkey2(int k)
{
	// return the 1st scancode
	return (k >> 8) & 0x0FF;
}


BITMAP *find_datafile_bmp(TW_DATAFILE *datafile, char *identif)
{
	char objname[128];

	if (strlen(identif) > 120) {
		tw_error("string exceeds max length");
	}

	strcpy(objname, identif);
	strcat(objname, "_BMP");	 // default extension for .bmp files.

	TW_DATAFILE *dat = tw_find_datafile_object(datafile, objname);

	if (!dat) {
		//char txt[512];
		//sprintf(txt, "Could not find %s", objname);
		//tw_error(txt);
		return 0;
	}

	if (dat->type == DAT_BITMAP)
		return (BITMAP*) dat->dat;
	else
		return 0;
}


BITMAP *clone_bitmap(int bpp, BITMAP *src, double scale, bool vidmem)
{
	BITMAP *dest, *convert;

	if ( !src )
		return 0;

	int W, H;

	W = iround( src->w * scale );
	H = iround( src->h * scale );

	dest = create_bitmap_ex(bpp, W, H);

	convert = create_bitmap_ex(bpp, src->w, src->h);
								 // use this to convert color depth
	blit(src, convert, 0, 0, 0, 0, src->w, src->h);

	if (W != src->w || H != src->h )
		stretch_blit(convert, dest, 0, 0, convert->w, convert->h, 0, 0, dest->w, dest->h);
	else
		blit(convert, dest, 0, 0, 0, 0, W, H);

	del_bitmap(&convert);

	// try to store this in memory, if there's enough room for it
	// cause the menu-bitmaps are pretty large, and take lotsa time to draw ...
	if (vidmem) {
		convert = create_video_bitmap(W, H);
		if (convert) {
			blit(dest, convert, 0, 0, 0, 0, W, H);
			del_bitmap(&dest);
			dest = convert;
		}
	}

	return dest;
}


normalmouse::normalmouse()
{
	bmp.original_mouse_sprite = mouse_sprite;
	bmp.newmousebmp = 0;

	reset();
}


void normalmouse::reset()
{
	update();					 // initialize with current mouse values.
	pos2 = pos;
	oldpos = pos;
}


void normalmouse::copyinfo(normalmouse *othermouse)
{
	pos    = othermouse->pos;
	oldpos = othermouse->oldpos;
	pos2 = othermouse->pos2;
	left   = othermouse->left;
	mid    = othermouse->mid;
	right  = othermouse->right;
	bmp.newmousebmp = othermouse->bmp.newmousebmp;
	bmp.original_mouse_sprite = othermouse->bmp.original_mouse_sprite;
}


// NOTE:
// this means that pos and oldpos must always be re-set to the default
// (unmoved) position every iteration.
void normalmouse::move(int dx, int dy)
{
	pos.x += dx;
	pos.y += dy;

	oldpos.x += dx;
	oldpos.y += dy;
}


int normalmouse::vx()
{
	return pos.x - oldpos.x;
}


int normalmouse::vy()
{
	return pos.y - oldpos.y;
}


void normalmouse::update()
{
	if (mouse_needs_poll())
		poll_mouse();

	oldpos = pos2;				 // this is exactly the "old" position, unmoved
	pos.set(mouse_x, mouse_y, mouse_z);
	pos2 = pos;

	int b = mouse_b;

	left.update(bool(b & 1));	 // make distinction here; individual buttons shouldn't know of each other
	mid.update(bool(b & 4));
	right.update(bool(b & 2));
}


void normalmouse::mousebutton::update(bool newstatus)
{
	// we are mainly interested in changes in the button status.
	if (newstatus != button.status) {
		oldbutton = button;

								 // on or of ;)
		button.status = newstatus;

		if (button.status != oldbutton.status)
								 // detect timing of a change in button status
			button.time = get_time() * 1E-3;

		button.change = true;

	} else
	button.change = false;
}


bool normalmouse::mousebutton::press()
{
	if (!button.change)
		return false;

	if (button.status && (!oldbutton.status))
		return true;
	else
		return false;

}


bool normalmouse::mousebutton::release()
{
	if (!button.change)
		return false;

	if ((!button.status) && oldbutton.status)
		return true;
	else
		return false;
}


bool normalmouse::mousebutton::dclick()
{
	if (!button.change)			 // check if the mouse is clicked
		return false;

	if (!button.status)			 // check if the mouse button is pressed
		return false;

								 // check time between release and press
	if (button.time - oldbutton.time < 0.2)
		return true;

	return false;
}


bool normalmouse::mousebutton::hold()
{
	return button.status;
}


void normalmouse::bmpstr::init(BITMAP *newbmp)
{
	newmousebmp = newbmp;
}


void normalmouse::bmpstr::set()
{
	if (newmousebmp) {
		set_mouse_sprite(newmousebmp);
	}
}


void normalmouse::bmpstr::restore()
{
	set_mouse_sprite(original_mouse_sprite);
}


void normalmouse::posstr::set(int xnew, int ynew, int znew)
{
	x = xnew;
	y = ynew;
	wheel = znew;
}


void normalmouse::posstr::move(int dx, int dy)
{
	x += dx;
	y += dy;
}


/*
extern int (*keyboard_callback)(int key);
If set, this function is called by the keyboard handler in response
to every keypress. It is passed a copy of the value that is about to
be added into the input buffer, and can either return this value
unchanged, return zero to cause the key to be ignored, or return
a modified value to change what readkey() will later return.
This routine executes in an interrupt context, so it must be
in locked memory.
*/

// just make a copy of the input.
static int my_callback(int key)
{
	keyhandler.add(key);
	return key;
}


TKeyHandler::TKeyHandler()
{
	keyboard_callback = my_callback;

	clear();
}


void TKeyHandler::clear()
{
	Nbuf = 0;
	Nbackbuf = 0;

	int i;
	for ( i = 0; i < KEY_MAX; ++i ) {
		keynew[i] = key[i];
		keyold[i] = key[i];
	}
}


void TKeyHandler::add(int key)
{
	if (Nbackbuf >= buffmax)
		return;

	keybackbuf[Nbackbuf] = key;

	++Nbackbuf;
}


void TKeyHandler::clearbuf()
{
	Nbuf = 0;
}


void TKeyHandler::update()
{
	if (keyboard_needs_poll())
		poll_keyboard();

	// detect key changes ...
	// (note that changes in-between updates are not seen by this so it's not 100% accurate)
	int i;
	for ( i = 0; i < KEY_MAX; ++i ) {
		// keep record of prev and new states
		keyold[i] = keynew[i];
		keynew[i] = key[i];

		// detect changes
		keyhit[i] = 0;
		keyreleased[i] = 0;

		if (keynew[i] != keyold[i]) {
			if (keynew[i])
				keyhit[i] = 1;
			else
				keyreleased[i] = 1;
		}

	}

	Nbuf = Nbackbuf;
	for ( i = 0; i < Nbuf; ++i ) {
		keybuf[i] = keybackbuf[i];
	}
	// reset the back-buffer for reading new stuf.
	Nbackbuf = 0;

}


bool TKeyHandler::pressed(char key)
{
	// only compare if the key != 0, otherwise you can get matches with
	// wierd key combos or something. So, key==0 has the meaning of, don't compare me!
	if (key == 0)
		return false;

	//char teststring[128];
	int i;
	for (i = 0; i < Nbuf; ++i) {
		//teststring[i] = keybuf[i] & 0x0FF;

								 // only compare by ascii code...
		if ( (keybuf[i] & 0x0FF) == key )
			return true;
	}

	//teststring[i] = 0;
	//message.out(teststring);

	return false;
}
