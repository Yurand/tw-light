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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif
#include <allegro/internal/aintern.h>
#include "melee.h"
REGISTER_FILE

#include "scp.h"				 // for menu sounds
#include "gui.h"
#include "melee/mgame.h"
#include "melee/mfleet.h"
#include "util/aastr.h"
#include "other/twconfig.h"

#include "other/dialogs.h"
/*

TimeWarp Dialog

like Allegro's popup-dialog, except
1.  scales for screen resolution, based upon a
default of 640x480
2.  automatically sets background/forground colors
to black and white
3.  like popup, only automatic

*/

#define TW_DIALOG_DEFAULT_RES_X 640
#define TW_DIALOG_DEFAULT_RES_Y 480

static int tw_gui_color_transform ( int c )
{
	if (0) ;
	else if (c ==255) c = makecol(0,0,0);
	else if (c ==  0) c = makecol(255,255,255);
	else if (c < 256) c = palette_color[c];
	else c = tw_color(c&255,(c>>8)&255,(c>>16)&255) | (c &0xff000000);
	return c;
}


static FONT *tw_gui_get_font ( int c )
{
	int f, g, h;
	if (0) ;
	else if (videosystem.width <= 250) g = 0;
	else if (videosystem.width <= 320) g = 1;
	else if (videosystem.width <= 400) g = 2;
	else if (videosystem.width <= 640) g = 3;
	else if (videosystem.width <= 800) g = 4;
	else if (videosystem.width <= 1024) g = 5;
	else if (videosystem.width <= 1280) g = 6;
	else if (videosystem.width <= 1600) g = 7;
	else g = 8;
	if (0) ;
	else if (videosystem.height <= 180) h = 0;
	else if (videosystem.height <= 240) h = 1;
	else if (videosystem.height <= 300) h = 2;
	else if (videosystem.height <= 480) h = 3;
	else if (videosystem.height <= 600) h = 4;
	else if (videosystem.height <= 768) h = 5;
	else if (videosystem.height <= 1024) h = 6;
	else if (videosystem.height <= 1280) h = 7;
	else h = 8;
	f = (c + g + h + 1) / 2;
	return videosystem.get_font(f);
}


void TW_Dialog_Player::redraw()
{
	STACKTRACE;
	int i;
	if (!player) return;
	for (i = 0; player->dialog[i].proc; i++) player->dialog[i].flags |= D_DIRTY;
	update();
	return;
}


void TW_Dialog_Player::_event( Event * e)
{
	STACKTRACE;
	switch (e->type) {
		case Event::VIDEO:
		{
			if (e->subtype == VideoEvent::REDRAW)
				this->redraw();
		} break;
	}
}


void TW_Dialog_Player::init(VideoWindow *w, DIALOG *d, int focus)
{
	STACKTRACE;
	dialog = d;
	window = w;
	ifocus = focus;
	player = NULL;
	if (!window) window = &videosystem.window;
	window->add_callback(this);
	int i;
	for (i = 0; d[i].proc; i += 1) ;
	length = i;

	level = d[i].d2;
	d[length].d2 += 1;

	prev_level = (TW_Dialog_Player*) d[length].dp;
	d[length].dp = this;

	old_sizes = new int[length * 6];

	int ox = d[length].x, oy = d[length].y;
	int ow = d[length].w, oh = d[length].h;
	if (!ox) ox = 0;
	if (!oy) oy = 0;
	if (!ow) ow = TW_DIALOG_DEFAULT_RES_X;
	if (!oh) oh = TW_DIALOG_DEFAULT_RES_Y;

	for (i = 0; i < length; i += 1) {
		old_sizes[i*6+0] = d[i].x;
		old_sizes[i*6+1] = d[i].y;
		old_sizes[i*6+2] = d[i].w;
		old_sizes[i*6+3] = d[i].h;
		old_sizes[i*6+4] = d[i].fg;
		old_sizes[i*6+5] = d[i].bg;
	}

	for (i = 0; i < length; i += 1) {
		d[i].fg = tw_gui_color_transform(d[i].fg);
		d[i].bg = tw_gui_color_transform(d[i].bg);
		d[i].x = window->x + window->w * (d[i].x - ox) / ow;
		d[i].y = window->y + window->h * (d[i].y - oy) / oh;
		d[i].w = window->w * d[i].w / ow;
		d[i].h = window->h * d[i].h / oh;
	}

	d[length].dp2 = font;
	font = tw_gui_get_font(d[length].d1);

	subscreen = create_sub_bitmap( window->surface, window->x, window->y, window->w, window->h);

	return;
}


