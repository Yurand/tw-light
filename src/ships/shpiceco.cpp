/* $Id: shpiceco.cpp,v 1.15 2005/08/14 16:14:32 geomannl Exp $ */
/*
This is a variation on the Xchagger exclave, to see if the confusion
concept can be used in a different version of gameplay.
*/

#include "ship.h"
REGISTER_FILE

#include "frame.h"

// allows other ships to affect control over a ship.
class OverrideControlIceci : public OverrideControl
{
	int     key_order[5];
	int     key_flags[5];
	public:
		OverrideControlIceci();
		virtual void calculate(short *key);
};

class IceciConfusion : public Ship
{
	public:
		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		double       weaponArmour, weaponTurnrate;

		double       specialRange;
		double       specialVelocity;
		double       specialArmour;
		int          specialFrames;
		double        specialTurnRate;

		double        confusionLifeTime;

	public:
		IceciConfusion(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
};

class Confusionator : public Presence
{
	public:
		OverrideControlIceci *oci;
		Ship    *t;
		double  lifetime;

	public:
		Confusionator(Ship *target, double olifetime);
		virtual void calculate();
};

class ConfusionDart : public HomingMissile
{
	public:

		double confusionLifeTime;

	public:
		ConfusionDart(Vector2 opos, double oangle,
			double ov, int oframes, double orange, double oarmour, double otrate, Ship *oship,
			SpaceSprite *osprite, double confusionLifeTime);

		void inflict_damage(SpaceObject *other);

};

IceciConfusion::IceciConfusion(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponTurnrate = scale_turning(tw_get_config_int("Weapon", "TurnRate", 0));

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	specialFrames   = scale_frames(tw_get_config_int("Special", "Frames", 0));
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));

	confusionLifeTime = tw_get_config_float("Confusion", "LifeTime", 0);

}


//	HomingMissile(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, double odamage,
//			double orange, double oarmour, double otrate, SpaceLocation *opos,
//			SpaceSprite *osprite, SpaceObject *target);
int IceciConfusion::activate_weapon()
{
	STACKTRACE;

	//double da = 60 * ANGLE_RATIO;

	/*
	add(new Missile(this, Vector2(16.0, 0.0), angle+da,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, 1.0));
	*/

	add(new HomingMissile(this, Vector2(0.0, 16.0), angle,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		weaponTurnrate,
		this, data->spriteWeapon, target));

	return(TRUE);
}


int IceciConfusion::activate_special()
{
	STACKTRACE;

	double da = 60 * ANGLE_RATIO;

	add( new ConfusionDart(Vector2(0.0, 16.0), angle+da,
		specialVelocity, specialFrames, specialRange, specialArmour,
		specialTurnRate, this, data->spriteSpecial, confusionLifeTime));

	add( new ConfusionDart(Vector2(0.0, 16.0), angle-da,
		specialVelocity, specialFrames, specialRange, specialArmour,
		specialTurnRate, this, data->spriteSpecial, confusionLifeTime));

	return(TRUE);
}


ConfusionDart::ConfusionDart(Vector2 opos, double oangle,
double ov, int oframes, double orange, double oarmour, double otrate, Ship *oship,
SpaceSprite *osprite, double oLifeTime)
:
HomingMissile (oship, opos,  oangle, ov, 0, orange, oarmour, otrate, oship, osprite,
oship->target)
{
	STACKTRACE;
	confusionLifeTime = oLifeTime;
}


void ConfusionDart::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

	if ( other->isShip() ) {
		game->add(new Confusionator((Ship*)other, confusionLifeTime));
	}

	state = 0;
	return;
}


OverrideControlIceci::OverrideControlIceci()
{
	STACKTRACE;
	// new key ordering:

	int key_available[5];

	key_flags[0] = keyflag::left;
	key_flags[1] = keyflag::right;
	key_flags[2] = keyflag::thrust;
	key_flags[3] = keyflag::fire;
	key_flags[4] = keyflag::special;

	int i, k;
	for ( k = 0; k < 5; ++k )
		key_available[k] = k;

	for ( i = 0; i < 5; ++i ) {
		k = tw_random(5-i);
		key_order[i] = key_available[k];
		key_available[k] = key_available[5-i-1];
	}
}


void OverrideControlIceci::calculate(short *key)
{
	STACKTRACE;
	// randomize the (most) important keys ... i.e., randomize the bits in the KeyCode

	short newkeys;

	newkeys = 0;

	int i;
	for ( i = 0; i < 5; ++i ) {
		if (  ((*key) & key_flags[i]) != 0 ) {
			newkeys |= key_flags[ key_order[i] ];
		}
	}

	*key = newkeys;
}


Confusionator::Confusionator(Ship *target, double olifetime)
{
	STACKTRACE;
	t = target;
	lifetime = olifetime;

	oci = new OverrideControlIceci();
	t->set_override_control(oci);
}


void Confusionator::calculate()
{
	STACKTRACE;
								 // in seconds
	lifetime -= frame_time * 1E-3;

	if ( !(t && t->exists()) ) {
		//t = 0;	no, we need this once more
		state = 0;
		//return;
	}

	if ( lifetime < 0 ) {
		state = 0;
		//t->del_override_control(oci);
		//return;
	}

	if (!exists()) {
		t->del_override_control(oci);
	}
}


REGISTER_SHIP(IceciConfusion)
