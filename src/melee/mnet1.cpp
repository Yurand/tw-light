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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "melee.h"
REGISTER_FILE
#include "mnet1.h"
#include "mview.h"
#include "mcontrol.h"
//#include "mcbodies.h"
#include "scp.h"

GameEventMessage::GameEventMessage (const char *text)
{
	int l = strlen(text);
	if (l > max_message_length) l = max_message_length;
	memcpy(message, text, l);
	size = sizeof(GameEvent) + l;
	type = Game::event_message;
}


void GameEventMessage::execute( int source )
{
	STACKTRACE;
	char buffy[64+max_message_length];
	char *tmp = buffy;
	int c = 15;
	if ((unsigned short int)size > max_message_length + sizeof(GameEvent)) { tw_error("GameEventMessage - message overflow"); }
	//if (source == Game::channel_server) tmp += sprintf(tmp, "Server says: ");
	//else if (source == Game::channel_client) tmp += sprintf(tmp, "Client says: ");
	if (source == Game::channel_server) c = 13; else c = 9;
	int s = size - sizeof(GameEvent);
	memcpy(tmp, message, s);
	tmp[s] = 0;
	::message.out(buffy, 6000, c);
}


void GameEventChangeLag::execute( int source )
{
	STACKTRACE;
	if (source != Game::channel_server) return;
	if (old_lag != game->lag_frames) return;
	int i;
	for (i = old_lag; i != new_lag; ) {
		if (i < new_lag) {
			game->increase_latency();
			i += 1;
		}
		else if (i > new_lag) {
			game->decrease_latency();
			i -= 1;
		}
	}
}


int read_length_code (int max, int *clen, int *len, unsigned char *where)
{
	if (max < 1) return -1;
	*clen = 1;
	*len = where[0];
	if (*len < 255) return 0;
	*clen = 3;
	if (*clen > max) return -1;
	*len = where[1] + (where[2] << 8);
	if (*len < 65535) return 0;
	*clen = 7;
	if (*clen > max) return -1;
	*len = where[3] + (where[4] << 8) + (where[5] << 16) + (where[6] << 24);
	return 0;
}


int write_length_code (int max, int *clen, int len, unsigned char *where)
{
	if (len <= 0) { tw_error( "write_length_code -- bad length"); }
	if (max < 1) return -1;
	if (len < 255) {
		where[0] = len;
		*clen = 1;
		return 0;
	} else {
		if (max < 3) return -1;
		where[0] = 255;
		if (len < 65535) {
			where[1] = len & 255;
			where[2] = len >> 8;
			*clen = 3;
			return 0;
		} else {
			if (max < 7) return -1;
			where[1] = 255;
			where[2] = 255;
			where[3] = (len >> 0) & 255;
			where[4] = (len >> 8) & 255;
			where[5] = (len >> 16) & 255;
			where[6] = (len >> 24) & 255;
			*clen = 7;
			return 0;
		}
	}
}


void NetLog::init()
{
	STACKTRACE;
}


void NetLog::deinit()
{
	STACKTRACE;
}


NetLog::~NetLog()
{
}


void NetLog::send_packet()
{
	STACKTRACE;
}


void NetLog::recv_packet()
{
	STACKTRACE;
}


void NetLog::expand_logs(int num_channels)
{
	STACKTRACE;
	return;
}


void NetLog::_log(int channel, const void *data, int size)
{
	STACKTRACE;
}


void NetLog::_unlog(int channel, void *data, int size)
{
	STACKTRACE;
	return;
}


void NetLog::log_file(const std::string& fname)
{
	STACKTRACE;
}


void NetLog::flush()
{
	STACKTRACE;
}


bool NetLog::listen()
{
	STACKTRACE;
	return false;
}


int NetLog::ready(int channel)
{
	STACKTRACE;
	return 0;
}
