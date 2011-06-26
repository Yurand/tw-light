/* $Id: shpfweav.cpp,v 1.1 2006/01/29 16:14:34 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE
#include "melee/mship.h"

class FweiksAvian : public Ship
{
	public:
	public:
		double       weaponTime;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		double       specialTime;
		double       specialVelocity;
		int          specialDamage;
		int          specialArmour;

	public:
		FweiksAvian(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
};

class FweiksCrystal : public TimedShot
{
	public:
		FweiksCrystal(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
			double ovel, double otime, double oarmour, double odamage);

		virtual void inflict_damage(SpaceObject *other);
};

class FweiksFeather : public TimedShot
{
	public:
		double vref, period, angle0;
		double alpha, dalpha;
	public:
		FweiksFeather(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
			double ovel, double otime, double oarmour, double odamage, double perturbedangle);

		virtual void calculate();
};

FweiksCrystal::FweiksCrystal(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
double ovel, double otime, double oarmour, double odamage)
:
TimedShot(creator, orelpos, orelangle, osprite, ovel, otime, oarmour, odamage)
{
	STACKTRACE;
	// it needs mass, otherwise it won't bounce off.
	mass = 0.1;
}


void FweiksCrystal::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!state)
		return;

	TimedShot::inflict_damage(other);

	// don't die, just bounce (so, override the state=0 thingy in timedshot).
	state = 1;
}


FweiksFeather::FweiksFeather(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
double ovel, double otime, double oarmour, double odamage, double perturbedangle)
:
TimedShot(creator, orelpos, orelangle, osprite, ovel, otime, oarmour, odamage)
{
	STACKTRACE;
	vref = ovel;

	angle0 = angle;

	period = 1.0;

	alpha = perturbedangle;		 // perturbed position.
	dalpha = 0;					 // at rest
}


void FweiksFeather::calculate()
{
	STACKTRACE;
	// the velocity of a falling feather is ... somewhat weird.

	/*
	Vector2 vchange;

	double si, co;
	si = sin(PI2 * existtime / period);
	co = cos(PI2 * existtime / period);
	vchange = vref * Vector2(2*(fabs(si)-0.2), co);

	int k;
	k = existtime / period;
	*/

	// pendulum?
	double dt, g, R;

	dt = frame_time * 1E-3;

	g = 1.0;
	R = 0.1;

	dalpha += (g/R) * sin(alpha) * dt;
	alpha += dalpha * dt;

	//vel = vref*unit_vector(angle0) + rotate( vchange, angle0 );
	vel = Vector2(0.1, 0);

	vel += R*dalpha * unit_vector(alpha-PI/2);

	vel = vref * rotate( vel, angle0 );

	TimedShot::calculate();
}


FweiksAvian::FweiksAvian(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponTime     = tw_get_config_float("Weapon", "Time", 0);
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialTime     = tw_get_config_float("Special", "Time", 0);
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
}


int FweiksAvian::activate_weapon()
{
	STACKTRACE;
	//creator, orelpos, orelangle, osprite, ovel, otime, oarmour, odamage
	TimedShot *tmp = new FweiksCrystal( this,
		Vector2(0.0, size.y / 1.5), 0.0, data->spriteWeapon,
		weaponVelocity, weaponTime, weaponArmour, weaponDamage);
	add(tmp);

	//	tmp->explosionSprite = data->spriteWeaponExplosion;
	//	tmp->explosionSample = 0;
	//	tmp->explosionFrameCount = data->spriteWeaponExplosion->frames();
	//	tmp->explosionFrameSize = 100;

	return(TRUE);
}


int FweiksAvian::activate_special()
{
	STACKTRACE;
	int i;

	for ( i = -1; i < 2; ++i ) {
		if (i == 0)
			continue;

		FweiksFeather *tmp = new FweiksFeather( this,
			Vector2(0.0, -size.y / 1.5), PI*(1+0.1*i), data->spriteSpecial,
			specialVelocity, specialTime, specialArmour, specialDamage,
			i*0.4*PI);
		add(tmp);
	}

	//	tmp->explosionSprite = data->spriteSpecialExplosion;
	//	tmp->explosionSample = 0;
	//	tmp->explosionFrameCount = data->spriteSpecialExplosion->frames();
	//	tmp->explosionFrameSize = 100;

	return(TRUE);
}


REGISTER_SHIP(FweiksAvian)
