/*
 *      helper.h
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

#ifndef __HELPER_H__
#define __HELPER_H__

#ifdef __cplusplus
extern "C"
{
	#endif

	int tw_exists(const char *file);
	const char *tw_get_filename(const char *file);
	char *tw_replace_extension(char *dest, const char *filename, const char *ext, int size);
	char *tw_append_filename(char *dest, const char *path, const char *filename, int size);
	int tw_is_relative_filename(const char *filename);
	char *tw_canonicalize_filename(char *dest, const char *filename, int size);

	const char *tw_get_config_string(const char *section, const char *name, const char *def);
	int tw_get_config_int(const char *section, const char *name, int def);
	float tw_get_config_float(const char *section, const char *name, float def);

	int tw_get_desktop_resolution(int *width, int *height);
	int tw_desktop_color_depth();

	#ifdef __cplusplus
}
#endif
#endif
