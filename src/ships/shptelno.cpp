/* $Id: shptelno.cpp,v 1.10 2004/03/24 23:51:42 yurand Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class TelluriNovaShield;

class TelluriNova : public Ship
{
	TelluriNovaShield *shield;
	double  weaponRange, weaponVelocity, weaponDamage, weaponDamagePeriod;
	double  specialWeaponCost, specialDamage, specialDamagePeriod;
	int     specialColor;

	double  lasertime, laserperiod;

	bool shipmode;

	public:
		TelluriNova(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:

		virtual int activate_weapon();
		virtual int activate_special();

};

class TelluriNovaMissile : public Missile
{
	public:
		double damageperiod, inflicttime;

		TelluriNovaMissile(SpaceLocation *creator, Vector2 rpos, double oangle,
			double ov, double odamage, double odamageperiod, double orange,
			SpaceLocation *opos, SpaceSprite *osprite, double relativity);

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		//virtual void calculate();
};

class TelluriNovaShield : public SpaceObject
{
	public:
		TelluriNova *mother;
		double damageperiod, inflicttime;
		int power, maxpower;

		TelluriNovaShield(TelluriNova *creator, SpaceSprite *osprite,
			double odamage, double odamageperiod);

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);

		virtual void calculate();

		bool power_up();
};

TelluriNova::TelluriNova(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponDamagePeriod   = get_config_float("Weapon", "Damage", 0);

	specialWeaponCost = get_config_int("Special", "WeaponCost", 0);
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialDamagePeriod   = get_config_float("Special", "DamagePeriod", 0);

	lasertime = 0;
	laserperiod = 2.0;

	shield = new TelluriNovaShield(this, data->spriteSpecial,
		specialDamage, specialDamagePeriod);
	add(shield);
}


int TelluriNova::activate_weapon()
{
	STACKTRACE;
	if (shield->power >= specialWeaponCost && (target && target->exists()) ) {
		shield->power -= iround(specialWeaponCost);

		// add a missile
		//	TelluriNovaMissile(SpaceLocation *creator, Vector2 rpos, double oangle,
		//			double ov, double odamage, double odamageperiod, double orange,
		//			SpaceLocation *opos, SpaceSprite *osprite, double relativity);

		Vector2 P;
		double a;

		//a = trajectory_angle(target);
		if (!target->isInvisible())
			a = intercept_angle2(pos, 0*vel, weaponVelocity, target->pos, target->vel);
		else
			a = angle;

		P = 0;

		TelluriNovaMissile *m;
		m = new TelluriNovaMissile(this, P, a,
			weaponVelocity, weaponDamage, weaponDamagePeriod,
			weaponRange, this, data->spriteWeapon, 0.0);

		add(m);

		return true;

	} else
	return false;

}


int TelluriNova::activate_special()
{
	STACKTRACE;

	if (shield->power_up())
		return true;
	else
		return false;
}


TelluriNovaMissile::TelluriNovaMissile(SpaceLocation *creator, Vector2 rpos, double oangle,
double ov, double odamage, double odamageperiod, double orange,
SpaceLocation *opos, SpaceSprite *osprite, double relativity)
:
Missile(creator, rpos, oangle, ov, odamage, orange, 1,
opos, osprite, relativity)
{
	STACKTRACE;
	layer = LAYER_SHOTS;

	mass = 0;
	damageperiod = odamageperiod;
	inflicttime = 0;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
}


void TelluriNovaMissile::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (game->game_time - inflicttime >= damageperiod*1E3) {
		inflicttime = game->game_time;

		if (other && other->exists())
			damage(other, damage_factor);
	}
}


int TelluriNovaMissile::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	// this thing cannot be killed ...
	return 0;
}


TelluriNovaShield::TelluriNovaShield(TelluriNova *creator, SpaceSprite *osprite,
double odamage, double odamageperiod)
:
SpaceObject(creator, 0, 0, osprite)
{
	STACKTRACE;
	layer = LAYER_SPECIAL;

	mother = creator;

	maxpower = sprite->frames() - 1;
	power = 0;

	mass = 0;
	damage_factor = odamage;
	damageperiod = odamageperiod;
	inflicttime = 0;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;

	isblockingweapons = false;
}


void TelluriNovaShield::calculate()
{
	STACKTRACE;
	if (!(mother && mother->exists())) {
		mother = 0;
		state = 0;
		return;
	}

	sprite_index = power;

	pos = mother->pos;
	vel = mother->vel;

	SpaceObject::calculate();
}


void TelluriNovaShield::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	// just the default behaviour.
	if (game->game_time - inflicttime >= damageperiod*1E3) {
		inflicttime = game->game_time;
		SpaceObject::inflict_damage(other);
	}
}


int TelluriNovaShield::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	// take at least 1 damage ?
	double t;
	t = normal + direct;
	if (t < 1)
		t = 1;

	power -= iround(t);
	if (power < 0)
		power = 0;

	if (!state) {				 // just in case the planet or so overrides its state.
		power = 0;
		state = 1;
	}

	return 1;
}


bool TelluriNovaShield::power_up()
{
	STACKTRACE;
	if (power < maxpower) {
		++power;
		return true;

	} else
	return false;
}


REGISTER_SHIP(TelluriNova)
