/* $Id: shpleimu.cpp,v 1.10 2005/07/11 00:25:32 geomannl Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class LeiMule : public Ship
{
	public:
		double  weaponRange, weaponVelocity, weaponDamage, weaponArmour;
		double  specialRange, specialVelocity, specialDamage, specialArmour;

		bool shipmode;

	public:
		LeiMule(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:

		virtual int activate_weapon();
		virtual int activate_special();

		void engage_forward(double dweight);
		void engage_backward(double dweight);

};

LeiMule::LeiMule(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);

}


void LeiMule::engage_forward(double dweight)
{
	STACKTRACE;
	Vector2 P0;
	double A0, R0, Aweapon;

	P0 = Vector2(0,0);
	R0 = 50;

	Aweapon = 0.025 * PI;

	A0 = 0.5 * PI + Aweapon;

	int i, N;

	// per wing
	N = 1;

	for ( i = -N; i <= N; ++i ) {
		if (i == 0)
			continue;

		Vector2 rpos;

		rpos = P0 + (fabs(i*R0) / N) * unit_vector(A0*sign(i) - PI/2);

		double aoffs;
		aoffs = -i * Aweapon;

		Shot *s;
		s = new Shot(this, rpos, angle + aoffs,
			weaponVelocity, weaponDamage * dweight, weaponRange, weaponArmour,
			this, data->spriteWeapon, 0.0);
		s->isblockingweapons = true;
		add(s);
	}
}


void LeiMule::engage_backward(double dweight)
{
	STACKTRACE;
	Vector2 P0;
	double A0, R0, Aweapon;

	P0 = Vector2(0,0);
	R0 = 50;

	Aweapon = 0.025 * PI;

	A0 = 0.5 * PI + Aweapon;

	int i, N;

	// per wing
	N = 2;

	for ( i = -N; i <= N; ++i ) {
		if (i == 0)
			continue;

		Vector2 rpos;

		rpos = P0 + (fabs(i*R0) / N) * unit_vector(A0*sign(i) - PI/2);

		double aoffs;
		aoffs = -i * Aweapon + PI;

		Shot *s;
		s = new Shot(this, rpos, angle + aoffs,
			specialVelocity, specialDamage * dweight, specialRange, specialArmour,
			this, data->spriteSpecial, 0.0);
		s->isblockingweapons = true;
		add(s);
	}
}


int LeiMule::activate_weapon()
{
	STACKTRACE;
	engage_forward(1);

	//	engage_backward(1);

	return TRUE;
}


int LeiMule::activate_special()
{
	STACKTRACE;

	// do something else, but what
	// fire a dummy shot to confuse/irritate the enemy; this costs no energy.

	//	engage_forward(0);

	engage_backward(1);

	return TRUE;
}


REGISTER_SHIP(LeiMule)
