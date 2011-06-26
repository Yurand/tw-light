/* $Id: shptougr.cpp,v 1.8 2004/03/24 23:51:42 yurand Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class ToUl : public Ship
{
	double  weaponRange, weaponVelocity, weaponDamage, weaponArmour;
	double  specialRange, specialDamage;
	int     specialColor;

	double  lasertime, laserperiod;

	bool shipmode;

	public:
		ToUl(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:

		virtual int activate_weapon();
		virtual int activate_special();

		virtual void calculate();

};

ToUl::ToUl(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialColor  = get_config_int("Special", "Color", 0);
	specialRange  = scale_range(get_config_float("Special", "Range", 0));
	specialDamage = get_config_int("Special", "Damage", 0);

	lasertime = 0;
	laserperiod = 2.0;
}


int ToUl::activate_weapon()
{
	STACKTRACE;

	int i;
	double v, x, y;

	for ( i = -3; i < 4; ++i ) {
		v = weaponVelocity * (0.8 + 0.2 * abs(i) / 3.0);
		x = i * 10;
		y = 50 + abs(i) * 10;

		add(new Missile(this, Vector2(x, y), angle,
			v, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon, 0.0));
	}

	return TRUE;
}


int ToUl::activate_special()
{
	STACKTRACE;

	double L;
	L = specialRange * (0.1 + 0.9 * sin(PI2 * lasertime / laserperiod));

	add(new Laser(this, angle + PI/2,
		pallete_color[specialColor], L, specialDamage, special_rate,
		this, Vector2(30.0, 0.0), true));

	add(new Laser(this, angle - PI/2,
		pallete_color[specialColor], L, specialDamage, special_rate,
		this, Vector2(-30.0, 0.0), true));

	return TRUE;
}


void ToUl::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if (!state)
		return;

	lasertime += frame_time * 1E-3;
	while (lasertime > laserperiod)
		lasertime -= laserperiod;
}


REGISTER_SHIP(ToUl)
