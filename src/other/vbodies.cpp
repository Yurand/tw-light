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

#include <allegro.h>
#include <string.h>
#include <stdio.h>

#include "melee.h"
REGISTER_FILE
#include "id.h"
#include "scp.h"
#include "frame.h"

#include "vbodies.h"
#include "util/vector2.h"

#include "melee/mgame.h"
#include "melee/mview.h"
#include "melee/mcbodies.h"
#include "melee/manim.h"
#include "melee/mship.h"
#include "melee/mgame.h"
#include "other/orbit.h"
#include "other/vtarget.h"

//this file belongs in the \other directory.

#define V_ACCEL_KICK 3200

void VNebulaColorEffects (Color *c)
{
	c->r = ((c->r * 2) / 3) + 256/3;
	c->b = (unsigned char)(c->b * 0.75 + 60);
	//gamma_color_effects (c);
	return;
}


SpaceSprite *VRedFlare::mySprite = NULL;
SpaceSprite *VOrangeFlare::mySprite = NULL;
SpaceSprite *VYellowFlare::mySprite = NULL;
SpaceSprite *VWhiteFlare::mySprite = NULL;

SpaceSprite *VMetalShard::mySprite = NULL;
SpaceSprite *VMetalAsteroid::mySprite = NULL;
SpaceSprite *VSmallAsteroid::mySprite = NULL;
SpaceSprite *VLargeAsteroid::mySprite = NULL;
SpaceSprite *VMoon::mySprite = NULL;
SpaceSprite *VRockballPlanet::mySprite = NULL;
SpaceSprite *VInhospitablePlanet::mySprite = NULL;
SpaceSprite *VHabitablePlanet::mySprite = NULL;
SpaceSprite *VGasGiant::mySprite = NULL;
SpaceSprite *VBrownDwarf::mySprite = NULL;
SpaceSprite *VRedDwarf::mySprite = NULL;
SpaceSprite *VWhiteDwarf::mySprite = NULL;
SpaceSprite *VRedStar::mySprite = NULL;
SpaceSprite *VOrangeStar::mySprite = NULL;
SpaceSprite *VYellowStar::mySprite = NULL;
SpaceSprite *VWhiteStar::mySprite = NULL;
SpaceSprite *VRedGiant::mySprite = NULL;
SpaceSprite *VBlueGiant::mySprite = NULL;
SpaceSprite *VNeutronStar::mySprite = NULL;
SpaceSprite *VHypermass::mySprite = NULL;
SpaceSprite *VDustCloud::mySprite = NULL;
SpaceSprite *VGasCloud::mySprite1 = NULL;
SpaceSprite *VGasCloud::mySprite2 = NULL;
SpaceSprite *VSpaceMine::mySprite = NULL;

SpaceSprite *VDefSat::mySprite = NULL;
SpaceSprite *VDeepSpaceOutpost::mySprite = NULL;
SpaceSprite *VDeepSpaceStation::mySprite = NULL;
SpaceSprite *VDeepSpaceColony::mySprite = NULL;
SpaceSprite *VSpaceMine::explosionSprite = NULL;

double VMetalShard::myArmour = 0.0;
double VMetalShard::myMass = 0.0;
double VMetalShard::myDamage = 0.0;
int VMetalShard::noBounce = 0;

double VSmallAsteroid::myArmour = 0.0;
double VSmallAsteroid::myMass = 0.0;
double VSmallAsteroid::myDamage = 0.0;
int VSmallAsteroid::noBounce = 0;

double VLargeAsteroid::myArmour = 0.0;
double VLargeAsteroid::myMass = 0.0;
double VLargeAsteroid::myDamage = 0.0;
int VLargeAsteroid::noBounce = 0;
int VLargeAsteroid::numberOfChildren = 3;
double VLargeAsteroid::childVelocity = 0;

double VMetalAsteroid::myArmour = 0.0;
double VMetalAsteroid::myMass = 0.0;
double VMetalAsteroid::myDamage = 0.0;
int VMetalAsteroid::noBounce = 0;
int VMetalAsteroid::numberOfChildren1 = 3;
int VMetalAsteroid::numberOfChildren2 = 5;
double VMetalAsteroid::childVelocity1 = 5;
double VMetalAsteroid::childVelocity2 = 15;

//adding generic sprite handling routines here

SpaceSprite* _VGetSprite(char *fileName, char *spriteName, int attribs, int rotations)
{

	TW_DATAFILE *tmpdata;
	tmpdata= tw_load_datafile_object(fileName,spriteName);
	if (tmpdata==NULL) {
		#ifdef STATION_LOG
		sprintf(msgStr,"Unable to load %s#%s",fileName,spriteName);
		message.out(msgStr);
		#endif

		return NULL;
	}

	SpaceSprite *spr=new SpaceSprite(tmpdata, 1, attribs, rotations);
	tw_unload_datafile_object(tmpdata);

	return spr;
};

bool _VGetSpriteTo64Rot(SpaceSprite *Pics[], char *fileName, char *cmdStr,
int numSprites, int attribs)
{
	STACKTRACE;

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=0; num<numSprites; num++) {
		sprintf(dataStr,cmdStr,num);
		spr=_VGetSprite(fileName, dataStr, attribs, 64);
		if (!spr) {
			return FALSE;
		}
		Pics[num]=spr;
	}
	return TRUE;
};

SpaceSprite* _VGetMultiframeSprite(char *fileName, char *spriteName, int attribs, int numberOfFrames)
{

	TW_DATAFILE *tmpdata;
	tmpdata= tw_load_datafile_object(fileName,spriteName);
	if (tmpdata==NULL) {
		#ifdef STATION_LOG
		sprintf(msgStr,"Unable to load %s#%s",fileName,spriteName);
		message.out(msgStr);
		#endif

		return NULL;
	}

	SpaceSprite *spr=new SpaceSprite(tmpdata, numberOfFrames, attribs, 1);
	tw_unload_datafile_object(tmpdata);

	return spr;
};

bool _VGetSpriteGroup(SpaceSprite *Pics[], char *fileName, char *cmdStr,
int numSprites, int attribs, int firstSpriteNumber)
{
	STACKTRACE;

	SpaceSprite *spr;
	char dataStr[100];

	for(int num=firstSpriteNumber; num<(numSprites+firstSpriteNumber); num++) {
		sprintf(dataStr,cmdStr,num);
		//message.print(500,5,dataStr);
		spr=_VGetSprite(fileName, dataStr, attribs, 1);
		if (!spr) {
			return FALSE;
		}
		Pics[num]=spr;
	}
	return TRUE;
};

//end sprite handling routines

VSpaceInstallation::VSpaceInstallation():SpaceObject(NULL, random(map_size), random(PI2), meleedata.asteroidSprite)
{
	STACKTRACE;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_CBODIES;
	set_depth(DEPTH_ASTEROIDS);
}


void VSpaceInstallation::Initialize(void)
{
	STACKTRACE;
}


void VSpaceInstallation::Initialize(const char* nameInIni)
{
	STACKTRACE;
	game->log_file ("vobject.ini");
	this->armour = get_config_float(nameInIni, "Armour", 0);
	this->mass = get_config_float(nameInIni, "Mass", 0);
	this->friction = get_config_float(nameInIni, "Friction", 0);
}


bool VSpaceInstallation::AddInstallation(VGroundInstallation* oGI)
{
	STACKTRACE;
	int i;
	for(i=0;i<12;i++) {
		if (this->Installation[i]==NULL) {
			this->Installation[i] = oGI;
			this->Installation[i]->location = this;
			game->add(this->Installation[i]);
			return(true);
		}
	}
	return(false);
}


void VSpaceInstallation::calculate(void)
{
	STACKTRACE;
	this->vel *= (1 - this->friction * game->frame_time);
}


