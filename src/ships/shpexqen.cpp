/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#define maxbarriers 16
#define UNIT_RATIO 25
#define EXQUIVAN_BARRIER 0x2E

class ExquivanBarrier;

class ExquivanEnigma : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  double       weaponRing;
  double       weaponMass;

  double specialRange;

  int    numbarriers;

  ExquivanBarrier **weaponObject;

  public:
  ExquivanEnigma(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual void calculate();
  virtual void calculate_fire_weapon();
  virtual int activate_weapon();
  virtual int activate_special();
  int get_barrier_number(ExquivanBarrier *obarrier);
};

class ExquivanBarrier : public Missile {
public:

  double safedist;
  double veloc;
  double brange;

  ExquivanEnigma	*exqship;
  int				remembernumber;

  public:
  ExquivanBarrier(Vector2 opos, double oangle, double ov,
    int odamage, double orange, int oarmour, ExquivanEnigma *oship,
    SpaceSprite *osprite, double osrange, double omass);

  virtual void calculate();
};

ExquivanEnigma::ExquivanEnigma(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code) {

  weaponRange    = scale_range(get_config_float("Weapon", "OuterRange", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponRing     = scale_range(get_config_float("Weapon","Ring",0));
  weaponMass     = get_config_float("Weapon","Mass",0);

  specialRange  = scale_range(get_config_float("Special", "Range", 0));

  numbarriers = 0;
	 weaponObject = new ExquivanBarrier*[maxbarriers];
	 for (int i = 0; i < maxbarriers; i += 1)
		  weaponObject[i] = NULL;
}

void ExquivanEnigma::calculate_fire_weapon()
{
  weapon_sample = tw_random(2);
  Ship::calculate_fire_weapon();
}

int ExquivanEnigma::activate_weapon() {
  if (numbarriers == maxbarriers)
    return(FALSE);
  weaponObject[numbarriers] = new ExquivanBarrier(
    Vector2(0.0, size.y/2),angle, weaponVelocity, weaponDamage, weaponRange,
    weaponArmour, this, data->spriteWeapon,weaponRing,weaponMass);
	 add(weaponObject[numbarriers]);
	 numbarriers += 1;
  return(TRUE);
}

int ExquivanEnigma::activate_special() {
  int fire = FALSE;
  SpaceObject *o;

	 Query a;
  double multi = ((double)((random () % 20) + 10) / 10);
  for (a.begin(this, bit(LAYER_SHIPS), specialRange); a.current; a.next()) {
		o = a.currento;
  if( (!o->sameTeam(this)) && (distance(o) < specialRange) &&
			 o->isShip() && o->exists()) {
     double dist = distance(o) * multi;
     double a = trajectory_angle(o);
		    o->translate(unit_vector(a) * dist);
     fire = TRUE;
				}
			}
	return(fire);
}

void ExquivanEnigma::calculate()
{
 Ship::calculate();
	int j = 0;
	for (int i = 0; i < numbarriers; i += 1) {
		weaponObject[i-j] = weaponObject[i];
		if (!weaponObject[i]->exists()) j += 1;
		if (j) weaponObject[i] = NULL;
		}
	numbarriers -= j;
 return;
}

int ExquivanEnigma::get_barrier_number(ExquivanBarrier *obarrier)
{
for (int k=0;k<numbarriers;k++)
  if (obarrier == weaponObject[k])
    return (k);
return (-1);
}

ExquivanBarrier::ExquivanBarrier(Vector2 opos, double oangle,
  double ov, int odamage, double orange, int oarmour,ExquivanEnigma *oship,
  SpaceSprite *osprite,double osrange, double omass) :
  Missile(oship, opos, oangle, ov, odamage, -1 , oarmour, oship,
		osprite),
  safedist(osrange),
  veloc(ov),
  brange(orange)
{
  layer = LAYER_SPECIAL;
  mass = omass;
  exqship = oship;
  remembernumber = 0;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}

void ExquivanBarrier::calculate()
{
  Missile::calculate();
  SpaceLocation *R = 0;
  double final_angle = 0;

  if (!(ship && ship->exists()))
  {
	  state = 0;	// remove the weapon when the ship is destroyed.
	  ship = 0;
	  return;
  }

  if (!(exqship && exqship->exists()))
  {
	  exqship = 0;
  }

  //int number = ((ExquivanEnigma *)ship)->get_barrier_number(this);
  int number = -1;
  if (exqship)
  {
	  number = exqship->get_barrier_number(this);
	  remembernumber = number;
	  if (number == -1) {
		  state = 0;
		  return;
	  }
  } else
	  number = remembernumber;

  if ((ship->target) && (ship->target->exists()) &&
    !(ship->target->isInvisible()) && !(ship->target->sameTeam(this)))
    {
      SpaceObject *o, *t = NULL;
      double dist = distance(ship->target);
      double oldrange = 9999999;
      Query a;
      for (a.begin(this,bit(LAYER_SHOTS) + bit(LAYER_SPECIAL), dist); a.current; a.next()) {
		      o = a.currento;
		      if( (!o->sameTeam(this)) && (distance(o) < oldrange) &&
			       canCollide(o) && o->exists() && !(o->isAsteroid() || o->isPlanet()) &&
          !((o->normal_pos().x >= normalize(ship->normal_pos().x - ship->get_size().x/3,map_size.x)) &&
            (o->normal_pos().x <= normalize(ship->normal_pos().x + ship->get_size().x/3,map_size.x)) &&
            (o->normal_pos().y >= normalize(ship->normal_pos().y - ship->get_size().y/3,map_size.y))  &&
            (o->normal_pos().y <= normalize(ship->normal_pos().y + ship->get_size().y/3,map_size.y)))){
          t = o;
          oldrange = distance(o);
        }
      }
     if (!t)
       t=ship->target;
     if (ship->distance(t) < brange) {
       final_angle = trajectory_angle(t);
       R = new SpaceLocation(NULL, t->normal_pos(),0);
     } else {
       int sign = (-2 * (number % 2)) + 1;
       int units = (number+1) / 2;
       final_angle = ship->trajectory_angle(ship->target);
       R = new SpaceLocation (NULL, 
         (ship->normal_pos() + (unit_vector(final_angle) * brange) +
         (unit_vector(final_angle + (90 * sign)*ANGLE_RATIO) * units * UNIT_RATIO)),
         0);
     } }
  else
    {
      SpaceObject *o, *t = NULL;
      double oldrange = 9999999;
      Query a;
      for (a.begin(this,bit(LAYER_SHOTS) + bit(LAYER_SPECIAL),
        safedist * 2.5); a.current; a.next()) {
		      o = a.currento;
		      if( (!o->sameTeam(this)) && (distance(o) < oldrange) &&
			       canCollide(o) && o->exists() && !(o->isAsteroid() || o->isPlanet())) {
          t = o;
          oldrange = distance(o);
          }
        }
      if (t) {
        final_angle = trajectory_angle(t);
        R = new SpaceLocation(NULL, t->normal_pos(),0);
      } else {
      final_angle = (ship->get_angle())+ (number * PI2/maxbarriers);
      R = new SpaceLocation(NULL, 
        (ship->normal_pos()+(unit_vector(final_angle) * safedist)),
        0);
    } }
    if (distance(R) > (veloc * frame_time)) {
      angle = trajectory_angle(R);
      vel = veloc * unit_vector(angle );
    } else {
      pos = R->normal_pos();
      angle = final_angle;
      vel = 0;
    }
    sprite_index = get_index(angle);

	if (R)
		delete R;
}



REGISTER_SHIP(ExquivanEnigma)
