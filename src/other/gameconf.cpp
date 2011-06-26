/*
 *      gameconf.c
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

#include <stdio.h>
#include <string.h>
#include <string>

#include <sys/stat.h>

#include "melee.h"
#include "util/helper.h"

std::string home_ini_full_path(std::string path)
{
	char * home = getenv("HOME");
	char dest[2040] = {0};

	if (tw_exists(path.c_str())) {
		return path;
	}
	if (home == NULL) {
		if (strstr(path.c_str(), TWLIGHT_DATADIR))
			return path;
		std::string pth = tw_append_filename(dest, TWLIGHT_DATADIR, "default_ini", 2039);
		tw_append_filename(dest, pth.c_str(), path.c_str(), 2039);
		return std::string(dest);
	}
	else {
		if (strstr(path.c_str(), home))
			return path;
		return std::string(home) + std::string("/.tw-light/") + path;
	}
}


/// \brief create full data path
std::string data_full_path(std::string path)
{
	char data[2040] = {0};
	std::string ret;
	std::string base_dir;

	if (tw_exists("../data/gob.dat")) {
		base_dir = "../data";
	}
	else {

		base_dir = TWLIGHT_DATADIR;
	}

	if (path.length()) {
		ret = tw_append_filename(data, base_dir.c_str(), path.c_str(), 2039);
	}
	else {
		return base_dir;
	}
	return ret;
}


bool CopyFile(const char * source, const char * target)
{
	STACKTRACE;
	FILE * fsrc = fopen(source, "rb");
	if (!fsrc)
		return false;

	FILE * ftrg = fopen(target, "wb+");
	if (!ftrg) {
		tw_error("ACK!!! copy file '%s' failed!!!", target);
		fclose(fsrc);
		return false;
	}

	unsigned char buffer[1024];
	int readed = 1;
	while(readed!=0) {
		readed = fread(buffer, 1, 1024, fsrc);
		int written = fwrite(buffer, 1, readed,ftrg);
		if (readed != written)
			tw_error("ACK!!! copy file failed");
	}
	fclose(fsrc);
	fclose(ftrg);
	return true;
}


#ifdef WIN32
#define mkdir(x,y) mkdir(x)
#endif

int create_user_ini()
{
	STACKTRACE;

	int i;
	char *tmp;

	std::string curFile = home_ini_full_path("");
	if (!tw_exists(curFile.c_str()))
		mkdir(curFile.c_str(), 0777);

	curFile = home_ini_full_path("client.ini");
	if (!tw_exists(curFile.c_str()))
		CopyFile(data_full_path("client.ini").c_str(),
			curFile.c_str());

	curFile = home_ini_full_path("fleets.ini");
	if (!tw_exists(curFile.c_str()))
		CopyFile(data_full_path("fleets.ini").c_str(),
			curFile.c_str());

	curFile = home_ini_full_path("scp.ini");
	if (!tw_exists(curFile.c_str()))
		CopyFile(data_full_path("scp.ini").c_str(),
			curFile.c_str());

	curFile = home_ini_full_path("server.ini");
	if (!tw_exists(curFile.c_str()))
		CopyFile(data_full_path("server.ini").c_str(),
			curFile.c_str());

	curFile = home_ini_full_path("vobject.ini");
	if (!tw_exists(curFile.c_str()))
		CopyFile(data_full_path("vobject.ini").c_str(),
			curFile.c_str());
	return 0;
}
