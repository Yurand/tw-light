/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

class DoogConstructor : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  public:
  DoogConstructor(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int activate_weapon();
  virtual int activate_special();
};
DoogConstructor::DoogConstructor(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
}

int DoogConstructor::activate_weapon()
{
	double a;
	if (target && !target->isInvisible()) {
		double r = distance(target);
		r = r / weaponVelocity;
		Vector2 blah = min_delta( normal_pos() , target->normal_pos() - 
				(get_vel() - target->get_vel()) * r);
		a = atan(blah) - PI;
		}
	else a = angle;
  add(new Missile(this, 0, a , weaponVelocity,
    weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon));
  return(TRUE);
}

int DoogConstructor::activate_special()
{
  int regen;

  if (crew >= crew_max)
    regen = FALSE;
  else {
    crew++;
    regen = TRUE;}
  return (regen);
}



REGISTER_SHIP(DoogConstructor)
