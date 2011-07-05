#include <string.h>
#include <allegro.h>
#include "melee.h"
#include "melee/mframe.h"
#include "melee/mship.h"
#include "melee/mcbodies.h"
#include "melee/mview.h"
#include "frame.h"
#include "games/ggob.h"
#include "gup.h"

#include "ships/shpsupbl.h"
#include "ships/shporzne.h"
#include "ships/shpkohma.h"
#include "ships/shputwju.h"
#include "ships/shparisk.h"
#include "ships/shpandgu.h"
#include "ships/shpchebr.h"
#include "ships/shpearcr.h"
#include "ships/shpkzedr.h"
#include "ships/shpsyrpe.h"
#include "ships/shpmycpo.h"
#include "ships/shpspael.h"

/* this file contains the ship upgrades used by Gob */

/*
Generic Upgrades
*/

/*
NOTE that execute() is called BEFORE charge(), so
num is not yet incremented when execute() is running
*/
void Upgrade::clear(Ship *oship, Ship *nship, GobPlayer *gs)
{
	if (oship) gs->total -= num;
	num = 0;
	return;
}


								 //called AFTER execute
void Upgrade::charge(GobPlayer *gs)
{
	gs->total += 1;
	num += 1;
	gs->value_starbucks += this->starbucks;
	gs->value_buckazoids += this->buckazoids;
	return;
}


#define BASE_BZ (gs->total / 4)
#define BASE_BZ_(a) ((gs->total+(a)) / 4)
#define UPGRADE(a) virtual Upgrade *duplicate() {return new a();}
class UpCrewpod : public Upgrade
{
	UPGRADE(UpCrewpod)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Add Crewpod";
		if (ship->crew_max >= 42) return false;
		starbucks = 1+num;
		buckazoids = 1 + BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		ship->crew_max += 4;
		if (ship->crew_max > 42) ship->crew_max = 42;
		ship->crew += 4;
		if (ship->crew > 42) ship->crew = 42;
	}
} crewpod;

class UpBattery : public Upgrade
{
	UPGRADE(UpBattery)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Add Battery";
		if (ship->batt_max >= 42) return false;
		starbucks = 1;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		ship->batt_max += 8;
		if (ship->batt_max > 42) ship->batt_max = 42;
		ship->batt += 8;
		if (ship->batt > 42) ship->batt = 42;
	}
} battery;

class UpThrusters : public Upgrade
{
	UPGRADE(UpThrusters)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Thrusters";
		starbucks = 3 + num * 3;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		ship->speed_max *=  1 + .3  / (.30*num + 1);
		ship->accel_rate *= 1 + .18 / (.08*num + 1);
	}
} thrusters;

class UpControlJets : public Upgrade
{
	UPGRADE(UpControlJets)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Control Jets";
		starbucks = 2 + num * 2;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		ship->turn_rate  *=  1 + .3  / (.08*num + 1);
		ship->accel_rate *=  1 + .1  / (.1*num + 1);
	}
} controljets;

class UpDynamo : public Upgrade
{
	UPGRADE(UpDynamo)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Dynamo";
		if (ship->recharge_amount < 0) return false;
		starbucks = 16 / (1 + ship->recharge_amount-num);
		if (ship->recharge_amount == 0) starbucks *= 6;
		if (ship->weapon_rate < 100) starbucks /= 2;
		if (!strcmp("supbl", ship->type->id)) starbucks /= 2;
		starbucks += num;
		if (ship->special_drain > 16) starbucks *= 2;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		ship->recharge_amount += 1;
	}
} dynamo;

class UpSensor : public Upgrade
{
	UPGRADE(UpSensor)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Sensor System";
		starbucks = 1 + num + (num * num * num + num * num * 10 + num * 10) / 100;
		buckazoids = num / 3 + BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
	}
} sensor;

