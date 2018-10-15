/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

class DaktaklakpakMine;

class DaktaklakpakVivisector : public Ship {
public:

  int          weaponFrames;
  int 	       drillFrames;
  int          weaponDamage;
  int          damageFrameAmount;
  int          drillDamagePerDamageFrame;
  int          damageFrameLeft;
  int          drillDamageLeft;
  int          latched;
  Ship         *grabbed;
  double       grabangle;
  double       grabdistance;
  double       grabshipangle;
  
  double       specialLaunch;
  double       specialRange;
  double       specialVelocity;
  int          specialDamage;
  int          specialArmour;
  int          specialArming;
  DaktaklakpakMine **weaponObject;

  public:
  DaktaklakpakVivisector(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();
  virtual int canCollide(SpaceObject *other);
  virtual void animate(Frame *space);
  virtual void inflict_damage(SpaceObject *other);
  
  int numMines;
  int maxMines;
};

class DaktaklakpakMine : public AnimatedShot {
public:

  double       missileVelocity;
  double       missileRange;
  int          missileDamage;
  int          missileArmour;
  int          mineArming;
  int          mineactive;
  double       grabangle;
  double       grabdistance;

  public:
  DaktaklakpakMine(Vector2 opos,double ov, double oangle, int odamage, int oarmour,
    DaktaklakpakVivisector *oship, SpaceSprite *osprite, int ofcount, int ofsize, double misv, double misr,
    int misd, int misa,int misaf);

