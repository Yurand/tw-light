/*
 *      gameconf.c
 *
 *      Copyright 2001-2018  TimeWarp development team
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
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <codecvt>

#ifdef WIN32
#include <io.h>
#include <shlobj.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "util/errors.h"
#include "util/helper.h"
#include "other/twconfig.h"

static void static_get_home_directory(char* buff, int len)
{
#ifdef WIN32
	char homedir[2048];
	const char *twname = "tw-light";
	const wchar_t *wtwname = L"tw-light";
	wchar_t whomedir[2048];
	SHGetFolderPathW(0, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, whomedir);

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	strcpy(homedir, convert.to_bytes(whomedir).c_str());
#else
	char homedir[2048];
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
	} else if (tw_exists("./data/gob.dat")) {
		base_dir = "./data";
	} else {
		base_dir = TWLIGHT_DATADIR;
	}

	if (tw_is_relative_filename(base_dir.c_str())) {
		char buffer[50000];
#ifdef WIN32
		wchar_t wbuffer[5000];
		wchar_t *wcwd = _wgetcwd(wbuffer, sizeof(wbuffer) / sizeof(wbuffer[0]));

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
		std::string dest = convert.to_bytes(wcwd);
		strcpy(buffer, convert.to_bytes(wcwd).c_str());
#else
		char *cwd = getcwd(buffer, sizeof(buffer));
#endif
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

int create_user_ini()
{
	STACKTRACE;

	std::string curFile = home_ini_full_path("");
	if (!tw_exists(curFile.c_str()))
		tw_mkdir(curFile.c_str());

	bool needUpdate = false;
	curFile = home_ini_full_path("client.ini");
	if (!tw_exists(curFile.c_str())) {
		needUpdate = true;
	} else {
		tw_set_config_file("client.ini");
		const char* homeVersion = tw_get_config_string("System", "Version", "0.0");
		if (strcmp(homeVersion, VERSION) != 0)
			needUpdate = true;
	}

	curFile = home_ini_full_path("client.ini");
	if (!tw_exists(curFile.c_str()) || needUpdate)
		if (!tw_copy_file(data_full_path("client.ini").c_str(), curFile.c_str())) {
			tw_error(tw_string_format("unable to copy client.ini file to '%s'", curFile.c_str()).c_str());
		}

	curFile = home_ini_full_path("fleets.ini");
	if (!tw_exists(curFile.c_str()) || needUpdate)
		if (!tw_copy_file(data_full_path("fleets.ini").c_str(), curFile.c_str())) {
			tw_error(tw_string_format("unable to copy fleets.ini file to '%s'", curFile.c_str()).c_str());
		}

	curFile = home_ini_full_path("scp.ini");
	if (!tw_exists(curFile.c_str()) || needUpdate)
		if (!tw_copy_file(data_full_path("scp.ini").c_str(), curFile.c_str())) {
			tw_error(tw_string_format("unable to copy scp.ini file to '%s'", curFile.c_str()).c_str());
		}

	curFile = home_ini_full_path("server.ini");
	if (!tw_exists(curFile.c_str()) || needUpdate)
		if (!tw_copy_file(data_full_path("server.ini").c_str(), curFile.c_str())) {
			tw_error(tw_string_format("unable to copy server.ini file to '%s'", curFile.c_str()).c_str());
		}

	curFile = home_ini_full_path("vobject.ini");
	if (!tw_exists(curFile.c_str()) || needUpdate)
		if (!tw_copy_file(data_full_path("vobject.ini").c_str(), curFile.c_str())) {
			tw_error(tw_string_format("unable to copy vobject.ini file to '%s'", curFile.c_str()).c_str());
		}
	return 0;
}
