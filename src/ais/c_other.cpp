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

#include "ais.h"
REGISTER_FILE

/*! \brief Get control name */
const char *ControlVegetable::getTypeName()
{
	STACKTRACE;
	return "VegetableBot";
}


/*! \brief This AI do nothing */
int ControlVegetable::think()
{
	STACKTRACE;
	return 0;
}


/*! \brief This function do nothing
  \return -1
*/
int ControlVegetable::choose_ship(VideoWindow *window, char * prompt, class Fleet *fleet )
{
	STACKTRACE;
	return -1;
}


ControlVegetable::ControlVegetable (const char *name, int channel) : Control(name, channel)
{
}


const char *ControlMoron::getTypeName()
{
	STACKTRACE;
	return "MoronBot";
}


/*! \brief Simple AI
  \param ship with this stupid AI
*/
int stupid_bot(Ship *ship)
{
	STACKTRACE;

	int r = 0;
	double a;
	if (!ship->target) return 0;
	if (!ship->target->exists()) {
		ship->target = NULL;
		return 0;
	}
	a = ship->trajectory_angle(ship->target) - ship->get_angle();
	a = fmod(a + PI2, PI2);
	if (a < PI) {
		r |= keyflag::right;
	} else {
		r |= keyflag::left;
		return r;
	}
	a = int(ship->distance(ship->target));
	if (a > 2000) {
		r |= keyflag::thrust;
	} else {
		r |= keyflag::thrust;
		r |= keyflag::fire;
	}
	return r;
}


/*! \brief Summon stupid_bot() */
int ControlMoron::think()
{
	STACKTRACE;
	if (ship)
		return stupid_bot(ship);
	else
		return 0;
}


ControlMoron::ControlMoron(const char *name, int channel) : Control(name, channel)
{
	STACKTRACE;
}
