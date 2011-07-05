/* $Id: shpdyzha.cpp,v 1.15 2005/08/28 20:34:07 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

//#include "sc1ships.h"

#include "../melee.h"
#include "../melee/mshot.h"
#include "../melee/mship.h"
#include "../melee/manim.h"

class DyzunHarbringer;

class SlownessMine : public SpaceObject
{
	public:
		double range, maxvel, armour;
	public:
		SlownessMine(Ship *creator, Vector2 opos, double oangle, SpaceSprite *osprite,
			double orange, double omaxvel, double oarmour);
		virtual int handle_damage(SpaceLocation* source, double normal, double direct = 0);
		virtual void calculate();
};

class DyzunMissile : public HomingMissile
{
	public:
	public:
		DyzunMissile(DyzunHarbringer* ocreator, double ox, double oy, double oangle, double ov,
			int odamage, double orange, int oarmour, double oturnrate, SpaceLocation* opos, SpaceSprite* osprite, SpaceObject* otarget);
		Ship* creator;
		int lifetimeTimer;
		double relativity;
		double facingAngle;
		double framesToIgnition;
		int isActivated;
		int burnFrames1;
		int burnFrames2;
		int coastFrames;
		double acceleration;
		int wasCoasting;
		int thrustOn;
		double coastVelocity;
		double burnVelocity1;
		double burnVelocity2;
		void thrust_on(void);
		virtual void calculate(void);
		virtual void inflict_damage(SpaceObject *other);
};

class DyzunHarbringer : public Ship
{
	public:
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;
		int          weaponTandemFire;

		double       specialVelocityCoast;
		double       specialVelocityBurn1;
		double       specialVelocityBurn2;
		int          specialDamage;
		int          specialArmour;
		double       specialReleaseAngle;
		double       specialReleaseFacingAngle;
		double       specialTurnRate;

		int          specialCoastFrames;
		int          specialBurnFrames1;
		int          specialBurnFrames2;

		int          gunToFire;

		int           maxmines, currentmine;
		SlownessMine  **slmine;

		double        weaponMaxvel;

	public:
		DyzunHarbringer(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void death();
};

DyzunHarbringer::DyzunHarbringer(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponMaxvel   = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	//  weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	//  weaponTandemFire = tw_get_config_int("Weapon", "TandemFire", 0);
	maxmines       = tw_get_config_int("Weapon", "Nmines", 3);

	specialVelocityCoast = scale_velocity(tw_get_config_float("Special", "VelocityCoast", 5));
	specialVelocityBurn1 = scale_velocity(tw_get_config_float("Special", "VelocityBurn1", 10));
	specialVelocityBurn2 = scale_velocity(tw_get_config_float("Special", "VelocityBurn2", 20));

	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	specialReleaseAngle = tw_get_config_float("Special", "ReleaseAngle", 0) * ANGLE_RATIO;
	specialReleaseFacingAngle = tw_get_config_float("Special", "ReleaseFacingAngle", 0) * ANGLE_RATIO;
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));

	specialCoastFrames = tw_get_config_int("Special", "CoastFrames", 0);
	specialBurnFrames1 = tw_get_config_int("Special", "BurnFrames1", 0);
	specialBurnFrames2 = tw_get_config_int("Special", "BurnFrames2", 0);
	gunToFire = 1;

	currentmine = 0;
	slmine = new SlownessMine* [maxmines];
	int i;
	for ( i = 0; i < maxmines; ++i)
		slmine[i] = 0;
}


void DyzunHarbringer::death(void)
{
	STACKTRACE;
	Ship::death();
}


int DyzunHarbringer::activate_weapon()
{
	STACKTRACE;
	/*
	if (gunToFire==1 || weaponTandemFire) {
	game->add(new Missile(this, Vector2(size.y*(0.24), (size.y * +0.25)),
	angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
	this, data->spriteWeapon));
	}
	if (gunToFire==2 || weaponTandemFire) {
	game->add(new Missile(this, Vector2(size.y*(-0.24), (size.y * +0.25)),
	angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
	this, data->spriteWeapon));
	}
	if (gunToFire==1)gunToFire=2;
	else gunToFire=1;
	return(TRUE);
	*/

	// if this mine already exists, clear it first
	if (slmine[currentmine])
		slmine[currentmine]->state = 0;

	slmine[currentmine] = new SlownessMine(this, pos, angle, data->spriteWeapon,
		weaponRange, weaponMaxvel, weaponArmour);

	game->add(slmine[currentmine]);

	++currentmine;
	if (currentmine >= maxmines)
		currentmine = 0;

	return 1;
}


