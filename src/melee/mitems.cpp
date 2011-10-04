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

#include <allegro.h>
#include "melee.h"
REGISTER_FILE

#include "frame.h"
#include "mframe.h"
#include "mview.h"
#include "mitems.h"
#include "mgame.h"
#include "mship.h"

HealthBar::HealthBar(Ship *creator, int *toggle)
{
	STACKTRACE;
	scale = 2.0;
	bartoggle = toggle;
	mother = creator;
}


void HealthBar::calculate()
{
	STACKTRACE;

	if ( !(mother && mother->exists()) ) {
		state = 0;
		return;
	}

}


void HealthBar::draw_bar(Ship *s, double yoffs, int len, double H, double fraction, int col1, int col2, Frame *space)
{
	STACKTRACE;

	Vector2 center;
	int d;

	H = iround(H * space_zoom);
	if (H < 1)
		H = 1;					 // minimum thickness.

								 // scale
	len = iround(len * space_zoom);

	center = corner(s->pos);	 // scales and shifts onto screen coord. of the (center of the) ship

	int ix, iy;
	ix = iround(center.x - len/2);
	iy = iround(center.y - (0.6 * s->size.y + H/2 + yoffs) * space_zoom);

	d = iround(len * fraction);

	H -= 1;						 // for plotting, pixel 0 also counts

	if (ix > space->surface->w) return;
	if (iy > space->surface->h) return;
	if (ix+len < 0) return;
	if (iy+H < 0) return;

	if (d > 0)
		rectfill(space->surface, ix, iy, ix+d-1, iy+(int)H, col1);
	rectfill(space->surface, ix+d, iy, ix+len, iy+(int)H, col2);

	space->add_box(ix, iy, ix+len, iy+H);
}


void HealthBar::animate(Frame *space)
{
	STACKTRACE;

	if (!*bartoggle)
		return;

	if (mother->isInvisible())
		return;

	int H = 2;
	double dy = 4;

	if ((dy - H/2) * space_zoom < 1)
		dy = H/2 + 1/space_zoom;

	draw_bar(mother, -dy, iround(mother->crew_max * scale), H, mother->crew/mother->crew_max,
		makecol(0, 255, 0), makecol(150, 0, 0), space);

	draw_bar(mother,  dy, iround(mother->batt_max * scale), H, mother->batt/mother->batt_max,
		makecol(255, 50, 50), makecol(150, 0, 0), space);

}


Indicator::Indicator() : Presence()
{
	STACKTRACE;
}


bool Indicator::coords(Frame *space, SpaceLocation *l, Vector2 *pos, Vector2 *a_pos)
{
	STACKTRACE;
	Vector2 p = corner(l->normal_pos());
	Vector2 op = p;

	p -= space_view_size / 2;

	if (p.x < -space->surface->w/2) {
		double a = p.x / -(space->surface->w/2 + 10);
		p /= a;
	}
	if (p.y < -space->surface->h/2) {
		double a = p.y / -(space->surface->h/2 + 10);
		p /= a;
	}
	if (p.x > space->surface->w/2) {
		double a = p.x / (space->surface->w/2 + 10);
		p /= a;
	}
	if (p.y > space->surface->h/2) {
		double a = p.y / (space->surface->h/2 + 10);
		p /= a;
	}
	p += space_view_size / 2;
	*pos = p;
	if (a_pos) *a_pos = op;

	if ((op.x > 0) && (op.x < space->surface->w) &&
		(op.y > 0) && (op.y < space->surface->h)) return false;
	return true;
}


BlinkyIndicator::BlinkyIndicator(SpaceObject *target, int color) : Indicator()
{
	STACKTRACE;
	this->target = target;
	this->color = color;
}


