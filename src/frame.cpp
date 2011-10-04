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
#ifdef WIN32
#include <winalleg.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "melee.h"
REGISTER_FILE
#include "frame.h"

extern FILE* debug_file;
#define LIST_INCREMENT 10

int BACKGROUND_COLOR = 0;

/*! \brief compare DirtyItems
  \param p1 first item
  \param p2 second item
  \return 0 if equal, >0 if first > second, <0 if first < second
*/
int item_cmp(const void* p1, const void* p2)
{
	return( ( ((DirtyItem*) p1)->y) -  ( ((DirtyItem*) p2)->y) );
}


/*! \brief set pixel color to BACKGROUND_COLOR */
void erase_pixel(DirtyItem *item, BITMAP *frame)
{
	putpixel(frame, item->x, item->y, BACKGROUND_COLOR);
}


/*! \brief Draw pixel to child from frame */
void draw_pixel(DirtyItem *item, BITMAP *frame, BITMAP *child)
{
	putpixel(child, item->x, item->y, getpixel(frame, item->x, item->y));
}


/*! \brief set item color to BACKGROUND_COLOR */
void erase_box(DirtyItem *item, BITMAP *frame)
{
	rectfill(frame, item->x, item->y, item->x + item->a - 1, item->y + item->b - 1, BACKGROUND_COLOR);
}


/*! \brief copy item to chield */
void draw_box(DirtyItem *item, BITMAP *frame, BITMAP *child)
{
	if ((item->a == 1) && (item->b == 1))
		putpixel(child, item->x, item->y, getpixel(frame, item->x, item->y));
	else
		blit(frame, child,
			item->x,
			item->y,
			item->x,
			item->y,
			item->a,
			item->b
			);
}


/*! \brief Set line color to BACKGROUND_COLOR */
void erase_line(DirtyItem *item, BITMAP *frame)
{
	if ((item->x == item->a) && (item->y == item->b))
		putpixel(frame, item->x, item->y, BACKGROUND_COLOR);
	else
		line(frame, item->x, item->y, item->a, item->b, BACKGROUND_COLOR);
}


static BITMAP *line_frame;

/*! \brief Draw line on line_child, this function use global variable line_frame to detect color */
void line_pixel(BITMAP *line_child, int x, int y, int d)
{
	putpixel(line_child, x, y, getpixel(line_frame, x, y));
}


/*! \brief Set pixel color to BACKGROUND_COLOR */
void erase_a_pixel(BITMAP *child, int x, int y, int d)
{
	putpixel(child, x, y, BACKGROUND_COLOR);
}


/*! \brief Set circele color to BACKGROUND_COLOR */
void erase_circle(DirtyItem *item, BITMAP *frame)
{
	do_circle(frame, item->x, item->y, item->a, 0, erase_a_pixel);
}


/*! \brief Draw circle from Item */
void draw_circle(DirtyItem *item, BITMAP *frame, BITMAP *child)
{
	line_frame = frame;
	do_circle(child, item->x, item->y, item->a, 0, line_pixel);
}


/*! \brief Draw line from Item */
void draw_line(DirtyItem *item, BITMAP *frame, BITMAP *child)
{
	if ((item->x == item->a) && (item->y == item->b))
		putpixel(child, item->x, item->y, getpixel(frame, item->x, item->y));
	else {
		line_frame = frame;
		do_line(child, item->x, item->y, item->a, item->b, 0, line_pixel);
	}
}


Frame::Frame(int max_items)
{
	window = new VideoWindow();
	window->preinit();
	surface = NULL;
	full_redraw = 0;

	background_red = background_green = background_blue = 0;
	list_size = max_items;

	item_count = 0;
	item       = new DirtyItem[list_size];
	old_item_count = 0;
	old_item       = new DirtyItem[list_size];
	drawn_items = 0;
}


Frame::~Frame()
{
	destroy_bitmap(surface);
	window->deinit();
	delete window;

	delete[] item;
	item_count = 0;

	delete[] old_item;
	old_item_count = 0;
}


/*! \brief Used for define new BACKGROUND_COLOR */
void Frame::set_background ( int r, int g, int b)
{
	background_red = r;
	background_green = g;
	background_blue = b;
	return;
}


/*! \brief weild code to enlarge amount of items */
void Frame::enlarge_list(int increment)
{
	DirtyItem *temp;

	temp = item;
	item = new DirtyItem[list_size + increment];
	memcpy(item, temp, item_count * sizeof(DirtyItem));
	delete[] temp;

	temp = old_item;
	old_item = new DirtyItem[list_size + increment];
	memcpy(old_item, temp, old_item_count * sizeof(DirtyItem));
	delete[] temp;

	list_size += increment;
	return;
}


/*! \brief add item to item list */
void Frame::add_to_list(int x, int y, int a, int b,
void (*erase_item)(DirtyItem *item, BITMAP *frame),
void (*draw_item)(DirtyItem *item, BITMAP *frame, BITMAP *child))
{
	if (item_count == list_size)
		enlarge_list(LIST_INCREMENT);

	item[item_count].x = x;
	item[item_count].y = y;
	item[item_count].a = a;
	item[item_count].b = b;

	item[item_count].erase_item = erase_item;
	item[item_count].draw_item  = draw_item;

	item_count++;
}