void VSpaceInstallation::death()
{
	STACKTRACE;
	int i;
	for(i=0;i<12;i++)
		if (this->Installation[i]!=NULL)
			this->Installation[i]->state = 0;
}


int VSpaceInstallation::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	message.print(500,5,"Installation armour=%d", (long int)armour);
	if (source->isAsteroid()) return(0);
	armour -= (normal + direct);
	if (armour<0) state=0;
	return(1);
}


VDefSat::VDefSat()
{
	STACKTRACE;
	this->set_sprite(VDefSat::mySprite);
	this->Initialize();
}


void VDefSat::Initialize(void)
{
	STACKTRACE;
	VSpaceInstallation::Initialize("DefSat");
}


VDeepSpaceOutpost::VDeepSpaceOutpost()
{
	STACKTRACE;
	this->set_sprite(VDeepSpaceOutpost::mySprite);
	this->Initialize();
}


void VDeepSpaceOutpost::Initialize()
{
	STACKTRACE;
	VSpaceInstallation::Initialize("DeepSpaceOutpost");

}


VDeepSpaceColony::VDeepSpaceColony()
{
	STACKTRACE;
	this->set_sprite(VDeepSpaceColony::mySprite);
	this->Initialize();
}


void VDeepSpaceColony::Initialize()
{
	STACKTRACE;
	VSpaceInstallation::Initialize("DeepSpaceColony");
}


VDeepSpaceStation::VDeepSpaceStation()
{
	STACKTRACE;
	this->set_sprite(VDeepSpaceStation::mySprite);
	this->Initialize();
}


void VDeepSpaceStation::Initialize()
{
	STACKTRACE;
	VSpaceInstallation::Initialize("DeepSpaceStation");
}


VGroundInstallation::VGroundInstallation(SpaceLocation* olocation):Presence()
{
	STACKTRACE;
	this->totalDamageTaken = 0;
	this->location = olocation;
	actionCounter = 0;
	actionsPerSecond = 0;
	this->isFunctioning = true;
	this->isDestroyed = false;
	this->isShield = false;
	this->isWeapon = false;
}


void VGroundInstallation::Initialize(void)
{
	STACKTRACE;
	;
}


void VGroundInstallation::Initialize(const char* nameInIni)
{
	STACKTRACE;
	game->log_file ("vobject.ini");
	this->damageToDisable = get_config_float(nameInIni, "DamageToDisable", 0);
	this->damageToDestroy = get_config_float(nameInIni, "DamageToDestroy", 0);
	this->actionsPerSecond = get_config_float(nameInIni, "ActionsPerSecond", 0);
}


int VGroundInstallation::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (source->isPlanet()||source->isAsteroid()) return(0);
	this->totalDamageTaken += (normal + direct);
	message.print(350,5,"DamageTaken=%d", (long int)this->totalDamageTaken);
	if (totalDamageTaken>=damageToDisable) {
		this->isFunctioning = false;
		if (totalDamageTaken-(normal+direct)<damageToDisable)
			message.print(1000,10,"Installation disabled!");
	}
	if (totalDamageTaken>=damageToDestroy) {
		this->isDestroyed = true;
		if (totalDamageTaken-(normal+direct)<damageToDestroy)
			message.print(1000,12,"Installation destroyed!");
	}
	return(1);
}


void VGroundInstallation::calculate(void)
{
	STACKTRACE;
	if (!this->isFunctioning)return;
	Presence::calculate();
	actionCounter += ((double)frame_time / 1000.0) * this->actionsPerSecond;
	if (actionCounter>=1.00) {
		if (this->BattleAction())
			actionCounter -=1.00;
		else
			actionCounter -=0.37;
	}
}


bool VGroundInstallation::BattleAction(void)
{
	STACKTRACE;
	return(false);
}


VWilderness::VWilderness(SpaceLocation* olocation):VGroundInstallation(olocation)
{
	STACKTRACE;
	this->Initialize();
}


void VWilderness::Initialize(void)
{
	STACKTRACE;
	VGroundInstallation::Initialize("Wilderness");
}


VColony::VColony(SpaceLocation* olocation):VGroundInstallation(olocation)
{
	STACKTRACE;
	this->Initialize();
}


void VColony::Initialize(void)
{
	STACKTRACE;
	VGroundInstallation::Initialize("Colony");
}


VMine::VMine(SpaceLocation* olocation):VGroundInstallation(olocation)
{
	STACKTRACE;
	this->Initialize();
}


void VMine::Initialize(void)
{
	STACKTRACE;
	VGroundInstallation::Initialize("Mine");
}


VFortification::VFortification(SpaceLocation* olocation):VGroundInstallation(olocation)
{
	STACKTRACE;
	this->Initialize();
}


void VFortification::Initialize(void)
{
	STACKTRACE;
	VGroundInstallation::Initialize("Fortification");
}


VPlanetaryShield::VPlanetaryShield(SpaceLocation* olocation):VGroundInstallation(olocation)
{
	STACKTRACE;
	this->Initialize();
	this->isFunctioning = true;
	this->isDestroyed = false;
	this->isShield = true;
	this->isWeapon = false;
}


void VPlanetaryShield::Initialize(void)
{
	STACKTRACE;
	VGroundInstallation::Initialize("PlanetaryShield");
	this->maxShield = get_config_float("PlanetaryShield", "Shield", 0);
	this->currentShield = maxShield;
	this->shieldRegenerationPerSecond = get_config_float("PlanetaryShield", "ShieldRegenPerSecond", 0);
}


int VPlanetaryShield::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	if (this->currentShield>0) {
		this->currentShield -= (normal + direct);
		message.print(500,5,"Shield level=%d", (long int)this->currentShield);
		return(1);
	}
	else
		return VGroundInstallation::handle_damage(source, normal, direct);
}


void VPlanetaryShield::calculate(void)
{
	STACKTRACE;
	this->currentShield += ((double)frame_time / 1000) * this->shieldRegenerationPerSecond;
	if (this->currentShield>this->maxShield) this->currentShield=this->maxShield;
	if (this->currentShield<0) this->isShield = false;
	else this->isShield = true;
}


VGroundDefenseLaser::VGroundDefenseLaser(SpaceLocation* olocation):VGroundInstallation(olocation)
{
	STACKTRACE;
	this->Initialize();
	this->isFunctioning = true;
	this->isDestroyed = false;
	this->isShield = false;
	this->isWeapon = true;
}


void VGroundDefenseLaser::Initialize()
{
	STACKTRACE;
	VGroundInstallation::Initialize("PlanetaryDefenseLaser");
	weaponRange = scale_range(get_config_float("PlanetaryDefenseLaser", "LaserRange", 0));
	weaponColor = get_config_int("PlanetaryDefenseLaser", "LaserColor", 0);
	weaponDamage = get_config_int("PlanetaryDefenseLaser", "LaserDamage", 0);
	weaponFrames = get_config_int("PlanetaryDefenseLaser", "LaserFrames", 0);
}


bool VGroundDefenseLaser::BattleAction()
{
	STACKTRACE;
	Vector2 RelLoc;
	double ta;
	SpaceObject *o = NULL;

	double r = 99999;
	Query a;
	for (a.begin(this->location, bit(LAYER_SHIPS)|bit(LAYER_SHOTS)|bit(LAYER_SPECIAL), weaponRange + 200); a.current; a.next()) {
		if ((this->location->distance(a.current) < r) && !a.current->isInvisible()) {
			if (a.current) {
				ta = a.current->trajectory_angle(this->location);
				ta = ta - a.current->angle;
				ta = normalize(ta, PI2);
				if (a.currento->isShip()) {
					o = a.currento;
					r = this->location->distance(o);
				}
								 //only stuff pointed at the planet???
				else if ((ta<(0.5*PI))||(ta>(1.5*PI))) {
					o = a.currento;
					r = this->location->distance(o);
				} else {
					;			 //do nothing
				}
			}
		}
	}
	if (!o)return(false);
	if (o) r = this->location->trajectory_angle(o); else r = this->location->angle;
	//RelLoc = unit_vector(r) * scale_range(2); //fudge for now.
	RelLoc.x = 0; RelLoc.y = 0;
	game->add(new Laser(this->location, r, pallete_color[weaponColor],
		weaponRange, weaponDamage, weaponFrames, this->location, RelLoc));
	return(true);
}