class UpRepairSystem : public Upgrade
{
	UPGRADE(UpRepairSystem)
		RepairSystem *rs;
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Field Repair System";
		if (num) name = "Upgrade Field Repair System";
		starbucks = 4 * (num+1);
		buckazoids = 2 + BASE_BZ;
		if (!strcmp("orzne", ship->type->id) ||
			!strcmp("urqdr", ship->type->id)
		) {
			starbucks += 2 * (num + 1);
			buckazoids += 2 * (num + 1);
		}
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		double eff = 1 - 3 / (4.+num*num+num);
		//		double eff = 1 - pow(0.5, (num+2) / 3.0);
		double rate = (num + 1.0) * 0.025 * pow(1.15, num) / eff;
		if (!rs) {
			rs = new RepairSystem ( ship );
			game->add(rs);
		}
		rs->rate = rate;
		rs->efficiency = eff;
		rs->reset();
	}
	void clear(Ship *oship, Ship *nship, GobPlayer *gs) {
		Upgrade::clear(oship, nship, gs);
		if (oship && rs) rs->die();
		rs = NULL;
	}
} repairsystem;

/*
Supox Upgrades
*/

class UpSupoxRange : public Upgrade
{
	UPGRADE(UpSupoxRange)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Glob Hurler (Supox)";
		if (strcmp("supbl", ship->type->id)) return false;
		starbucks = 2 + num * 2;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((SupoxBlade*)ship)->weaponRange *= 1 + .20 / (1 + num*.20);
		((SupoxBlade*)ship)->weaponVelocity *= 1 + .05 / (1 + num*.25);
	}
} supoxrange;

class UpSupoxDamage : public Upgrade
{
	UPGRADE(UpSupoxDamage)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Glob Former (Supox)";
		if (strcmp("supbl", ship->type->id)) return false;
		if (gs->ship->recharge_amount < (1<<num)) return false;
		if (num >= 6) return false;
		starbucks = 5;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((SupoxBlade*)ship)->weaponDamage += 1;
		((SupoxBlade*)ship)->weaponArmour += 1;
		((SupoxBlade*)ship)->weapon_drain += num + 1;
	}
} supoxdamage;

class UpSupoxBLADE : public Upgrade
{
	UPGRADE(UpSupoxBLADE)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Add B.L.A.D.E. (Supox)";
		if (strcmp("supbl", ship->type->id)) return false;
		starbucks = 2 + num;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((SupoxBlade*)ship)->damage_factor += 3 + num;
	}
} supoxblade;

/*
Orz Upgrades
*/

class UpOrzMissile : public Upgrade
{
	UPGRADE(UpOrzMissile)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Missiles (ORZ)";
		if (strcmp("orzne", ship->type->id)) return false;
		if (num > 30) return false;
		starbucks = 6 + num;
		buckazoids = BASE_BZ + 2;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((OrzNemesis*)ship)->weaponDamage += 1;
		((OrzNemesis*)ship)->weaponArmour += 1;
		((OrzNemesis*)ship)->weaponRange *= 1 + .15 / (1+num*.05);
		((OrzNemesis*)ship)->weapon_drain += 1;
	}
} orzmissile;

class UpOrzMarineSpeed : public Upgrade
{
	UPGRADE(UpOrzMarineSpeed)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Marine Suits (ORZ)";
		if (strcmp("orzne", ship->type->id)) return false;
		starbucks = 3 + num * 3;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((OrzNemesis*)ship)->specialArmour += 1 + num/2;
		((OrzNemesis*)ship)->specialSpeedMax *= 1 + .2 / (.2*num+1);
		((OrzNemesis*)ship)->specialAccelRate *= 1 + .15 / (.2 * num + 1);
	}
} orzmarinespeed;

class UpOrzAbsorbtion : public Upgrade
{
	UPGRADE(UpOrzAbsorbtion)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Absorbtion (ORZ)";
		if (strcmp("orzne", ship->type->id)) return false;
		if (num) return false;
		starbucks = 15;
		buckazoids = BASE_BZ + BASE_BZ_(1) + BASE_BZ_(2);
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((OrzNemesis*)ship)->absorption = iround_down(256 / 3.6);
	}
	void charge(GobPlayer *gs) {
		Upgrade::charge(gs);
		gs->total += 2;
	}
} orzabsorption;