void BlinkyIndicator::animate(Frame *space)
{
	STACKTRACE;
	if ((game->game_time >> 8) & 1) return;
	Vector2 p;
	int a = coords(space, target, &p);
	if (!a) return;
	SpaceSprite *s = target->get_sprite();
	p -= s->size() * space_zoom / 2;
	p -= space_view_size / 2;

	p.x -= sign(p.x) * s->width()  / 4 * sqrt(space_zoom);
	p.y -= sign(p.y) * s->height() / 4 * sqrt(space_zoom);
	//shouldn't be sqrt ... maybe there's a bug somewhere

	p += space_view_size / 2;
	if (color == -1) {
		s->draw(p, s->size() * space_zoom,
			target->get_sprite_index(), space);
	} else {
		Vector2 size = s->size() * space_zoom;
		s->draw_character((int)p.x, (int)p.y, (int)size.x, (int)size.y,
			target->get_sprite_index(), palette_color[color], space);
	}
	return;
}


void BlinkyIndicator::calculate()
{
	STACKTRACE;
	if (!target->exists()) die();
}


WedgeIndicator::WedgeIndicator(SpaceLocation *target, int length, int color) : Indicator()
{
	STACKTRACE;
	this->target = target;
	this->length = length;
	this->color = color;
}


void WedgeIndicator::animate(Frame *space)
{
	STACKTRACE;
	double a, a2;
	Vector2 p, p2, tmp;
	if (target->isInvisible() > 0.5) return;
	if (!coords(space, target, &p, &p2)) return;
	a = PI + atan(p - space_view_size / 2);
	int ix, iy;
	tmp = p + unit_vector(a+PI*.15) * length;
	a2 = PI + atan(tmp-p2);
	ix = int(tmp.x);
	iy = int(tmp.y);
	line(space->surface, ix, iy, int(ix+cos(a2)*length), int(iy+sin(a2)*length), pallete_color[color]);
	space->add_line  ( ix, iy, int(ix+cos(a2)*length), int(iy+sin(a2)*length));
	tmp = p + unit_vector(a-PI*.15) * length;
	a2 = PI + atan(tmp-p2);
	ix = int(tmp.x);
	iy = int(tmp.y);
	line(space->surface, ix, iy, int(ix+cos(a2)*length), int(iy+sin(a2)*length), pallete_color[color]);
	space->add_line  ( ix, iy, int(ix+cos(a2)*length), int(iy+sin(a2)*length));
	return;
}


void WedgeIndicator::calculate()
{
	STACKTRACE;
	if (!target->exists()) die();
}


Orbiter::Orbiter ( SpaceSprite *pic, SpaceLocation *orbit_me, double distance) :
SpaceObject(NULL, orbit_me->normal_pos(), random(PI2), pic)
{
	layer = LAYER_CBODIES;
	mass = 99;
	center = orbit_me;
	radius = distance;
	pos -= unit_vector(angle) * radius;
	accelerate(this, angle + PI/2 + PI*(random()&1), 0.15, MAX_SPEED);
}


void Orbiter::calculate()
{
	STACKTRACE;
	angle = trajectory_angle(center) + PI;
	sprite_index = get_index(angle);
	double r = distance(center) / radius;

	if (r < 1) {
		accelerate(this, angle +PI, 0.0001 * (r-1) * frame_time, MAX_SPEED);
	} else {
		accelerate(this, angle, 0.0001 * (1-r) * frame_time, MAX_SPEED);
		if (r > 4) translate(unit_vector(angle) * radius*(4-r));
	}
	if (random() & 3) return;
	double  va, vb, a;
	Vector2 d;
	d = pos - nearest_pos(center);
	a = magnitude(d);
	d /= a;
	va = vel.x * d.x + vel.y * d.y;
	vb = vel.x * d.y - vel.y * d.x;
	if ((r > 1) && (va > 0)) {
		va *= 1 - 0.001 * frame_time;
	}
	else if ((r < 1) && (va < 0)) {
		vb *= 1 + 0.001 * frame_time;
		va *= 1 - 0.001 * frame_time;
	}
	vel.x = va * cos(angle) + vb * sin(angle);
	vel.y = va * sin(angle) - vb * cos(angle);
	return;
}
