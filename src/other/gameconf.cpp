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
#include <stdlib.h>
#include <string.h>
#include <string>

#ifdef WIN32
#include <io.h>
#include <shlobj.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <sys/stat.h>

#include "util/errors.h"
#include "util/helper.h"

static void static_get_home_directory(char* buff, int len)
{
	char homedir[2048];
	#ifdef WIN32
	const char *twname = "tw-light";
	SHGetFolderPath( 0, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, homedir );
	#else
	const char *twname = ".tw-light";
	strcpy(homedir, getenv("HOME"));
	#endif
	tw_append_filename(buff, homedir, twname, len);
}


std::string home_ini_full_path(std::string path)
{
	char dest[2040] = {0};

	if (!tw_is_relative_filename(path.c_str())) {
		return path;
	}
	char homedir[2048];
	static_get_home_directory(homedir, sizeof(homedir));

	if (path.length() == 0)
		return homedir;
	std::string pth = homedir;
	return tw_append_filename(homedir, pth.c_str(), tw_get_filename(path.c_str()), sizeof(homedir));
}


/// \brief create full data path
std::string data_full_path(std::string path)
{
	char data[2040] = {0};
	std::string ret;
	std::string base_dir;

	if (tw_exists("../data/gob.dat")) {
		base_dir = "../data";
	} else {

		base_dir = TWLIGHT_DATADIR;
	}

	if (tw_is_relative_filename(base_dir.c_str())) {
		char buffer[50000];
		char *cwd = getcwd(buffer, sizeof(buffer));
		tw_append_filename(buffer, buffer, base_dir.c_str(), sizeof(buffer));
		base_dir = tw_canonicalize_filename(buffer, buffer, sizeof(buffer));
	}

	if (path.length()) {
		ret = tw_append_filename(data, base_dir.c_str(), path.c_str(), 2039);
	} else {
		return base_dir;
	}
	return ret;
}


static bool static_copy_file(const char * source, const char * target)
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

	std::string curFile = home_ini_full_path("");
	if (!tw_exists(curFile.c_str()))
		mkdir(curFile.c_str(), 0777);

	curFile = home_ini_full_path("client.ini");
	if (!tw_exists(curFile.c_str()))
		static_copy_file(data_full_path("client.ini").c_str(),
			curFile.c_str());

	curFile = home_ini_full_path("fleets.ini");
	if (!tw_exists(curFile.c_str()))
		static_copy_file(data_full_path("fleets.ini").c_str(),
			curFile.c_str());

	curFile = home_ini_full_path("scp.ini");
	if (!tw_exists(curFile.c_str()))
		static_copy_file(data_full_path("scp.ini").c_str(),
			curFile.c_str());

	curFile = home_ini_full_path("server.ini");
	if (!tw_exists(curFile.c_str()))
		static_copy_file(data_full_path("server.ini").c_str(),
			curFile.c_str());

	curFile = home_ini_full_path("vobject.ini");
	if (!tw_exists(curFile.c_str()))
		static_copy_file(data_full_path("vobject.ini").c_str(),
			curFile.c_str());
	return 0;
}
