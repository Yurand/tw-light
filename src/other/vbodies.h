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

#ifndef __VBODIES_H__
#define __VBODIES_H__

#include "melee.h"
#include "melee/mcbodies.h"
#include "other/orbit.h"
#include "melee/mframe.h"
#include "melee/mshot.h"

#include "other/vtarget.h"

SpaceSprite* _VGetSprite(char *fileName, char *spriteName, int attribs, int rotations);
bool _VGetSpriteTo64Rot(SpaceSprite *Pics[], char *fileName, char *cmdStr,
int numSprites, int attribs);

SpaceSprite* _VGetMultiframeSprite(char *fileName, char *spriteName, int attribs, int numberOfFrames);
bool _VGetSpriteGroup(SpaceSprite *Pics[], char *fileName, char *cmdStr,
int numSprites, int attribs, int firstSpriteNumber);

class VGroundInstallation;

class VSpaceInstallation : public SpaceObject
{
	public:
		double armour;
		double friction;
		VGroundInstallation* Installation[12];
		VSpaceInstallation();
		virtual void Initialize();
		virtual void Initialize(const char* nameInIni);
		virtual bool AddInstallation(VGroundInstallation* GI);
		virtual int handle_damage(SpaceLocation* source, double normal, double direct=0);
		virtual void calculate(void);
		virtual void death(void);
};

class VDefSat : public VSpaceInstallation
{
	public:
		VDefSat(void);
		virtual void Initialize();
		static SpaceSprite* mySprite;
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
		static void LoadStaticVariables(void) {
			;
		}
};

class VDeepSpaceOutpost : public VSpaceInstallation
{
	public:
		VDeepSpaceOutpost(void);
		virtual void Initialize();
		static SpaceSprite* mySprite;
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
};

class VDeepSpaceColony : public VSpaceInstallation
{
	public:
		VDeepSpaceColony(void);
		virtual void Initialize();
		static SpaceSprite* mySprite;
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
};

class VDeepSpaceStation : public VSpaceInstallation
{
	public:
		VDeepSpaceStation(void);
		virtual void Initialize();
		static SpaceSprite* mySprite;
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
};

class VGroundInstallation : public Presence
{
	public:
		double actionsPerSecond;
		double actionCounter;
		double totalDamageTaken;
		double damageToDisable;
		double damageToDestroy;
		bool isFunctioning;
		bool isDestroyed;
		bool isShield;
		bool isWeapon;
		SpaceLocation* location;
		VGroundInstallation(SpaceLocation* olocation);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void calculate(void);
		virtual void Initialize(void);
		virtual void Initialize(const char* nameInIni);
		virtual bool BattleAction(void);
};

class VWilderness : public VGroundInstallation
{
	public:
		VWilderness(SpaceLocation* olocation);
		virtual void Initialize();
};

class VColony : public VGroundInstallation
{
	public:
		VColony(SpaceLocation* olocation);
		virtual void Initialize();
};

class VMine : public VGroundInstallation
{
	public:
		VMine(SpaceLocation* olocation);
		virtual void Initialize();
};

class VFortification : public VGroundInstallation
{
	public:
		VFortification(SpaceLocation* olocation);
		virtual void Initialize();
};

class VPlanetaryShield : public VGroundInstallation
{
	public:
		double maxShield;
		double currentShield;
		double shieldRegenerationPerSecond;
		VPlanetaryShield(SpaceLocation* olocation);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void calculate();
		virtual void Initialize();
};

class VGroundDefenseLaser : public VGroundInstallation
{
	public:
		double weaponRange;
		int weaponColor;
		int weaponDamage;
		int weaponFrames;
		VGroundDefenseLaser(SpaceLocation* olocation);
								 // returns true if action taken
		virtual bool BattleAction(void);
		virtual void Initialize(void);
};

class VGroundIonCannon : public VGroundInstallation
{
	public:
		AimSys* AimingSystem;
		double weaponVelocity;
		double weaponRange;
		double weaponDamage;
		double weaponArmour;
		VGroundIonCannon(SpaceLocation* olocation);
								 // returns true if action taken
		virtual bool BattleAction(void);
		virtual void Initialize(void);
};

