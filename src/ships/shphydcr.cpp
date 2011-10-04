/*
This file is part of "TW-Light"
					http://tw-light.appspot.com/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "ship.h"

REGISTER_FILE

#include "../melee/mcbodies.h"

class HydrovarEsFighter;
class HydrovarCruiser;

class HydrovarBeam : public Laser
{
	protected:
		double startingLength;
		double float_damage_factor, current_damage_factor;
		double range_attenuation_factor, time_attenuation_factor;
		int starting_r, starting_g, starting_b;

	public:
		HydrovarBeam(SpaceLocation *creator, double langle, int lcolor, double lrange,
			double ldamage, int lfcount, double rel_x, double rel_y, double ra, double ta);

		virtual void inflict_damage(SpaceObject *other);
		virtual void calculate();
};

class HydrovarCruiser : public Ship
{
	public:
		int     weaponColor;
		int     weaponFrames0, weaponFrames1, weaponFrames2;
		double  weaponDamage0, weaponDamage1, weaponDamage2;
		double  weaponRange0, weaponRange1, weaponRange2;
		double  weaponAngle1, weaponAngle2;
		double  weaponRangeAttenuation, weaponTimeAttenuation;

		int     specialFrames;
		int     specialLaserDamage;
		int     specialLaserColor;
		double  specialLaserRange;
		int     specialLaserFrames;
		int     specialLaserDrain;
		double  specialVelocity;
		//	double	specialRange;
		int     specialArmour;
		double  specialTurningRate;

		double  specialTrackingRange;
		double  specialTrackingArc;
		double  specialFiringArcDefense;
		double  specialFiringArcOffense;
		int     specialOrphanFrames;
		double  specialNumberOfShots;
		int     specialCrewCost;
		int     specialDamageOnExplosion;
		int     specialSearchPeriod;
		int   specialTargetsAsteroids;
		int   specialTargetsShots;
		int   specialTargetsShips;
		int   specialTargetsSpecial;
		int   specialBouncesOffAsteroids;
		int   specialBouncesOffShips;

								 //how much it will last, and how  soon it will need refueling
		int     specialFuelFull, specialFuelCritical;

		bool    specialFiringTakesFuel;

		HydrovarEsFighter* esFighter[20];

	public:
		HydrovarCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code);
		int laserCounter;
		int laserNumber;
		virtual int activate_weapon(void);
		virtual int activate_special(void);
		//	virtual void calculate(void);

		double calculateLaserDamage(HydrovarBeam* HB);

		int fightersOut;
		int maxFightersOut;
		//int adoptCurrentFighters(void);
		//int orphanCurrentFighters(void);
};

class HydrovarEsFighter : public HomingMissile
{
	int     retreat_frames;
	int     hasEnemy;
	int     locationNumber;
	double  dist;

	public:
		int     laser_damage;
		int   targetsAsteroids;
		int   targetsShips;
		int   targetsShots;
		int   targetsSpecial;

		int   bouncesOffShips;
		int   bouncesOffAsteroids;

		int     laser_color;
		double  laser_range;
		int     laser_frames;

		int     damage_on_explosion;

		int     fuel, fuel_critical, shot_fuel_cost;

		double  trackingRange;
		double  trackingArc;
		double  firingArcOffense;
		double  firingArcDefense;
		int     numberOfShots;
		//	int		orphanTime; //(how long it will last) - excessive, unused
		int     orphanFrames;

		int     search_frames, search_period;

		//	double	orphanRange; //the countdown

		HydrovarCruiser*  creator;
		HydrovarEsFighter** pointerToMe;

		HydrovarEsFighter(double ox, double oy, double oangle, double ov,
			int odamage, double orange, int oarmour, double otrate, Ship *oship,
			SpaceSprite *osprite, SpaceObject* oTarget);

		~HydrovarEsFighter(void);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual int isShot(void);
		void getTarget(SpaceObject* other);
		void searchForTarget(void);
		//	void returnToFormation(void); // unused (big plans, huh? ;)
		void tryToFire(void);
		void UTurn(int m, int r);//only used ONCE - no need for a separate  function
		void RetreatFrom(SpaceLocation* L, int r, int m);

};

HydrovarCruiser::HydrovarCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code) :
Ship(opos, angle, data, code)
{
	STACKTRACE;
	int i=0;
	laserCounter=0;
	fightersOut=0;
	weaponColor  = tw_get_config_int("Weapon", "Color", 0);

	weaponFrames0   = tw_get_config_int("Weapon", "Frames0", 1);
	weaponFrames1   = tw_get_config_int("Weapon", "Frames1", 1);
	weaponFrames2   = tw_get_config_int("Weapon", "Frames2", 1);

	weaponDamage0   = tw_get_config_float("Weapon", "Damage0", 0) / weaponFrames0;
	weaponDamage1   = tw_get_config_float("Weapon", "Damage1", 0) / weaponFrames1;
	weaponDamage2   = tw_get_config_float("Weapon", "Damage2", 0) / weaponFrames2;

	weaponRange0    = scale_range(tw_get_config_float("Weapon", "Range0", 0));
	weaponRange1    = scale_range(tw_get_config_float("Weapon", "Range1", 0));
	weaponRange2    = scale_range(tw_get_config_float("Weapon", "Range2", 0));

	weaponAngle1    = tw_get_config_float("Weapon", "Angle1", 30) * ANGLE_RATIO;
	weaponAngle2    = tw_get_config_float("Weapon", "Angle2", 60) * ANGLE_RATIO;

	weaponRangeAttenuation = tw_get_config_float("Weapon", "RangeAttenuation", 0);
	weaponTimeAttenuation  = tw_get_config_float("Weapon", "TimeAttenuation", 0);

	specialFrames           = tw_get_config_int("Special", "Frames", 0);
	specialLaserDamage      = tw_get_config_int("Special", "LaserDamage", 0);
	specialLaserColor       = tw_get_config_int("Special", "LaserColor", 0);
	specialLaserRange       = scale_range(tw_get_config_float("Special", "LaserRange", 0));
	specialLaserFrames      = tw_get_config_int("Special", "LaserFrames", 0);
	specialVelocity         = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	//	specialRange			= scale_range(tw_get_config_float("Special", "Range", 0));
	specialArmour           = tw_get_config_int("Special", "Armour", 0);
	specialTrackingRange    = scale_range(tw_get_config_float("Special", "TrackingRange",0));
	specialTurningRate      = scale_turning(tw_get_config_float("Special", "TurningRate", 0));
	specialTargetsAsteroids = tw_get_config_int("Special", "TargetsAsteroids", 0);
	specialTargetsShots = tw_get_config_int("Special", "TargetsShots", 0);
	specialTargetsShips = tw_get_config_int("Special", "TargetsShips", 0);
	specialTargetsSpecial = tw_get_config_int("Special", "TargetsSpecial", 0);
	specialBouncesOffAsteroids = tw_get_config_int("Special", "BouncesOffAsteroids", 0);
	specialBouncesOffShips = tw_get_config_int("Special", "BouncesOffShips", 0);
	maxFightersOut          = tw_get_config_int("Special", "MaxFightersOut", 0);
	if (maxFightersOut > 20)
		maxFightersOut = 20;

	for(i=0;i<maxFightersOut;i++)
		esFighter[i]=NULL;
	specialDamageOnExplosion= tw_get_config_int("Special", "ExplosionDamage", 0);
	specialFuelFull         = tw_get_config_int("Special", "FuelFull", 0);
	specialFuelCritical     = tw_get_config_int("Special", "FuelCritical", 0);

	specialFiringTakesFuel  = (tw_get_config_int("Special", "FiringTakesFuel", 0) != 0);
	specialSearchPeriod     = tw_get_config_int("Special", "SearchPeriod", 0);

	specialTrackingArc      = tw_get_config_float("Special", "TrackingArc", 0) * ANGLE_RATIO;
	specialFiringArcOffense = tw_get_config_float("Special", "FiringArcOffense", 0) * ANGLE_RATIO;
	specialFiringArcDefense = tw_get_config_float("Special", "FiringArcDefense", 0) * ANGLE_RATIO;
	specialOrphanFrames     = tw_get_config_int("Special", "OrphanFrames", 0);
	specialNumberOfShots    = tw_get_config_int("Special", "NumberOfShots", 0);
	specialCrewCost         = tw_get_config_int("Special", "CrewCost", 0);
}


int HydrovarCruiser::activate_weapon()
{
	STACKTRACE;
	game->add(new HydrovarBeam(this, angle, tw_get_palete_color(weaponColor),
		weaponRange0, weaponDamage0, weaponFrames0, 0, 36,
		weaponRangeAttenuation, weaponTimeAttenuation));

	game->add(new HydrovarBeam(this, angle-weaponAngle1, tw_get_palete_color(weaponColor),
		weaponRange1, weaponDamage1, weaponFrames1, 3, 36,
		weaponRangeAttenuation, weaponTimeAttenuation));
	game->add(new HydrovarBeam(this, angle+weaponAngle1, tw_get_palete_color(weaponColor),
		weaponRange1, weaponDamage1, weaponFrames1, -3, 36,
		weaponRangeAttenuation, weaponTimeAttenuation));

	game->add(new HydrovarBeam(this, angle-weaponAngle2, tw_get_palete_color(weaponColor),
		weaponRange2, weaponDamage2, weaponFrames2, 6, 38,
		weaponRangeAttenuation, weaponTimeAttenuation));
	game->add(new HydrovarBeam(this, angle+weaponAngle2, tw_get_palete_color(weaponColor),
		weaponRange2, weaponDamage2, weaponFrames2, -6, 38,
		weaponRangeAttenuation, weaponTimeAttenuation));

	return true;
}


int HydrovarCruiser::activate_special()
{
	STACKTRACE;
	if ((fightersOut>=maxFightersOut) || (crew<=specialCrewCost))
		return(FALSE);

	int i;
	HydrovarEsFighter* F;
	crew -= specialCrewCost;
	for(i=0;i<maxFightersOut;i++)
		if (esFighter[i] == NULL) break;

	F = new HydrovarEsFighter(0, 0,
		angle, specialVelocity, 1, 1e30, specialArmour,
		specialTurningRate, this, data->spriteSpecial, this);
	F->targetsAsteroids = this->specialTargetsAsteroids;
	F->targetsShips = this->specialTargetsShips;
	F->targetsShots = this->specialTargetsShots;
	F->targetsSpecial = this->specialTargetsSpecial;
	F->bouncesOffAsteroids = this->specialBouncesOffAsteroids;
	F->bouncesOffShips = this->specialBouncesOffShips;

	esFighter[i]=F;
	game->add(F);
	fightersOut++;

	//	specialDamageOnExplosion= tw_get_config_int("Special", "ExplosionDamage",  0);
	//	specialFuelFull			= tw_get_config_int("Special", "FuelFull", 0);
	//	specialFuelCritical		= tw_get_config_int("Special", "FuelCritical", 0);
	//
	//	specialFiringTakeFuel	= (tw_get_config_int("Special", "FiringTakesFuel", 0)  != 0);

	F->pointerToMe      =&esFighter[i];
	F->trackingRange    = specialTrackingRange;
	F->laser_color      = specialLaserColor;
	F->laser_damage     = specialLaserDamage;
	F->laser_frames     = specialLaserFrames;
	F->laser_range      = specialLaserRange;
	F->trackingArc      = specialTrackingArc;
	F->firingArcDefense = specialFiringArcDefense;
	F->firingArcOffense = specialFiringArcOffense;
	//    F->orphanRange		= 9999999; // unused anyway
								 // initially - inactive
	F->orphanFrames     = -specialOrphanFrames;
	//    F->range			= 9999999;
	F->numberOfShots    = iround(specialNumberOfShots);
	if (specialFiringTakesFuel && specialFuelFull)
		F->shot_fuel_cost   = iround((specialFuelFull - specialFuelCritical) / specialNumberOfShots);
	else
		F->shot_fuel_cost   = 0;
	F->fuel                 = specialFuelFull;
	F->fuel_critical        = specialFuelCritical;
	F->search_period        = specialSearchPeriod;
	F->damage_on_explosion  = specialDamageOnExplosion;

	return true;
}


/*void HydrovarCruiser::calculate(void)
{
	STACKTRACE;
  STACKTRACE;
  Ship::calculate();

}
*/

