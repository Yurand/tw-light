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

/*------------------------------
		Video mode
------------------------------*/

#include <list>
#include <algorithm>

#include "mvideowindow.h"
#include "mvideosystem.h"

#include "other/gameconf.h"

#include "util/round.h"
#include "util/errors.h"

struct VW_lock_data
{
	short int x, y, w, h;
};

void VideoWindow::lock ( )
{
	//int i = lock_level;
	lock_level += 1;
	//if (!surface) return;
	if (lock_level == 1) {
		/*		if (!lock_data) lock_data = (VW_lock_data*)malloc(sizeof(VW_lock_data));
				if (w && h) {
					lock_data[i].x = surface->cl;
					lock_data[i].y = surface->ct;
					lock_data[i].w = surface->cr - surface->cl;
					lock_data[i].h = surface->cb - surface->ct;
					set_clip(surface, x, y, x+w-1, y+h-1);
				} else {
					lock_data[i].x = 0;
					lock_data[i].y = 0;
					lock_data[i].w = 0;
					lock_data[i].h = 0;
				}*/
//		set_clip(surface, x, y, x+w-1, y+h-1);
		set_clip_rect(surface, x, y, x + w - 1, y + h - 1);
		set_clip_state(surface, TRUE);
		acquire_bitmap(surface);
	}
}


void VideoWindow::unlock ( )
{
	//int i = lock_level - 1;
	if (lock_level == 0) {
		tw_error("VideoWindow unlocked too many times");
		return;
	}
	lock_level -= 1;
	//if (!surface) return;
	/*	if (w && h) {
			set_clip(surface, lock_data[i].x, lock_data[i].y,
				lock_data[i].x + lock_data[i].w - 1,
				lock_data[i].y + lock_data[i].h - 1
				);
		}*/
	if (lock_level == 0) {
		release_bitmap(surface);
		set_clip_rect(surface, 0, 0, surface->w - 1, surface->h - 1);
		set_clip_state(surface, TRUE);
		//set_clip(surface, 0, 0, surface->w-1, surface->h-1);
	}
}


#define VideoWindow_callbacklist_units 4
void VideoWindow::match ( VideoWindow *old )
{
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	if (!parent && old->parent) init( old->parent );
	locate (
		old->const_x, old->propr_x,
		old->const_y, old->propr_y,
		old->const_w, old->propr_w,
		old->const_h, old->propr_h
		);
	return;
}


void VideoWindow::hide()
{
	STACKTRACE;
	locate(0,0,0,0,0,0,0,0);
	return;
}


void VideoWindow::add_callback( BaseClass *callee )
{
	STACKTRACE;
	std::list<BaseClass*>::iterator cb = std::find(callback_list.begin(),callback_list.end(), callee);
	if (cb != callback_list.end()) {
		tw_error("adding VideoWindow callback twice");
	}
	callback_list.push_back(callee);
	return;
}


void VideoWindow::remove_callback( BaseClass *callee )
{
	callback_list.remove(callee);
	return;
}


void VideoWindow::event(int subtype)
{
	STACKTRACE;
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	VideoEvent ve;
	ve.type = Event::VIDEO;
	ve.subtype = subtype;
	ve.window = this;
	issue_event( callback_list, &ve);
	return;
}


void VideoWindow::update_pos()
{
	STACKTRACE;
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	int nx = 0, ny = 0, nw = 0, nh = 0;
	if (parent == this) {
		surface = videosystem.surface;
		if (surface) {
			nx = 0;
			ny = 0;
			nw = videosystem.width;
			nh = videosystem.height;
		}
	} else {
		if (parent) surface = parent->surface;
		else surface = NULL;
		if (surface) {
			nx = parent->x;
			ny = parent->y;
			nw = parent->w;
			nh = parent->h;
		}
	}

	x = nx + iround_up(const_x + propr_x * nw - 0.05);
	y = ny + iround_up(const_y + propr_y * nh - 0.05);
	w = iround_down(const_w + propr_w * nw + 0.05);
	h = iround_down(const_h + propr_h * nh + 0.05);

	if ((w <= 0) || (h <= 0)) surface = NULL;
	return;
}


void VideoWindow::_event( Event *e )
{
	STACKTRACE;
	if (e->type == Event::VIDEO) {
		const VideoEvent *ve = (const VideoEvent *) e;
		if (ve->window != parent) {tw_error ("VideoWindow event not from parent?");}
		VideoEvent nve;
		nve.type = Event::VIDEO;
		nve.window = this;
		nve.subtype = ve->subtype;
		switch (ve->subtype) {
			case VideoEvent::RESIZE :
			{
				update_pos();
			} break;
			case VideoEvent::INVALID:
			{
				x = y = w = h = 0;
				surface = NULL;
			} break;
			case VideoEvent::VALID:
			{
				update_pos();
			} break;
			case VideoEvent::REDRAW:
			{
				//update_pos();
			} break;
		}
		issue_event( callback_list, &nve);
	} else {
		tw_error ( "VideoWindow got non-video event" );
	}
	return;
}


void VideoWindow::preinit ()
{
	parent = NULL;
	surface = NULL;
	callback_list.clear();

	lock_level = 0;

	const_x = const_y = const_w = const_h = 0;
	propr_x = propr_y = 0;
	propr_w = propr_h = 1;
	x = y = w = h = 0;
	return;
}


void VideoWindow::init ( VideoWindow *parent_window)
{
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	if (parent) parent->remove_callback( this );
	parent = parent_window;
	//if (parent == this) {tw_error("VideoWindow - incest");}
	if (parent && (parent != this)) parent->add_callback ( this );
	update_pos();
	event(VideoEvent::RESIZE);
	event(VideoEvent::REDRAW);
	return;
}


void VideoWindow::locate ( double x1, double x2, double y1, double y2, double w1, double w2, double h1, double h2)
{
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	const_x = x1;
	propr_x = x2;
	const_y = y1;
	propr_y = y2;
	const_w = w1;
	propr_w = w2;
	const_h = h1;
	propr_h = h2;
	update_pos();
	event(VideoEvent::RESIZE);
	event(VideoEvent::REDRAW);
	return;
}


void VideoWindow::deinit()
{
	if (lock_level) {tw_error("VideoWindow - illegal while locked");}
	if (parent) {
		parent->remove_callback( this );
		parent = NULL;
	}
	if (callback_list.size())
		{tw_error("VideowWindow - deinit illegal while child windows remain");}
		/*for (int i = 0; i < num_callbacks; i += 1) {
			Event e;
			e.type =
			callback_list[i]->_event(e);
		}*/

		//free(callback_list);
		//callback_list = NULL;
		//num_callbacks = 0;
		callback_list.clear();

	//	free(lock_data);
	//	lock_data = NULL;
}


VideoWindow::~VideoWindow()
{
	deinit();
}
