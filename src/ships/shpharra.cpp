/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

#define BCC 3

class HarikaYornRavager : public Ship {
public:
  int          regenrateFrames;
  int          regenrateCount;
  int          regenrating;

  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  int          specialFrames;
  int          shieldFrames;
  int          changethrust;
  double       specialThrust;

  public:
  HarikaYornRavager(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate_thrust();
  virtual void calculate();
  virtual void animate(Frame *space);
};

class LassoMissile : public Missile {
public:
  public:
  LassoMissile(Vector2 opos, double oangle, double ov, int odam,
    double orange, int oarmour, Ship *oship, SpaceSprite *osprite);
};

class LassoLaser : public Laser {
public:
  double oldnx[BCC];
  double oldny[BCC];
  double oldex[BCC];
  double oldey[BCC];
  double oldlen[BCC];
  LassoMissile *LeftMissile;
  LassoMissile *RightMissile;
  int     snapped;

  public:
  virtual void calculate();
  virtual void collide(SpaceObject *o);
  LassoLaser (LassoMissile *oLeft ,LassoMissile *oRight,Ship *oship);
};

HarikaYornRavager::HarikaYornRavager(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{
  regenrateFrames = 4000;
  regenrating    = FALSE;

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  specialThrust  = scale_velocity(get_config_float("Special","Thrust",0));
  specialFrames  = get_config_int("Special", "Frames", 0);
  shieldFrames   = 0;
}

int HarikaYornRavager::activate_weapon() {
  LassoMissile *LeftMissile;
  LassoMissile *RightMissile;
  LeftMissile = new LassoMissile(Vector2(-9.0, (size.y/2)+12),
    angle-10*ANGLE_RATIO, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon);
  RightMissile = new LassoMissile(Vector2(9.0, (size.y/2)+12),
    angle+10*ANGLE_RATIO, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon);
  add(LeftMissile);
  add(RightMissile);
  add(new LassoLaser(LeftMissile,RightMissile,this));
  return(TRUE);
}

int HarikaYornRavager::activate_special()
{
  if (crew > 1) {
    accelerate(this, angle, specialThrust, MAX_SPEED);
    crew--;
    shieldFrames = specialFrames;
    changethrust = TRUE;
    return(TRUE);
  } else return(FALSE);
}

void HarikaYornRavager::calculate_thrust()
{
       if(!changethrust) Ship::calculate_thrust();
       changethrust = FALSE;
}

void HarikaYornRavager::calculate()
{
   if (regenrating) {
     if (crew < crew_max) {
       if ((regenrateCount -= frame_time) < 0) {
         crew++;
         play_sound2(data->sampleExtra[1]);
         regenrateCount = regenrateFrames;
     } } else regenrating = FALSE;
   } else if (!(regenrating) && (crew < crew_max)) {
     regenrating = TRUE;
     regenrateCount = regenrateFrames;
    }
   if(shieldFrames > 0)
     shieldFrames-= frame_time;
   Ship::calculate();
}

void HarikaYornRavager::animate(Frame *space)
{
  if(shieldFrames > 0)
    data->spriteSpecial->animate( pos, sprite_index, space);
  else
    sprite->animate( pos, sprite_index, space);
}

LassoMissile::LassoMissile(Vector2 opos, double oangle,
  double ov, int odamage, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite) 
	: 
	Missile(oship, opos, oangle, ov, odamage, orange,
  oarmour, oship, osprite,0)
{
	collide_flag_sameship = bit(LAYER_SHOTS);
}

LassoLaser::LassoLaser(LassoMissile *oLeft, LassoMissile* oRight,Ship *oship) :
  Laser(oship, (oLeft->trajectory_angle(oRight)+0), palette_color[4],
  (oLeft->distance(oRight)),0,500,oLeft,Vector2(4,15))
{
  collide_flag_sameship = bit(LAYER_SHIPS) | bit(LAYER_SHOTS);
  for (int i=0;i<BCC;i++) {
    oldnx[i] = normal_pos().x;
    oldny[i] = normal_pos().y;
    oldex[i] = edge_x();
    oldey[i] = edge_y();
    oldlen[i] = length;
  }
  LeftMissile = oLeft;
  RightMissile = oRight;
  snapped = FALSE;
}

void LassoLaser::calculate()
{
  if (!((LeftMissile->exists()) && (LeftMissile->sameShip(this))))
    {
      if ((RightMissile->exists()) && (RightMissile->sameShip(this)) &&
      (!snapped))
        RightMissile->changeDirection(RightMissile->get_angle()-90);
      state = 0;
      snapped = TRUE;
    }
  if (!((RightMissile->exists()) && (RightMissile->sameShip(this))))
    {
      if ((LeftMissile->exists()) && (LeftMissile->sameShip(this)) &&
      (!snapped))
        LeftMissile->changeDirection(LeftMissile->get_angle()+90);
      state = 0;
      snapped = TRUE;
    }
  frame -= frame_time;
  angle = LeftMissile->trajectory_angle(RightMissile)+0;
  length = distance(RightMissile);
  Laser::calculate();
  for (int i=0; (i+1) < BCC; i++){
  oldnx[i+1] = oldnx[i];
  oldny[i+1] = oldny[i];
  oldex[i+1] = oldex[i];
  oldey[i+1] = oldey[i];
  oldlen[i+1] = oldlen[i];
  }
  oldnx[0] = normal_pos().x;
  oldny[0] = normal_pos().y;
  oldex[0] = edge_x();
  oldey[0] = edge_y();
  oldlen[0] = length;
}

void LassoLaser::collide(SpaceObject *o)
{
  double old_length = length;
  double old_oldlen;
  int collison = FALSE;

  if((!canCollide(o)) || (!o->canCollide(this)))
    return;
  for (int i=0; i < BCC ; i++) {
  old_oldlen = o->collide_ray(Vector2(oldnx[i], oldny[i]), Vector2(oldnx[i] + oldex[i],
    oldny[i] + oldey[i]), oldlen[i]);
  collison = (collison || (old_oldlen != oldlen[i]));
  }
  length = o->collide_ray(normal_pos(), normal_pos() + edge(), length);
  if ( (length == old_length) && (!collison) )
    return;

  play_sound2(LeftMissile->data->sampleExtra[0]);

  if ((LeftMissile->exists()) && (LeftMissile->sameShip(this)) && (!snapped))
    LeftMissile->changeDirection(normalize(LeftMissile->get_angle()+PI/2,PI2));
  if ((RightMissile->exists()) && (RightMissile->sameShip(this)) && (!snapped))
    RightMissile->changeDirection(normalize(RightMissile->get_angle()-PI/2,PI2));
  state = 0;
  snapped = TRUE;
}


REGISTER_SHIP(HarikaYornRavager)
