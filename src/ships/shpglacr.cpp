/* $Id: shpglacr.cpp,v 1.11 2005/07/11 00:25:31 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

class GlavriaCrTorpedo : public AnimatedShot
{
	public:
		//int       passive;
		//double    passiveRange;
		double    Relativity;
		int oorange;

	public:
		GlavriaCrTorpedo(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, int ofcount, int ofsize, double orelativity);
		virtual void calculate();
};

class GlavriaCruiser : public Ship
{
	public:
	public:
		double       weaponRange;
		double       weaponVelocity[5];
		double       weaponAngle[5];
		double       weaponRelativity;

		int          weaponDamage;
		int          weaponArmour;

		double specialRange;
		int    specialFrames;

		double       specialVelocity;
		int          specialDamage;
		int          specialArmour;
		double       specialRelativity;

		GlavriaCrTorpedo  *torpedo[5];

	public:
		GlavriaCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
};

GlavriaCruiser::GlavriaCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity[0] = scale_velocity(tw_get_config_float("Weapon", "Velocity1", 0));
	weaponVelocity[1] = scale_velocity(tw_get_config_float("Weapon", "Velocity2", 0));
	weaponVelocity[2] = scale_velocity(tw_get_config_float("Weapon", "Velocity3", 0));
	weaponVelocity[3] = scale_velocity(tw_get_config_float("Weapon", "Velocity4", 0));
	weaponVelocity[4] = scale_velocity(tw_get_config_float("Weapon", "Velocity5", 0));

	weaponAngle[0] = tw_get_config_float("Weapon", "Angle1", 0) * ANGLE_RATIO;
	weaponAngle[1] = tw_get_config_float("Weapon", "Angle2", 0) * ANGLE_RATIO;
	weaponAngle[2] = tw_get_config_float("Weapon", "Angle3", 0) * ANGLE_RATIO;
	weaponAngle[3] = tw_get_config_float("Weapon", "Angle4", 0) * ANGLE_RATIO;
	weaponAngle[4] = tw_get_config_float("Weapon", "Angle5", 0) * ANGLE_RATIO;

	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponRelativity = tw_get_config_float("Weapon", "Relativity", 0.5);

	//  specialFrames = tw_get_config_int("Special", "Frames", 0);
	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour =   tw_get_config_int("Special", "Armour", 0);
	specialRelativity = tw_get_config_float("Special", "Relativity", 0.5);
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));

	int i;
	for ( i = 0; i < 5; ++i ) {
		torpedo[i] = 0;
	}
}


int GlavriaCruiser::activate_special()
{
	STACKTRACE;

	SpaceSprite *spr;
	spr = data->spriteWeapon;

	game->add(new GlavriaCrTorpedo(Vector2(0.0, size.y *(-0.3)), angle+PI, specialVelocity,
		specialDamage, specialRange, specialArmour, this, spr,
		spr->frames(), 16, specialRelativity));

	/*
		// special modified by GeomanNL (and also removed, cause it's a bit lame as it turns out).

		int k;
		k = 0;	// the center element

		// check if the center torpedo is active, otherwise, don't do anything
		if (!torpedo[k])
			return false;

		int i;
		for ( i = 0; i < 5; ++i )
		{
			if (!torpedo[i])
				continue;

			// also, let the missiles persist (a lot) longer
			// for at least as long as the special lasts.

			torpedo[i]->d -= torpedo[i]->v * special_rate;

			if (i != k)
			{
				double a;
				a = torpedo[i]->trajectory_angle(torpedo[k]);

				// the special moves them away from each other
				Vector2 dv;
				dv = specialVelocity * frame_time*1E-3 * unit_vector(a);

				if (!fire_weapon)
					torpedo[i]->vel -= dv;

				// special + fire moves them closer to each other
				else
					torpedo[i]->vel += dv;

			}
		}
	*/

	return true;
}


int GlavriaCruiser::activate_weapon()
{
	STACKTRACE;

	int i;

	for ( i = 0; i < 5; ++i ) {

		torpedo[i] = new GlavriaCrTorpedo(Vector2(0.0,size.y*0.3),
			angle+weaponAngle[i], weaponVelocity[i],
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			100, 16, weaponRelativity);

		game->add(torpedo[i]);

	}

	return(TRUE);
}


void GlavriaCruiser::calculate()
{
	STACKTRACE;
	Ship::calculate();

	// check the state of yer weapons in space

	int i;

	for ( i = 0; i < 5; ++i ) {
		if (!(torpedo[i] && torpedo[i]->exists()))
			torpedo[i] = 0;
	}
}


GlavriaCrTorpedo::GlavriaCrTorpedo(Vector2 opos, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship,
SpaceSprite *osprite, int ofcount, int ofsize, double orelativity) :
AnimatedShot(oship, opos, oangle, ov, odamage, orange, oarmour, oship,
osprite, 16, 45, orelativity)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = explosionSprite->frames();
	explosionFrameSize  = 40;
	oorange=iround(orange);
	//vx = vx + ddx;
	//vy = vy + ddy;
}


void GlavriaCrTorpedo::calculate()
{
	STACKTRACE;
	AnimatedShot::calculate();
	//d += v * frame_time;
	//if (d >= oorange) state = 0;
}


REGISTER_SHIP(GlavriaCruiser)
