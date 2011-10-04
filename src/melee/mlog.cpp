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

#include <stdio.h>
#include <string.h>
#include <allegro.h>

#include "melee.h"

#include "mlog.h"
#include "mgame.h"
#include "other/twconfig.h"
#include "scp.h"

////////////////////////////////////////////////////////////////////////
//				Logging stuff
////////////////////////////////////////////////////////////////////////

void Log::init()
{
	STACKTRACE;
	log_len  = NULL;
	log_size = NULL;
	log_pos  = NULL;
	log_data = NULL;
	log_dir = NULL;
	default_direction = direction_write | direction_read;
	log_num = 0;
	playback = false;
	type = Log::log_normal;
	return;
}


void Log::set_direction ( int channel, char direction )
{
	if (channel < 0) { tw_error("set_direction - channel < 0"); }
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	log_dir[channel] = direction;
	return;
}


char Log::get_direction ( int channel )
{
	if (channel < 0) {tw_error("get_direction - channel < 0");}
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	return (log_dir[channel]);
}


void Log::set_all_directions ( char direction )
{
	default_direction = direction;
	int i;
	for (i = 0; i < log_num; i += 1) {
		log_dir[i] = direction;
	}
	return;
}


Log::~Log()
{
	if (log_data) for (int i = 0; i < log_num; i += 1) {
		free(log_data[i]);
	}
	free(log_len);
	free(log_size);
	free(log_pos);
	free(log_data);
	free(log_dir);
	log_num = 0;
}


void Log::log ( int channel, void *data, int size)
{
	if (!size) return;
	if (channel < 0) {tw_error ("Log::log - negative channel!");}
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	if (log_dir[channel] & direction_write) _log ( channel, data, size);
	if (log_dir[channel] & direction_read) _unlog ( channel, data, size);
	return;
}


void Log::_log(int channel, const void *data, int size)
{
	STACKTRACE;
	log_len[channel] += size;
	while (log_len[channel] > log_size[channel]) {
		if (log_size[channel]) log_size[channel] = log_size[channel] * 2;
		else log_size[channel] = 32;
		log_data[channel] = (unsigned char *) realloc(log_data[channel], log_size[channel]);
	}
	memcpy(log_data[channel]+(log_len[channel]-size), data, size);
	return;
}


void Log::_unlog(int channel, void *data, int size)
{
	STACKTRACE;
	if (log_len[channel] < log_pos[channel] + size) {tw_error ("Game::_unlog - went past end (%d+%d/%d on %d)", log_pos[channel], size, log_len[channel], channel);}
	memcpy(data, log_data[channel]+log_pos[channel], size);
	log_pos[channel] += size;
	return;
}


void Log::save (const char *fname)
{
	PACKFILE *f;
	int i, j;
	f = pack_fopen ( home_ini_full_path(fname).c_str(), F_WRITE_PACKED);
	if (!f) throw "Log::save failed";
	j = 0;
	for (j = 0, i = 0; i < log_num; i += 1) if (log_len[i] && !(log_dir[i] & direction_forget)) j += 1;
	pack_iputl ( j, f);
	for (i = 0; i < log_num; i += 1) {
		if (!log_len[i] || (log_dir[i] & direction_forget)) continue;
		f = pack_fopen_chunk(f, 0);
		pack_iputl ( i, f);
		pack_iputl ( log_len[i], f);
		pack_fwrite ( log_data[i], log_len[i], f);
		f = pack_fclose_chunk(f);
	}
	pack_fclose (f);
	return;
}


void Log::load (const char *fname)
{
	PACKFILE *f;
	char buffy[1024];
	int i, oi, j, mj;
	f = pack_fopen ( home_ini_full_path(fname).c_str(), F_READ_PACKED);
	if (!f) throw("Log::load() failed");
	mj = pack_igetl ( f);
	oi = -1;
	for (j = 0; j < mj; j += 1) {
		f = pack_fopen_chunk(f, 0);
		i = pack_igetl ( f);
		if (i <= oi) { tw_error("invalid log file"); }
		if (i >= log_num) expand_logs(i+1);
		int l = pack_igetl ( f);
		while ( l > 0) {
			int tl = 1024;
			if (tl > l) tl = l;
			int k = pack_fread(buffy, tl, f);
			if (k == 0) { tw_error("invalid log file"); }
			l -= k;
			_log(i, buffy, tl);
		}
		f = pack_fclose_chunk(f);
	}
	pack_fclose (f);
	return;
}


void Log::expand_logs(int num_channels)
{
	STACKTRACE;
	int old_log_num = log_num;
	if (num_channels <= log_num) { tw_error ("Log::expand_logs - shrinking logs?"); }
	log_num = num_channels;
	log_data = (unsigned char**) realloc(log_data, sizeof(char*) * log_num);
	log_size    = (int*) realloc(log_size, sizeof(int) * log_num);
	log_pos     = (int*) realloc(log_pos,  sizeof(int) * log_num);
	log_len     = (int*) realloc(log_len,  sizeof(int) * log_num);
	log_dir     = (char*) realloc(log_dir,  sizeof(char) * log_num);
	for (int i = old_log_num; i < log_num; i += 1) {
		log_data[i] = NULL;
		log_size[i] = 0;
		log_len[i] = 0;
		log_pos[i] = 0;
		log_dir[i] = default_direction;
	}
	return;
}