int DyzunHarbringer::activate_special()
{
	STACKTRACE;
	DyzunMissile* NM;
	NM = new DyzunMissile(this, size.y*(0.5), (size.y * 0.6),
		angle+specialReleaseAngle, specialVelocityCoast, specialDamage, -1, specialArmour, specialTurnRate,
		this, data->spriteSpecial, this->target);
	NM->facingAngle = angle+specialReleaseFacingAngle;
	NM->creator=this;
	NM->coastFrames = specialCoastFrames;
	NM->burnFrames1 = specialBurnFrames1;
	NM->burnFrames2 = specialBurnFrames2;

	NM->coastVelocity = specialVelocityCoast;
	NM->burnVelocity1 = specialVelocityBurn1;
	NM->burnVelocity2 = specialVelocityBurn2;
	game->add(NM);

	NM = new DyzunMissile(this, size.y*(-0.5), (size.y * 0.6),
		angle-specialReleaseAngle, specialVelocityCoast, specialDamage, -1, specialArmour, specialTurnRate,
		this, data->spriteSpecial, this->target);
	NM->facingAngle = angle-specialReleaseFacingAngle;
	NM->creator=this;
	NM->coastFrames = specialCoastFrames;
	NM->burnFrames1 = specialBurnFrames1;
	NM->burnFrames2 = specialBurnFrames2;
	NM->coastVelocity = specialVelocityCoast;
	NM->burnVelocity1 = specialVelocityBurn1;
	NM->burnVelocity2 = specialVelocityBurn2;
	game->add(NM);

	return(TRUE);
}


void DyzunHarbringer::calculate(void)
{
	STACKTRACE;
	Ship::calculate();

	// check if your precious mines were destroyed; update pointers.
	int i;
	for ( i = 0; i < maxmines; ++i)
		if (!(slmine[i] && slmine[i]->exists()))
			slmine[i] = 0;

}


DyzunMissile::DyzunMissile(DyzunHarbringer* ocreator, double ox, double oy, double oangle, double ov,
int odamage, double orange, int oarmour, double oturnrate, SpaceLocation* opos, SpaceSprite* osprite, SpaceObject* otarget)
:
HomingMissile(ocreator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oturnrate, opos, osprite, otarget)

{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	facingAngle=oangle;
	isActivated=0;
	this->relativity = 1.0;
	thrustOn = FALSE;
	lifetimeTimer = 0;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void DyzunMissile::calculate(void)
{
	STACKTRACE;
	lifetimeTimer += frame_time;
	if (lifetimeTimer<coastFrames) {
		wasCoasting = TRUE;
		Shot::calculate();
	}
	else if (lifetimeTimer<(coastFrames+burnFrames1)) {
		this->sprite_index = get_index(this->angle) + 64;
		HomingMissile::calculate();
		this->sprite_index = get_index(this->angle) + 64;
		this->v = burnVelocity1;
		this->relativity = 0.0;
		this->sprite_index = get_index(this->angle) + 64;
	}
	else if (lifetimeTimer<(coastFrames+burnFrames1+burnFrames2)) {
		sprite_index = get_index(this->angle) + 128;
		HomingMissile::calculate();
		this->sprite_index = get_index(this->angle) + 128;
		this->v =  burnVelocity2;
		this->relativity = 0.0;
		this->sprite_index = get_index(this->angle) + 128;
	} else {
		state = 0;
		this->sprite_index = get_index(this->angle) + 128;
		HomingMissile::calculate();
	}
}


void DyzunMissile::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	Shot::inflict_damage(other);
}


void DyzunMissile::thrust_on(void)
{
	STACKTRACE;
	;
}


SlownessMine::SlownessMine(Ship *creator, Vector2 opos, double oangle, SpaceSprite *osprite,
double orange, double omaxvel, double oarmour)
:
SpaceObject(creator, opos, oangle, osprite)
{
	STACKTRACE;
	range = orange;
	maxvel = omaxvel;
	armour = oarmour;

	layer = LAYER_SPECIAL;
	vel = 0;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	isblockingweapons = false;
}


int SlownessMine::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	armour -= (normal + direct);

	if (armour <= 0)
		state = 0;

	return 1;
}


void SlownessMine::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	// check if anything is in range ... which should be maxed speeded ...
	Query a;
	for (a.begin(this, OBJECT_LAYERS, range); a.currento; a.next()) {
		if (!a.current->isObject())
			continue;

		SpaceObject *o = a.currento;

		// skip same team objects
		if (sameTeam(o))
			continue;

		double v;
		v = o->vel.length();
		if (v > maxvel) {
			o->vel *= maxvel / v;
			v = maxvel;
		}

		if (o->isShot())
			((Shot*)o)->v = v;

	}
}


REGISTER_SHIP(DyzunHarbringer)
