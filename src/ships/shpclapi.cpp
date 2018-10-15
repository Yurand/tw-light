/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE
#include "../melee/mcbodies.h"

class ClairconctlarPinnacle : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  double       specialVelocity;
  int          specialArmour;
  int          specialFired;
  Shot        *specialObject;
  int          specialSoundCount;
  int          beep;

  public:
  ClairconctlarPinnacle(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);
  void set_beep (int set);
  int isBad_Warp(SpaceObject *o);
  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();
};

class ClairconctlarBeacon : public Shot {
public:
  ClairconctlarPinnacle *mother;

  public:
  ClairconctlarBeacon(Vector2 opos, double oangle, double ov, int odamage,
    double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
    ClairconctlarPinnacle *opinnacle);

  virtual void inflict_damage(SpaceObject *other);
  void soundExplosion();

};

ClairconctlarPinnacle::ClairconctlarPinnacle(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponVelocity = scale_velocity(get_config_int("Weapon", "Velocity", 0));

  specialVelocity    = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialArmour      = get_config_int("Special", "Armour", 0);
  specialObject      = NULL;
  specialFired       = FALSE;
  specialSoundCount  = 0;
  beep               = FALSE;
}

void ClairconctlarPinnacle::set_beep(int set)
{
  beep = set;
}

int ClairconctlarPinnacle::activate_weapon()
{
  add(new AnimatedShot(this, 
    Vector2(0.0, (size.y / 2.0)), angle, weaponVelocity, weaponDamage, weaponRange,
    weaponArmour, this, data->spriteWeapon, 10, 1 ));
  add(new AnimatedShot(this, 
    Vector2(-(size.x / 3.0), (size.y/20.0)), angle - PI/2, weaponVelocity, weaponDamage,
    weaponRange, weaponArmour, this, data->spriteWeapon, 10, 1, 1.0 ));
  add(new AnimatedShot(this, 
    Vector2(size.x / 3.0,  -(size.y / 8.0)), angle + PI/2, weaponVelocity, weaponDamage,
    weaponRange, weaponArmour, this, data->spriteWeapon, 10, 1, 1.0 ));
  add(new AnimatedShot(this, 
    Vector2(-(size.x / 3.0), -(size.y / 8.0)), angle - PI/2, weaponVelocity, weaponDamage,
    weaponRange, weaponArmour, this, data->spriteWeapon, 10, 1, 1.0 ));
  add(new AnimatedShot(this, 
    Vector2(size.x / 3.0, (size.y/20.0)), angle + PI/2, weaponVelocity, weaponDamage,
    weaponRange, weaponArmour, this, data->spriteWeapon, 10, 1, 1.0 ));

  return(TRUE);

}

int ClairconctlarPinnacle::activate_special() {
	if(specialFired)
		return(FALSE);
	specialFired = TRUE;
	if (!specialObject) {
		specialObject = new ClairconctlarBeacon(Vector2(0.0, -(size.y / 2.0)), angle + PI,
			specialVelocity, 0, -1.0, specialArmour, this, data->spriteSpecial, this);
		specialSoundCount = 0;
		beep = TRUE;
		add(specialObject);
		return(TRUE);
		}
	else {
		Vector2 opos, ovel;
		double oangle;
		beep = FALSE;
		opos=specialObject->normal_pos();
		ovel=specialObject->get_vel();
		oangle=specialObject->get_angle();
		specialObject->destroy();
		pos=opos;
		//pos=(opos-(size/2.0));
		vel = ovel;
		angle=oangle;
		SpaceLocation *spacePlanet = nearest_planet();
		if (spacePlanet && isBad_Warp((SpaceObject*)spacePlanet)) {
			pos = spacePlanet->normal_pos();
			this->handle_damage(spacePlanet, 0, 999);
			} 
		else if (target) if (isBad_Warp((SpaceObject*)target)) {
			double dist = distance(target);
			dist = size.x + ((SpaceObject*)target)->get_size().x - dist;
			if (dist < 10) dist = 10;
			translate( unit_vector(trajectory_angle(target) + PI) * dist);
			}
		specialObject = NULL;
		//sound.stop(data->sampleSpecial[1]);
		play_sound2(data->sampleExtra[0]);
		return(FALSE);
		}
	}

void ClairconctlarPinnacle::calculate()
{
  Ship::calculate();
  if((specialObject != NULL) && (!specialObject->exists())) {
    //sound.stop(data->sampleSpecial[1]);
    specialObject = NULL;
  }

  //if(specialFired && (!fire_special))
  //  specialFired = FALSE;

  if((specialObject) && (fire_special) && (beep)) {
    specialSoundCount += frame_time;
    if (specialSoundCount >= 2500) {
       specialSoundCount = 0;
	   play_sound2(data->sampleSpecial[1]);
		}
    }

  // release special
  if((specialObject) && (!fire_special))
  {
	  specialFired = FALSE;
	  activate_special();
	  specialFired = FALSE;
  }

}

ClairconctlarBeacon::ClairconctlarBeacon(Vector2 opos, double oangle,
  double ov, int odamage, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite, ClairconctlarPinnacle *omother) :
  Shot (oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite)
  {
    mother = omother;
  }

void ClairconctlarBeacon::soundExplosion()
{
  //sound.stop(data->sampleSpecial[1]);
  mother->set_beep(FALSE);
}

void ClairconctlarBeacon::inflict_damage(SpaceObject *other)
{
  //sound.stop(data->sampleSpecial[1]);
  mother->set_beep(FALSE);
  Shot::inflict_damage(other);
}

int ClairconctlarPinnacle::isBad_Warp(SpaceObject *o)
{
 double _d = distance_from(o->normal_pos(), normal_pos());

  if (_d <= size.x * 1.8)  //too dirty, but enough for planet
   return (TRUE);

 return (FALSE);
}



REGISTER_SHIP(ClairconctlarPinnacle)