class VGroundMissileLauncher : public VGroundInstallation
{
	public:
		double weaponVelocity;
		double weaponRange;
		double weaponDamage;
		double weaponArmour;
		double weaponTurnRate;
		VGroundMissileLauncher(SpaceLocation* olocation);
								 // returns true if action taken
		virtual bool BattleAction(void);
		virtual void Initialize(void);
};

class VSpaceMine : public SpaceObject
{
	private:
	public:
		static SpaceSprite* explosionSprite;
		VSpaceMine(void);
		void Initialize(void);
		int willRespawn;
		static SpaceSprite* mySprite;
		static void SetMySprite(SpaceSprite* S, SpaceSprite* Ex)
			{mySprite = S; explosionSprite=Ex;};
		double armour;
		double damage;
		double kick;
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
		virtual void death(void);
		void animateExplosion(void);
		void soundExplosion(void);
};

class VNebula : public Presence
{
	private:
	public:
		double friction;
		int ionStorms;
		VNebula();
		VNebula(double friction);
		VNebula(double friction, int ionStorms);
		virtual void calculate(void);
		virtual ~VNebula();
};

class VIonStorm : public SpaceObject
{
	private:
	public:
		double zapRange;
		double zapDamage;
		VIonStorm(void);
};

class VRedFlare;

class VDustCloud : public SpaceObject
{
	private:
	public:
		static SpaceSprite* mySprite;
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
		double minRelSpeedForDamage;
		double damageFactor;
		double armour;
		double maxVelocity;
		double friction;
		Vector2 originalVel;
		double velocityRestoreFactor;
		int canBeDestroyed;
		int willRespawn;
		void Initialize(void);
		void Initialize(const char* nameInIni);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
		virtual void death(void);
		virtual void calculate(void);
		VDustCloud(void);
};

class VGasCloud : public SpaceObject
{
	private:
	public:
		static SpaceSprite* mySprite1;
		static SpaceSprite* mySprite2;
		static void SetMySprite1(SpaceSprite* S) {mySprite1 = S;};
		static void SetMySprite2(SpaceSprite* S) {mySprite2 = S;};
		//double minRelSpeedForDamage;
		Vector2 originalVel;
		double velocityRestoreFactor;
		double damageFactor;
		bool isEnergized;
		double energizingDamageFactor;
		double armour;
		double maxVelocity;
		int canBeDestroyed;
		int willRespawn;
		int energizedByLineDamage;
		int energizedByShotDamage;
								 //also stars
		int energizedByPlanetDamage;
		int energizedByAsteroidDamage;
		double friction;
		virtual void calculate(void);
		void Initialize(void);
		void Initialize(const char* nameInIni);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
		virtual void death(void);
		VGasCloud(void);
};

class VPlanet : public Planet
{
	//general additions to the planet object
	public:
		VPlanet();
		VGroundInstallation* Installation[12];
		double getRadius(void);
		double CollisionDamage;
		void Initialize(const char* nameInIni);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
		virtual bool AddInstallation(VGroundInstallation* GI);
		virtual void death(void);
};