//HydrovarCruiser::~HydrovarCruiser(void); no longer needed.

HydrovarEsFighter::HydrovarEsFighter (double ox, double oy, double oangle, double ov,
int odamage, double orange, int oarmour, double otrate, Ship *oship,
SpaceSprite *osprite, SpaceObject* oTarget) :
HomingMissile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otrate, oship, osprite, oship)
{
	explosionSprite = data->spriteSpecialExplosion;
	explosionSample = data->sampleExtra[2];
	search_frames = 0;
	//	mass = 1; //n-eh!
	locationNumber=0;
	retreat_frames=0;
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);
	//	if (oship==NULL)tw_error("oship==NULL"); //debugging code
	//	if (oTarget==NULL)tw_error("otarget==NULL"); //debugging code
	target = oTarget;
	creator = (HydrovarCruiser*) oship;
	//	if (creator==NULL)tw_error("creator==NULL"); //debugging code

}


void HydrovarEsFighter::calculate()
{
	STACKTRACE;
	if (!(creator && creator->exists())) {
		creator = 0;
		orphanFrames = -orphanFrames;
	}

	if (creator) {
		if (creator->distance(this)<scale_range(0.5) && numberOfShots<1) {
								 //Honey, I'm home!
			damage(creator, 0, -creator->specialCrewCost);
			play_sound(data->sampleExtra[1]);
			state = 0; return;
		}
	}

	if (orphanFrames > 0) {		 //time to die when orphaned - if enabled
								 //all it milliseconds now, to be indipendent  on tick time
		orphanFrames -= frame_time;
		if (orphanFrames <= 0) {
			state=0; return;
		}
	}

	if (fuel > 0) {				 //if fuel is used (optional)...
		fuel -= frame_time;
		if (fuel <= 0) {		 //die if no fuel left
			state=0; return;
		}
		else
								 //run back to  the mothership if low on fuel
		if ((creator) && (fuel < fuel_critical + shot_fuel_cost))
			numberOfShots = 0;
	}

	if (retreat_frames>0) {		 //running away, huh?
		retreat_frames -= frame_time;
		Missile::calculate();
		return;
	}

	Planet *p = nearest_planet();
	if (p) {
		if (distance(p) < p->size.x * 2.2) {
			//FIXME: need 'planet_radius' above - WHEN IMPLEMENTED
			changeDirection(trajectory_angle(p)+PI);
								 //CHECK TIMES (in ms)
			retreat_frames = random()%200+75;
			Missile::calculate();
			return;
		}
	}

	//if (creator)	// activation of "orphan dies" mechanism, unless disabled  (orphanFrames==0)
	//	if (!creator->exists()) {
	//		creator=NULL;
	//		orphanFrames = -orphanFrames; }

	range = 1e30;				 // never ever die
	HomingMissile::calculate();

	if (search_frames <= 0) {
		searchForTarget();
		if (search_period > 0)
			while (search_frames <= 0)
				search_frames += search_period;
	}
	else
	if (search_period > 0)
		search_frames -= frame_time;

	if (target==NULL)
		hasEnemy=0;
	else
	if (target->state==0)
		hasEnemy=0;
	else
	if (target==creator)
		hasEnemy=0;

	tryToFire();
}


