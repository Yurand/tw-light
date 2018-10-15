/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

class KtangMine;

class KtangCrippler : public Ship {
public:

  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  double       specialLaunch;
  double       specialRange;
  double       specialVelocity;
  int          specialDamage;
  int          specialArmour;
  int          specialArming;
  KtangMine    **weaponObject;

  public:
  KtangCrippler(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();

  int numMines;
  int maxMines;
};

class KtangMine : public Shot {
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
  KtangMine(Vector2 opos,double ov, double oangle, int odamage, int oarmour,
    KtangCrippler *oship, SpaceSprite *osprite, double misv, double misr,
    int misd, int misa,int misaf);

  virtual void calculate();

};

KtangCrippler::KtangCrippler(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)

{
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialRange    = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);
	specialArming   = get_config_int("Special","Arming",0);
	specialLaunch   = scale_velocity(get_config_int("Special","Launch",0));

	numMines=0;
	maxMines=8;
	weaponObject = new KtangMine*[maxMines];
	for (int i = 0; i < maxMines; i += 1) {
		weaponObject[i] = NULL;
	}
}

int KtangCrippler::activate_weapon()
{
  add(new Shot(this, Vector2(23.0, 75),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon,1));
  add(new Shot(this, Vector2(-23.0, 75),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon,1));
  add(new Shot(this, Vector2(26.0, 60),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon,1));
  add(new Shot(this, Vector2(-26.0, 60),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon,1));
  return (TRUE);
}

int KtangCrippler::activate_special()
{
	if (numMines == maxMines) {
		weaponObject[0]->state = 0;
		numMines -= 1;
		for (int i = 0; i < numMines; i += 1) {
			weaponObject[i] = weaponObject[i + 1];
			}
		weaponObject[numMines] = NULL;
		}
	weaponObject[numMines] = new KtangMine(Vector2(0.0, -(size.y / 2.0)),specialLaunch, (angle + PI),
	   4, 4, this, data->spriteSpecial,specialVelocity,specialRange,
	   specialDamage, specialArmour, specialArming);
	add(weaponObject[numMines]);
	numMines += 1;
	return(TRUE);
}

void KtangCrippler::calculate()
{
  Ship::calculate();

  int j = 0;
  for (int i = 0; i < numMines; i += 1) {
    weaponObject[i-j] = weaponObject[i];
    if (!weaponObject[i]->exists()) j += 1;
    if (j) weaponObject[i] = NULL;
    }
  numMines -= j;
}

KtangMine::KtangMine(Vector2 opos,double ov, double oangle, int odamage,
  int oarmour,KtangCrippler *oship, SpaceSprite *osprite, double misv,
  double misr, int misd, int misa,int misaf) :
  Shot(oship, opos, oangle, ov, odamage, -1.0, oarmour, oship, 
    osprite)

{
  missileVelocity = misv;
  missileRange = misr;
  missileDamage =misd;
  missileArmour = misa;
  mineArming = misaf;
  mineactive = FALSE;
}

void KtangMine::calculate()

{
  Shot::calculate();
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
      add(new Missile(this,0,(trajectory_angle(t)+5*ANGLE_RATIO),
        missileVelocity,missileDamage,missileRange,missileArmour,
        this,data->spriteExtra));
      add(new Missile(this,0,(trajectory_angle(t)+15*ANGLE_RATIO),
        missileVelocity,missileDamage,missileRange,missileArmour,
        this,data->spriteExtra));
      add(new Missile(this,0,(trajectory_angle(t)+25*ANGLE_RATIO),
        missileVelocity,missileDamage,missileRange,missileArmour,
        this,data->spriteExtra));
      add(new Missile(this,0,(trajectory_angle(t)+35*ANGLE_RATIO),
        missileVelocity,missileDamage,missileRange,missileArmour,
        this,data->spriteExtra));
      add(new Missile(this,0,(trajectory_angle(t)-5*ANGLE_RATIO),
        missileVelocity,missileDamage,missileRange,missileArmour,
        this,data->spriteExtra));
      add(new Missile(this,0,(trajectory_angle(t)-15*ANGLE_RATIO),
        missileVelocity,missileDamage,missileRange,missileArmour,
        this,data->spriteExtra));
      add(new Missile(this,0,(trajectory_angle(t)-25*ANGLE_RATIO),
        missileVelocity,missileDamage,missileRange,missileArmour,
        this,data->spriteExtra));
      add(new Missile(this,0,(trajectory_angle(t)-35*ANGLE_RATIO),
        missileVelocity,missileDamage,missileRange,missileArmour,
        this,data->spriteExtra));
      play_sound2(data->sampleExtra[0]);
      state=0;
    }
  }
}



REGISTER_SHIP(KtangCrippler)
