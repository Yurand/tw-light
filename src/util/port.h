/*
 *      port.h
 *
 *      Copyright 2008 Yura Siamashka <yurand2@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef __TW_PORT_H__
#define __TW_PORT_H__

#define PLATFORM_IS_ALLEGRO

#ifdef __cplusplus
extern "C"
{
	#endif

	typedef struct Color
	{
		unsigned char r, g, b;
		unsigned char filler;
	}Color;

	#ifdef PLATFORM_IS_ALLEGRO

	struct BITMAP;
	typedef BITMAP Surface;

	// FIXME: to many allegro details is open
	typedef struct TW_RGB
	{
		unsigned char r, g, b;
	}TW_RGB;

	typedef struct TW_DATAFILE
	{
		void *dat;				 /**< pointer to the actual data */
		int type;				 /**< type of the data */
		long size;				 /**< size of the data in bytes */
		void *prop;				 /**< list of object properties */
	}TW_DATAFILE;
	#endif

	//#ifndef pallete_color
	//extern int pallete_color[256];
	//#endif

	enum {
		TW_DRAW_MODE_SOLID,
		TW_DRAW_MODE_XOR,
		TW_DRAW_MODE_COPY_PATTERN,
		TW_DRAW_MODE_SOLID_PATTERN,
		TW_DRAW_MODE_MASKED_PATTERN,
		TW_DRAW_MODE_TRANS,
	};

	int tw_get_palete_color(int idx);
	void tw_drawing_mode(int mode, Surface *pattern, int x_anchor, int y_anchor);
	void tw_set_trans_blender(int r, int g, int b, int a);

	int tw_makecol(int r, int g, int b);
	void tw_clear_to_color(Surface *bitmap, int color);

	int tw_getr(int c);
	int tw_getg(int c);
	int tw_getb(int c);
	int tw_geta(int c);

	int tw_bitmap_mask_color(Surface *bmp);
	int tw_getpixel(Surface *bmp, int x, int y);
	void tw_blit(Surface *source, Surface *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
	void tw_line(Surface *bmp, int x1, int y1, int x2, int y2, int color);
	void tw_putpixel(Surface *bmp, int x, int y, int color);
	void tw_circle(Surface *bmp, int x, int y, int radius, int color);
	void tw_circlefill(Surface *bmp, int x, int y, int radius, int color);

	// Datafiles
	TW_DATAFILE *tw_load_datafile_object(const char *filename, const char *objectname);
	TW_DATAFILE *tw_find_datafile_object(const TW_DATAFILE *dat, const char *objectname);
	TW_DATAFILE *tw_load_datafile(const char *filename);
	void tw_unload_datafile(TW_DATAFILE *dat);
	void tw_unload_datafile_object(TW_DATAFILE *dat);

	#ifdef __cplusplus
}
#endif
#endif