int HydrovarEsFighter::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (this->bouncesOffShips && source->isShip() && source->damage_factor < 1.0 && source!=this->creator) {
		this->state = 1;
		if (this->armour < 1) this->armour = 1;
		return (0);
	}
	if (this->bouncesOffAsteroids && source->isAsteroid()) {
		this->RetreatFrom(source, 250, 100);
		this->state = 1;
		if (this->armour < 1) this->armour = 1;
		return (0);
	}

	HomingMissile::handle_damage(source, normal, direct);

								 //if enabled, ....
	if ((state == 0) && (damage_on_explosion)) {
								 //inflict damage to what have  killed you
		damage(source, damage_on_explosion, 0);
		damage_on_explosion = 0; // but do not do it twice
	}
	return (1);
}


void HydrovarEsFighter::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	int df = iround(damage_factor);
	damage_factor = 0;
	if (this->bouncesOffAsteroids && other->isAsteroid()) {
		this->RetreatFrom(other, 250, 100);
		return;
	}
	if (this->bouncesOffShips && other->isShip() && other->damage_factor < 1) {
		this->RetreatFrom(other, 250, 100);
		return;
	}
	Shot::inflict_damage(other); // it will take care of "solid object"  collisions
	if ((state == 0) && (damage_on_explosion)) {
		damage(other, damage_on_explosion, 0);
		damage_on_explosion = 0;
	}
	damage_factor = df;
	return;
}