/*
Kohr-Ah Upgrades
*/

class UpKohrAhBladeDamage : public Upgrade
{
	UPGRADE(UpKohrAhBladeDamage)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Increase Shuriken Sharpness (Kohr-Ah)";
		if (strcmp("kohma", ship->type->id)) return false;
		starbucks = 7 + num * 2;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((KohrAhMarauder*)ship)->weaponDamage += 2;
		((KohrAhMarauder*)ship)->weaponArmour += 2;
		((KohrAhMarauder*)ship)->weapon_drain += 2;
	}
} kohrahbladedamage;

class UpKohrAhBladeSpeed : public Upgrade
{
	UPGRADE(UpKohrAhBladeSpeed)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		if (strcmp("kohma", ship->type->id)) return false;
		name = "Increase Shuriken Velocity (Kohr-Ah)";
		starbucks = 2 + num * 2;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((KohrAhMarauder*)ship)->weaponVelocity *= 1 + .2 / (1 + num * .2);
	}
} kohrahbladespeed;

class UpKohrAhFireRange : public Upgrade
{
	UPGRADE(UpKohrAhFireRange)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "increase F.R.I.E.D. range (Kohr-Ah)";
		if (strcmp("kohma", ship->type->id)) return false;
		if (num >= 5) return false;
		starbucks = 5 * (num + 2);
		buckazoids = 2 + BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((KohrAhMarauder*)ship)->specialRange *= 1 + .2 / (1 + .1 * num);
		((KohrAhMarauder*)ship)->specialVelocity *= 1 + .05 / (1 + .1 * num);
		((KohrAhMarauder*)ship)->special_drain += 2;
	}
} kohrahfirerange;

class UpKohrAhFireDamage : public Upgrade
{
	UPGRADE(UpKohrAhFireDamage)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "increase F.R.I.E.D. damage (Kohr-Ah)";
		if (strcmp("kohma", ship->type->id)) return false;
		if (num >= 5) return false;
		starbucks = 4 + num * 2;
		buckazoids = 1 + BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((KohrAhMarauder*)ship)->specialDamage += 1;
		((KohrAhMarauder*)ship)->special_drain += 2;
	}
} kohrahfiredamage;

/*
Utwig Upgrades
*/

class UpUtwigJuggerRange : public Upgrade
{
	UPGRADE(UpUtwigJuggerRange)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Bolt Regulator (Utwig)";
		if (strcmp("utwju", ship->type->id)) return false;
		starbucks = 3 + num;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((UtwigJugger*)ship)->weaponRange += 80;
	}
} utwigrange;
class UpUtwigJuggerDamage : public Upgrade
{
	UPGRADE(UpUtwigJuggerDamage)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Bolt Charger (Utwig)";
		if (strcmp("utwju", ship->type->id)) return false;
		starbucks = (num + 3) * 4;
		buckazoids = BASE_BZ;
		if (ship->weapon_rate > 1000) return false;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((UtwigJugger*)ship)->weaponDamage += 1;
		((UtwigJugger*)ship)->weaponArmour += 1;
		ship->weapon_rate += 250;
	}
} utwigdamage;
class UpUtwigJuggerROF : public Upgrade
{
	UPGRADE(UpUtwigJuggerROF)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Bolt Generator (Utwig)";
		if (strcmp("utwju", ship->type->id)) return false;
		if (ship->weapon_rate < 425) return false;
		starbucks = num / 2 + 2;
		buckazoids = BASE_BZ;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		ship->weapon_rate -= 50;
	}
} utwigrof;
class UpUtwigJuggerMaskOfHonestDemeanor : public Upgrade
{
	UPGRADE(UpUtwigJuggerMaskOfHonestDemeanor)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Mask of Honest Demeanor (max 1 mask)";
		if (strcmp("utwju", ship->type->id)) return false;
		if (strcmp(station->build_type, "utwju")) return false;
		if ((num + gs->upgrade_list[UpgradeIndex::utwigmask2]->num) >= 1) return false;
		starbucks = 99;
		buckazoids = 0;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		gs->value_starbucks += 251;
		gs->value_buckazoids += 250;
	}
} utwigmask1;
class UpUtwigJuggerMaskOfElephantineFortitude : public Upgrade
{
	UPGRADE(UpUtwigJuggerMaskOfElephantineFortitude)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Mask of Elephantine Fortitude (max 1 mask)";
		if (strcmp("utwju", ship->type->id)) return false;
		if ((num + gs->upgrade_list[UpgradeIndex::utwigmask1]->num) >= 1) return false;
		starbucks = 99;
		buckazoids = 0;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((UtwigJugger*)ship)->fortitude = 1;
	}
} utwigmask2;