VGroundIonCannon::VGroundIonCannon(SpaceLocation* olocation):VGroundInstallation(olocation)
{
	STACKTRACE;
	this->Initialize();
	this->isFunctioning = true;
	this->isDestroyed = false;
	this->isShield = false;
	this->isWeapon = true;
	//this->AimingSystem = new AimSys(this->location, NULL, this->weaponVelocity);
}


void VGroundIonCannon::Initialize()
{
	STACKTRACE;
	VGroundInstallation::Initialize("PlanetaryIonCannon");
	weaponRange = scale_range(get_config_float("PlanetaryIonCannon", "CannonRange", 0));
	weaponVelocity = scale_velocity(get_config_float("PlanetaryIonCannon", "CannonSpeed", 0));
	weaponDamage = get_config_float("PlanetaryIonCannon", "CannonDamage", 0);
	weaponArmour = get_config_float("PlanetaryIonCannon", "CannonArmour", 0);
}


bool VGroundIonCannon::BattleAction(void)
{
	STACKTRACE;
	//double rAnticipated;
	Vector2 RelLoc;

	SpaceObject *o = NULL;

	double r = 99999;
	Query a;
	for (a.begin(this->location, bit(LAYER_SHIPS), weaponRange + 200); a.current; a.next()) {
		if ((this->location->distance(a.current) < r) && !a.current->isInvisible()) {
			o = a.currento;
			r = this->location->distance(o);
		}
	}
	if (!o)return(false);
	if (o) r = this->location->trajectory_angle(o); else r = this->location->angle;
	//RelLoc = unit_vector(r) * scale_range(2); //fudge for now.
	RelLoc.x = 0; RelLoc.y = 0;
	//this->AimingSystem->setNewTarget(o);
	//AimingSystem->CalcTrialValues();
	//rAnticipated = AimingSystem->getBestTrialAlpha();
	//rAnticipated *= (PI / 180); //both in radians
	game->add(new Missile(this->location, RelLoc, r, weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, this->location, VRedFlare::mySprite));
	return(true);
}


VGroundMissileLauncher::VGroundMissileLauncher(SpaceLocation* olocation):VGroundInstallation(olocation)
{
	STACKTRACE;
	this->Initialize();
	this->isFunctioning = true;
	this->isDestroyed = false;
	this->isShield = false;
	this->isWeapon = true;

}


void VGroundMissileLauncher::Initialize()
{
	STACKTRACE;
	VGroundInstallation::Initialize("PlanetaryMissileLauncher");
	weaponRange = scale_range(get_config_float("PlanetaryMissileLauncher", "MissileRange", 0));
	weaponVelocity = scale_velocity(get_config_float("PlanetaryMissileLauncher", "MissileSpeed", 0));
	weaponDamage = get_config_float("PlanetaryMissileLauncher", "MissileDamage", 0);
	weaponArmour = get_config_float("PlanetaryMissileLauncher", "MissileArmour", 0);
	weaponTurnRate = scale_turning(get_config_float("PlanetaryMissileLauncher", "MissileTurnRate", 0));
}


bool VGroundMissileLauncher::BattleAction(void)
{
	STACKTRACE;
	Vector2 RelLoc;

	SpaceObject *o = NULL;

	double r = 99999;
	Query a;
	for (a.begin(this->location, bit(LAYER_SHIPS), weaponRange + 200); a.current; a.next()) {
		if ((this->location->distance(a.current) < r) && !a.current->isInvisible()) {
			o = a.currento;
			r = this->location->distance(o);
		}
	}
	if (!o)return(false);
	if (o) r = this->location->trajectory_angle(o); else r = this->location->angle;
	//RelLoc = unit_vector(r) * scale_range(2); //fudge for now.
	RelLoc.x = 0; RelLoc.y = 0;
	game->add(new HomingMissile(this->location, RelLoc, r, weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, weaponTurnRate,
		this->location, VWhiteFlare::mySprite, o));
	return(true);
}


VSpaceMine::VSpaceMine():SpaceObject(NULL, random(map_size), random(PI2), meleedata.asteroidSprite)
{
	STACKTRACE;
	this->set_sprite(VSpaceMine::mySprite);
	this->explosionSprite = VSpaceMine::explosionSprite;
	Initialize();
	willRespawn = FALSE;
	//this->collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_CBODIES;
	set_depth(DEPTH_SPECIAL);
	this->mass = 0;
}


void VSpaceMine::death(void)
{
	STACKTRACE;
	this->animateExplosion();
	SpaceObject::death();
	if (this->willRespawn) game->add(new VSpaceMine());
}


void VSpaceMine::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	Vector2 V;
	if (!other->isShip()) return;//only damages ships!!!
	this->damage_factor = this->damage;
	SpaceObject::inflict_damage(other);
	V = unit_vector(this->trajectory_angle(other));
	if (other->mass > 0.001) other->accelerate(this, this->kick * V / other->mass, MAX_SPEED);
	state = 0;
}


int VSpaceMine::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (source->isLine()||source->isShot()) {
		armour -= normal + direct;
		if (armour<0) state = 0;
		return(1);
	}
	else
		return(0);
}


void VSpaceMine::Initialize()
{
	STACKTRACE;
	game->log_file ("vobject.ini");
	this->sprite_index = 0;
	this->armour = get_config_float("SpaceMine", "Armour", 0);
	this->damage = get_config_float("SpaceMine", "Damage", 0);
	this->kick = get_config_float("SpaceMine", "Kick", 0);
}


void VSpaceMine::animateExplosion(void)
{
	STACKTRACE;
	//	Animation(SpaceLocation *creator, Vector2 opos, SpaceSprite *osprite,
	//			int first_frame, int num_frames, int frame_size, double depth, double scale = 1.0) ;
	game->add(new Animation(this, normal_pos(),
		this->explosionSprite, 0, 6,
		20, DEPTH_EXPLOSIONS));
	return;
}


void VSpaceMine::soundExplosion(void)
{
	STACKTRACE;
	if (damage_factor > 0) {
		int i = iround_down(damage_factor / 2);
		if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	}
	return;
}


VNebula::VNebula():Presence()
{
	STACKTRACE;
	//  game->all_damage_direct = true;
	//  game->is_nebula = true;
	friction = 0.0004;
	videosystem.color_effects = VNebulaColorEffects;
	videosystem.update_colors();
}


VNebula::VNebula(double ofriction):Presence()
{
	STACKTRACE;
	//  game->all_damage_direct = true;
	friction = ofriction;
	videosystem.color_effects = VNebulaColorEffects;
	videosystem.update_colors();

}


VNebula::VNebula(double ofriction, int oionStorms):Presence()
{
	STACKTRACE;
	//  game->all_damage_direct = true;
	friction = ofriction;
	ionStorms = oionStorms;
	videosystem.color_effects = VNebulaColorEffects;
	videosystem.update_colors();
}


void VNebula::calculate(void)
{
	STACKTRACE;
	Presence::calculate();

	for(std::list<SpaceLocation*>::iterator i=game->item.begin();i!=game->item.end();i++) {
		if ((*i)->exists() && !(*i)->isPlanet()) {
			(*i)->vel *= 1 - this->friction * game->frame_time;
			if ((*i)->isShot()) {
				Shot *s = (Shot*)(*i);
				s->v *= 1 - this->friction * game->frame_time;
				s->range = s->d + (s->range - s->d) * (1 - this->friction * game->frame_time);
			}
		}
	}
}


