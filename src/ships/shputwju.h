/*
 *      shputwju.h
 *
 *      Copyright 2008 Yura Siamashka <yurand2@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef __SHPUTWJU_H__
#define __SHPUTWJU_H__

#include "ship.h"

class UtwigJugger : public Ship
{
	public:

		int fortitude;			 //added for gob

		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		UtwigJugger(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);
		virtual void calculate();
		virtual double isProtected() const;

	protected:
		virtual void calculate_fire_weapon();
		virtual void animate(Frame *space);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};
#endif
