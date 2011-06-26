/* $Id: shpbubex.cpp,v 1.10 2004/03/24 23:51:40 yurand Exp $ */
/* Modified from Xchagger */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class BubalosExecutioner : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	int          weaponOffset;
	//  double       weaponDriftVelocity;
	//  int          shield;

	double       specialRange;
	double       specialVelocity;
	int          specialArmour;
	int          specialFrames;

	int          reverse_count, reverse_frame;

	public:
		BubalosExecutioner (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:
		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

class BubalosMissile : public Missile
{
	public:
		BubalosMissile(double ox, double oy, double oangle, double ov, int odamage,
			/*double weaponDriftVelocity, */double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite);
		virtual void inflict_damage (SpaceObject *other);
		double kick;
};

BubalosExecutioner::BubalosExecutioner(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weaponRange           = scale_range(get_config_int("Weapon", "Range", 0));
	weaponVelocity        = scale_velocity(get_config_int("Weapon", "Velocity", 0));
	weaponDamage          = get_config_int("Weapon", "Damage", 0);
	weaponArmour          = get_config_int("Weapon", "Armour", 0);
	//  weaponDriftVelocity   = scale_velocity(get_config_int("Weapon", "DriftVelocity", 0));
	//  shield                = get_config_int("Extra", "AbsorberShield", 0);

	reverse_frame         = 0;
	weaponOffset = 0;

	specialRange    = scale_range(get_config_int("Special", "Range", 0));
	specialVelocity = scale_velocity(get_config_int("Special", "Velocity", 0));
	specialArmour   = get_config_int("Special", "Armour", 0);
	specialFrames   = scale_frames(get_config_int("Special", "Frames", 0));
}


void BubalosExecutioner::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if (reverse_frame > 0) {
		if (reverse_count > 0) reverse_count -= frame_time;
		else {
			reverse_frame--;
			reverse_count += 160;

			if (reverse_frame > 0)
				add(new PositionedAnimation(this, this, Vector2((reverse_frame-3)*6.0, 0),
					data->spriteExtra, 0, 1, 160, LAYER_EXPLOSIONS));
			else
				angle = normalize(angle + PI, PI2);
		}
	}
}


int BubalosExecutioner::activate_weapon()
{
	STACKTRACE;
	add(new BubalosMissile(10.5*(weaponOffset-1), 44, angle,
		weaponVelocity, weaponDamage,
		/*weaponDriftVelocity,*/weaponRange, weaponArmour,
		this, data->spriteWeapon));
	weaponOffset = (weaponOffset + 1) % 3;
	return(TRUE);
}


int BubalosExecutioner::activate_special()
{
	STACKTRACE;

	//double shpAngle;

	reverse_frame = 6;
	reverse_count = 0;

	return(TRUE);
}


int BubalosExecutioner::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	if (normal > 0) {

		// remove the armor damage reduction
		//      normal -= shield;
		//      if (normal  <= 0)
		//         normal = 1;

		// remove the batt for damage quirk
		//      batt += normal;
		if (batt > batt_max) batt = batt_max;
	}
	return Ship::handle_damage(source, normal, direct);
}


BubalosMissile::BubalosMissile(double ox, double oy, double oangle, double ov,
int odamage, /*double weaponDriftVelocity,*/ double orange, int oarmour,
Ship *oship, SpaceSprite *osprite) :
Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship, osprite)
/*,
	kick(weaponDriftVelocity)*/
{
	STACKTRACE;
	return;
}


void BubalosMissile::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	// remove the weapon-bounce quirk.
	//if (other->mass)
	//	other->accelerate (this, angle, kick / other->mass, MAX_SPEED);
	Missile::inflict_damage(other);
}


REGISTER_SHIP(BubalosExecutioner)