VNebula::~VNebula()
{
	//  game->all_damage_direct = false;
	//  game->is_nebula = false;
	videosystem.color_effects = gamma_color_effects;
	videosystem.update_colors();
}


VIonStorm::VIonStorm(void):SpaceObject(NULL, Vector2(0,0), 0, NULL)
{
	STACKTRACE;
	//	SpaceObject(SpaceLocation *creator, Vector2 opos, double oangle,
	//			SpaceSprite *osprite);

	;
}


VDustCloud::VDustCloud(void)
:
SpaceObject(NULL, random(map_size), random(PI2), meleedata.asteroidSprite)
{
	STACKTRACE;
	double speed, angle;
	this->velocityRestoreFactor = 0.00;
	this->mass = 0;
	Initialize();
	willRespawn = TRUE;
	//this->collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_CBODIES;
	set_depth(DEPTH_SPECIAL);
	speed = (random(this->maxVelocity));
	angle = random(PI2);
	this->vel = unit_vector(angle) * (speed);
	this->sprite = mySprite;
	this->originalVel = this->vel;
}


void VDustCloud::Initialize(const char* nameInIni)
{
	STACKTRACE;
	game->log_file ("vobject.ini");
	this->sprite_index = 0;
	mass = get_config_int(nameInIni, "Mass", 0);
	armour = get_config_float(nameInIni, "Armour", 0);
	this->maxVelocity = scale_velocity(get_config_float(nameInIni, "MaxVelocity", 0));
	this->minRelSpeedForDamage = scale_velocity(get_config_float(nameInIni, "MinRelSpeedForDamage", 0));
	this->canBeDestroyed = get_config_int(nameInIni, "CanBeDestroyed", 0);
	this->damageFactor = get_config_float(nameInIni, "DamageFactor", 0);
	this->friction = get_config_float(nameInIni, "Friction", 0);
	this->velocityRestoreFactor = get_config_float(nameInIni, "VelocityRestoreFactor", 0);
}


void VDustCloud::Initialize(void)
{
	STACKTRACE;
	Initialize("DustCloud");
}


int VDustCloud::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	if (source->isAsteroid()) return(0);
	if (source->isLine()||source->isShot()) {
		armour -= (normal + direct);
	}
	if (this->canBeDestroyed && armour < 0)
		this->state = 0;
	return(1);
}


void VDustCloud::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	Vector2 relV;
	double excessSpeed;
	double d;
	//friction effects first
	if (other->exists() && !other->isPlanet()) {
		other->vel *= 1 - this->friction * game->frame_time;
		if (other->isShot()) {
			Shot *s = (Shot*)(other);
			s->v *= 1 - this->friction * game->frame_time;
			s->range = s->d + (s->range - s->d) * (1 - this->friction * game->frame_time);
		}
	}
	relV = this->vel - other->vel;
	if (relV.magnitude()<this->minRelSpeedForDamage) return;
	excessSpeed = relV.magnitude() - this->minRelSpeedForDamage;
	d = this->damageFactor * excessSpeed * (frame_time / 1000.0);
	this->damage_factor = d;
	damage(other, d/2, d/2);
	if (other->isShip()) ((Ship*)other)->update_panel=1;

}


void VDustCloud::death(void)
{
	STACKTRACE;
	SpaceObject::death();
	if (this->willRespawn) game->add(new VDustCloud());
}


void VDustCloud::calculate(void)
{
	STACKTRACE;
	double x;
	x = (double)frame_time * this->velocityRestoreFactor;
	if (x>1)x=1;
	this->vel = (1-x) * this->vel + x * this->originalVel;
	SpaceObject::calculate();
}


VGasCloud::VGasCloud(void)
:
SpaceObject(NULL, random(map_size), random(PI2), meleedata.asteroidSprite)
{
	STACKTRACE;
	double speed, angle;
	this->velocityRestoreFactor = 0.00;
	this->mass = 0;
	Initialize();
	willRespawn = TRUE;
	//this->collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_CBODIES;
	set_depth(DEPTH_SPECIAL);
	speed = (random(this->maxVelocity));
	angle = random(PI2);
	vel = unit_vector(angle) * (speed);
	this->sprite = mySprite1;
	this->isEnergized = false;
	this->originalVel = this->vel;
}


void VGasCloud::Initialize(const char* nameInIni)
{
	STACKTRACE;
	game->log_file ("vobject.ini");
	this->sprite_index = 0;
	mass = get_config_int(nameInIni, "Mass", 0);
	armour = get_config_float(nameInIni, "Armour", 0);
	this->maxVelocity = scale_velocity(get_config_float(nameInIni, "MaxVelocity", 0));
	//this->minRelSpeedForDamage = scale_velocity(get_config_float(nameInIni, "MinRelSpeedForDamage", 0));
	this->canBeDestroyed = get_config_int(nameInIni, "CanBeDestroyed", 0);
	this->damageFactor = get_config_float(nameInIni, "DamageFactor", 0);
	this->friction = get_config_float(nameInIni, "Friction", 0);
	this->velocityRestoreFactor = get_config_float(nameInIni, "VelocityRestoreFactor", 0);
	this->energizingDamageFactor = get_config_float(nameInIni, "EnergizingDamageFactor", 0);
	this->energizedByLineDamage = get_config_int(nameInIni, "EnergizedByLineDamage", 0);
	this->energizedByShotDamage = get_config_int(nameInIni, "EnergizedByShotDamage", 0);
	this->energizedByPlanetDamage = get_config_int(nameInIni, "EnergizedByPlanetDamage", 0);
	this->energizedByAsteroidDamage = get_config_int(nameInIni, "EnergizedByAsteroidDamage", 0);
}


void VGasCloud::Initialize(void)
{
	STACKTRACE;
	Initialize("GasCloud");
}


int VGasCloud::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	if (isEnergized = false) {
		this->damageFactor=0;
		this->damage_factor=0;
	}
	if (!source->isAsteroid())armour -= normal + direct;
	if (this->canBeDestroyed && armour < 0)
		state = 0;
	if ( (source->isLine()&&this->energizedByLineDamage) ||
		(source->isAsteroid()&& this->energizedByAsteroidDamage) ||
		(source->isPlanet() && this->energizedByPlanetDamage) ||
	(source->isShot() && this->energizedByShotDamage) ) {
		//source->state = 0;
		this->damageFactor += normal + direct;
		this->damage_factor = this->damageFactor;
		this->isEnergized = true;
	}
	if (this->canBeDestroyed) return(1);
	else return(0);
}


void VGasCloud::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	if (this->damageFactor>0.001) {
		//message.print(500,5,"energizing damage");
		damage(other, this->energizingDamageFactor * this->damageFactor / 2, this->energizingDamageFactor * this->damageFactor / 2);
		if (other->isShip()) ((Ship*)other)->update_panel=1;
	}
	if (other->exists() && !other->isPlanet()) {
		other->vel *= 1 - this->friction * game->frame_time;
		if (other->isShot()) {
			Shot *s = (Shot*)(other);
			s->v *= 1 - this->friction * game->frame_time;
			s->range = s->d + (s->range - s->d) * (1 - this->friction * game->frame_time);
		}
	}
}


void VGasCloud::calculate(void)
{
	STACKTRACE;
	double x;
	if (this->isEnergized) {
		this->isEnergized = false;
		this->set_sprite(VGasCloud::mySprite2);
	} else {
		this->damageFactor = 0;
		this->damage_factor = this->damageFactor;
		this->set_sprite(VGasCloud::mySprite1);
	}
	SpaceObject::calculate();
	x = (double)frame_time * this->velocityRestoreFactor;
	if (x>1)x=1;
	this->vel = (1-x) * this->vel + x * this->originalVel;
}


