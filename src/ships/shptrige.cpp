/* $Id: shptrige.cpp,v 1.10 2004/03/24 23:51:42 yurand Exp $ */
/********************************************************/
/*	The Tridemin Gemini, based on the Ploxis	*/
/*	Plunderer code by Launchpad.			*/
/*				-Corona688, 2001	*/
/********************************************************/

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"
#include "../melee/mview.h"
#include <stdio.h>

#define ARM_LENGTH 45
#define TDegree 4

class TrideminTurret : public SpaceObject
{
	public:
		TrideminTurret(SpaceLocation *creator, Vector2 opos,
			double oangle, SpaceSprite *osprite);

		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

int TrideminTurret::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	//Redirects any damage from the turret module back
	//to the ship, but only if it exists.
	if (ship->exists()) {
		damage(ship, 0, normal + direct);
		return 0;
	}
	else    state=0;			 //If it doesn't exist, remove the
	//turret.
	return iround(normal + direct);
}


void TrideminTurret::calculate()
{
	STACKTRACE;
	if (!ship->exists()) {
		state=0;
		return;
	}
	sprite_index = get_index(angle);
}


TrideminTurret::TrideminTurret(SpaceLocation *creator, Vector2 opos,
double oangle, SpaceSprite *osprite):
SpaceObject(creator,opos,oangle,osprite)
{
	STACKTRACE;
	layer=LAYER_SPECIAL;
	mass=5.;
}


class TrideminGemini : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       turretMax;
	double       turretMin;
	double       turretAngle;
	double       angularSpeed;
	double       TSpeedMax;
	double       TSpeedMin;

	double calc_vf(double m1, double v1, double m2, double v2) {
		if ((m1+m2)<.05) return 0.;
		else
			return((m1*v1+m2*v2)/(m1+m2));
	}

	double Lim360(double a) {
		if (a>=PI2)  return(a-PI2);
		else if (a<0.)   return(PI2+a);
		else        return a;
	}

	SpaceObject *turret;
	public:
		TrideminGemini(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:

		double anp1(double an, double max, int degree) {
			return ((an*degree)+max)/(degree+1.);
		}

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate();
};

void TrideminGemini::calculate_turn_left()
{
	STACKTRACE;
	return;
}


void TrideminGemini::calculate_turn_right()
{
	STACKTRACE;
	return;
}


TrideminGemini::TrideminGemini(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	turretMax      = get_config_float("Special","TurretMax",0);
	turretMin      = get_config_float("Special","TurretMin",0);
	TSpeedMin      = get_config_float("Special","TSpeedMin",0) * ANGLE_RATIO;
	TSpeedMax      = get_config_float("Special","TSpeedMax",0) * ANGLE_RATIO;

	angularSpeed=.5;
	turretAngle=0;				 //(turretMax - turretMin) / 2;
	turret=new TrideminTurret(this,Vector2(0,0),turretAngle,data->spriteSpecial);
	add(turret);
}


int TrideminGemini::activate_weapon()
{
	STACKTRACE;
	double turAngle = normalize(angle+turretAngle,PI2);

	add(new Missile(this, Vector2(ARM_LENGTH/32.0,0.0),
		turAngle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		turret, data->spriteWeapon,0.));

	angularSpeed=TSpeedMin;

	return(TRUE);
}


int TrideminGemini::activate_special()
{
	STACKTRACE;
	if (turn_left&&(turretAngle>turretMin)) {
		turretAngle-=angularSpeed;
		angularSpeed=anp1(angularSpeed,TSpeedMax,TDegree);
	}
	else if (turn_right&&(turretAngle<turretMax)) {
		turretAngle+=angularSpeed;
		angularSpeed=anp1(angularSpeed,TSpeedMax,TDegree);
	}

	else angularSpeed=TSpeedMin;

	return FALSE;
}


void TrideminGemini::calculate()
{
	STACKTRACE;

	double Dist=ARM_LENGTH;
	Vector2 vf;

	if (!fire_special) {
								 //Calculate turn_step BEFORE
		Ship::calculate_turn_left();
								 //Ship::calculate() is called.
		Ship::calculate_turn_right();
	}

	turret->angle=Lim360(angle+turretAngle+turn_step*2);

	vf.x=calc_vf(mass,vel.x,turret->mass,turret->vel.x);
	vf.y=calc_vf(mass,vel.y,turret->mass,turret->vel.y);

	//	turret->x=x+Dist*sin(angle+turn_step);
	//	turret->y=y-Dist*cos(angle+turn_step);
	turret->pos = pos - Dist * unit_vector(angle+turn_step+0.5*PI);

	//	turret->vx=vx=vfx;
	//	turret->vy=vy=vfy;
	turret->vel = vel = vf;

	turn_left=FALSE;			 //Ensure that ship_calculate_turn_xxxx()
	turn_right=FALSE;			 //isn't done twice.

	Ship::calculate();
}


REGISTER_SHIP(TrideminGemini)
