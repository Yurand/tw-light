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

#ifndef __GAMEBUTTONEVENT_H__
#define __GAMEBUTTONEVENT_H__

/** \brief Referenced by pointer by a button; if the value is non-zero, it
calls the routines in this struct, based on the mouse state. In this way you
can define specific funtions inside your game code. It's a template, so that it allows
you to pass pointers to game-class functions, i.e., functions which have access
to all the variables you need.
*/
template <class G>
class BEvent : public IButtonEvent
{
	public:
		typedef void (G::* ftest) ();

		G *g;
		ftest mainpress, mainhold, specialpress, specialhold;

		BEvent(G *gg, ftest m1, ftest m2);
		BEvent(G *gg, ftest m1, ftest m2, ftest s1, ftest s2);

		virtual void handle_main_press();
		virtual void handle_main_hold();
		virtual void handle_special_press();
		virtual void handle_special_hold();
};

template <class G>
BEvent<G>::BEvent<G>(G *gg, ftest m1, ftest m2)
{
	g = gg;
	mainpress = m1;
	mainhold = m2;
}


template <class G>
BEvent<G>::BEvent<G>(G *gg, ftest m1, ftest m2, ftest s1, ftest s2)
{
	g = gg;
	mainpress = m1;
	mainhold = m2;
	specialpress = s1;
	specialhold = s2;
}


template <class G>
void BEvent<G>::handle_main_press()
{
	if (mainpress)
		(g->*mainpress) ();
}


template <class G>
void BEvent<G>::handle_main_hold()
{
	if (mainhold)
		(g->*mainhold) ();
}


template <class G>
void BEvent<G>::handle_special_press()
{
	if (specialpress)
		(g->*specialpress) ();
}


template <class G>
void BEvent<G>::handle_special_hold()
{
	if (specialhold)
		(g->*specialhold) ();
}
#endif							 // __GAMEBUTTONEVENT_H__