/*
Miscilaneous Ship-Specific Upgrades
*/

class UpAndrosynthBubbleRate : public Upgrade
{
	UPGRADE(UpAndrosynthBubbleRate)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Bubble Former (Androsynth)";
		if (strcmp("andgu", ship->type->id)) return false;
		if (num >= 3) return false;
		if (gs->ship->recharge_amount < (1<<(num+1))) return false;

		starbucks = 2 * pow(5.0,num);
		buckazoids = BASE_BZ + 1 * pow(5.0,num);
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((AndrosynthGuardian*)ship)->weapon_rate /= 2;
	}
} androsynthbubblerate;
class UpAndrosynthComet : public Upgrade
{
	UPGRADE(UpAndrosynthComet)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Comet Form (Androsynth)";
		if (strcmp("andgu", ship->type->id)) return false;
		starbucks = 3 + num;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((AndrosynthGuardian*)ship)->specialDamage += 3;
		((AndrosynthGuardian*)ship)->specialVelocity *= 1 + .05 / (1+num*.04);
	}
} androsynthcomet;
class UpArilouLaserDamage : public Upgrade
{
	UPGRADE(UpArilouLaserDamage)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Laser Wattage (Arilou)";
		if (strcmp("arisk", ship->type->id)) return false;
		starbucks = 5 + num * 5;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((ArilouSkiff*)ship)->weaponDamage += num;
		((ArilouSkiff*)ship)->weapon_drain += num;
	}
} ariloulaserdamage;
class UpArilouLaserRange : public Upgrade
{
	UPGRADE(UpArilouLaserRange)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Laser Range (Arilou)";
		if (strcmp("arisk", ship->type->id)) return false;
		starbucks = 5 + num * 5;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((ArilouSkiff*)ship)->weaponRange *= 1 + .20 / (1 + .10 * num);
	}
} ariloulaserrange;
class UpChenjesuMainWeapon : public Upgrade
{
	UPGRADE(UpChenjesuMainWeapon)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Photon Shards (Chenjesu)";
		if (strcmp("chebr", ship->type->id)) return false;
		starbucks = 4 + num * 2;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((ChenjesuBroodhome*)ship)->weaponDamage += 3;
		((ChenjesuBroodhome*)ship)->weaponArmour += 3;
		((ChenjesuBroodhome*)ship)->shardDamage += 1;
		((ChenjesuBroodhome*)ship)->shardArmour += 1;
		((ChenjesuBroodhome*)ship)->shardRange *= 1 + 0.2 / (1 + num * 0.25);
		((ChenjesuBroodhome*)ship)->weaponVelocity *= 1 + 0.2 / (1 + num * 0.25);
	}
} chenjesumainweapon;
class UpChenjesuSpecial : public Upgrade
{
	UPGRADE(UpChenjesuSpecial)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade DOGI Systems (Chenjesu)";
		if (strcmp("chebr", ship->type->id)) return false;
		starbucks = 3 + num * 3;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((ChenjesuBroodhome*)ship)->specialArmour += 4 + num * 2;
		((ChenjesuBroodhome*)ship)->specialDamage += 1;
		((ChenjesuBroodhome*)ship)->specialFuelSap += 2 + num;
	}
} chenjesuspecial;
class UpEarthlingWarhead : public Upgrade
{
	UPGRADE(UpEarthlingWarhead)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Warheads (Earthling)";
		if (strcmp("earcr", ship->type->id)) return false;
		starbucks = 4 + num * 2;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((EarthlingCruiser*)ship)->weaponDamage *= 2;
	}
} earthlingwarhead;
class UpEarthlingMissile : public Upgrade
{
	UPGRADE(UpEarthlingMissile)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Missiles (Earthling)";
		if (strcmp("earcr", ship->type->id)) return false;
		starbucks = 4 + num * 2;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((EarthlingCruiser*)ship)->weaponArmour += 1;
		((EarthlingCruiser*)ship)->weaponRange *= 1 + .25 / (1 + .15*num);
		((EarthlingCruiser*)ship)->weaponVelocity *= 1 + .15 / (1 + .15*num);
	}
} earthlingmissile;
class UpEarthlingHoming : public Upgrade
{
	UPGRADE(UpEarthlingHoming)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Missile Homing (Earthling)";
		if (strcmp("earcr", ship->type->id)) return false;
		starbucks = 4 + num * 2;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((EarthlingCruiser*)ship)->weaponTurnRate *= 1 + .35 / (1 + .15*num);
		((EarthlingCruiser*)ship)->weaponVelocity *= 1 + .15 / (1 + .15*num);
	}
} earthlinghoming;
class UpEarthlingDefense : public Upgrade
{
	UPGRADE(UpEarthlingDefense)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Point Defense System Range (Earthling)";
		if (strcmp("earcr", ship->type->id)) return false;
		starbucks = 3 + num * 3;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((EarthlingCruiser*)ship)->specialRange *= 1 + .25 / (1 + .20*num);
	}
} earthlingdefense;
class UpMyconPlasmaShield : public Upgrade
{
	UPGRADE(UpEarthlingDefense)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Plasma Shield (Mycon)";
		if (strcmp("mycpo", ship->type->id)) return false;
		if (num >= 3) return false;
		starbucks = 12 + num * 12;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		MyconPodship *mp = (MyconPodship *)ship;
		if (mp->plasma_shield) mp->plasma_shield += 0.25;
		else mp->plasma_shield = 0.5;
	}
} myconplasmashield;
class UpUrquanFusion : public Upgrade
{
	UPGRADE(UpUrquanFusion)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Fusion Blaster (Green Urquan)";
		if (strcmp("kzedr", ship->type->id)) return false;
		starbucks = 4 + num * 4;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((KzerZaDreadnought*)ship)->weaponDamage += 2;
		((KzerZaDreadnought*)ship)->weaponArmour += 2;
		((KzerZaDreadnought*)ship)->weaponRange *= 1 + .2 / (1 + .15*num);
	}
} urquanfusion;
class UpSpathiGun : public Upgrade
{
	UPGRADE(UpSpathiGun)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Spathi Gun (Spathi)";
		if (strcmp("spael", ship->type->id)) return false;
		starbucks = 4 + num * 4;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((SpathiEluder*)ship)->weaponDamage += 1;
		((SpathiEluder*)ship)->weaponArmour += 1;
		((SpathiEluder*)ship)->weaponRange *= 1 + .2 / (1 + .15*num);
	}
} spathigun;
class UpSyreenGun : public Upgrade
{
	UPGRADE(UpSpathiGun)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Upgrade Stileto (Syreen)";
		if (strcmp("spael", ship->type->id)) return false;
		starbucks = 4 + num * 3;
		buckazoids = BASE_BZ + 1;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		((SyreenPenetrator*)ship)->weaponDamage += 1;
		((SyreenPenetrator*)ship)->weaponArmour += 1;
		((SyreenPenetrator*)ship)->weaponRange *= 1 + .2 / (1 + .15*num);
	}
} syreengun;

