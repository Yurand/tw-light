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

#ifndef __MNET1_H__
#define __MNET1_H__

#include "mmain.h"
#include "mlog.h"
#include "mship.h"

class GameEvent
{
	public:
		short int size;
		short int type;
		GameEvent() {
			size = sizeof(this);
			type = Game::event_invalid;
		}
		void *operator new (size_t size) {return malloc(size);}
};
class GameEventChangeLag : public GameEvent
{
	public:
		short int old_lag;
		short int new_lag;
		GameEventChangeLag( int lag_frames ) {
			size = sizeof(this);
			type = Game::event_change_lag;
			old_lag = game->lag_frames;
			new_lag = lag_frames;
		}
		void execute ( int source );
};
class GameEventMessage : public GameEvent
{
	public:
		enum { max_message_length = 150 };
		char message[max_message_length];
		GameEventMessage( const char *text ) ;
		void execute ( int source );
};

/*
class LagHandler {
	unsigned char lag;  //a copy of lag_frames
	unsigned char size; //bytes per frame
	char channel;       //channel to transmit on
	void *data;         //serialized
//	void log ( void *data,
};*/

class NetLog : public Log		 //Logging system, usefull for networking & demo recording/playback
{
	protected:

		int *log_transmitted;	 //the number of bytes transmitted in each channel
								 //intializes these extensions to the logging
		void expand_logs(int num_channels) ;
								 //a buffer for sending and recieving packets
		unsigned char buffy[4096];
		//	void handle_code(unsigned int code) ;
		//	void send_code(unsigned int code) ;
		enum code
		{
			NET1_CODE_QUIT    = 0x80000001,
			NET1_CODE_PAUSE   = 0x80000002,
			NET1_CODE_UNPAUSE = 0x80000003,
			NET1_CODE_INCREASE_LAG = 0x80000004,
			NET1_CODE_DECREASE_LAG = 0x80000005,
			NET1_CODE_MESSAGE = 0x90000000
		};
		//	void send_message(char *string) ;

		void send_packet();		 //sends a packet

	public:

		bool need_to_transmit;

		enum {
			direction_immediate = 16
		};

		void recv_packet();		 //recieves a packet

		int remote_time;		 //used in calculating ping
		int ping;				 //the most recently measured ping

		virtual void init();
		virtual void deinit();
		virtual ~NetLog();
								 //used for recording data.  this version may transmit it over the network
		virtual void _log(int channel, const void *data, int size);
								 //used for playing back data.  this version may recieve it over the network
		virtual void _unlog(int channel, void *data, int size);
		void log_file(const std::string& fname) ;
		virtual int ready(int channel);

		virtual void flush() ;
		virtual bool listen();

};
#endif							 // __MNET1_H__
