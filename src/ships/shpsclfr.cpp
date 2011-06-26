/* $Id: shpsclfr.cpp,v 1.15 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"
#include "../melee/mview.h"
#include <math.h>

REGISTER_FILE

class ScloreSting;

class ScloreFrigate : public Ship
{
	public:
	public:

		double weaponRange;
		double weaponVelocity;
		int    weaponDamage;
		int    weaponArmour;
		double weaponRelativity;
		double weaponMultiplicity;
		int weaponAngleVariance;

		double specialRange;
		double specialVelocity;
		double specialRelativity;
		int    specialMaxPower;
		int    specialFiresBackwards;

		int stingIsCharging;
		double stingPowerLevel;

		int launchPoint;

	public:
		ScloreSting* Sting;
		ScloreFrigate(Vector2 opos, double angle, ShipData *data, unsigned int code);
		virtual void death();
	protected:
		void fireSting();
		void energizeSting(int Energy);
		void createSting();
		int addPowerToSting();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();

	private:

};

class ScloreShot : public Missile
{
	public:
	public:
		ScloreShot(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, double orelativity);
		int spriteToUse;
		virtual void calculate();
};

class ScloreSting : public Missile
{
	public:
	public:
		int launched;
		int powerLevel;
		ScloreFrigate* creator;
		ScloreSting(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, double orelativity);
		virtual void calculate();
		virtual void setPowerLevel(int powerLevel);
		void resetRange(void);
		virtual bool die(void);
		virtual void inflict_damage(SpaceObject* other);
		virtual int handle_damage(SpaceLocation* source, double normal, double direct = 0);
		virtual void death();
};

ScloreFrigate::ScloreFrigate(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponRelativity = tw_get_config_float("Weapon", "Relativity", 0);
	weaponMultiplicity = tw_get_config_int("Weapon", "Multiplicity" ,1);
	weaponAngleVariance = tw_get_config_int("Weapon", "AngleVariance", 0);

	specialRange = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialRelativity = tw_get_config_float("Special", "Relativity", 0);
	specialMaxPower = tw_get_config_int("Special", "MaxPower", 9);
	specialFiresBackwards = tw_get_config_int("Special", "FiresBackwards", 0);

	launchPoint = -1;
	stingIsCharging = 0;
	stingPowerLevel = 0.00001;
	Sting = NULL;

}


void ScloreFrigate::fireSting(void)
{
	STACKTRACE;
	Vector2 v, vr;
	if (Sting!=NULL) {
		Sting->range = specialRange;
		Sting->resetRange();
		Sting->range = specialRange;
		Sting->launched = TRUE;
		v = unit_vector(Sting->angle);
		v.x *= specialVelocity;
		v.y *= specialVelocity;
		if (specialFiresBackwards) {
			v.x = -v.x; v.y = -v.y;
		}
		vr = this->vel * specialRelativity;
		Sting->set_vel( v + vr );
		Sting->v = (v + vr).magnitude();
		Sting=NULL;				 //sting is released.
		this->play_sound2(data->sampleSpecial[1]);

	}
}


int ScloreFrigate::addPowerToSting(void)
{
	STACKTRACE;
	if (!Sting) return(FALSE);
	if (!Sting->exists()) {
		Sting = 0;
		return(FALSE);
	}
	if (Sting->powerLevel>=specialMaxPower) return(FALSE);
	Sting->damage_factor++;		 // a kludge
	Sting->armour++;
	Sting->powerLevel++;
	return(TRUE);
}


void ScloreFrigate::death()
{
	STACKTRACE;
	Ship::death();
								 // possible crash problem.
	if (Sting!=NULL && Sting->exists()) Sting->state = 0;
}


void ScloreFrigate::calculate()
{
	STACKTRACE;
	if (fire_special && fire_weapon) fireSting();
	if (Sting!=NULL) {
		Sting->resetRange();

		if (!Sting->exists())
			Sting = 0;
	}

	Ship::calculate();
}


int ScloreFrigate::activate_weapon()
{
	STACKTRACE;
	double deflection, deflectionRad;
	int i;
	if (fire_weapon && fire_special && this->batt>=1) {
		return(FALSE);			 //handeled in the calculate routine
	}
	for(i=0;i<weaponMultiplicity;i++) {
		deflection = (tw_random(weaponAngleVariance * 2 + 1))-weaponAngleVariance;
		deflectionRad = (double)deflection * ANGLE_RATIO;
		launchPoint = -launchPoint;
		game->add(new ScloreShot(size.y*launchPoint*(0.4), (size.y * 0.35),
			angle+deflectionRad, weaponVelocity, weaponDamage, weaponRange,
			weaponArmour, this, data->spriteWeapon, weaponRelativity));
	}
	return(TRUE);
}


int ScloreFrigate::activate_special()
{
	STACKTRACE;
	if (fire_weapon && fire_special) {
		return(FALSE);			 //handeled in the calculate routine
	}
	if (this->batt>0) {
		stingIsCharging = TRUE;
		if (this->Sting==NULL) createSting();
		else return(addPowerToSting());
	}
	else
		return(FALSE);
	return(FALSE);
}


void ScloreFrigate::createSting(void)
{
	STACKTRACE;
	if (Sting!=NULL) return;
	Sting = new ScloreSting(this->pos.x, this->pos.y, this->angle,
		specialVelocity, 1, specialRange, 1, this, data->spriteSpecial, 0);
	Sting->creator = this;
	game->add(Sting);
	Sting->pos = this->pos;
	Sting->setPowerLevel(1);
}


void ScloreSting::setPowerLevel(int powerLevel)
{
	STACKTRACE;
	if (powerLevel<0) powerLevel = 0;
	if (powerLevel>9) powerLevel = 9;
	this->damage_factor = powerLevel;
	this->armour = powerLevel;
}


ScloreShot::ScloreShot(double ox, double oy, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double orelativity)
:
Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship, osprite, orelativity),
spriteToUse(odamage)
{
	STACKTRACE;
	;
}


ScloreSting::ScloreSting(double ox, double oy, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double orelativity)
:
Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship, osprite, orelativity)
{
	STACKTRACE;
	launched = FALSE;
	powerLevel = 1;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void ScloreSting::calculate(void)
{
	STACKTRACE;

	if (!(creator && creator->exists())) {
		creator = 0;
	}

	int x, si;
	double dx, dy;
	x = iround(this->damage_factor - 1);
	if (x<0) x=0;
	if (x>8) x=8;
	if (!launched && creator!=NULL && creator->exists()) {
		dx = -cos(creator->angle) * scale_range(1.1);
		dy = -sin(creator->angle) * scale_range(1.1);
		this->pos.x = creator->pos.x + dx;
		this->pos.y = creator->pos.y + dy;
		this->changeDirection(creator->angle);
		this->v = creator->vel.magnitude();
		set_vel( creator->vel );
	}

	Missile::calculate();

	si = get_index(this->angle);
	if (!launched) si = (si + 32) % 64;
	sprite_index = si + (x * 64);

}


void ScloreSting::resetRange(void)
{
	STACKTRACE;
	this->d = 0;
	return;
}


bool ScloreSting::die(void)
{
	STACKTRACE;
	if (this->powerLevel<=1)
		return(Shot::die());
	state = 1;
	powerLevel--;
	this->damage_factor = powerLevel;
	this->armour = powerLevel;
	return(false);
}


int ScloreSting::handle_damage(SpaceLocation* source, double normal, double direct)
{
	STACKTRACE;
	int x;
	x = Shot::handle_damage(source, normal, direct);
	if (state==0 && powerLevel > 1) {
		state = 1;
		if (normal+direct>1.5) powerLevel--;
		this->damage_factor = powerLevel;
		this->armour = powerLevel;
	}
	return(x);
}


void ScloreSting::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	Shot::inflict_damage(other);
	if (other->isShip() || powerLevel <=1) {
		powerLevel = 0;
		return;
	}
	powerLevel--;				 //power level should be decremented in handle_damage??
	state = 1;
	this->damage_factor = powerLevel;
	this->armour = powerLevel;
	return;
}


void ScloreSting::death()
{
	STACKTRACE;

	Missile::death();

	if (!creator) return;
	if (!creator->exists()) return;
	if (creator->state == 0) return;
	if (creator->Sting == this) creator->Sting = NULL;
}


void ScloreShot::calculate()
{
	STACKTRACE;
	Missile::calculate();

}


REGISTER_SHIP(ScloreFrigate)