void VGasCloud::death(void)
{
	STACKTRACE;
	SpaceObject::death();
	if (this->willRespawn) game->add(new VGasCloud());
}


VPlanet::VPlanet()
:
								 // a sprite is needed to avoid a crash...WTF?
Planet(Vector2(0,0), VMoon::mySprite, 0)
{
	STACKTRACE;
	int i;
	for(i=0; i<12; i++) this->Installation[i]=NULL;
	this->id = ID_PLANET;
	this->angle = 0;
}


double VPlanet::getRadius(void)
{
	STACKTRACE;
	return(((this->size.x + this->size.y) / 4) / 1000);
}


void VPlanet::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	Planet::inflict_damage(other);
}


int VPlanet::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int i,j;
	for(i=0;i<12;i++) {
		if (this->Installation[i]!=NULL)
		if (this->Installation[i]->isShield) {
			this->Installation[i]->handle_damage(source, normal, direct);
			return(1);
		}
	}

	// select a random installation to handle the damage.
	int k;
	k = 0;
	for ( i = 0; i < 12; i++) {
		if (Installation[i] && !Installation[i]->isDestroyed)
			++k;
	}
	if (k == 0)
		return 0;

	j = random(k);

	for ( i = 0; i < 12; i++) {
		if (Installation[i] && !Installation[i]->isDestroyed)
			--k;
		if ( k == 0 ) {
			Installation[j]->handle_damage(source, normal, direct);
			return(1);
		}
	}
	//twenty tries, can't find a valid damage recipient, give up now.
	tw_error("about to try to VPlanet::handle damage #3 -- this should never ever happen!");
	return(0);
}


void VPlanet::Initialize(const char* nameInIni)
{
	STACKTRACE;
	game->log_file ("vobject.ini");
	this->sprite_index = 0;
	gravity_mindist = scale_range(get_config_float(nameInIni, "GravityMinDist", 0));
	gravity_range = scale_range(get_config_float(nameInIni, "GravityRange", 0));
	gravity_power = get_config_float(nameInIni, "GravityPower", 0);
	gravity_force = scale_acceleration(get_config_float(nameInIni, "GravityForce", 0), 0);
	gravity_whip = get_config_float(nameInIni, "GravityWhip", 0);
	gravity_whip2 = get_config_float(nameInIni, "GravityWhip2", 0);

}


bool VPlanet::AddInstallation(VGroundInstallation* oGI)
{
	STACKTRACE;
	int i;
	for(i=0;i<12;i++) {
		if (this->Installation[i]==NULL) {
			this->Installation[i] = oGI;
			game->add(this->Installation[i]);
			return(true);
		}
	}
	return(false);
}


void VPlanet::death()
{
	STACKTRACE;
	int i;
	for(i=0;i<12;i++)
		if (this->Installation[i]!=NULL)
			this->Installation[i]->state = 0;
}


void VStar::AttractShots(int warping_power, double intensity)
{
	STACKTRACE;
	//sort of like gravity, but no minimum distance
	//and only changes direction, not speed.
	//only affects massless objects.
	//not called by default by VStar::calculate().  Thus, it must
	//be specifically invoked in the calculate section.
	Vector2 V;
	Shot* S;
	SpaceLine* SL;
	double originalVelocity;
	double angle;
	double angleSum;
	double distEffect = 1;
	double dist;
	int i;
	//warping_power = 0; intensity = 10.0; //debug
	SpaceObject::calculate();
	SpaceObject *o;
	Query a;
	a.begin(this, bit(LAYER_LINES)|bit(LAYER_SHOTS)|bit(LAYER_SPECIAL), gravity_range);
	for (;a.currento;a.next()) {
		o = a.currento;
		if (true) {
			angle = o->trajectory_angle(this);
			dist = o->distance(this);
			V = unit_vector(angle);
			if (warping_power<0) {
				for(i=0;i<(-warping_power);i++)
					distEffect *= scale_range(10.0) / dist;
			}
			else if (warping_power>0) {
				for(i=0;i<(warping_power);i++)
					distEffect *= scale_range(10.0) * dist;
				;
			} else {
				distEffect = scale_range(10.0);
			}
			V *= distEffect * intensity;
			angleSum = (o->vel + V).angle();
			if (o->isShot()) {
				S = (Shot*)o;
				S->changeDirection(angleSum);
			}
			else if (o->isLine()) {
				SL = (SpaceLine*)o;
				originalVelocity = SL->vel.magnitude();
				SL->vel = unit_vector(angleSum) * originalVelocity;
				SL->angle = angleSum;
			}
		}
	}
	return;
}


VStar::VStar()
:
								 //sprite problem?
Planet(Vector2(0,0), VBrownDwarf::mySprite, 0)
{
	STACKTRACE;
	this->id = ID_PLANET;		 // done for grav-whip purposes!!!!
	this->starEnergy = 0.0;
	this->angle = 0;
	//this->stormEnergyLostPerSecond = 0.0;
	gravity_mindist = 0;
	gravity_range = 0;
	gravity_power = 0;
	gravity_force = 0;
	gravity_whip = 0;
	gravity_whip2 = 0;
	mass = 0;
	this->flareCounter = 0.0;
	this->frictionRadius = 0.0;
	this->frictionAtCore = 0.0;
}


double VStar::getRadius(void)
{
	STACKTRACE;
	return((this->size.x + this->size.y) / 4);
}


void VStar::calculate()
{
	STACKTRACE;
	double r;
	double maxFlares;
	double flareFactor;
	double percentFrictionEffect, fracVelocityLost;
	flareFactor = (double)this->maxFlaresPerSecond * (double)frame_time / 1000.0;
	flareCounter += flareFactor;
	if (flareCounter>flareFactor)flareCounter=flareFactor;
	this->starEnergy -= (this->starEnergyLostPerSecond) * ((double)frame_time / 1000.0);
	if (this->starEnergy<0.0) this->starEnergy = 0.0;
	maxFlares = (double)frame_time * this->maxFlaresPerSecond / (double)1000.0;
	while(this->starEnergy>this->starEnergyToFlare && flareCounter>0.0) {
		r = (double)(random() % 1000000) / (double)1000000.0;
		LaunchFlare(PI2*r, this->flareSpeed, this->flareRange, this->flareDamage, this->finalFlareSpeed);
		this->starEnergy -=this->starEnergyPerFlare;
		flareCounter -= 1.00;
		//if (flareCounter<0.0)break;
	}
	Planet::calculate();
	SpaceObject *o;
	Query a;
	a.begin(this, OBJECT_LAYERS, this->frictionRadius);
	for (;a.currento;a.next()) {
		o = a.currento;
		if (o->mass > 0.001) {
			percentFrictionEffect = 1-(o->distance(this) / this->frictionRadius);
			if (percentFrictionEffect>1) percentFrictionEffect = 1.0;
			if (percentFrictionEffect<0) percentFrictionEffect = 0.0;
			fracVelocityLost = percentFrictionEffect * this->frictionAtCore * (double)frame_time;
			fracVelocityLost = fracVelocityLost / (1 + fracVelocityLost);
			o->vel *= (1-fracVelocityLost);
		}
	}
}


void VStar::LaunchFlare(double oangle, double oinitialVelocity, double orange, double odamage, double ofinalVelocity)
{
	STACKTRACE;
	;							 // this SHOULD be overridden as needed by individual stars...?
	VRedFlare* VRF;
	VRF = new VRedFlare(this, oangle, oinitialVelocity, orange, odamage, ofinalVelocity);
	game->add(VRF);
	VRF->pos += unit_vector(oangle) * this->getRadius();
}


