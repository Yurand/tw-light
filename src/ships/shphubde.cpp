/* $Id: shphubde.cpp,v 1.12 2005/08/28 20:34:08 geomannl Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class Hellenian : public Ship
{
	public:
		double  weaponRange, weaponVelocity, weaponDamage, weaponArmour;
		double  specialRange1, specialRange2, specialDuration, specialDamage;

		bool shipmode;

	public:
		Hellenian(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:

		virtual int activate_weapon();
		virtual int activate_special();

};

class HellenianShot : public Missile
{
	public:
	public:
		SpaceSprite *ripplesprite;
		double t, trepeat;

		HellenianShot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
			double odamage, double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, SpaceSprite *oripplesprite);

		virtual void calculate();
};

class MortarFire : public Shot
{
	public:
	public:
		double t, t_exist;

		MortarFire(SpaceLocation *creator, Vector2 rpos,
			double odamage, double lifetime, SpaceSprite *explsprite);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

/*
class MovingAnimation : public Animation
{
public:
	MovingAnimation(SpaceLocation *creator, Vector2 opos,
		SpaceSprite *osprite, int first_frame, int num_frames, int frame_length,
		double depth);

	virtual void calculate();
}
*/

Hellenian::Hellenian(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialRange1    = scale_range(tw_get_config_float("Special", "Range1", 0));
	specialRange2    = scale_range(tw_get_config_float("Special", "Range2", 0));
	specialDuration  = tw_get_config_float("Special", "Duration", 0);
	specialDamage   = tw_get_config_int("Special", "Damage", 0);

}


int Hellenian::activate_weapon()
{
	STACKTRACE;
	Vector2 rpos;
	rpos = 0;

	add(new HellenianShot(this, rpos, angle,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, data->spriteWeaponExplosion));

	// shooting slows you down, a lot !
	vel *= 0.5;

	return TRUE;
}


int Hellenian::activate_special()
{
	STACKTRACE;

	double a, a0;

	a0 = (0.5 - tw_random(1.0)) * 0.1*PI;

	int i, N;

	N = 5;

	for ( i = 0; i < N; ++i ) {
		a = a0 + i*PI2/N;

		Vector2 rpos;

		double R;
		R = specialRange1 + random(specialRange2 - specialRange1);

		rpos = R * unit_vector(a);

		MortarFire *m;
		m = new MortarFire(this, rpos, specialDamage, specialDuration, data->spriteSpecialExplosion);
		add(m);
	}

	return TRUE;
}


HellenianShot::HellenianShot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
double odamage, double orange, double oarmour, SpaceLocation *opos,
SpaceSprite *osprite, SpaceSprite *oripplesprite)
:
Missile(creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite)
{
	STACKTRACE;
	t = 0;
	trepeat = 0.1;

	ripplesprite = oripplesprite;
}


void HellenianShot::calculate()
{
	STACKTRACE;
	t += frame_time * 1E-3;
	if (t > trepeat) {
		t -= trepeat;

		Vector2 vel2;
		vel2 = 0.2 * normalize(Vector2(-vel.y, vel.x));

		Animation *a;
		//	Animation(SpaceLocation *creator, Vector2 opos,
		//		SpaceSprite *osprite, int first_frame, int num_frames, int frame_length,
		//		double depth);
		int i;
		for ( i = -1; i <=1; i+=2 ) {
			if (random(2)) {
				a = new Animation(this, pos, data->spriteSpecialExplosion, 0,
					data->spriteSpecialExplosion->frames(), 100, DEPTH_SHOTS);
				a->set_vel ( i * vel2 );
				add(a);
			}
		}
	}

	Missile::calculate();
}


//Shot::Shot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
//	double odamage, double orange, double oarmour, SpaceLocation *opos,
//	SpaceSprite *osprite, double relativity)

MortarFire::MortarFire(SpaceLocation *creator, Vector2 rpos,
double odamage, double lifetime, SpaceSprite *explsprite)
:
Shot(creator, rpos, 0, 0, odamage, 1E9, 0, creator, explsprite)
{
	STACKTRACE;
	t = 0;
	t_exist = lifetime;
	vel = 0;
	v = 0;
	sprite_index = 0;
	armour = 0;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void MortarFire::calculate()
{
	STACKTRACE;
	if (t > t_exist || !(ship && ship->exists())) {
		state = 0;
		return;
	}

	Shot::calculate();

	t += frame_time * 1E-3;

	int i;
	i = int(sprite->frames() * t / t_exist);

	if (i >= sprite->frames())
		i = sprite->frames() - 1;
	if (i < 0)
		i = 0;

	sprite_index = i;

}


void MortarFire::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	Shot::inflict_damage(other);
	state = 0;
}


REGISTER_SHIP(Hellenian)