int HydrovarEsFighter::isShot(void)
{
	STACKTRACE;
	return false;				 //that makes more sense, and also takes care of many shot  collision issues;
}


void HydrovarEsFighter::getTarget(SpaceObject *other)
{
	STACKTRACE;
	double a1, a2;
	if (other==NULL)
		{tw_error("Error: other==NULL -- not good.");return;}
		if (other->isPlanet())
			return;
	a1 = angle; a2 = trajectory_angle(other);
	if (fabs(a1-a2)>trackingArc/2&&fabs(a1-a2)<(PI2-trackingArc/2)) return;
	if (this->distance(other)<dist) {
		dist = this->distance(other);
		target = other;
		hasEnemy = 1;
	}
}


void HydrovarEsFighter::tryToFire()
{
	STACKTRACE;
	if (fuel > 0)
		if (fuel < fuel_critical + shot_fuel_cost)
			return;				 //do not shot if there's not enough fuel

	double angleDif;
	if (!hasEnemy)
		return;
	if (target==NULL)
		return;
	if (target->state==0)
		return;
	angleDif=fabs(this->trajectory_angle((SpaceObject*)target)-angle);
	if (angleDif>PI) angleDif=PI2-angleDif;
	if (this->distance(target)>laser_range * 0.975)
		return;
	if (angleDif>firingArcDefense/2&&target->isShot())
		return;
	if (angleDif>firingArcOffense/2&&!target->isShot())
		return;

	play_sound2(data->sampleExtra[0]);
	game->add(new Laser(this, angle, tw_get_palete_color(laser_color), laser_range, laser_damage, laser_frames, this, Vector2(0.0, 0.0)));

	UTurn(250, 100);			 // TIMES MIGHT NEED ADJUSTING!

	numberOfShots--;
	if (fuel > 0)
		fuel -= shot_fuel_cost;

	return;
}