int VStar::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	return(TRUE);
	if (other->id==COMET_ID) return FALSE;
	return(!other->isPlanet());
}


void VStar::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	double percentCoreDamage;

	double totalDamage;
	percentCoreDamage = 1 - (other->distance(this) / this->getRadius());
	if (percentCoreDamage<0) percentCoreDamage=0;
	totalDamage = (this->edgeDamagePerSecond + percentCoreDamage * this->coreDamagePerSecond) * ((double)frame_time / (double)1000.0);
	damage(other, totalDamage/2, totalDamage/2);
	if (other->isShip()) ((Ship*)other)->update_panel=1;
	if (other->isShot())
		starEnergy += 1.0000 * (double)(((Shot*)other)->damage_factor);
	else
		starEnergy += totalDamage;
}


int VStar::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return(0);
	/*int x;
	x = VStar::handle_damage(source, normal, direct);
	starEnergy += (normal + direct) * 1.000;
	if (starEnergy>1.00000) {
	  game->add(new VRedFlare(this, this->trajectory_angle(source), scale_velocity(90), scale_range(30)));
	}
	return(x);*/
}


void VStar::Initialize(const char* nameInIni)
{
	STACKTRACE;
	starEnergy = 0.0;
	game->log_file ("vobject.ini");
	this->sprite_index = 0;
	gravity_mindist = scale_range(get_config_float(nameInIni, "GravityMinDist", 0));
	gravity_range = scale_range(get_config_float(nameInIni, "GravityRange", 0));
	gravity_power = get_config_float(nameInIni, "GravityPower", 0);
	gravity_force = scale_acceleration(get_config_float(nameInIni, "GravityForce", 0), 0);
	gravity_whip = get_config_float(nameInIni, "GravityWhip", 0);
	gravity_whip2 = get_config_float(nameInIni, "GravityWhip2", 0);
	edgeDamagePerSecond = get_config_float(nameInIni, "EdgeDamagePerSecond", 0);
	coreDamagePerSecond = get_config_float(nameInIni, "CoreDamagePerSecond", 0);
	starEnergyToFlare = get_config_float(nameInIni, "StarEnergyToFlare", 0);
	starEnergyPerFlare = get_config_float(nameInIni, "StarEnergyPerFlare", 0);
	starEnergyLostPerSecond = get_config_float(nameInIni, "StarEnergyLostPerSecond", 0);
	maxFlaresPerSecond = get_config_float(nameInIni, "MaxFlaresPerSecond", 0);
	flareRange = scale_range(get_config_float(nameInIni, "FlareRange", 0));
	flareSpeed = scale_velocity(get_config_float(nameInIni, "FlareSpeed", 0));
	finalFlareSpeed = scale_velocity(get_config_float(nameInIni, "FinalFlareSpeed", -100));
	flareDamage = (get_config_float(nameInIni, "FlareDamage", 0));
	warpingPower = get_config_int(nameInIni, "WarpingPower", 0);
	warpingIntensity = get_config_float(nameInIni, "WarpingIntensity", 0);
	frictionRadiusRaw = get_config_float(nameInIni, "FrictionRadius", -1.0);
	if (frictionRadiusRaw==-1.0)
		frictionRadius = this->getRadius();
	else
		frictionRadius = scale_range(frictionRadiusRaw);
	frictionAtCore = get_config_float(nameInIni, "FrictionAtCore", 0);
}


bool VStar::isPlanet(void)
{
	STACKTRACE;
	return(true);				 // a kludge, should help grav-whip???
}


VMetalShard::VMetalShard()
:
Asteroid()
{
	STACKTRACE;
	this->set_sprite(mySprite);
	armour = VMetalShard::myArmour;
	damage_factor = VMetalShard::myDamage;
	mass = VMetalShard::myMass;
	explosionSprite = meleedata.sparkSprite;
	mass = 1;
	this->willRespawn = FALSE;
}


VMetalShard::VMetalShard(Vector2 pos, Vector2 vel)
:
Asteroid()
{
	STACKTRACE;
	if (VMetalShard::mySprite==NULL) {
		;
	}
	this->set_sprite(mySprite);
	armour = VMetalShard::myArmour;
	damage_factor = VMetalShard::myArmour;
	explosionSprite = meleedata.sparkSprite;
	mass = VMetalShard::myMass;
	//
	this->pos = pos;
	this->vel = vel;
}


int VMetalShard::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (!exists()) return 0;
	if ((normal > 0) || (direct > 0)) {
		armour -= normal;
		armour -= direct;
		if (armour <= 0) {
			normal += armour;
			armour = 0;
			state = 0;
			animateExplosion();
			soundExplosion();
		}
	}
	return 1;
}


void VMetalShard::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->exists()) return;
	damage(other, 0, damage_factor);
	if (other->isShip()) ((Ship*)other)->update_panel=1;
	if (VMetalShard::noBounce)
		//if (!other->isShot()) state=0;
		if (other->isblockingweapons) state = 0;
	return;
}


void VMetalShard::animateExplosion(void)
{
	STACKTRACE;
	game->add(new Animation(this, normal_pos(),
		explosionSprite, 0, 10,
		scale_frames(0), DEPTH_EXPLOSIONS));
	return;
}


void VMetalShard::soundExplosion(void)
{
	STACKTRACE;
	if (damage_factor > 0) {
		int i = iround_down(damage_factor / 2);
		if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	}
	return;
}


void VMetalShard::death(void)
{
	STACKTRACE;
	SpaceObject::death();
	if (this->willRespawn) game->add(new VMetalShard());
}


void VMetalShard::compareSprites(void)
{
	STACKTRACE;
	if (this->sprite==this->mySprite) {
		tw_error("Identical sprites");
	} else {
		tw_error("non-identical sprites!");
	}
}


VMetalAsteroid::VMetalAsteroid()
:
Asteroid()
{
	STACKTRACE;
	this->set_sprite(mySprite);
	armour = VMetalAsteroid::myArmour;
	damage_factor = VMetalAsteroid::myDamage;
	explosionSprite = meleedata.sparkSprite;
	mass = VMetalAsteroid::myMass;
	this->willRespawn = TRUE;
}


int VMetalAsteroid::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	Vector2 V;
	//tw_error("about to handle damage");
	if (!exists()) return 0;
	if ((normal > 0) || (direct > 0)) {
		armour -= normal;
		armour -= direct;
		if (armour <= 0) {
			normal += armour;
			armour = 0;
			state = 0;
			animateExplosion();
			soundExplosion();
		}
	}
	if (source->isShot()) {
		if (source->vel.magnitude() > scale_velocity(10))
			V = unit_vector(source->vel) * scale_acceleration((normal + direct));
		else
			V = unit_vector(source->trajectory_angle(this)) * scale_acceleration((normal + direct));
		this->accelerate(source, V_ACCEL_KICK * V / this->mass, MAX_SPEED);
	}
	return 1;
}


void VMetalAsteroid::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	//tw_error("about to inflict damage");
	if (!other->exists()) return;
	damage(other, 0, damage_factor);
	if (other->isShip()) ((Ship*)other)->update_panel=1;
	if (VMetalAsteroid::noBounce)
		//if (!other->isShot()) state=0;
		if (other->isblockingweapons) state = 0;
	return;
}


void VMetalAsteroid::animateExplosion(void)
{
	STACKTRACE;
	game->add(new Animation(this, normal_pos(),
		explosionSprite, 0, 10,
		scale_frames(0), DEPTH_EXPLOSIONS));
	return;

}


void VMetalAsteroid::soundExplosion(void)
{
	STACKTRACE;
	if (damage_factor > 0) {
		int i = iround_down(damage_factor / 2);
		if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	}
	return;
}


