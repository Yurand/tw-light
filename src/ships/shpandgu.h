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

#ifndef __SHPANDGU_H__
#define __SHPANDGU_H__

class AndrosynthGuardian : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		int bounce_status;

		SpaceSprite *specialSprite;
		SpaceSprite *shipSprite;
		double       specialVelocity;
		double       specialTurnRate;
		int          specialDamage;
		int          specialBounceDistance;
		int          specialBounceTime;
		int          specialActive;
		double       shipTurnRate;
		int          shipRechargeAmount;
		double specialMass;
		double normalMass;

	public:
		AndrosynthGuardian(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void calculate();
		virtual void calculate_thrust();
		virtual void calculate_hotspots();

		virtual void inflict_damage(SpaceObject *other);

		virtual int activate_weapon();
		virtual int activate_special();
};

class AndrosynthBubble : public AnimatedShot
{
	int courseFrames;

	public:
		AndrosynthBubble(Vector2 opos, double oangle, double ov,
			int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
			int ofcount, int ofsize);

		void calculate();
};
#endif
