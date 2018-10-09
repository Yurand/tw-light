/*
 *      helper.c
 *
 *      Copyright 2008-2018 Yury Siamashka <yurand2@gmail.com>
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

#include <string>
#include <codecvt>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>

#define PLATFORM_IS_ALLEGRO

#ifdef PLATFORM_IS_ALLEGRO
#include <allegro.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "helper.h"

/** Check if file or directory exists on filesystem
 *
 * @param filename path to file
 * @return not 0 if file exists, 0 otherwise
 */
int tw_exists(const char *filename)
{
#ifdef WIN32
	struct _stat buffer;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wfilename = converter.from_bytes(filename);
	return (_wstat(wfilename.c_str(), &buffer) == 0);
#else
	struct stat   buffer;
	return (stat (filename, &buffer) == 0);
#endif
}

/** Open file, expect utf-8 string as filename
 * @param filename File name.
 * @param mode Kind of access that's enabled.
 * @return returns a pointer to the open file. A null pointer value indicates an error.
*/
FILE *tw_fopen(const char *filename, const char *mode)
{
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wfilename = converter.from_bytes(filename);
	std::wstring wmode = converter.from_bytes(mode);

	return _wfopen(wfilename.c_str(), wmode.c_str());
#else
	return fopen(filename, mode);
#endif
}

/** Copy file from source to target,
 *
 * @param source utf-8 source encoded path
 * @param target utf-8 target encoded path
 * @return Returns TRUE on success, FALSE on error
 */
int tw_copy_file(const char * source, const char * target)
{
	PACKFILE * fsrc = pack_fopen(source, F_READ);
	if (!fsrc)
		return FALSE;

	PACKFILE * ftrg = pack_fopen(target, F_WRITE);
	if (!ftrg) {
		pack_fclose(fsrc);
		return FALSE;
	}

	unsigned char buffer[1024];
	int readed = 1;
	while (readed != 0) {
		readed = pack_fread(buffer, 1024, fsrc);
		if (!readed)
			break;
		int written = pack_fwrite(buffer, readed, ftrg);
	}
	pack_fclose(fsrc);
	pack_fclose(ftrg);
	return TRUE;
}

/** Creates a new directory.
 *
 * @param dirname utf-8 encoded path for a new directory
 * @return returns the value 0 if the new directory was created. On an error, the function returns –1 and sets errno
 */
int tw_mkdir(const char *dirname)
{
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wdirname = converter.from_bytes(dirname);
	return _wmkdir(wdirname.c_str());
#else
	return mkdir(dirname, 0777);
#endif
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