void TW_Dialog_Player::deinit()
{
	STACKTRACE;
	int i;
	window->remove_callback(this);
	dialog[length].d2 -= 1;
	if (dialog[length].d2 != level) {
		tw_error("TW_Dialog_Player::deinit - inconsistent GUI order");
	}
	dialog[length].dp = prev_level;
	font = (FONT*) dialog[length].dp2;
	for (i = 0; i < length; i += 1) {
		dialog[i].x  = old_sizes[i*6+0];
		dialog[i].y  = old_sizes[i*6+1];
		dialog[i].w  = old_sizes[i*6+2];
		dialog[i].h  = old_sizes[i*6+3];
		dialog[i].fg = old_sizes[i*6+4];
		dialog[i].bg = old_sizes[i*6+5];
	}
	delete[] old_sizes;
	if (player) i = shutdown_dialog ( player );
	else i = -1;
	destroy_bitmap(subscreen);
	subscreen = NULL;
	return;
}


int TW_Dialog_Player::update()
{
	STACKTRACE;
	videosystem.poll_redraw();
	BITMAP *old = screen;
	screen = subscreen;
	if (!player) player = init_dialog(dialog, ifocus);
	int i = update_dialog ( player );
	screen = old;
	if (!i) {
		return player->obj;
	}
	return -2;
}


/*
static void tw_dialog_post ( VideoWindow *window, DIALOG *d, int n ) {
}*/

int tw_do_dialog ( VideoWindow *window, DIALOG *d, int index )
{
	int return_value;
	TW_Dialog_Player bob;
	if (!window) window = &videosystem.window;
	bob.init(window, d, index);

	while (keypressed()) readkey();

	show_mouse(window->surface);
	while ((-2 == (return_value = bob.update())));
	bob.deinit();
	show_mouse(NULL);

	return return_value;
}


int tw_popup_dialog ( VideoWindow *window, DIALOG *d, int index )
{
	BITMAP *tmp;
	int n, i, return_value;
	if (!window) window = &videosystem.window;
	for (n = 0; d[n].proc; n += 1) ;
	if (!window->surface) tw_error ("tw_dialog_pre - no drawing surface");
	if ((index >= n) || (index < 0))
		tw_error("tw_dialog - index invalid");

	TW_Dialog_Player bob;
	bob.init( window, d, index);

	int x=9999, y=9999, w=0, h=0;
	for (i = 0; i < n; i += 1) if (d[i].w && d[i].h) {
		if (d[i].x < x) x = d[i].x;
		if (d[i].y < y) y = d[i].y;
		if (d[i].x + d[i].w > w) w = d[i].x + d[i].w;
		if (d[i].y + d[i].h > h) h = d[i].y + d[i].h;
	}
	w -= x;
	h -= y;
	tmp = create_bitmap_ex(bitmap_color_depth(window->surface), w, h);
	window->lock();
	blit(window->surface, tmp, x, y, 0, 0, w, h);
	window->unlock();

	while (keypressed()) readkey();

	show_mouse(window->surface);
	while ((-2 == (return_value = bob.update())));
	bob.deinit();
	show_mouse(NULL);

	window->lock();
	blit(tmp, window->surface, 0, 0, x, y, w, h);
	window->unlock();
	destroy_bitmap(tmp);
	return return_value;
}


/*

Just some minor extensions to Allegros GUI
It looks much more complicated than it is
or something

*/

/* d_bitmap_proc:
 *  Simple dialog procedure: draws the bitmap which is pointed to by dp.
 */
