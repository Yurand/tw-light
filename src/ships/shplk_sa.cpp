/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class LkSanctorum : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  public:
  LkSanctorum(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual void animate(Frame *space);
  virtual void calculate();
};

LkSanctorum::LkSanctorum(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
}

int LkSanctorum::activate_weapon() {
  add(new Missile(this, 
    Vector2(0.0, (size.y / 2.0+5)), angle - (tw_random(5) - 2) * (3.0 * ANGLE_RATIO),
    weaponVelocity, weaponDamage, weaponRange, weaponArmour, this,
    data->spriteWeapon));
  return(TRUE);
}

void LkSanctorum::animate(Frame *space) {
  if (special_recharge > 0) {
		sprite->animate_character(pos, sprite_index, pallete_color[hot_color[((int)(special_recharge/10)) % HOT_COLORS]], space);
		}
  else sprite->animate( pos, sprite_index, space);
	return;
	}

void LkSanctorum::calculate() {
  if (special_recharge > 0)
    collide_flag_anyone=0;
	 else collide_flag_anyone=ALL_LAYERS;
  Ship::calculate();
}




REGISTER_SHIP(LkSanctorum)