/*
Special Upgrades
*/

class UpDivineFavor : public Upgrade
{
	UPGRADE(UpDivineFavor)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Divine Favor";
		if (strcmp(station->build_type, "orzne")) return false;
		if (num) return false;
		starbucks = 99;
		buckazoids = 0;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
	}
	void clear(Ship *oship, Ship *nship, GobPlayer *gs) {
		if (!oship) num = 0;
		return;
	}
	void charge(GobPlayer *gs) {
		num += 1;
	}
} divinefavor;

class UpUnholyAura : public Upgrade
{
	UPGRADE(UpUnholyAura)
		int last_time;
	UnholyAura *aura;
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "the Devil protects his own...";
		starbucks = 6;
		buckazoids = 66;
		if (num) starbucks += 60;
		if (num > 1) starbucks += 600;
		if (num > 2) buckazoids += 600;
		//if (strcmp(station->build_type, "orzne")) return false;
		if (num && game->game_time < last_time + 666) return false;
		if (((game->game_time / 1000) % 1000) == 666) return true;
		if (((game->game_time / 1000) % 666) == 0) return true;
		//666, 1332, 1666, 1998, etc.
		return false;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		last_time = game->game_time;
		if (!num) {
			aura = new UnholyAura ( ship );
			game->add ( aura );
		}
		else aura->level ++;
	}
	void clear(Ship *oship, Ship *nship, GobPlayer *gs) {
		if (!oship) num = 0;
		else if (num) {
			aura->die();
			aura = new UnholyAura ( nship );
			aura->level = num;
			game->add ( aura );
		}
		return;
	}
	void charge(GobPlayer *gs) {
		num += 1;
	}
} unholyaura;