void VMetalAsteroid::death(void)
{
	STACKTRACE;
	VMetalShard* VS;
	Vector2 Loc, UV;
	int i;
	Loc = this->pos;
	for(i=0; i<VMetalAsteroid::numberOfChildren1; i++) {
		UV = unit_vector(this->pos.angle() + i * (PI2 / VMetalAsteroid::numberOfChildren1));
		VS = new VMetalShard(this->pos + (UV * scale_range(.6)),
			this->vel + (UV * VMetalAsteroid::childVelocity1));
		VS->willRespawn = FALSE;
		game->add(VS);
	}
	for(i=0; i<VMetalAsteroid::numberOfChildren2; i++) {
		UV = unit_vector(this->pos.angle() + PI + i * (PI2 / VMetalAsteroid::numberOfChildren2));
		VS = new VMetalShard(this->pos + (UV * scale_range(1.2)),
			this->vel + (UV * VMetalAsteroid::childVelocity2));
		VS->willRespawn = FALSE;
		game->add(VS);
	}

	SpaceObject::death();
	if (this->willRespawn) game->add(new VMetalAsteroid());
}


VSmallAsteroid::VSmallAsteroid()
:
Asteroid()
{
	STACKTRACE;
	this->set_sprite(mySprite);
	armour = VSmallAsteroid::myArmour;
	damage_factor = VSmallAsteroid::damage_factor;
	explosionSprite = meleedata.sparkSprite;
	mass = VSmallAsteroid::mass;
	willRespawn = TRUE;
}


VSmallAsteroid::VSmallAsteroid(Vector2 pos, Vector2 vel)
:
Asteroid()
{
	STACKTRACE;
	this->set_sprite(mySprite);
	armour = 10;
	damage_factor = 0;
	explosionSprite = meleedata.sparkSprite;
	mass = 10;
	//
	this->pos = pos;
	this->vel = vel;
}


int VSmallAsteroid::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	Vector2 V;
	if (!exists()) return 0;
	if ((normal > 0) || (direct > 0)) {
		armour -= normal;
		armour -= direct;
		if (armour <= 0) {
			normal += armour;
			armour = 0;
			state = 0;
			animateExplosion();
			soundExplosion();
		}
	}
	if (source->isShot()) {
		if (source->vel.magnitude() > scale_velocity(10))
			V = unit_vector(source->vel) * scale_acceleration((normal + direct));
		else
			V = unit_vector(source->trajectory_angle(this)) * scale_acceleration((normal + direct));
		this->accelerate(source, V_ACCEL_KICK * V / this->mass, MAX_SPEED);
	}
	return 1;
}


void VSmallAsteroid::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->exists()) return;
	damage(other, 0, damage_factor);
	if (other->isShip()) ((Ship*)other)->update_panel=1;
	if (VSmallAsteroid::noBounce)
		//if (!other->isShot()) state=0;
		if (other->isblockingweapons) state = 0;
	return;
}


void VSmallAsteroid::animateExplosion(void)
{
	STACKTRACE;
	game->add(new Animation(this, normal_pos(),
		explosionSprite, 0, 10,
		scale_frames(0), DEPTH_EXPLOSIONS));
	return;

}


void VSmallAsteroid::soundExplosion(void)
{
	STACKTRACE;
	if (damage_factor > 0) {
		int i = iround_down(damage_factor / 2);
		if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	}
	return;
}


void VSmallAsteroid::death(void)
{
	STACKTRACE;
	SpaceObject::death();
	if (this->willRespawn) game->add(new VSmallAsteroid());

}


VLargeAsteroid::VLargeAsteroid()
:
Asteroid()
{
	STACKTRACE;
	this->set_sprite(mySprite);
	armour = VLargeAsteroid::myArmour;
	damage_factor = VLargeAsteroid::myDamage;
	explosionSprite = meleedata.sparkSprite;
	mass = VLargeAsteroid::myMass;
	willRespawn = TRUE;
}


int VLargeAsteroid::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	Vector2 V;
	if (!exists()) return 0;
	if ((normal > 0) || (direct > 0)) {
		armour -= normal;
		armour -= direct;
		if (armour <= 0) {
			normal += armour;
			armour = 0;
			state = 0;
			animateExplosion();
			soundExplosion();
		}
	}
	if (source->isShot()) {
		if (source->vel.magnitude() > scale_velocity(10))
			V = unit_vector(source->vel) * scale_acceleration((normal + direct));
		else
			V = unit_vector(source->trajectory_angle(this)) * scale_acceleration((normal + direct));
		this->accelerate(source, V_ACCEL_KICK * V / this->mass, MAX_SPEED);
	}
	return 1;
	//	other->accelerate (this, angle, kick / other->mass, MAX_SPEED);

}


void VLargeAsteroid::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->exists()) return;
	damage(other, 0, damage_factor);
	if (other->isShip()) ((Ship*)other)->update_panel=1;
	if (VLargeAsteroid::noBounce)
		//if (!other->isShot()) state=0;
		if (other->isblockingweapons) state = 0;
	return;
}


void VLargeAsteroid::animateExplosion(void)
{
	STACKTRACE;
	game->add(new Animation(this, normal_pos(),
		explosionSprite, 0, 10,
		scale_frames(0), DEPTH_EXPLOSIONS));
	return;

}


void VLargeAsteroid::soundExplosion(void)
{
	STACKTRACE;
	if (damage_factor > 0) {
		int i = iround_down(damage_factor / 2);
		if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	}
	return;
}


void VLargeAsteroid::death(void)
{
	STACKTRACE;
	VSmallAsteroid* VS;
	Vector2 Loc, UV;
	int i;
	Loc = this->pos;
	for(i=0; i<VLargeAsteroid::numberOfChildren; i++) {
		UV = unit_vector(this->pos.angle() + i * (PI2 / VLargeAsteroid::numberOfChildren));
		VS = new VSmallAsteroid(this->pos + (UV * scale_range(1.25)),
			this->vel + (UV * VLargeAsteroid::childVelocity));
		VS->willRespawn = FALSE;
		game->add(VS);
	}
	SpaceObject::death();
	if (this->willRespawn) game->add(new VLargeAsteroid());
}


VMoon::VMoon(void)
:
VPlanet()
{
	STACKTRACE;
	this->set_sprite(VMoon::mySprite);
	this->Initialize("Moon");
}


void VMoon::calculate()
{
	STACKTRACE;
	VPlanet::calculate();
}


VRockballPlanet::VRockballPlanet(void)
:
VPlanet()
{
	STACKTRACE;
	this->set_sprite(VRockballPlanet::mySprite);
	this->Initialize("RockballPlanet");
}


VInhospitablePlanet::VInhospitablePlanet(void)
:
VPlanet()
{
	STACKTRACE;
	this->set_sprite(VInhospitablePlanet::mySprite);
	this->Initialize("InhospitablePlanet");
}


VHabitablePlanet::VHabitablePlanet(void)
:
VPlanet()
{
	STACKTRACE;
	this->set_sprite(VHabitablePlanet::mySprite);
	this->Initialize("HabitablePlanet");
}


VGasGiant::VGasGiant(void)
:
VPlanet()
{
	STACKTRACE;
	this->set_sprite(VGasGiant::mySprite);
	this->Initialize("GasGiant");

}


VBrownDwarf::VBrownDwarf(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VBrownDwarf::mySprite);
	this->Initialize("BrownDwarf");
}


void VBrownDwarf::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VBrownDwarf::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return VStar::handle_damage(source, normal, direct);
}


VRedDwarf::VRedDwarf(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VRedDwarf::mySprite);
	this->Initialize("RedDwarf");
}


void VRedDwarf::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VRedDwarf::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return VStar::handle_damage(source, normal, direct);
}


VWhiteDwarf::VWhiteDwarf(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VWhiteDwarf::mySprite);
	this->Initialize("WhiteDwarf");
}