int d_tw_bitmap_proc(int msg, DIALOG *d, int c)
{
	STACKTRACE;
	BITMAP *b = (BITMAP *)d->dp;

	if (b && ((msg==MSG_IDLE) || (msg==MSG_DRAW))) {
		int ocl, oct, ocr, ocb;
		ocl = screen->cl; oct = screen->ct; ocr = screen->cr; ocb = screen->cb;
		screen->cl = d->x; screen->ct = d->y; screen->cr = d->x+d->w-1; screen->cb = d->y+d->h+1;
		rotate_sprite ( screen, b, d->x+d->w/2 - b->w/2, d->y+d->h/2 - b->h/2, get_time() * 4096 );
		screen->cl = ocl; screen->ct = oct; screen->cr = ocr; screen->cb = ocb;
	}
	return D_O_K;
}


/* d_yield_proc:
 *  Simple dialog procedure which just yields the timeslice when the dialog
 *  is idle.
 * changed: calls idle instead of yield
 * added: checks for redraw, handles it if necessary
 */
#include "melee/mview.h"
int d_tw_yield_proc(int msg, DIALOG *d, int c)
{
	STACKTRACE;
	if (msg == MSG_IDLE) {
		rest(20);
	}

	return D_O_K;
}


char *genericListboxGetter(int index, int *list_size, char **_list)
{
	if (index < 0) {
		index = 0;
		while (_list[index]) index += 1;
		*list_size = index;
		return NULL;
	} else {
		return(_list[index]);
	}
}


char *shipListboxGetter(int index, int *list_size)
{
	if (index < 0) {
		*list_size = reference_fleet->getSize();
		return NULL;
	}
	else if (reference_fleet->getShipType(index))
		return (char *) reference_fleet->getShipType(index)->name;
	else return "(none)";
}


char *shippointsListboxGetter(int index, int *list_size)
{
	static char buffy[256];
	if (index < 0) {
		*list_size = reference_fleet->getSize();
		return NULL;
	} else {
		ShipType * type = reference_fleet->getShipType(index);
		sprintf(buffy, "%3d %s", type->cost, type->name);
	}
	return buffy;
}


char *fleetListboxGetter(int index, int *list_size, Fleet *fleet)
{
	if (index < 0) {
		*list_size = fleet->getSize();
		return NULL;
	}
	else if (fleet->getShipType(index))
		return (char *) fleet->getShipType(index)->name;
	else return "(none)";
}


char *fleetpointsListboxGetter(int index, int *list_size, Fleet *fleet)
{
	//return "Disco!";
	ASSERT(fleet);
	static char buffy[256];

	if (index < 0) {
		*list_size = fleet->getSize();
		return NULL;
	} else
	if (fleet->getShipType(index)) {
		sprintf(buffy, "%3d %s", fleet->getShipType(index)->cost, fleet->getShipType(index)->name);
		return buffy;
	}
	else return "(none)";
}


/* idle_cb:
 *  rest_callback() routine to keep dialogs animating nice and smoothly.
 */
static void idle_cb(void)
{
	STACKTRACE;
	broadcast_dialog_message(MSG_IDLE, 0);
}


typedef char *(*getfuncptr2)(int, int *, char **);

/* _handle_listbox_click:
 *  Helper to process a click on a listbox, doing hit-testing and moving
 *  the selection.
 */
void _handle_listbox_click2(DIALOG *d)
{
	STACKTRACE;
	char *sel = (char*)d->dp2;
	int listsize, height;
	int i, j;

	(*(getfuncptr2)d->dp)(-1, &listsize, (char**)d->dp3);
	if (!listsize)
		return;

	height = (d->h-4) / text_height(font);

	i = MID(0, ((gui_mouse_y() - d->y - 2) / text_height(font)),
		((d->h-4) / text_height(font) - 1));
	i += d->d2;
	if (i < d->d2)
		i = d->d2;
	else {
		if (i > d->d2 + height-1)
			i = d->d2 + height-1;
		if (i >= listsize)
			i = listsize-1;
	}

	if (gui_mouse_y() <= d->y)
		i = MAX(i-1, 0);
	else if (gui_mouse_y() >= d->y+d->h-1)
		i = MIN(i+1, listsize-1);

	if (i != d->d1) {
		if (sel) {
			if (key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG)) {
				if ((key_shifts & KB_SHIFT_FLAG) || (d->flags & D_INTERNAL)) {
					for (j=MIN(i, d->d1); j<=MAX(i, d->d1); j++)
						sel[j] = TRUE;
				}
				else
					sel[i] = !sel[i];
			}
		}

		d->d1 = i;
		i = d->d2;
		_handle_scrollable_scroll(d, listsize, &d->d1, &d->d2);

		d->flags |= D_DIRTY;

		if (i != d->d2)
			rest_callback(MID(10, text_height(font)*16-d->h-1, 100), idle_cb);
	}
}