class VStar : public Planet
{
	//must be based on planet for proper grav-whipping!!!
	//current Sun code in orbit.cpp/orbit.h doesn't work correctly
	// for gravwhips
	public:
		double starEnergy;
		//double stormEnergyLostPerSecond;
		int willRespawn;
		virtual bool isPlanet(void);
		VStar();
		double getRadius(void);
		double edgeDamagePerSecond;
		double coreDamagePerSecond;
		double starEnergyToFlare;
		double starEnergyPerFlare;
		double starEnergyLostPerSecond;
		double maxFlaresPerSecond;
		double flareRange;
		double flareSpeed;
		double finalFlareSpeed;
		double flareDamage;
		double flareCounter;
		double frictionRadiusRaw;
		double frictionRadius;
		double frictionAtCore;
		int warpingPower;
		double warpingIntensity;
		void Initialize(const char* nameInIni);
		virtual void inflict_damage(SpaceObject* other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void calculate(void);
		virtual int canCollide(SpaceLocation *other);
		virtual void LaunchFlare( double oangle, double oinitialVelocity, double orange, double odamage=1.0, double ofinalVelocity=-1);
		virtual void AttractShots(int powerFunction, double intensity);
};

class VFlare : public Missile
{
	public:
		SpaceLocation* creator;
		double originalVelocity;
		double finalVelocity;
		VFlare(VStar* source, double angle, double initialVelocity, double range, double odamage=1.0, double finalVelocity=-1);
		virtual void calculate(void);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
};

class VRedFlare : public VFlare
{
	private:
	public:
		static SpaceSprite* mySprite;
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
		VRedFlare(VStar* source, double angle, double initialVelocity, double range, double odamage=1, double ofinalVelocity=-1);
};

class VOrangeFlare : public VFlare
{
	private:
		static SpaceSprite* mySprite;
	public:
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
		VOrangeFlare(VStar* source, double angle, double initialVelocity, double range, double odamage=1, double ofinalVelocity=-1);
};

class VYellowFlare : public VFlare
{
	private:
		static SpaceSprite* mySprite;
	public:
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
		VYellowFlare(VStar* source, double angle, double initialVelocity, double range, double odamage=1, double ofinalVelocity=-1);
};

class VWhiteFlare : public VFlare
{
	private:
	public:
		static SpaceSprite* mySprite;
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
		VWhiteFlare(VStar* source, double angle, double initialVelocity, double range, double odamage=1, double ofinalVelocity=-1);
};

class VMetalShard : public Asteroid
{
	private:
		static SpaceSprite* mySprite;
		static double myArmour;
		static double myMass;
		static double myDamage;
		static int noBounce;

	public:
		int willRespawn;
		SpaceSprite* explosionSprite;
		double armour;
		double damage_factor;
		VMetalShard();
		VMetalShard(Vector2 pos);
		VMetalShard(Vector2 pos, Vector2 vel);
		static void SetMySprite(SpaceSprite* S) {mySprite = S;};
		static void InitStatics(void) {
			game->log_file ("vobject.ini");
			VMetalShard::myArmour = get_config_float("MetalShard", "Armour", 0);
			VMetalShard::myMass = get_config_float("MetalShard", "Mass", 0);
			VMetalShard::myDamage = get_config_float("MetalShard", "Damage", 0);
			VMetalShard::noBounce = get_config_int("MetalShard", "NoBounce" ,1);
		}
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		void animateExplosion(void);
		void soundExplosion(void);
		virtual void death(void);
								 //purely debugging
		void compareSprites(void);
};

class VMetalAsteroid : public Asteroid
{
	private:
		static SpaceSprite* mySprite;

	public:
		static double myArmour;
		static double myMass;
		static double myDamage;
		static int noBounce;
		static int numberOfChildren1;
		static int numberOfChildren2;
		static double childVelocity1;
		static double childVelocity2;

		int willRespawn;
		SpaceSprite* explosionSprite;
		double armour;
		double damage_factor;
		VMetalAsteroid();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		static void InitStatics(void) {
			game->log_file ("vobject.ini");
			VMetalAsteroid::myArmour = get_config_float("MetalAsteroid", "Armour", 1);
			VMetalAsteroid::myMass = get_config_float("MetalAsteroid", "Mass", 1);
			VMetalAsteroid::myDamage = get_config_float("MetalAsteroid", "Damage", 1);
			VMetalAsteroid::noBounce = get_config_int("MetalAsteroid", "NoBounce" ,0);
			VMetalAsteroid::numberOfChildren1 = get_config_int("MetalAsteroid", "NumberOfChildren1", 0);
			VMetalAsteroid::numberOfChildren2 = get_config_int("MetalAsteroid", "NumberOfChildren2", 0);
			VMetalAsteroid::childVelocity1 = scale_velocity(get_config_float("MetalAsteroid", "ChildVelocity1", 0));
			VMetalAsteroid::childVelocity2 = scale_velocity(get_config_float("MetalAsteroid", "ChildVelocity2", 0));
		}

		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		void animateExplosion(void);
		void soundExplosion(void);
		virtual void death(void);

};

class VSmallAsteroid : public Asteroid
{
	private:

	public:
		static double myArmour;
		static double myMass;
		static double myDamage;
		static int noBounce;