int Log::ready(int channel)
{
	STACKTRACE;
	if (channel < 0) { tw_error ("log_ready - negative channel!"); }
	if (channel >= log_num) return 0;
	return log_len[channel] - log_pos[channel];
}


int Log::file_ready(const char *fname, void **location)
{
	STACKTRACE;
	if (log_num <= channel_file_data) return -1;
	int i = 0, j = 0;
	while (i < log_len[channel_file_names]) {
		if (strcmp((char*) log_data[channel_file_names] + i, home_ini_full_path(fname).c_str())) {
			i += strlen((char*) log_data[channel_file_names] + i) + 1;
			//			j += intel_ordering(*((int*)(log_data[channel_file_names] + i)));
			int k;
			memcpy(&k, (log_data[channel_file_names] + i), sizeof(int));
			j += intel_ordering(k);
			i += sizeof(int);
		} else {
			i += strlen((char*) log_data[channel_file_names] + i) + 1;
			int k;
			memcpy(&k, (log_data[channel_file_names] + i), sizeof(int));
			k = intel_ordering(k);

			if (j+k > log_len[channel_file_data]) { tw_error ("Log::file_ready - uh, that's bad"); }
			if (location) *location = log_data[channel_file_data] + j;
			return k;
		}
	}
	return -1;
}


void Log::log_file(const std::string& fname)
{
	STACKTRACE;
	void *loc;
	if (!(log_dir[channel_file_data] & direction_read)) {
		set_config_file(home_ini_full_path(fname).c_str());
		return;
	}
	int len = file_ready(home_ini_full_path(fname).c_str(), &loc);
	if (len >= 0) {
		set_config_data((char*)loc, len);
		return;
	}
	if (!(log_dir[channel_file_data] & direction_write)) {
		tw_error("Log::log_file - file logs read only, \"%s\" not found",
			fname.c_str());
	}
	if (log_num <= channel_file_data) {
		expand_logs(channel_file_data + 1);
	}
	char buffy[2048];
	PACKFILE *f;
	int i, j = 0;
	f = pack_fopen(home_ini_full_path(fname).c_str(), F_READ);
	if (!f) {
		tw_error("tw_log_file - bad file name %s", home_ini_full_path(fname).c_str());
	}
	while (1) {
		i = pack_fread(buffy, 1024, f);
		if (i > 0) {
			_log(channel_file_data, buffy, i);
			j += i;
		}
		else break;
	}
	sprintf(buffy, "%s", home_ini_full_path(fname).c_str());
	j = intel_ordering(j);
	memcpy(&buffy[strlen(buffy)+1], &j, sizeof(int));
	_log (channel_file_names, buffy, strlen(buffy)+5);
	len = file_ready(fname.c_str(), &loc);
	set_config_data((char*)loc, len);
	return;
}


void Log::deinit()
{
	STACKTRACE;
	return;
}


bool Log::buffer ( int channel, void *data, int size )
{
	char zeros[128];
	if (!size) return false;	 //return true?  error?
	if (channel < 0) { tw_error ("Log::log - negative channel!"); }
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	if (data == NULL) {
		data = zeros;
		if (size > 128) { tw_error("Log::buffer - overflow"); }
		memset(zeros, 0, size);
	}
	if (log_dir[channel] & direction_write) {
		_log ( channel, data, size);
		//		if (!(log_dir[channel] & direction_immediate)) send_packet();
		return true;
	}
	//	if (log_dir[channel] & direction_read) _unlog ( channel, data, size);
	return false;
}


bool Log::unbuffer ( int channel, void *data, int size )
{
	char zeros[128];
	if (!size) return false;	 //return true?  error?
	if (channel < 0) { tw_error ("Log::unbuffer - negative channel!"); }
	if (channel >= log_num) {
		expand_logs(channel+1);
	}
	if (data == NULL) {
		data = zeros;
		if (size > 128) { tw_error("Log::buffer - overflow"); }
	}

	//	if (log_dir[channel] & direction_write) _log ( channel, data, size);
	if (log_dir[channel] & direction_read) {
		_unlog ( channel, data, size);
		return true;
	}
	return false;
}


void Log::flush()
{
	STACKTRACE;
	return;
}


bool Log::listen()
{
	STACKTRACE;
	return false;
}


void Log::reset()
{
	STACKTRACE;
	int i;
	for (i = 0 ; i < this->log_num; i += 1) {
		log_pos[i] = 0;
	}
	return;
}


void PlaybackLog::init()
{
	STACKTRACE;
	Log::init();
	playback = true;
	default_direction = Log::direction_read;
	Log::set_direction(Game::channel_playback, Log::direction_read | Log::direction_write | Log::direction_forget);
	Log::set_direction(Game::channel_playback + Game::_channel_buffered, Log::direction_read | Log::direction_write | Log::direction_forget);
	return;
}


void PlaybackLog::set_direction (int channel, char direction)
{
	tw_error("set_direction - your not supposed to do that in a demo playback!");
	return;
}


void PlaybackLog::set_all_directions( char direction )
{
	STACKTRACE;
	tw_error("set_all_directions - your not supposed to do that in a demo playback!");
	return;
}
