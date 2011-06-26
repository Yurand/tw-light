/* $Id: shpvioge.cpp,v 1.10 2005/08/14 16:14:32 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

const int ID_VIOGENPLASMA    = 0x0F84D19C;

class Viogen : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;
		double       specialRange;
		double       specialVelocity;
		int          specialDamage;
		int          specialArmour;

	public:
		Viogen(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
};

class ViogenMissile : public Missile
{
	public:
	public:

		double  period, rotatetime, accel, aoffs;

		ViogenMissile(Vector2 opos, double oaoffs, double oangle, double ov,
			int odamage, double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite);

		virtual void calculate();
};

class ViogenPlasma : public Shot
{
	public:
	public:

		double default_range;

		ViogenPlasma(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
			double odamage, double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, double relativity);

		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void calculate();

		virtual int canCollide(SpaceLocation *other);
};

Viogen::Viogen(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
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


int Viogen::activate_weapon()
{
	STACKTRACE;
	add(new ViogenMissile(
		Vector2( 20.0, 0.5*get_size().y), -0.5*PI, angle, weaponVelocity, weaponDamage, weaponRange,
		weaponArmour, this, data->spriteWeapon));

	add(new ViogenMissile(
		Vector2(-20.0, 0.5*get_size().y), 0.5*PI, angle, weaponVelocity, weaponDamage, weaponRange,
		weaponArmour, this, data->spriteWeapon));

	return(TRUE);
}


int Viogen::activate_special()
{
	STACKTRACE;

	/*
	old special: addition of fuel at the cost of speed... that's a bit lame.

	 if (batt - special_drain > batt_max)
		return false;

	double a, v, dv;
	v = vel.length();
	a = vel.atan();

	dv = 2 * speed_max / batt_max;
	// full speed gives half batt.

	if (v - dv < 0)
		return false;

	v -= dv;
	vel = v * unit_vector(a);
	*/

	// add a defensive shot or so ...
	Vector2 rpos;

	rpos = Vector2(0, 100);

	add ( new ViogenPlasma( this, rpos, angle, specialVelocity,
		specialDamage, specialRange, specialArmour, this,
		data->spriteSpecial, 0));

	return true;
}


ViogenMissile::ViogenMissile(Vector2 opos, double oaoffs, double oangle,
double ov, int odamage, double orange, int oarmour,
Ship *oship, SpaceSprite *osprite) :
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite)
{
	STACKTRACE;
	period = 1.0;
	rotatetime = 0;

	accel = scale_acceleration(7);

	aoffs = oaoffs;
}


void ViogenMissile::calculate()
{
	STACKTRACE;
	Missile::calculate();

	if (!state)
		return;

	// sorta rotate ?
	Vector2 normal;
	normal = Vector2(-vel.y, vel.x);
	normalize(normal);

	rotatetime += frame_time * 1E-3;

	double a;
	a = sin(PI2 * rotatetime / period + aoffs);

	vel += (accel*frame_time) * a * normal;
	angle = vel.atan();

	// release some smoke trail.
	Vector2 relpos;

	relpos = Vector2( tw_random(10.0)-5.0, -40.0 + tw_random(10.0) );
	Animation *anim;
	anim = new Animation(this, pos + rotate(relpos, angle-0.5*PI),
		data->spriteExtra, 0, data->spriteExtra->frames(),
		100, LAYER_HOTSPOTS);
	add(anim);

	//Animation::Animation(SpaceLocation *creator, Vector2 opos,
	//	SpaceSprite *osprite, int first_frame, int num_frames, int frame_length,
	//	double depth, double _scale)
}


ViogenPlasma::ViogenPlasma(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
double odamage, double orange, double oarmour, SpaceLocation *opos,
SpaceSprite *osprite, double relativity)
:
Shot(creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite, relativity)
{
	STACKTRACE;
	default_range = orange;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	id = ID_VIOGENPLASMA;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


int ViogenPlasma::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	armour += normal + direct;

	return 0;
}


void ViogenPlasma::calculate()
{
	STACKTRACE;

	Shot::calculate();

	if (d > default_range) {
		state = 1;

		d = 0;
		range = default_range;
		--armour;

		if (armour <= 0) {
			state = 0;
			return;
		}

	}

	sprite_index = iround(armour-1);
	if (sprite_index >= sprite->frames())
		sprite_index = sprite->frames()-1;

	damage_factor = armour;
}


int ViogenPlasma::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	if (other->id == ID_VIOGENPLASMA)
		return FALSE;

	return SpaceObject::canCollide(other);
}


REGISTER_SHIP(Viogen)
