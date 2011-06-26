/*
 *      shpsupbl.h
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

#ifndef __SHPSUPBL_H__
#define __SHPSUPBL_H__

class SupoxBlade : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		SupoxBlade(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual void calculate_thrust();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_hotspots();

		virtual int activate_weapon();
		virtual void calculate_fire_special();
};
#endif