class UpDefender : public Upgrade
{
	UPGRADE(UpDefender)
		public:
		GobDefender *def[6];
		bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
			name = "External Defense System";
			if (strcmp(station->build_type, "kohma")) return false;
			if (num >= 6) return false;
			starbucks = 5 + 5 * (num+1) * num;
			buckazoids = 12 + 5 * num;
			return true;
		}
		void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
			def[num] = new GobDefender(ship);
			int i;
			for (i = 0; i <= num; i += 1) def[i]->base_phase = i * PI2 / (num+1);
			gobgame->add (def[num]);
		}
		void clear(Ship *oship, Ship *nship, GobPlayer *gs) {
			if (!oship) num = 0;
			if (oship) {
				for (int i = 0; i < num; i += 1) {
					def[i]->die();
					def[i] = new GobDefender(nship);
					def[i]->base_phase = i * PI2 / num;
					if (i < gs->upgrade_list[UpgradeIndex::defender2]->num) def[i]->advanced = 1;
					game->add(def[i]);
				}
			}
			//Upgrade::clear(oship, nship, gs);
			return;
		}
		void charge(GobPlayer *gs) {
			num += 1;
		}
} defender;
class UpDefender2 : public Upgrade
{
	UPGRADE(UpDefender2)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Advanced External Defense System";
		if (strcmp(station->build_type, "kohma")) return false;
		if (num >= gs->upgrade_list[UpgradeIndex::defender]->num) return false;
		starbucks = 25 + 25 * (num+1) * num;
		buckazoids = 75 + 15 * num * (num+1);
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		for (int i = 0; i <= num; i++)
			((UpDefender*)gs->upgrade_list[UpgradeIndex::defender])->def[i]->advanced = 1;
	}
	void clear(Ship *oship, Ship *nship, GobPlayer *gs) {
		if (!oship) num = 0;
		return;
	}
	void charge(GobPlayer *gs) {
		num += 1;
	}
} defender2;

