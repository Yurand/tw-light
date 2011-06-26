/* $Id: shpklidr.cpp,v 1.9 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

//#include "sc1ships.h"

#include "../melee.h"
#include "../melee/mshot.h"
#include "../melee/mship.h"
#include "../melee/manim.h"
#include "../melee/mview.h"

class KlisruDragon;

class KlisruTorpedo : public Shot
{
	public:
		KlisruTorpedo(KlisruDragon* ocreator, Vector2 rpos,
			double oangle, double oStartVelocity, double oEndVelocity,
			double oStartDamage, double oEndDamage,
			double orange, double oStartArmour, double oEndArmour,
			SpaceLocation* opos, SpaceSprite* osprite, double orelativity);
		KlisruDragon* creator;
		double startDamage;
		double endDamage;
		double startArmour;
		double endArmour;
		double startVelocity;
		double endVelocity;
		double lifetimeCounter;
		double lifetimeMax;
		double startFriction, endFriction;
		double friction;
		virtual void calculate(void);
		virtual void animateExplosion(void);
		void inflict_damage(SpaceObject *other);
};

class KlisruMissile : public HomingMissile
{
	public:
		KlisruMissile(KlisruDragon* ocreator, Vector2 rpos, double oangle, double ov,
			int odamage, double orange, int oarmour, double oturnrate, SpaceLocation* opos, SpaceSprite* osprite, SpaceObject* otarget);
		Ship* creator;
		virtual void inflict_damage(SpaceObject *other);
		virtual void calculate(void);
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void animate (Frame *frame);
		bool die(void);

};

class KlisruDragon : public Ship
{
	public:
		//double       weaponRange;
		double       weaponStartVelocity;
		//double       weaponEndVelocity;
		double       weaponStartDamage;
		double       weaponEndDamage;
		double       weaponStartArmour;
		double       weaponEndArmour;
		double       weaponRelativity;
		double       weaponStartFriction;
		double       weaponEndFriction;
		int          weaponLifetime;

		double       specialRange;
		double       specialVelocity;
		double       specialDamage;
		double       specialArmour;
		double       specialTurnRate;

	public:
		KlisruDragon(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		SpaceObject* FindMissileTarget();
		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void death();
};

KlisruDragon::KlisruDragon(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	//weaponRange         = scale_range(get_config_float("Weapon", "Range", 0));
	weaponStartVelocity = scale_velocity(get_config_float("Weapon", "StartVelocity", 0));
	weaponStartDamage   = get_config_float("Weapon", "StartDamage", 0);
	weaponStartArmour   = get_config_float("Weapon", "StartArmour", 0);
	//weaponEndVelocity   = scale_velocity(get_config_float("Weapon", "EndVelocity", 0));
	weaponEndDamage     = get_config_float("Weapon", "EndDamage", 0);
	weaponEndArmour     = get_config_float("Weapon", "EndArmour", 0);
	weaponRelativity    = get_config_float("Weapon", "Relativity", 0);
	weaponStartFriction = get_config_float("Weapon", "StartFriction", 0);
	weaponEndFriction   = get_config_float("Weapon", "EndFriction", 0);
	weaponLifetime      = get_config_int("Weapon", "Lifetime", 0);

	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialRange    = scale_range(get_config_float("Special", "Range", 0));
	specialDamage   = get_config_float("Special", "Damage", 0);
	specialArmour   = get_config_float("Special", "Armour", 0);
	specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));
}


void KlisruDragon::death(void)
{
	STACKTRACE;
	Ship::death();
}


int KlisruDragon::activate_weapon()
{
	STACKTRACE;
	KlisruTorpedo* KT;
	KT = new KlisruTorpedo(this, Vector2(0, size.y * 0.4), this->angle,
		this->weaponStartVelocity, this->weaponStartVelocity,
		this->weaponStartDamage, this->weaponEndDamage,
		-1, this->weaponStartArmour, this->weaponEndArmour,
		this, this->data->spriteWeapon, 0.0);
	KT->startFriction = this->weaponStartFriction;
	KT->endFriction = this->weaponEndFriction;
	KT->lifetimeCounter = 0;
	KT->lifetimeMax = this->weaponLifetime;
	game->add(KT);
	return(TRUE);
}


int KlisruDragon::activate_special()
{
	STACKTRACE;
	SpaceObject* SO;
	double launchAngle = this->angle;
	SO = this->FindMissileTarget();
	if (SO!=NULL) launchAngle = this->trajectory_angle(SO);
	KlisruMissile* KM;
	KM = new KlisruMissile(this, Vector2(0,0), launchAngle, this->specialVelocity,
		iround(this->specialDamage), this->specialRange, iround(this->specialArmour),
		this->specialTurnRate, this, this->data->spriteSpecial, SO);
	game->add(KM);
	return(TRUE);
}


void KlisruDragon::calculate(void)
{
	STACKTRACE;
	Ship::calculate();
}


SpaceObject* KlisruDragon::FindMissileTarget(void)
{
	STACKTRACE;
	SpaceObject* bestTarget = NULL;
	double bestRange = 999999.9;
	//	for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
	//			bit(LAYER_CBODIES), specialRange); a.current; a.next()) {
	//		o = a.currento;
	//		if ( (!o->isInvisible()) && !o->sameTeam(this) && (o->collide_flag_anyone & bit(LAYER_LINES))) {

	if (this->target && this->target->exists()) {
		if (this->distance(target)<this->specialRange)
			return(target);
	}
	SpaceObject *o;
	Query a;
	a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
		bit(LAYER_CBODIES), this->specialRange);
	for (;a.current;a.next()) {
		o = a.currento;
		if (!o->isInvisible() && !o->sameTeam(this) && this->distance(o)<bestRange) {
			bestTarget = o;
			bestRange = this->distance(bestTarget);
		}
	}
	return(bestTarget);
}


KlisruMissile::KlisruMissile(KlisruDragon* ocreator, Vector2 rpos, double oangle, double ov,
int odamage, double orange, int oarmour, double oturnrate, SpaceLocation* opos, SpaceSprite* osprite, SpaceObject* otarget)
:
HomingMissile(ocreator, rpos, oangle, ov, odamage, orange, oarmour, oturnrate, opos, osprite, otarget)
{
	STACKTRACE;
	return;
}


void KlisruMissile::calculate(void)
{
	STACKTRACE;
	HomingMissile::calculate();
}


void KlisruMissile::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	HomingMissile::inflict_damage(other);
}


int KlisruMissile::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int x;
	x = this->Shot::handle_damage(source, normal, direct);
	return(x);
}


bool KlisruMissile::die()
{
	STACKTRACE;
	return(true);
}


void KlisruMissile::animate(Frame *frame)
{
	STACKTRACE;
	HomingMissile::animate(frame);
}


KlisruTorpedo::KlisruTorpedo(KlisruDragon* ocreator, Vector2 rpos,
double oangle, double oStartVelocity, double oEndVelocity,
double oStartDamage, double oEndDamage, double orange,
double oStartArmour, double oEndArmour, SpaceLocation* opos, SpaceSprite* osprite,
double orelativity)
:
Shot(ocreator, rpos, oangle, oStartVelocity, oStartDamage, orange,
oStartArmour, ocreator, ocreator->data->spriteWeapon,
orelativity)
{
	STACKTRACE;
	this->explosionSprite = ocreator->data->spriteWeapon;
	this->explosionFrameCount = 24;
	creator = ocreator;
	startDamage = oStartDamage;
	endDamage = oEndDamage;
	startArmour = oStartArmour;
	endArmour = oEndArmour;
	startVelocity = oStartVelocity;
	endVelocity = oEndVelocity;
	sprite_index = 0;
	this->lifetimeCounter = 0;
}


void KlisruTorpedo::calculate(void)
{
	STACKTRACE;
	double fractionDone = 0.0;
	int spriteToUse = 0;
	Shot::calculate();
	lifetimeCounter += frame_time;
	fractionDone = lifetimeCounter / lifetimeMax;
	friction = startFriction * (1 - fractionDone) + endFriction * fractionDone;
	if (fractionDone<0) fractionDone = 0.0;
	if (fractionDone>=0.999) {
		fractionDone = 0.999;
		state = 0;
	}
	spriteToUse = (int)(fractionDone * 16.0);
	sprite_index = spriteToUse;
	damage_factor = (1 - fractionDone) * startDamage + fractionDone * endDamage;
	armour = (1 - fractionDone) * startArmour + fractionDone * endArmour;
	this->v *= (1 - this->friction * frame_time);
	this->vel = unit_vector(this->angle) * this->v;

}


void KlisruTorpedo::animateExplosion()
{
	STACKTRACE;
	return;
}


void KlisruTorpedo::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	int x = this->sprite_index;
	Shot::inflict_damage(other);
	if (other->mass <= 0) return;
	if (other->isShip())
		game->add(new FixedAnimation(this, other,
			explosionSprite, x, explosionFrameCount - x,
			explosionFrameSize, DEPTH_EXPLOSIONS)
			);
	else
		game->add(new Animation(this, pos,
			explosionSprite, x, explosionFrameCount - x,
			explosionFrameSize, DEPTH_EXPLOSIONS)
			);

}


REGISTER_SHIP(KlisruDragon)
