/* $Id: shpvirli.cpp,v 1.8 2004/03/24 23:51:43 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

class VirtaoLimb : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	double       weaponTurnRate;

	double specialRange;
	double specialPower;

	public:
		VirtaoLimb(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
};

class VirtaoMissile : public HomingMissile
{
	public:
		VirtaoMissile(Vector2 opos, double oangle, double ov,
			int odamage, double orange, int oarmour, double otrate, Ship *oship,
			SpaceSprite *osprite);
};

VirtaoLimb::VirtaoLimb(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponTurnRate = scale_turning(get_config_float("Weapon", "TurnRate", 0));

	specialRange  = scale_range(get_config_float("Special", "Range", 0));
	specialPower  = scale_range(get_config_float("Special", "Power", 0));

}


int VirtaoLimb::activate_weapon()
{
	STACKTRACE;
	add(new VirtaoMissile(
		Vector2(0.0, 0.5*get_size().y), angle, weaponVelocity, weaponDamage, weaponRange,
		weaponArmour, weaponTurnRate, this, data->spriteWeapon));
	return(TRUE);
}


int VirtaoLimb::activate_special()
{
	STACKTRACE;
	int fire = FALSE;

	Query q;
	for (q.begin(this, ALL_LAYERS, specialRange);q.current;q.next()) {
		if (q.current->canCollide(this)) {
			double a = trajectory_angle(q.current);
			q.current->translate(specialPower * unit_vector(a));
			fire = TRUE;
		}
	}
	q.end();
	if (fire) play_sound((SAMPLE *)(melee[MELEE_BOOM + 0].dat));

	return(fire);
}


VirtaoMissile::VirtaoMissile(Vector2 opos, double oangle,
double ov, int odamage, double orange, int oarmour, double otrate,
Ship *oship, SpaceSprite *osprite) :
HomingMissile(oship, opos, oangle, ov, odamage, orange, oarmour, otrate, oship, osprite, oship->target)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
}


REGISTER_SHIP(VirtaoLimb)
