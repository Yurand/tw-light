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

class HealthBar : public Presence
{
	public:
		double  scale;
		int     *bartoggle;
		Ship    *mother;

		HealthBar(Ship *creator, int *toggle);
		void calculate();
		void animate(Frame *space );

		void draw_bar(Ship *s, double yoffs, int len, double H, double fraction, int col1, int col2, Frame *space);
};

class Indicator : public Presence
{
	public:
		Indicator();
		bool coords(Frame *space, SpaceLocation *l, Vector2 *pos, Vector2 *apos = NULL) ;
		void animate(Frame *space) = 0;
};

class BlinkyIndicator : public Indicator
{
	public:
		SpaceObject *target;
		int color;
		BlinkyIndicator(SpaceObject *target, int color = -1);
		virtual void animate(Frame *space);
		virtual void calculate();
};

class WedgeIndicator : public Indicator
{
	public:
		SpaceLocation *target;
		int color;
		int length;
		WedgeIndicator(SpaceLocation *target, int length, int color);
		virtual void animate(Frame *space);
		virtual void calculate();
};

class Orbiter : public SpaceObject
{
	public:
		SpaceLocation *center;
		double radius;
		virtual void calculate();
		Orbiter ( SpaceSprite *sprite, SpaceLocation *orbit, double distance);
};