class UpPlanetLocater : public Upgrade
{
	UPGRADE(UpPlanetLocater)
		Presence **locater;
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Planet Locater";
		if (strcmp(station->build_type, "supbl")) return false;
		if (num) return false;
		starbucks = 4;
		buckazoids = 5;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		locater = new Presence *[gobgame->num_planets];
		for (int i = 0; i < gobgame->num_planets; i += 1) {
			locater[i] = new WedgeIndicator ( gobgame->planet[i], 50, i+1 );
			gobgame->add (locater[i] );
		}
	}
	void clear(Ship *oship, Ship *nship, GobPlayer *gs) {
		Upgrade::clear(oship, nship, gs);
		if (oship && locater) for (int i = 0; i < gobgame->num_planets; i += 1) {
			locater[i]->die();
		}
		locater = NULL;
		return;
	}
} planetlocater;

class UpHyperDynamo : public Upgrade
{
	UPGRADE(UpHyperDynamo)
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Chronotronic Dynamo (ancient artifact)";
		if (game->game_time / 1000 < 21 * 60) return false;
		starbucks = 720 / (game->game_time / (1000*60*10) - 1);
		//unavailable before 20 minutes
		//720 starbucks at 20 minutes
		//360 starbucks at 30 minutes
		//240 starbucks at 40 minutes
		//180 starbucks at 50 minutes
		//144 starbucks at 60 minutes
		//120 starbucks at 70 minutes
		//102 starbucks at 80 minutes
		// 90 starbucks at 90 minutes
		buckazoids = starbucks/2 + BASE_BZ;
		if (gs->starbucks < starbucks / 4) return false;
		if (ship->recharge_amount == 0) return false;
		if (num) return false;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
		ship->recharge_rate /= 2;
	}
} hyperdynamo;

class UpRoswellDevice : public Upgrade
{
	UPGRADE(UpRoswellDevice);
	bool update(Ship *ship, GobStation *station, GobPlayer *gs) {
		name = "Roswell Device";
		if (strcmp(station->build_type, "utwju")) return false;
		starbucks = 3;
		buckazoids = 10 + BASE_BZ;
		if (num) return false;
		return true;
	}
	void execute(Ship *ship, GobStation *station, GobPlayer *gs) {
	}
} roswelldevice;

/*
note to future coders:
in order for an upgrade to show up, it must be added to this list
anything added to this list MUST be added, in the same order, to
the enum in gup.h
*/
static Upgrade *_upgrade_list[] =
{
	&crewpod,
	&battery,
	&thrusters,
	&controljets,
	&dynamo,
	&sensor,
	&repairsystem,
	&supoxrange,
	&supoxdamage,
	&supoxblade,
	&orzmissile,
	&orzmarinespeed,
	&orzabsorption,
	&kohrahbladedamage,
	&kohrahbladespeed,
	&kohrahfirerange,
	&kohrahfiredamage,
	&utwigrange,
	&utwigdamage,
	&utwigrof,
	&utwigmask1,
	&utwigmask2,

	&androsynthbubblerate,
	&androsynthcomet,
	&ariloulaserdamage,
	&ariloulaserrange,
	&chenjesumainweapon,
	&chenjesuspecial,
	&earthlingwarhead,
	&earthlingmissile,
	&earthlinghoming,
	&earthlingdefense,
	&myconplasmashield,
	&urquanfusion,
	&spathigun,
	&syreengun,

	&divinefavor,
	&unholyaura,
	&defender,
	&defender2,
	&planetlocater,
	&hyperdynamo,
	&roswelldevice,
	NULL
};

Upgrade **upgrade_list = _upgrade_list;

UnholyAura::UnholyAura ( SpaceLocation * ship )
{
	focus = ship;
	angle = 0;
	level = 1;
}


