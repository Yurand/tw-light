/* $Id: shpostdi.cpp,v 1.10 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

class OstokDisplacer : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double        specialRange;

	//This is from the Arilou code
	int just_teleported;

	public:
		OstokDisplacer(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();

		//This is from the Arilou code
	protected:
		virtual void inflict_damage(SpaceObject *other);
		virtual void calculate();

};

OstokDisplacer::OstokDisplacer(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	//From Arilou
	just_teleported = 0;

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialRange   = scale_range(get_config_float("Special", "Range", 0));

}


int OstokDisplacer::activate_weapon()
{
	STACKTRACE;
	add(new Missile(this, Vector2(0.0, (size.y / 2.0)),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}


//this is all from arilou
void OstokDisplacer::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (just_teleported && other->mass) {
		handle_damage(other, 0, 999);
	}
	Ship::inflict_damage(other);
	return;
}


int OstokDisplacer::activate_special()
{
	STACKTRACE;

	if (target && target->exists() && (!target->isInvisible()) && (target->mass > 0)) {
		//from Arilou code
		//dx = (double)(random() % (int)(3000)) - 1500;
		//dy = (double)(random() % (int)(3000)) - 1500;

		Vector2 ss = pos;
		Vector2 tt = target->normal_pos();

		double d;
		d = distance(target);
		if (d > specialRange)
			return false;

		target->translate(ss - tt);
		translate(tt-ss);
		just_teleported = 1;

	}

	return(TRUE);
}


void OstokDisplacer::calculate()
{
	STACKTRACE;
	just_teleported = 0;
	Ship::calculate();
}


REGISTER_SHIP(OstokDisplacer)