/* draw_listbox:
 *  Helper function to draw a listbox object.
 */
void _draw_listbox2(DIALOG *d)
{
	STACKTRACE;
	int height, listsize, i, len, bar, x, y, w;
	int fg_color, fg, bg;
	char *sel = (char*) d->dp2;
	char s[1024];
	int rtm;

	(*(getfuncptr2)d->dp)(-1, &listsize, (char**) d->dp3);
	height = (d->h-4) / text_height(font);
	bar = (listsize > height);
	w = (bar ? d->w-15 : d->w-3);
	fg_color = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;

	/* draw box contents */
	for (i=0; i<height; i++) {
		if (d->d2+i < listsize) {
			if (d->d2+i == d->d1) {
				fg = d->bg;
				bg = fg_color;
			}
			else if ((sel) && (sel[d->d2+i])) {
				fg = d->bg;
				bg = gui_mg_color;
			} else {
				fg = fg_color;
				bg = d->bg;
			}
			usetc(s, 0);
			ustrncat(s, (*(getfuncptr2)d->dp)(i+d->d2, NULL, (char**)d->dp3), sizeof(s)-ucwidth(0));
			x = d->x + 2;
			y = d->y + 2 + i*text_height(font);
			rtm = text_mode(bg);
			rectfill(screen, x, y, x+7, y+text_height(font)-1, bg);
			x += 8;
			len = ustrlen(s);
			while (text_length(font, s) >= MAX(d->w - 1 - (bar ? 22 : 10), 1)) {
				len--;
				usetat(s, len, 0);
			}
			textout(screen, font, s, x, y, fg);
			text_mode(rtm);
			x += text_length(font, s);
			if (x <= d->x+w)
				rectfill(screen, x, y, d->x+w, y+text_height(font)-1, bg);
		} else {
			rectfill(screen, d->x+2,  d->y+2+i*text_height(font),
				d->x+w, d->y+1+(i+1)*text_height(font), d->bg);
		}
	}

	if (d->y+2+i*text_height(font) <= d->y+d->h-3)
		rectfill(screen, d->x+2, d->y+2+i*text_height(font),
								 /**/
			d->x+w, d->y+d->h-3, d->bg);

	_draw_scrollable_frame(d, listsize, d->d2, height, fg_color, d->bg);
}


/* d_list_proc:
 *  A list box object. The dp field points to a function which it will call
 *  to obtain information about the list. This should follow the form:
 *     const char *<list_func_name> (int index, int *list_size);
 *  If index is zero or positive, the function should return a pointer to
 *  the string which is to be displayed at position index in the list. If
 *  index is  negative, it should return null and list_size should be set
 *  to the number of items in the list. The list box object will allow the
 *  user to scroll through the list and to select items list by clicking
 *  on them, and if it has the input focus also by using the arrow keys. If
 *  the D_EXIT flag is set, double clicking on a list item will cause it to
 *  close the dialog. The index of the selected item is held in the d1
 *  field, and d2 is used to store how far it has scrolled through the list.
 */
