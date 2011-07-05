/* $Id: shpoliha.cpp,v 1.14 2005/08/28 20:34:08 geomannl Exp $ */

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"
#include "../other/shippart.h"

class Olidandee : public BigShip
{
	public:
		double  weaponRange, weaponVelocity, weaponDamage, weaponArmour;

		bool shipmode;

	public:
		int Nspecial, max_special;

		Olidandee(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
		virtual void calculate();

		double specialRechargerange, laserPeriod;
		int specialCrewuse, specialBatt, specialRecrewtime;
		int laserColor, laserDamage, laserDrain;
		double laserRange;
	protected:

		virtual int activate_weapon();
		virtual int activate_special();

};

class OlidandeeArm : public BigShipPart
{
	public:
		double direction;
		int rel_sprite_index;
	public:
		OlidandeeArm(Olidandee *omother, Vector2 opos, double oangle, SpaceSprite *osprite,
			double odirection, int orel_sprite_index);

		virtual void calculate();
};

class OlidandeeHabitat : public SpaceObject
{
	public:
		Olidandee *mother;

		double armour, batt, battmax, t_c, t_recrew, t_l, t_laser;

	public:
		OlidandeeHabitat(Olidandee *omother, Vector2 opos, double oangle, SpaceSprite *osprite, double oarmour);
		virtual void calculate();

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage (SpaceLocation *source, double normal, double direct);
};

Olidandee::Olidandee(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
BigShip(opos,  shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialRechargerange  = scale_range(tw_get_config_float("Special", "RechargeRange", 0));
	specialCrewuse        = tw_get_config_int("Special", "CrewUse", 0);
	specialBatt           = tw_get_config_int("Special", "Batt", 0);
	specialRecrewtime     = tw_get_config_int("Special", "Recrewtime", 0);
	max_special            = tw_get_config_int("Special", "N", 0);
	Nspecial = 0;

	laserColor  = tw_get_config_int("Laser", "Color", 0);
	laserRange  = scale_range(tw_get_config_float("Laser", "Range", 0));
	laserDamage = tw_get_config_int("Laser", "Damage", 0);
	laserDrain  = tw_get_config_int("Laser", "Drain", 0);
	laserPeriod = tw_get_config_int("Laser", "Period", 0);

	//collide_flag_anyone = 0;
	//collide_flag_sameteam = 0;
	//collide_flag_sameship = 0;
	//attributes |= ATTRIB_UNDETECTABLE;

	Nparts = 2;
	parts = new BigShipPart* [Nparts];

	// and add the ship parts
	// left arm
	parts[0] = new OlidandeeArm(this, 0, -1, sprite, -1, 0);
	add(parts[0]);

	// right arm (with the yellow dot)
	parts[1] = new OlidandeeArm(this, 0, 1, sprite, 1, 64);
	add(parts[1]);

}


int Olidandee::activate_weapon()
{
	STACKTRACE;

	if (parts[0]) {
		Vector2 p;
		double a, da;

		da = 0.25 * PI;

		a = parts[0]->angle - 0.5*PI - da;
		p = rotate(Vector2(-30,0), -parts[0]->relangle);

		add(new Shot(this, p, a,
			weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon, 1.0));

		a = parts[1]->angle + 0.5*PI + da;
		p = rotate(Vector2(30,0), -parts[1]->relangle);

		add(new Shot(this, p, a,
			weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon, 1.0));
	}

	return TRUE;
}


int Olidandee::activate_special()
{
	STACKTRACE;

	if (crew > specialCrewuse && Nspecial < 5) {
		// deploy a "habitat"
		add( new OlidandeeHabitat(this, pos - 50*unit_vector(angle), 0, data->spriteSpecial, specialCrewuse) );

		++Nspecial;

		crew -= specialCrewuse;

		update_panel = true;

		return true;

	} else
	return false;
}


void Olidandee::calculate()
{
	STACKTRACE;
	BigShip::calculate();

	if (!state)
		return;

}


OlidandeeArm::OlidandeeArm(Olidandee *omother, Vector2 orelpos, double orelangle,
SpaceSprite *osprite, double odirection, int orel_sprite_index)
:
BigShipPart(omother, orelpos, orelangle, osprite, 0)
{
	STACKTRACE;
	direction = odirection;
	rel_sprite_index = orel_sprite_index;

	layer = LAYER_SHIPS;
	collide_flag_sameship = 0;

	collide_flag_sameship = 0;

	relangle = 0;
}


void OlidandeeArm::calculate()
{
	STACKTRACE;
	if (!(owner && owner->exists())) {
		owner = 0;
		state = 0;
		return;
	}

	// The angle depends slightly on velocity
	double da;
	da = owner->vel.length() / owner->speed_max;
	if (da > 1)
		da = 1;
	da *= -0.5 * PI;
	da += 0.25 * PI;
	da *= direction;

	da = da - relangle;			 // what should be added to relangle to achieve new position
	while (da > PI)     da -= PI2;
	while (da < -PI)    da += PI2;

	double damax;
	damax = 0.5 * PI * frame_time * 1E-3;

	if (da > damax)
		da = damax;

	if (da < -damax)
		da = -damax;

	relangle += da;

	BigShipPart::calculate();

	sprite_index = get_index(angle);

	sprite_index += rel_sprite_index;

	if (!state)
		return;

}


OlidandeeHabitat::OlidandeeHabitat(Olidandee *omother,
Vector2 opos, double oangle, SpaceSprite *osprite,
double oarmour)
:
SpaceObject(omother, opos, oangle, osprite)
{
	STACKTRACE;
	mother = omother;

	layer = LAYER_SPECIAL;

	//collide_flag_anyone = ALL_LAYERS;
	//collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = bit(LAYER_SHIPS) | bit(LAYER_SHOTS);

	isblockingweapons = false;

	mass = 0.001;

	armour = oarmour;

	battmax = mother->specialBatt;
	batt = battmax;

	t_c = 0;
	t_recrew = mother->specialRecrewtime;

	t_l = 0;
	t_laser = mother->laserPeriod;
}


void OlidandeeHabitat::calculate()
{
	STACKTRACE;

	if (!(mother && mother->exists())) {
		mother = 0;
		state = 0;
		//--mother->Nspecial; no use in this case... only if it dies normally
		return;
	}

	double dt;

	dt = frame_time * 1E-3;

	vel *= (1 - 0.1*dt);

	t_c += dt;

								 //batt > 2 &&
	if (t_c > t_recrew && armour < mother->crew_max) {
		++armour;
		t_c -= t_recrew;
		//batt -= 2;
	}

	// check for a small object closeby
	// copied from arilou
	double r = 99999;
	SpaceObject *o = 0;
	Query a;

	for (a.begin(this, OBJECT_LAYERS, mother->laserRange); a.current; a.next()) {
		if (!a.current->isObject())
			continue;

		if ((distance(a.current) < r) && !a.current->isInvisible()
		&& a.current->ship != mother) {
			o = a.currento;
			r = distance(o);
		}
	}

	// activate the laser ?

	if (t_l < t_laser)
		t_l += dt;

	if (o && t_l >= t_laser && batt >= mother->laserDrain) {
		batt -= mother->laserDrain;
		if (batt < 0)
			batt = 0;
		t_l -= t_laser;

		angle = trajectory_angle(o);
		add(new Laser(this, angle,
			pallete_color[mother->laserColor], mother->laserRange, mother->laserDamage,
			iround(t_laser * 1E3), this, Vector2(0,15), false));
		//		Laser::Laser(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage,
		//  int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle)

	}

	// recharge ?
	if (distance(mother) < mother->specialRechargerange) {
		if (batt < battmax) {	 // this check prevents loops (which are unstable :( )
			double b;
			batt += mother->batt;
			b = batt - battmax;

			if (b > 0) {
				mother->batt = b;// surplus fuel stays on the mothership
				batt = battmax;
			}
		}
	}

	SpaceObject::calculate();

}


// handle damage ?
void OlidandeeHabitat::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!state)
		return;

	if (other->ship == mother) {
		other->handle_damage(this, -armour, 0);
		state = 0;
		--mother->Nspecial;
	} else {
		other->handle_damage(this, 1, 0);
		--armour;

		if (armour < 0) {
			state = 0;
			--mother->Nspecial;
		}
	}
}


int OlidandeeHabitat::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (!state)
		return false;

	armour -= (normal + direct);

	if (armour < 0) {
		state = 0;
		--mother->Nspecial;
	}

	return true;
}


REGISTER_SHIP(Olidandee)