void VWhiteDwarf::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


void VWhiteDwarf::LaunchFlare(VStar* osource, double oangle, double oinitialVelocity, double orange, double odamage, double ofinalVelocity)
{
	STACKTRACE;
	VWhiteFlare* VRF;
	VRF = new VWhiteFlare(this, oangle, oinitialVelocity, orange, odamage, ofinalVelocity);
	game->add(VRF);
	VRF->pos += unit_vector(oangle) * this->getRadius();
}


int VWhiteDwarf::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return VStar::handle_damage(source, normal, direct);
}


VRedStar::VRedStar(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VRedStar::mySprite);
	this->Initialize("RedStar");
}


void VRedStar::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VRedStar::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return VStar::handle_damage(source, normal, direct);
}


VOrangeStar::VOrangeStar(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VOrangeStar::mySprite);
	this->Initialize("OrangeStar");

}


void VOrangeStar::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VOrangeStar::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return VStar::handle_damage(source, normal, direct);
}


void VOrangeStar::LaunchFlare(VStar* osource, double oangle, double oinitialVelocity, double orange, double odamage, double ofinalVelocity)
{
	STACKTRACE;
	VOrangeFlare* VRF;
	VRF = new VOrangeFlare(this, oangle, oinitialVelocity, orange, odamage, ofinalVelocity);
	game->add(VRF);
	VRF->pos += unit_vector(oangle) * this->getRadius();
}


VYellowStar::VYellowStar(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VYellowStar::mySprite);
	this->Initialize("YellowStar");
}


void VYellowStar::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VYellowStar::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return VStar::handle_damage(source, normal, direct);
}


void VYellowStar::LaunchFlare(double oangle, double oinitialVelocity, double orange, double odamage, double ofinalVelocity)
{
	STACKTRACE;
	VYellowFlare* VRF;
	VRF = new VYellowFlare(this, oangle, oinitialVelocity, orange, odamage, ofinalVelocity);
	game->add(VRF);
	VRF->pos += unit_vector(oangle) * this->getRadius();
}


VWhiteStar::VWhiteStar(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VWhiteStar::mySprite);
	this->Initialize("WhiteStar");
}


void VWhiteStar::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VWhiteStar::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return VStar::handle_damage(source, normal, direct);
}


void VWhiteStar::LaunchFlare(VStar* osource, double oangle, double oinitialVelocity, double orange, double odamage, double ofinalVelocity)
{
	STACKTRACE;
	VWhiteFlare* VRF;
	VRF = new VWhiteFlare(this, oangle, oinitialVelocity, orange, odamage, ofinalVelocity);
	game->add(VRF);
	VRF->pos += unit_vector(oangle) * this->getRadius();
}


VRedGiant::VRedGiant(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VRedGiant::mySprite);
	this->Initialize("RedGiant");
}


void VRedGiant::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VRedGiant::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return VStar::handle_damage(source, normal, direct);
}


VBlueGiant::VBlueGiant(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VBlueGiant::mySprite);
	this->Initialize("BlueGiant");
}


void VBlueGiant::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VBlueGiant::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return VStar::handle_damage(source, normal, direct);
}


void VBlueGiant::LaunchFlare(double oangle, double oinitialVelocity, double orange, double odamage, double ofinalVelocity)
{
	STACKTRACE;
	VWhiteFlare* VRF;
	VRF = new VWhiteFlare(this, oangle, oinitialVelocity, orange, odamage, ofinalVelocity);
	game->add(VRF);
	VRF->pos += unit_vector(oangle) * this->getRadius();
}


VNeutronStar::VNeutronStar(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VNeutronStar::mySprite);
	this->Initialize("NeutronStar");
	/*game->log_file ("vobject.ini");
	this->sprite_index = 0;
	gravity_mindist = scale_range(get_config_float("NeutronStar", "GravityMinDist", 0));
	gravity_range = scale_range(get_config_float("NeutronStar", "GravityRange", 0));
	gravity_power = get_config_float("NeutronStar", "GravityPower", 0);
	gravity_force = scale_acceleration(get_config_float("NeutronStar", "GravityForce", 0), 0);
	gravity_whip = get_config_float("NeutronStar", "GravityWhip", 0);*/
}


void VNeutronStar::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VNeutronStar::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return(0);
}


void VNeutronStar::calculate(void)
{
	STACKTRACE;
	this->sprite_index += 1;
	while(this->sprite_index>63) this->sprite_index -=64;
	while(this->sprite_index<0) this->sprite_index +=64;
	VStar::calculate();
}


VHypermass::VHypermass(void)
:
VStar()
{
	STACKTRACE;
	this->set_sprite(VHypermass::mySprite);
	this->Initialize("Hypermass");
}


void VHypermass::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	VStar::inflict_damage(other);
}


int VHypermass::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	return(0);
}


void VHypermass::calculate(void)
{
	STACKTRACE;
	this->sprite_index += 1;
	while(this->sprite_index>63) this->sprite_index -=64;
	while(this->sprite_index<0) this->sprite_index +=64;
	VStar::calculate();
	this->AttractShots(warpingPower,warpingIntensity);
}


VFlare::VFlare(VStar* source, double angle, double initialVelocity, double range, double odamage, double ofinalVelocity)
:
Missile(source, source->pos, angle, initialVelocity,
odamage, range, odamage, source, VRedFlare::mySprite, 0)
{
	STACKTRACE;
	creator = (SpaceLocation*)source;
	pos = source->pos;
	originalVelocity = initialVelocity;
	finalVelocity = ofinalVelocity;
	if (finalVelocity<0.0) {
		finalVelocity = originalVelocity;
		tw_error("encountered a default final velocity!");
	}
}


void VFlare::calculate()
{
	STACKTRACE;
	double amountFlown, amountToFly, currentAngle, calcVelocity;
	amountToFly = ((double)this->range - (double)this->d) / (double)this->range;
	amountToFly = amountToFly * amountToFly * amountToFly;
	amountFlown = 1.00 - amountToFly;
	currentAngle = (this->vel).angle();
	calcVelocity = amountToFly * originalVelocity + amountFlown * finalVelocity;
	this->v = calcVelocity;
	this->vel = unit_vector(currentAngle) * calcVelocity;
	Missile::calculate();
}


void VFlare::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	if ((SpaceLocation*)other == (SpaceLocation*)creator) return;
	Missile::inflict_damage(other);
}


int VFlare::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if ((SpaceLocation*)source == (SpaceLocation*)creator) return(0);
	return(Missile::handle_damage(source, normal, direct));
}


VRedFlare::VRedFlare(VStar* source, double angle, double initialVelocity, double range, double odamage, double ofinalVelocity)
:
VFlare(source, angle, initialVelocity, range, odamage, ofinalVelocity)
{
	STACKTRACE;
	this->set_sprite(VRedFlare::mySprite);
}


VOrangeFlare::VOrangeFlare(VStar* source, double angle, double initialVelocity, double range, double odamage, double ofinalVelocity)
:
VFlare(source, angle, initialVelocity, range, odamage, ofinalVelocity)
{
	STACKTRACE;
	this->set_sprite(VOrangeFlare::mySprite);
}


VYellowFlare::VYellowFlare(VStar* source, double angle, double initialVelocity, double range, double odamage, double ofinalVelocity)
:
VFlare(source, angle, initialVelocity, range, odamage, ofinalVelocity)
{
	STACKTRACE;
	this->set_sprite(VYellowFlare::mySprite);
}


VWhiteFlare::VWhiteFlare(VStar* source, double angle, double initialVelocity, double range, double odamage, double ofinalVelocity)
:
VFlare(source, angle, initialVelocity, range, odamage, ofinalVelocity)
{
	STACKTRACE;
	this->set_sprite(VWhiteFlare::mySprite);
}
