/*
 *      port_allegro.c
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

#include <allegro.h>
#include "port.h"

#ifdef PLATFORM_IS_ALLEGRO
#include <allegro.h>

/* Returns the mask color for the specified bitmap (the value which is
 * skipped when drawing sprites). For 256-color bitmaps this is zero,
 * and for truecolor bitmaps it is bright pink (maximum red and blue,
 * zero green). A frequent use of this function is to clear a bitmap
 * with the mask color so you can later use this bitmap with
 * masked_blit() or draw_sprite() after drawing other stuff on it.*/
int tw_bitmap_mask_color(Surface *bmp)
{
	return bitmap_mask_color(bmp);
}


/** Clears the bitmap to the specified color */
void tw_clear_to_color(Surface *bitmap, int color)
{
	clear_to_color(bitmap, color);
}


/** Reads a pixel from point (x, y) in the bitmap.
 *
 * @param x
 * @param y
 * \return -1 if the point lies outside the bitmap (ignoring the clipping
 * rectangle), otherwise the value of the pixel in the color format of the
 * bitmap.  Warning: -1 is also a valid value for pixels contained in
 * 32-bit bitmaps with alpha channel (when R,G,B,A are all equal to 255)
 * so you can't use the test against -1 as a predicate for such bitmaps.
 * In this cases, the only reliable predicate is is_inside_bitmap().
 */
int tw_getpixel(Surface *bmp, int x, int y)
{
	return getpixel(bmp, x, y);
}


/** Converts colors from a hardware independent format.
 *
 * (red, green, and blue values ranging 0-255) to the pixel format
 * required by the current video mode, calling the preceding
 * 8, 15, 16, 24, or 32-bit makecol functions as appropriate.
 *
 * @param r red
 * @param g green
 * @param b blue
 * @return color for current video mode
 */
int tw_makecol(int r, int g, int b)
{
	return makecol(r, g, b);
}


void tw_drawing_mode(int mode, Surface *pattern, int x_anchor, int y_anchor)
{
	drawing_mode(mode, pattern, x_anchor, y_anchor);
}


int tw_get_palete_color(int idx)
{
	return palette_color[idx];
}


void tw_set_trans_blender(int r, int g, int b, int a)
{
	set_trans_blender(r, g, b, a);
}


/** Given a color in the format being used by the current video mode,
 *  these functions extract one of the red, green, blue, or
 * alpha components (ranging 0-255)
 * @param c color
 */
int tw_getr(int c)
{
	return getr(c);
}


/** Given a color in the format being used by the current video mode,
 *  these functions extract one of the red, green, blue, or
 * alpha components (ranging 0-255)
 * @param c color
 */
int tw_getg(int c)
{
	return getg(c);
}


/** Given a color in the format being used by the current video mode,
 *  these functions extract one of the red, green, blue, or
 * alpha components (ranging 0-255)
 * @param c color
 */
int tw_getb(int c)
{
	return getb(c);
}


/** Given a color in the format being used by the current video mode,
 *  these functions extract one of the red, green, blue, or
 * alpha components (ranging 0-255)
 * @param c color
 */
int tw_geta(int c)
{
	return geta(c);
}


void tw_blit(Surface *source, Surface *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
	blit(source, dest, source_x, source_y, dest_x, dest_y, width, height);
}


/** Draws a line onto the bitmap, from point (x1, y1) to (x2, y2). */
void tw_line(Surface *bmp, int x1, int y1, int x2, int y2, int color)
{
	line(bmp, x1, y1, x2, y2, color);
}


/** Writes a pixel into a bitmap. */
void tw_putpixel(Surface *bmp, int x, int y, int color)
{
	putpixel(bmp, x, y, color);
}


/** Draws a circle  */
void tw_circle(Surface *bmp, int x, int y, int radius, int color)
{
	circle(bmp, x, y, radius, color);
}


void tw_circlefill(Surface *bmp, int x, int y, int radius, int color)
{
	circlefill(bmp, x, y, radius, color);
}


/** Loads a datafile into memory in one go. If the datafile has been
 * encrypted, you must first call packfile_password() to set the
 * appropriate key. If the datafile contains truecolor graphics, you
 * must set the video mode or call set_color_conversion() before
 * loading it.
 */
TW_DATAFILE *tw_load_datafile(const char *filename)
{
	return (TW_DATAFILE*)load_datafile(filename);
}


/** frees all the objects in a datafile. Use this to avoid memory leaks in your program.*/
void tw_unload_datafile(TW_DATAFILE *dat)
{
	unload_datafile((DATAFILE*)dat);
}


/** Searches an already loaded datafile for an object with the specified
 * name. In the name you can use `/' and `#' separators for nested
 * datafile paths.
 */
TW_DATAFILE *tw_find_datafile_object(const TW_DATAFILE *dat, const char *objectname)
{
	return (TW_DATAFILE*)find_datafile_object((DATAFILE*)dat, objectname);
}


/** Loads a specific object from a datafile. This won't work if you
 * strip the object names from the file, and it will be very slow if
 * you save the file with global compression.
 */
TW_DATAFILE *tw_load_datafile_object(const char *filename, const char *objectname)
{
	return (TW_DATAFILE*)load_datafile_object(filename, objectname);
}


/** Frees an object previously loaded by load_datafile_object(). Use
 * this to avoid memory leaks in your program.
 */
void tw_unload_datafile_object(TW_DATAFILE *dat)
{
	unload_datafile_object((DATAFILE*)dat);
}
#endif
