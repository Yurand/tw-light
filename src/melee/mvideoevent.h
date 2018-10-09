/*
This file is part of "TW-Light"
					https://tw-light.appspot.com/
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

#ifndef __TW_VIDEOEVENT_H__
#define __TW_VIDEOEVENT_H__

#include <allegro.h>

#include "util/base.h"

class VideoEvent : public Event
{
	public:
		class VideoWindow *window;
		enum {
			/// \brief happens before surface is invalidated
			/// before a resolution change, whatever
			INVALID,
			/// \brief the opposite of an invalid event
			/// after an alt-tab back, after a resolution change, whatever
			VALID,
			/// \brief happens when size is changed
			/// after a resolution change, or a window resize
			/// ?? after alt-tabbing back in ??
			RESIZE,
			/// \brief happens when the color format changes
			CHANGE_BPP,
			/// \brief happens when contents are changed
			/// ?? after alt-tabbing back in ??
			/// ?? after window translated ??
			/// ? after any resize event ?
			REDRAW
		};
		virtual int _get_size() const {return sizeof(*this);}
};
extern volatile int debug_value;

#endif