int d_list_proc2(int msg, DIALOG *d, int c)
{
	STACKTRACE;
	int listsize, i, bottom, height, bar, orig;
	char *sel = (char*) d->dp2;
	int redraw = FALSE;

	switch (msg) {

		case MSG_START:
			(*(getfuncptr2)d->dp)(-1, &listsize, (char**)d->dp3);
			_handle_scrollable_scroll(d, listsize, &d->d1, &d->d2);
			break;

		case MSG_DRAW:
			_draw_listbox2(d);
			break;

		case MSG_CLICK:
			(*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);
			height = (d->h-4) / text_height(font);
			bar = (listsize > height);
			if ((!bar) || (gui_mouse_x() < d->x+d->w-13)) {
				if ((sel) && (!(key_shifts & KB_CTRL_FLAG))) {
					for (i=0; i<listsize; i++) {
						if (sel[i]) {
							redraw = TRUE;
							sel[i] = FALSE;
						}
					}
					if (redraw) {
						scare_mouse();
						SEND_MESSAGE(d, MSG_DRAW, 0);
						unscare_mouse();
					}
				}
				_handle_listbox_click2(d);
				while (gui_mouse_b()) {
					broadcast_dialog_message(MSG_IDLE, 0);
					d->flags |= D_INTERNAL;
					_handle_listbox_click2(d);
					d->flags &= ~D_INTERNAL;
				}
			} else {
				_handle_scrollable_scroll_click(d, listsize, &d->d2, height);
			}
			break;

		case MSG_DCLICK:
			(*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);
			height = (d->h-4) / text_height(font);
			bar = (listsize > height);
			if ((!bar) || (gui_mouse_x() < d->x+d->w-13)) {
				if (d->flags & D_EXIT) {
					if (listsize) {
						i = d->d1;
						SEND_MESSAGE(d, MSG_CLICK, 0);
						if (i == d->d1)
							return D_CLOSE;
					}
				}
			}
			break;

		case MSG_WHEEL:
			(*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);
			height = (d->h-4) / text_height(font);
			if (height < listsize) {
				int delta = (height > 3) ? 3 : 1;
				if (c > 0)
					i = MAX(0, d->d2-delta);
				else
					i = MIN(listsize-height, d->d2+delta);
				if (i != d->d2) {
					d->d2 = i;
					scare_mouse();
					SEND_MESSAGE(d, MSG_DRAW, 0);
					unscare_mouse();
				}
			}
			break;

		case MSG_KEY:
			(*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);
			if ((listsize) && (d->flags & D_EXIT))
				return D_CLOSE;
			break;

		case MSG_WANTFOCUS:
			return D_WANTFOCUS;

		case MSG_CHAR:
			(*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);

			if (listsize) {
				c >>= 8;

				bottom = d->d2 + (d->h-4)/text_height(font) - 1;
				if (bottom >= listsize-1)
					bottom = listsize-1;

				orig = d->d1;

				if (c == KEY_UP)
					d->d1--;
				else if (c == KEY_DOWN)
					d->d1++;
				else if (c == KEY_HOME)
					d->d1 = 0;
				else if (c == KEY_END)
					d->d1 = listsize-1;
				else if (c == KEY_PGUP) {
					if (d->d1 > d->d2)
						d->d1 = d->d2;
					else
						d->d1 -= (bottom - d->d2) ? bottom - d->d2 : 1;
				}
				else if (c == KEY_PGDN) {
					if (d->d1 < bottom)
						d->d1 = bottom;
					else
						d->d1 += (bottom - d->d2) ? bottom - d->d2 : 1;
				}
				else
					return D_O_K;

				if (sel) {
					if (!(key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG))) {
						for (i=0; i<listsize; i++)
							sel[i] = FALSE;
					}
					else if (key_shifts & KB_SHIFT_FLAG) {
						for (i=MIN(orig, d->d1); i<=MAX(orig, d->d1); i++) {
							if (key_shifts & KB_CTRL_FLAG)
								sel[i] = (i != d->d1);
							else
								sel[i] = TRUE;
						}
					}
				}

				/* if we changed something, better redraw... */
				_handle_scrollable_scroll(d, listsize, &d->d1, &d->d2);
				d->flags |= D_DIRTY;
				return D_USED_CHAR;
			}
			break;
	}

	return D_O_K;
}


/**
 */
int my_d_button_proc(int msg, DIALOG * d, int c)
{
	STACKTRACE;
	int ret = 0;

	ret = d_agup_button_proc(msg, d, c);

	switch (msg) {
		case MSG_END:
			if (d->flags & D_DISABLED) {
				if (menuDisabled != NULL)
					sound.play(menuDisabled, 128);
			} else {
				if (menuAccept != NULL)
					sound.play(menuAccept, 64);
			}

			break;
	};

	return ret;
}
