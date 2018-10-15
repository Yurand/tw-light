/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

struct hitlist {
  SpaceObject *item;
  int          endframe;
  struct hitlist *next;
  };
void hitlist_clear(hitlist **list) {
	while (true) {
		if (!*list) break;
		hitlist *tmp = *list;
		*list = (*list)->next;
		delete tmp;
		}
	return;
	}
hitlist *hitlist_check(hitlist **list, SpaceObject *item) {
	while (true) {
		if (!*list) break;
		if ((*list)->endframe > game->game_time ) {
			if ((*list)->item == item) return *list;
			list = &(*list)->next;
			}
		else {
			hitlist *tmp = *list;
			*list = (*list)->next;
			delete tmp;
			}
		}
	return NULL;
	}
bool hitlist_hit(hitlist **list, SpaceObject *item, int duration) {
	if (hitlist_check(list, item)) return false;
	hitlist *old = *list;
	*list = new hitlist;
	(*list)->item = item;
	(*list)->endframe = game->game_time + duration;
	(*list)->next = old;
	return true;
	}

class VyroIngoSheild;

class VyroIngoInvader : public Ship {
public:

  public:

  VyroIngoSheild *sheild;
  int    weaponDamage;
  int    weaponFrames;
  int    specialDamage;
  int    specialFrames;

  public:
  VyroIngoInvader(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual void calculate();
  virtual int activate_weapon();
  virtual int activate_special();
};

class VyroIngoSheild : public SpaceObject {
public:

  int frames_left;
  int frames_total;
  hitlist *list;
  int dam;
  Ship *ship;

public:

VyroIngoSheild(Vector2 opos,int odamage,int oframes, Ship *oship,
  SpaceSprite *osprite);

void reset_time();
virtual void calculate();
virtual void inflict_damage(SpaceObject *other);
virtual void death();
};

class VyroIngoWake : public AnimatedShot {
public:

  int frames_total;
  int frames_left;
  SpaceSprite *mid;
  SpaceSprite *end;
  int status;
  public:

  VyroIngoWake (Vector2 opos,int odamage,int oframes,Ship *oship,
    SpaceSprite *osprite,SpaceSprite *ostartsprite, SpaceSprite *oendsprite);

  virtual void calculate();
  virtual void inflict_damage(SpaceObject *other);
};

VyroIngoInvader::VyroIngoInvader(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  specialDamage   = get_config_int("Special", "Damage", 0);
  specialFrames   = scale_frames(get_config_int("Special", "Frames", 0));
  sheild = NULL;
}

int VyroIngoInvader::activate_weapon()
{
  if (sheild)
    if (sheild->exists())
      sheild->reset_time();
  if ((!sheild) || (!sheild->exists())) {
    sheild = new VyroIngoSheild(normal_pos(), 
		weaponDamage,weapon_rate,this,data->spriteWeapon);
    add(sheild);
    }
  return(TRUE);
}

int VyroIngoInvader::activate_special()
{
  add(new VyroIngoWake(Vector2(0,-(size.y/2+32)),specialDamage,
    specialFrames,this,data->spriteSpecial,data->spriteExtra,
    data->spriteExtraExplosion));
  return(TRUE);
}
void VyroIngoInvader::calculate()

{
  Ship::calculate();

  if (sheild)
    if (sheild->exists()) {
      if (!fire_weapon)
        sheild->state = 0;
    } else sheild = NULL;
}

VyroIngoSheild::VyroIngoSheild (Vector2 opos, int odamage, int oframes,
  Ship *oship,SpaceSprite *osprite) : SpaceObject (oship,opos,0,osprite),
  frames_left(0),
  frames_total(oframes),
  list(NULL),
  dam(odamage),
  ship(oship)
{
  layer = LAYER_SPECIAL;
  damage_factor = odamage;
  sprite_index = 1;
	vel = ship->get_vel();

	isblockingweapons = true;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}

void VyroIngoSheild::calculate()
{
  SpaceObject::calculate();

  if(!(ship && ship->exists()))
  {
	  ship = 0;
    state = 0;
	// ship=0 is done elsewhere
	return;
  }

  if(frames_left >= frames_total)
    state = 0;

  frames_left += frame_time;

  sprite_index = 1;

	pos = ship->normal_pos();
	vel = ship->get_vel();
}

void VyroIngoSheild::inflict_damage(SpaceObject *other) {
	damage_factor = dam;
	if (!hitlist_hit(&list, other, frames_total)) {
		damage_factor = 0;
		}
	SpaceObject::inflict_damage(other);
	return;
	}

void VyroIngoSheild::death() {
	hitlist_clear(&list);
	SpaceObject::death();
	return;
	}

void VyroIngoSheild::reset_time()
{
  frames_left = 0;
}

VyroIngoWake::VyroIngoWake (Vector2 opos,int odamage,int oframes,
  Ship *oship, SpaceSprite *osprite, SpaceSprite *ostartsprite,
    SpaceSprite *oendsprite) : AnimatedShot(oship, opos,0,0,odamage,-1.0,99,
    oship, ostartsprite,60,30),
  frames_total(oframes),
  frames_left(0),
  mid(osprite),
  end(oendsprite),
  status(1)
{
  mass = 1000;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}

void VyroIngoWake::calculate()
{
  vel = 0;
  AnimatedShot::calculate();
  if ((status == 1) && (sprite_index == 59))
    {
      status = 2;
      frame_count = 20;
      sprite_index = 0;
      sprite = mid;
    }
  frames_left+=frame_time;
  if (frames_left >= frames_total) {
    if ((sprite_index == 19) && (status == 2))
      {
      status = 3;
      frame_count = 64;
      sprite_index = 0;
      sprite = end;
      }
    if ((status == 3) && (sprite_index == 62))
      state = 0;
   }
  vel = 0;
}

void VyroIngoWake::inflict_damage(SpaceObject *other)

{
  if ((other->isShip()) || (other->isAsteroid()))
    SpaceObject::inflict_damage(other);
  else AnimatedShot::inflict_damage(other);
  vel=0;
}




REGISTER_SHIP(VyroIngoInvader)