void HydrovarEsFighter::UTurn(int m, int r)
{
	STACKTRACE;
	angle = angle + PI;
	vel *= -1;
	retreat_frames = (random() % r) + m;
}


void HydrovarEsFighter::RetreatFrom(SpaceLocation* L, int r, int m)
{
	STACKTRACE;
	if (L==NULL || (!L->exists())) return;
	this->changeDirection(L->trajectory_angle(this));
	retreat_frames = (random() % r) + m;
}


void HydrovarEsFighter::searchForTarget(void)
{
	STACKTRACE;
	long int searchLayers = 0;
	SpaceObject *o;
	if (creator)
		target=creator;			 //default is return to base if nothing found.
	if (numberOfShots<1) return; //don't hunt for a target if out of ammo.
	dist=1e30;
	Query a;
	if (this->targetsAsteroids) searchLayers += bit(LAYER_CBODIES);
	if (this->targetsShips) searchLayers += bit(LAYER_SHIPS);
	if (this->targetsShots) searchLayers += bit(LAYER_SHOTS);
	if (this->targetsSpecial) searchLayers += bit(LAYER_SPECIAL);
	for (a.begin(this, searchLayers, trackingRange); a.current; a.next()) {
		//		if (a.currento==NULL)
		//			tw_error("Null object during search loop.");
		o = a.currento;
		if ( (!o->isInvisible()) && !o->sameTeam(this) && (o->collide_flag_anyone & bit(LAYER_LINES)))
			getTarget(o);
	}

	return;
}


HydrovarEsFighter::~HydrovarEsFighter()
{
	if (creator)
	if (creator->exists()) {
		*pointerToMe=NULL;
		creator->fightersOut--;
	}
}


HydrovarBeam::HydrovarBeam (SpaceLocation *creator, double langle, int lcolor,
double lrange, double ldamage, int lfcount, double rel_x, double rel_y,
double ra, double ta) :
Laser(creator, langle, lcolor, lrange, 1, lfcount, creator, Vector2(rel_x,rel_y), true),
range_attenuation_factor(ra), time_attenuation_factor(ta)
{
	startingLength = lrange;
	current_damage_factor = float_damage_factor = ldamage;

	starting_r = tw_getr(color);
	starting_g = tw_getg(color);
	starting_b = tw_getb(color);
}


void HydrovarBeam::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	double rr;
	double rdamage;
	rr = exp(range_attenuation_factor * log(1.01 - length/startingLength));
	if (rr > 1) rr = 1;

	//other->residual_damage += current_damage_factor * rr * frame_time; //time  attenuation included
								 //time attenuation  included
	rdamage = current_damage_factor * rr * frame_time;
	damage(other, rdamage);
	//int i;

	/*while (other->residual_damage >= 1) {
		damage(other, 1);
		other->residual_damage -= 1;

		i = damage_factor / 2; //yeah, I've put the sound back in - it's too sad
	without it
		if (i >= BOOM_SAMPLES)
			i = BOOM_SAMPLES - 1;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
		physics->add(new Animation( this,
			normal_x() + edge_x(), normal_y() + edge_y(), game->sparkSprite, 0,
			SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
	}*/
}


void HydrovarBeam::calculate()
{
	STACKTRACE;
	length = startingLength;

	double tt;
	tt = exp(time_attenuation_factor * log(1.01 - frame/frame_count));
	if (tt < 1) {
		color = tw_makecol(int(starting_r * tt), int(starting_g * tt), int(starting_b * tt));
		current_damage_factor = float_damage_factor * tt;
	}

	Laser::calculate();
}


REGISTER_SHIP(HydrovarCruiser)
