/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class PloxisPlunderer : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  double       weaponTurn;

  int          specialFrames;
  int          shieldFrames;

  double abss(double value);

  public:
  PloxisPlunderer(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

PloxisPlunderer::PloxisPlunderer(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponTurn     = scale_turning(get_config_float("Weapon", "Turning", 0));

  specialFrames = get_config_int("Special", "Frames", 0);
  shieldFrames  = 0;
}

double PloxisPlunderer::abss(double value)
{
  if (value < 0)
    return (value * -1);
  else return (value);
}

int PloxisPlunderer::activate_weapon()
{
  add(new HomingMissile(this, Vector2(0, 35),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    weaponTurn, this, data->spriteWeapon, target));
  return(TRUE);
}

int PloxisPlunderer::activate_special()
{
  shieldFrames = (shieldFrames % frame_time) + specialFrames;
  sprite = data->spriteSpecial;
  return(TRUE);
}

void PloxisPlunderer::calculate()
{
   Ship::calculate();

   if(shieldFrames > 0) {
     recharge_amount = 0;
     shieldFrames-= frame_time;
     Query a;
     for (a.begin(this, bit(LAYER_SHOTS),250); a.current; a.next()) {
       SpaceObject *o = a.currento;
       if ((!o->sameShip(this)) && (o->isShot()) && ((distance(o) 
				 - abss(cos(trajectory_angle(o) ) * (size.y /2))
         - abss(sin(trajectory_angle(o) ) * (size.x/3))
         - abss(sin(o->trajectory_angle(this) ) * (o->get_size().y/2))
         - abss(cos(o->trajectory_angle(this) ) * (o->get_size().x/2))
         - o->get_vel().magnitude()) < 0 )) {
          Shot *other= (Shot*)o;
          other->change_owner(this);
          other->changeDirection((other->get_angle()-
            trajectory_angle(other))*-1+(trajectory_angle(other)+PI));
            if ((other)->isHomingMissile())
          ((HomingMissile*) other)->target = this->target;
          play_sound(data->sampleSpecial[1]);
          }
        }
   if (shieldFrames <= 0)
     sprite = data->spriteShip;
     }
   else recharge_amount = 1;
}

int PloxisPlunderer::handle_damage(SpaceLocation *source, double normal, double direct)
{
	if(shieldFrames > 0) {
		normal = 0;
		direct = 0;
	}
	return Ship::handle_damage(source, normal, direct);
}




REGISTER_SHIP(PloxisPlunderer)
