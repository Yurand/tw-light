/*
This file is part of "TW-Light"
					https://tw-light.xyz
Copyright (C) 2001-2013  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef __TW_VIDEOWINDOW_H__
#define __TW_VIDEOWINDOW_H__

#include <allegro.h>

#include "util/port.h"
#include "util/base.h"

#include "melee/mvideoevent.h"

class VideoWindow : public BaseClass
{
	float const_x, const_y, const_w, const_h;
	float propr_x, propr_y, propr_w, propr_h;

	char lock_level;
	//struct VW_lock_data *lock_data;

	public:void update_pos();

	VideoWindow *parent;
	std::list<BaseClass*> callback_list;
	public:
		void add_callback ( BaseClass * );
		void remove_callback ( BaseClass * );
		virtual void _event ( Event *e );

		int x, y, w, h;
		Surface *surface;
		void event(int subtype);
		void redraw() {event(VideoEvent::REDRAW);}
		void preinit ();
		void init ( VideoWindow *parent_window);
		void locate ( double c_x, double p_x, double c_y, double p_y, double c_w, double p_w, double c_h, double p_h );
		void lock();
		void unlock();
		void hide();
		void match ( VideoWindow *old );
		void deinit();
		virtual ~VideoWindow();
		virtual int _get_size() const {return sizeof(*this);}
};


#endif