  virtual void calculate();

};

DaktaklakpakVivisector::DaktaklakpakVivisector(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)

{
  weaponFrames = get_config_int("Weapon", "Frames", 0);
  drillFrames  = 0;
  weaponDamage   = get_config_int("Weapon", "Damage", 0);

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialDamage   = get_config_int("Special", "Damage", 0);
  specialArmour   = get_config_int("Special", "Armour", 0);
  specialArming   = get_config_int("Special","Arming",0);
  specialLaunch   = scale_velocity(get_config_int("Special","Launch",0));
  latched         = FALSE;
  grabbed         = NULL;

  numMines=0;
  maxMines=8;
  weaponObject = new DaktaklakpakMine*[maxMines];
  for (int i = 0; i < maxMines; i += 1) {
    weaponObject[i] = NULL;
		}

}

int DaktaklakpakVivisector::activate_weapon()
{
  if (drillFrames > 0)
    return(FALSE);
  else
    {  drillFrames = weaponFrames;
       return(TRUE);
  }
}

int DaktaklakpakVivisector::activate_special()
{
	if (numMines == maxMines) {
		weaponObject[0]->state = 0;
		numMines -= 1;
		for (int i = 0; i < numMines; i += 1) {
			weaponObject[i] = weaponObject[i + 1];
			}
		weaponObject[numMines] = NULL;
		}
	weaponObject[numMines] = new DaktaklakpakMine(Vector2(0.0, -(size.y / 2.0)),specialLaunch, (angle + PI),
	   specialDamage, specialArmour, this, data->spriteSpecial, 8, 60, specialVelocity,specialRange,
	   specialDamage, specialArmour, specialArming);
	add(weaponObject[numMines]);
	numMines += 1;
	return(TRUE);
}

void DaktaklakpakVivisector::calculate()
{
   Ship::calculate();

   if(drillFrames > 0) {
     drillFrames-= frame_time;
     if ((drillFrames <= 0) && (!latched)) {
       play_sound2(data->sampleWeapon[0]);
      }
    }
   else latched = FALSE;
   if (grabbed != NULL)
     if (!(grabbed ->exists())){
       latched = FALSE;
       grabbed = NULL;
     }
   if (latched) {
      damageFrameLeft-=frame_time;
      if (damageFrameLeft <=0) {
        damageFrameLeft += damageFrameAmount;
        if (drillDamageLeft < drillDamagePerDamageFrame)
          damage(grabbed, drillDamageLeft);
        else {
          damage(grabbed, drillDamagePerDamageFrame);
          drillDamageLeft -= drillDamagePerDamageFrame; }
        }
      grabangle = (grabbed->get_angle() - grabshipangle) + grabangle;
      angle=grabangle;
      grabshipangle = grabbed->get_angle();
	  nextkeys &= ~(keyflag::left | keyflag::right | keyflag::thrust);

      pos = grabbed->normal_pos()-((unit_vector(grabangle )) * grabdistance);
   }
   int j = 0;
   for (int i = 0; i < numMines; i += 1) {
    weaponObject[i-j] = weaponObject[i];
    if (!weaponObject[i]->exists()) j += 1;
      if (j) weaponObject[i] = NULL;
    }
    numMines -= j;

}
int DaktaklakpakVivisector::canCollide(SpaceObject *other)
{
  if ((latched) && (grabbed!=NULL) && (grabbed->exists())) {
    if (grabbed == other)
      return (FALSE);
		}
  return (Ship::canCollide(other));
}
 
void DaktaklakpakVivisector::animate(Frame *space)
{
  if(drillFrames > 0)
    data->spriteWeapon->animate( pos, sprite_index, space);
  else
    sprite->animate( pos, sprite_index, space);

}


void DaktaklakpakVivisector::inflict_damage(SpaceObject *other)
{
  if (drillFrames > 0)
    if (!latched)
      if ((!(sameTeam(other))) &&
        (other->isShip())) {
          latched=TRUE;
          grabbed= (Ship *) other;
          grabangle= (trajectory_angle(other) );
          grabdistance = (distance(other) * 1.1);
          grabshipangle = (other->get_angle());
          drillDamageLeft = weaponDamage;
          play_sound2(data->sampleExtra[1]);
          if ((drillFrames / frame_time)< weaponDamage) {
            drillDamagePerDamageFrame = (weaponDamage/drillFrames)
              + ((weaponDamage % drillFrames) > 0.00001);
            damageFrameLeft = 1;
            damageFrameAmount = 1;
          } else {
            damageFrameAmount = (drillFrames/weaponDamage);
            damageFrameLeft = damageFrameAmount;
            drillDamagePerDamageFrame = 1;
            }
    }
  Ship::inflict_damage(other);
}

DaktaklakpakMine::DaktaklakpakMine(Vector2 opos,double ov, double oangle, int odamage,
  int oarmour,DaktaklakpakVivisector *oship, SpaceSprite *osprite, int ofcount, int ofsize, double misv,
  double misr, int misd, int misa,int misaf) :
  AnimatedShot(oship, opos, oangle, ov, odamage, -1.0, oarmour, oship, 
    osprite, ofcount, ofsize)

{
  missileVelocity = misv;
  missileRange = misr;
  missileDamage =misd;
  missileArmour = misa;
  mineArming = misaf;
  mineactive = FALSE;
}

void DaktaklakpakMine::calculate()

{
  AnimatedShot::calculate();
  if (!mineactive) {
    mineArming -= frame_time;
    if (mineArming <= 0) {
      mineactive = TRUE;
      v = 0;
      vel = 0;
      }
    }
  else {
    SpaceObject *o, *t = NULL;
    double oldrange = 999999;
    Query a;
    for (a.begin(this, bit(LAYER_SHIPS),(missileRange *.9));
           a.current; a.next()) {
		o = a.currento;
		if (!o->sameTeam(this) && (distance(o) < oldrange) && !(o->isAsteroid() || o->isPlanet())) {
			t = o;
			oldrange = distance(o);
			}
		}
    if (t) {
      add(new Missile(this,0,(trajectory_angle(t)),
        missileVelocity,missileDamage,missileRange,missileArmour,
        this,data->spriteExtra));
      play_sound2(data->sampleExtra[0]);
      destroy();
    }
  }
}



REGISTER_SHIP(DaktaklakpakVivisector)
