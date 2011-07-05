/*
 *      helper.c
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

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define PLATFORM_IS_ALLEGRO

#ifdef PLATFORM_IS_ALLEGRO
#include <allegro.h>
#endif

#include "helper.h"

/** Check if file or directory exists on filesystem
 *
 * @param file path to file
 * @return not 0 if file exists, 0 otherwise
 */
int tw_exists(const char *file)
{
	return !access(file, F_OK);
}


/** Finds out the filename portion of a completely specified file path.
 *
 * Both `\' and `/' are recognized as directory separators under DOS and Windows.
 * However, only `/' is recognized as directory separator under other platforms.
 *
 * @param file filename
 * @return a pointer to the portion of `path' where the filename starts,
 * or the beginning of `path' if no valid filename is found (eg. you are
 * processing a path with backslashes under Unix).
 */
const char *tw_get_filename(const char *file)
{
	return get_filename(file);
}


/** Replaces the specified filename+extension with a new extension tail,
 * storing at most `size' bytes into the `dest' buffer.
 * If the filename doesn't have any extension at all, `ext' will be
 * appended to it, adding a dot character if needed.
 * You can use the same buffer both as input and output because
 * Allegro internally works on a copy of the input before
 * touching `dest'.
 *
 * @param dest dest buffer
 * @param filename filename
 * @param ext extencion
 * @param size size of dest buffer
 * @return Returns a copy of the `dest' parameter
 */
char *tw_replace_extension(char *dest, const char *filename, const char *ext, int size)
{
	return replace_extension(dest, filename, ext, size);
}


char *tw_append_filename(char *dest, const char *path, const char *filename, int size)
{
	return append_filename(dest, path, filename, size);
}


int tw_is_relative_filename(const char *filename)
{
	return is_relative_filename(filename);
}


char *tw_canonicalize_filename(char *dest, const char *filename, int size)
{
	return canonicalize_filename(dest, filename, size);
}


/*///////////////////////////////////////////////////////////////////////////////////////
// INI like parser functions
///////////////////////////////////////////////////////////////////////////////////////*/
const char *tw_get_config_string(const char *section, const char *name, const char *def)
{
	return get_config_string(section, name, def);
}


int tw_get_config_int(const char *section, const char *name, int def)
{
	return get_config_int(section, name, def);
}


float tw_get_config_float(const char *section, const char *name, float def)
{
	return get_config_float(section, name, def);
}


int tw_get_desktop_resolution(int *width, int *height)
{
	return get_desktop_resolution(width, height);
}


int tw_desktop_color_depth()
{
	return desktop_color_depth();
}