void UnholyAura::animate (Frame *frame)
{
	Vector2 p = corner(focus->normal_pos());
	const int speed = 1500;
	int color = game->game_time % speed;
	if (color > speed/2) color = speed - color;
	color = tw_color ( color * 255 * 2 / speed, 0, 0);
	Vector2 r;
	r.y = space_zoom * 240;
	r.x = r.y * 1.0;
	double a = angle;
	line (frame->surface,
		p + r *(unit_vector(a +   0 * ANGLE_RATIO)),
		p + r * (unit_vector(a + 144 * ANGLE_RATIO)),
		color);
	a += 72 * ANGLE_RATIO;
	line (frame->surface,
		p + r * (unit_vector(a +   0 * ANGLE_RATIO)),
		p + r * (unit_vector(a + 144 * ANGLE_RATIO)),
		color);
	a += 72 * ANGLE_RATIO;
	line (frame->surface,
		p + r * (unit_vector(a +   0 * ANGLE_RATIO)),
		p + r * (unit_vector(a + 144 * ANGLE_RATIO)),
		color);
	a += 72 * ANGLE_RATIO;
	line (frame->surface,
		p + r * (unit_vector(a +   0 * ANGLE_RATIO)),
		p + r * (unit_vector(a + 144 * ANGLE_RATIO)),
		color);
	a += 72 * ANGLE_RATIO;
	line (frame->surface,
		p + r * (unit_vector(a +   0 * ANGLE_RATIO)),
		p + r * (unit_vector(a + 144 * ANGLE_RATIO)),
		color);
	frame->add_box(iround(p.x - r.x-1), iround(p.y - r.y-1), iround(r.x*2+3), iround(r.y*2+3));
}


void UnholyAura::calculate ()
{
	if (!focus->exists()) die();
	angle -= frame_time / 10.0;
	if (angle < 0) angle += 360;
	if (random(1700) < frame_time) {
		Query q;
		q.begin(focus, OBJECT_LAYERS, 666 * 2 * (0.8 + level * 0.2) * random(1.0));
		for (;q.current; q.next() ) {
			if (!(focus->sameTeam(q.current))) q.current->handle_damage(focus, 0, random(5+level));
		}
	}
}


/*
void GobRadar::animate_item ( SpaceLocation *item ) {
	int type = 0;
	int color = 0;
	double angle = 0;
	double radius = 0;
	double x, y;
	x = normalize(item->normal_x() - gx + X_MAX/2, X_MAX) - X_MAX/200+gx/2;
	x *= window->w / gw;
	y = normalize(item->normal_y() - gy + Y_MAX/2, Y_MAX) - Y_MAX/200+gy/2;
	y *= window->h / gh;
	TeamCode t = item->get_team();
	if (t == 0) color = palette_color[6];
	else if (t == team) color = palette_color[7];
	else color = palette_color[4];
	color = palette_color[7];
	if (item->isObject()) {
		SpaceSprite *sprite = ((SpaceObject*)item)->get_sprite();
		if (sprite) radius =
			sqrt(sprite->width() * sprite->width() + sprite->height() * sprite->height()) / 2 / gw;
		else return;
		if (radius < 5) radius = sqrt(radius * 5);
	}
	else if (item->isLine()) {
		type = 1;
		radius = (((SpaceLine*)item)->get_length()) / gw;
		angle = (((SpaceLine*)item)->get_angle());
	}

	if (type == 0) {
		circlefill ( window->surface,
			window->x + x,
			window->y + y,
			radius,
			color
		);
	}
	else if (type == 1) {
		line (  window->surface,
			window->x + x,
			window->y + y,
			window->x + x + cos(angle) * radius,
			window->y + y + sin(angle) * radius,
			color
		);
	}
}

void GobRadar::animate ( Frame * frame ) {
	Query q;
	if (!window->surface) return;

	window->lock();

	rectfill(window->surface,
		window->x, window->y,
		window->x + window->w,
		window->y + window->h,
		0
		);
	for (q.begin(0, 0, ALL_LAYERS, 999999999999);q.current;q.next()) {
		if (!q.current->exists()) continue;
		if (q.current->isInvisible())
			continue;
		animate_item(q.current);
	}
	q.end();

	window->unlock();
}

GobRadar::GobRadar() {
	attributes &= ~ATTRIB_SYNCHED;
	team = 0;
	gx = 0;
	gy = 0;
	gw = X_MAX;
	gh = Y_MAX;
	window = new VideoWindow();
	window->preinit();
}
*/
