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

#ifndef __SHPARISK_H__
#define __SHPARISK_H__

class ArilouSkiff : public Ship
{
	public:
		int    weaponColor;
		double weaponRange;
		int    weaponFrames;
		int    weaponDamage;

		int just_teleported;

		SpaceSprite *specialSprite;
		double       specialFrames;

	public:
		ArilouSkiff(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		virtual void inflict_damage(SpaceObject *other);
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void calculate_gravity();
		virtual int accelerate(SpaceLocation *source, double angle, double velocity,
			double max_speed);
		virtual int accelerate_gravwhip(SpaceLocation *source, double angle, double velocity,
			double max_speed);
};
#endif
