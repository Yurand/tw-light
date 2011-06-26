/* $Id: shpfopsl.cpp,v 1.11 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

class FopVob : public Ship
{
	public:
		double  weaponRange;
		int     weaponDamage;
		int     weaponArmour;
		double  weaponVmin;

		double  specialPeriod;
		double  specialR;

		int Nshots, maxshots;
		double *T0;				 // T-zero

	public:
		FopVob(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
		virtual ~FopVob();

		virtual void calculate();
		virtual void animate(Frame *f);

	protected:
		virtual int activate_weapon();

		Vector2 getP(int i);
		double  getV(int i);
		double  getY(int i);
};

FopVob::FopVob(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponVmin     = scale_velocity(tw_get_config_float("Weapon", "Vmin", 10));

	specialPeriod  = tw_get_config_float("Special", "Period", 1);
								 // oscillation wavelength
	specialR       = tw_get_config_float("Special", "specialR", 1);

	maxshots = tw_get_config_int("Weapon", "MaxShots", 8);
	T0 = new double [maxshots];
	Nshots = 0;
}


FopVob::~FopVob()
{
	delete[] T0;
}


int FopVob::activate_weapon()
{
	STACKTRACE;
	// add shots to the queue
	if (Nshots < maxshots ) {
		T0[Nshots] = physics->game_time * 1E-3;
		++Nshots;
		return TRUE;

	} else
	return FALSE;

}


// relative position:
double FopVob::getY(int i)
{
	STACKTRACE;
	double y, t;

	t = physics->game_time * 1E-3;

	y = specialR * sin(PI2 * (t - T0[i]) / specialPeriod);

	return y;
}


Vector2 FopVob::getP(int i)
{
	STACKTRACE;
	Vector2 P;
	//double y, t;

	//t = physics->game_time * 1E-3;

	//y = specialR * sin(PI2 * (t - T0[i]) / specialPeriod);

	P = getY(i) * unit_vector(angle);

	return P;
}


// relative velocity:
double FopVob::getV(int i)
{
	STACKTRACE;
	double V, t;
	t = physics->game_time * 1E-3;

	// the derivative of the getP equation;
								 //(1.0 / frame_time);
	V = specialR * (PI2/specialPeriod) * cos(PI2 * (t - T0[i]) / specialPeriod) * 1E-3;

	return V;
}


//Shot::Shot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
//	double odamage, double orange, double oarmour, SpaceLocation *opos,
//	SpaceSprite *osprite, double relativity)

void FopVob::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if (!fire_special || special_low) {
		// if special isn't held, release shots that are in the queue
		// tweak: but only if they are moving forward, otherwise you're wasting over half of your shots.

		int i;
		for ( i = 0; i < Nshots; ++i ) {
			double r, v;
			r = getY(i);
			v = getV(i);

			if (r > 0 && v > 0) {
				// ok: release this one !

				Vector2 P, Vtot;
				double  V, a;

				Vtot = v * unit_vector(angle) + vel;
				a = Vtot.angle();
				V = Vtot.length();

				if (V < weaponVmin)
					V = weaponVmin;

				//P = rotate(getP(Nshots), -PI/2);
				P = Vector2(0, r);

				// make real shots
				add(new Missile(this,
					P, a, V, weaponDamage, weaponRange,
					weaponArmour, this, data->spriteWeapon, 0));
				// don't use relativity in here, otherwise the missiles can travel a loooong distance !!

				T0[i] = T0[Nshots-1];
				--i;
				--Nshots;
			}
		}

		/* old code, releases them all at once.
		while (Nshots > 0)
		{
			--Nshots;

			Vector2	P, Vtot;
			double	V, a;

			Vtot = getV(Nshots) * unit_vector(angle) + vel;
			a = Vtot.angle();
			V = Vtot.length();
			if (V < weaponVmin)
				V = weaponVmin;
			//P = rotate(getP(Nshots), -PI/2);
			P = Vector2(0, getY(Nshots));

			// make real shots
			add(new Missile(this,
				P, a, V, weaponDamage, weaponRange,
				weaponArmour, this, data->spriteWeapon, 0));
			// don't use relativity in here, otherwise the missiles can travel a loooong distance !!
		}
		*/
	}
}


void FopVob::animate(Frame *f)
{
	STACKTRACE;
	// animate the ship, as usual:
	Ship::animate(f);

	// animate the queued shots:
	int i;
	for ( i = 0; i < Nshots; ++i ) {
		Vector2 P;
		P = pos + getP(i);
		data->spriteWeapon->animate(P, sprite_index, f);
	}
}


REGISTER_SHIP(FopVob)
