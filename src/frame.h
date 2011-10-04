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

#ifndef __FRAME_H__
#define __FRAME_H__

typedef struct DirtyItem
{
	int x, y, a, b;

	void (*erase_item)(DirtyItem *item, BITMAP *frame);
	void (*draw_item)(DirtyItem *item, BITMAP *frame, BITMAP *child);
} DirtyItem;

void erase_pixel(DirtyItem *item, BITMAP *frame);
void draw_pixel(DirtyItem *item, BITMAP *frame, BITMAP *child);

void erase_box(DirtyItem *item, BITMAP *frame);
void draw_box(DirtyItem *item, BITMAP *frame, BITMAP *child);

void erase_line(DirtyItem *item, BITMAP *frame);
void draw_line(DirtyItem *item, BITMAP *frame, BITMAP *child);

/*! \brief ??? */
class Frame
{
	public:
		int list_size;

		int full_redraw;		 //number of frames to draw without Dirty Rectangles

		DirtyItem *item;
		int        item_count;
		int        drawn_items;
		DirtyItem *old_item;
		int        old_item_count;

		char background_red;
		char background_green;
		char background_blue;

		Surface *surface;
		VideoWindow *window;

		Frame(int max_items);
		virtual ~Frame();

		void enlarge_list(int increment);

		void add_to_list(int x, int y, int a, int b,
			void (*erase_item)(DirtyItem *item, BITMAP *frame),
			void (*draw_item)(DirtyItem *item,
			BITMAP *frame, BITMAP *child));
		void add_to_old_list(int x, int y, int a, int b,
			void (*erase_item)(DirtyItem *item,
			BITMAP *frame),
			void (*draw_item)(DirtyItem *item,
			BITMAP *frame, BITMAP *child));

		void add_pixel(int x, int y);
		void add_box(double x, double y, double w, double h);
		void add_line(int x, int y, int x2, int y2);

		void add_circle(int x, int y, int a, int b);
		void add_circle_fast(int x, int y, int a, int b);
		void add_old_circle(int x, int y, int a, int b);
		void add_old_circle_fast(int x, int y, int a, int b);

		void add_old_pixel(int x, int y);
		void add_old_box(int x, int y, int a, int b);
		void add_old_line(int x, int y, int a, int b);

		virtual void erase();
		virtual void draw();
		virtual void prepare();
		void set_background ( int red, int green, int blue );
};
#endif							 // __FRAME_H__