/*! \brief strange function to be deleted I think */
void Frame::add_to_old_list(int x, int y, int a, int b,
void (*erase_item)(DirtyItem *item, BITMAP *frame),
void (*draw_item)(DirtyItem *item, BITMAP *frame, BITMAP *child))
{
	if (old_item_count == list_size)
		enlarge_list(LIST_INCREMENT);

	old_item[old_item_count].x = x;
	old_item[old_item_count].y = y;
	old_item[old_item_count].a = a;
	old_item[old_item_count].b = b;

	old_item[old_item_count].erase_item = erase_item;
	old_item[old_item_count].draw_item  = draw_item;

	old_item_count++;
}


/*! \brief Add pixel item to item list */
void Frame::add_pixel(int x, int y)
{
	add_to_list(x, y, 0, 0, erase_pixel, draw_pixel);
}


/*! \brief Add box item to item list */
void Frame::add_box(double x, double y, double w, double h)
{
	add_to_list((int)x, (int)y, (int)w, (int)h, erase_box, draw_box);
}


/*! \brief Add circle item to item list */
void Frame::add_circle(int x, int y, int a, int b)
{
	add_to_list(x, y, a, b, erase_circle, draw_circle);
}


/*! \brief to be deleted? */
void Frame::add_old_circle(int x, int y, int a, int b)
{
	add_to_old_list(x, y, a, b, erase_circle, draw_circle);
}


/*! \brief to be deleted? */
void Frame::add_line(int x, int y, int a, int b)
{
	add_to_list(x, y, a, b, erase_line, draw_line);
}


/*! \brief to be deleted? */
void Frame::add_old_pixel(int x, int y)
{
	add_to_old_list(x, y, 0, 0, erase_pixel, draw_pixel);
}


/*! \brief to be deleted? */
void Frame::add_old_box(int x, int y, int a, int b)
{
	add_to_old_list(
		x,
		y,
		a,
		b,
		erase_box, draw_box
		);
}


/*! \brief to be deleted? */
void Frame::add_old_line(int x, int y, int a, int b)
{
	add_to_old_list(x, y, a, b, erase_line, draw_line);
}


/*! \brief Clear screen, copy items to old items, destroy items */
void Frame::erase()
{
	STACKTRACE;

	int c;
	if (!surface)
		return;
	prepare();
	BACKGROUND_COLOR = makecol ( background_red, background_green, background_blue );

	if (full_redraw) {
		clear_to_color(surface, BACKGROUND_COLOR);
		old_item_count = 0;
		item_count = 0;
		return;
	}

	for(c = 0; c < item_count; c++) {
		item[c].erase_item(&item[c], surface);
	}

	if (full_redraw) {
		item_count = 0;
		return;
	}

	if (old_item_count == 0) {
		DirtyItem *tmp = old_item;
		old_item = item;
		item = tmp;
		old_item_count = item_count;
		item_count = 0;
		drawn_items = 0;
	} else {
		if (old_item_count + item_count > list_size) {
			full_redraw += 1;
		}
		memcpy ( &old_item[old_item_count], &item[0], item_count * sizeof(DirtyItem));
		old_item_count += item_count;
		item_count = 0;
		drawn_items = 0;
	}

	return;
}


/*! \brief Clear surface and set width and heigth to window w, h */
void Frame::prepare ()
{
	STACKTRACE;

	int w = 0, h = 0;
	if (surface) {
		w = surface->w;
		h = surface->h;
	}
	if ((window->w != w) || (window->h != h)) {
		if (surface) destroy_bitmap(surface);
		surface = NULL;
		w = window->w;
		h = window->h;
		if (w && h) {
			surface = create_bitmap(w, h);
			clear_to_color(surface, BACKGROUND_COLOR);
		}
		if (full_redraw >= 0)
			full_redraw += 1;
	}
}


/*! \brief Draw frame */
void Frame::draw()
{
	STACKTRACE;

	int c;
	BACKGROUND_COLOR = tw_color ( background_red, background_green, background_blue );

	BITMAP *tmp = window->surface;
	if (!tmp)
		return;

	prepare();

	if (!surface) {
		old_item_count = 0;
		item_count = 0;
		drawn_items = 0;
		return;
	}

	if (full_redraw) {
		old_item_count = 0;
		window->lock();
		blit(surface, tmp, 0, 0, window->x, window->y, window->w, window->h);
		window->unlock();
		if (full_redraw > 0) full_redraw -= 1;
	} else {
		tmp = create_sub_bitmap ( tmp, window->x, window->y, window->w, window->h);
		acquire_bitmap(tmp);

		for(c = drawn_items; c < item_count; c++) {
			item[c].draw_item(&item[c], surface, tmp);
		}
		for(c = 0; c < old_item_count; c++) {
			old_item[c].draw_item(&old_item[c], surface, tmp);
		}

		release_bitmap(tmp);
		destroy_bitmap(tmp);

		//drawn_items = item_count;
		old_item_count = 0;
	}
	return;
}