		int willRespawn;
		static SpaceSprite* mySprite;
		SpaceSprite* explosionSprite;
		double armour;
		double damage_factor;
		VSmallAsteroid();
		VSmallAsteroid(Vector2 pos, Vector2 vel);
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		static void InitStatics(void) {
			game->log_file ("vobject.ini");
			VSmallAsteroid::myArmour = get_config_float("SmallAsteroid", "Armour", 0);
			VSmallAsteroid::myMass = get_config_float("SmallAsteroid", "Mass", 0);
			VSmallAsteroid::myDamage = get_config_float("SmallAsteroid", "Damage", 0);
			VSmallAsteroid::noBounce = get_config_int("SmallAsteroid", "NoBounce" ,1);

		}
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		void animateExplosion(void);
		void soundExplosion(void);
		virtual void death(void);

};

class VLargeAsteroid : public Asteroid
{
	private:
	public:
		static double myArmour;
		static double myMass;
		static double myDamage;
		static int noBounce;
		static int numberOfChildren;
		static double childVelocity;

		static SpaceSprite* mySprite;
		int willRespawn;
		SpaceSprite* explosionSprite;
		double armour;
		double damage_factor;
		VLargeAsteroid();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		static void InitStatics(void) {
			game->log_file ("vobject.ini");
			VLargeAsteroid::myArmour = get_config_float("LargeAsteroid", "Armour", 0);
			VLargeAsteroid::myMass = get_config_float("LargeAsteroid", "Mass", 0);
			VLargeAsteroid::myDamage = get_config_float("LargeAsteroid", "Damage", 0);
			VLargeAsteroid::noBounce = get_config_int("LargeAsteroid", "NoBounce" ,1);
			VLargeAsteroid::numberOfChildren = get_config_int("LargeAsteroid", "NumberOfChildren", 0);
			VLargeAsteroid::childVelocity = scale_velocity(get_config_float("LargeAsteroid", "ChildVelocity", 0));
		}

		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		void animateExplosion(void);
		void soundExplosion(void);
		virtual void death(void);

};

class VMoon : public VPlanet
{
	private:
	public:
		static SpaceSprite* mySprite;
		VMoon();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		virtual void calculate(void);
};

class VRockballPlanet : public VPlanet
{
	private:
		static SpaceSprite* mySprite;
	public:
		VRockballPlanet();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
};

class VInhospitablePlanet : public VPlanet
{
	private:
		static SpaceSprite* mySprite;
	public:
		VInhospitablePlanet();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
};

class VHabitablePlanet : public VPlanet
{
	private:
		static SpaceSprite* mySprite;
	public:
		VHabitablePlanet();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
};

class VGasGiant : public VPlanet
{
	private:
		static SpaceSprite* mySprite;
	public:
		VGasGiant();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
};

class VBrownDwarf : public VStar
{
	private:
	public:
		static SpaceSprite* mySprite;
		VBrownDwarf();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
};

class VRedDwarf : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VRedDwarf();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
};

class VWhiteDwarf : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VWhiteDwarf();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		virtual void LaunchFlare(VStar* osource, double oangle, double oinitialVelocity, double orange, double odamage=1.0, double ofinalVelocity=-1);
};

class VRedStar : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VRedStar();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
};

class VOrangeStar : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VOrangeStar();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		virtual void LaunchFlare(VStar* osource, double oangle, double oinitialVelocity, double orange, double odamage=1.0, double ofinalVelocity=-1);
};

class VYellowStar : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VYellowStar();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		virtual void LaunchFlare(double oangle, double oinitialVelocity, double orange, double odamage=1.0, double ofinalVelocity = -1);
};

class VWhiteStar : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VWhiteStar();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		virtual void LaunchFlare(VStar* osource, double oangle, double oinitialVelocity, double orange, double odamage=1.0, double ofinalVelocity = -1);
};

class VRedGiant : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VRedGiant();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
};

class VBlueGiant : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VBlueGiant();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		virtual void LaunchFlare(double oangle, double oinitialVelocity, double orange, double odamage=1.0, double ofinalVelocity = -1);
};

class VNeutronStar : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VNeutronStar();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		void calculate(void);
};

class VHypermass : public VStar
{
	private:
		static SpaceSprite* mySprite;
	public:
		VHypermass();
		static void SetMySprite(SpaceSprite* S){mySprite = S;};
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void inflict_damage(SpaceObject *other);
		void calculate(void);
};
#endif							 // __VBODIES_H__
