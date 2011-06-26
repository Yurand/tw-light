/* $Id: shpktesa.cpp,v 1.9 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

# define NORMAL_SPEED 0
# define TURBO_SPEED 1
# define SUPER_SPEED 2
# define HYPER_SPEED 3

class KterbiSaber : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	int          boostlevel;
	int          normal_recharge;
	double       normal_speed;
	double       normal_acc;
	int          normal_rate;
	int          normal_rof;
	double       normal_turning;

	public:
		KterbiSaber(Vector2 opos, double shipAngle, ShipData *data, unsigned int code);

		virtual void calculate_fire_weapon();
		virtual void calculate_fire_special();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual int handle_speed_loss(SpaceLocation *source, int normal);

};

class KterbiIonBlast : public AnimatedShot
{

	int min_damage;

	public:
		KterbiIonBlast(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
			int ofcount, int ofsize);

		virtual void calculate();
};

KterbiSaber::KterbiSaber(Vector2 opos, double shipAngle,
ShipData *data, unsigned int code) :
Ship(opos, shipAngle, data, code)
{
	STACKTRACE;
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	boostlevel     = NORMAL_SPEED;
	normal_recharge= recharge_amount;
	normal_rate    = recharge_rate;
	normal_speed   = speed_max;
	normal_acc     = accel_rate;
	normal_rof     = weapon_rate;
	normal_turning = turn_rate;
	collide_flag_sameship |= bit(LAYER_SHOTS);
}


void KterbiSaber::calculate_fire_weapon()
{
	STACKTRACE;
	if ((batt < weapon_drain) && (crew > 1)) {
		crew--;
		batt += batt_max;
	}
	Ship::calculate_fire_weapon();
	if ((batt > batt_max) && (crew < crew_max)) {
		crew++;
		batt -= batt_max;
	}
	if (batt > batt_max)
		batt = batt_max;
}


void KterbiSaber::calculate_fire_special()
{
	STACKTRACE;
	if ((batt < special_drain) && (crew > 1)) {
		crew--;
		batt += batt_max;
	}
	Ship::calculate_fire_special();
	if ((batt > batt_max) && (crew < crew_max)) {
		crew++;
		batt -= batt_max;
	}
	if (batt > batt_max)
		batt = batt_max;
}


int KterbiSaber::activate_weapon()
{
	STACKTRACE;
	add(new KterbiIonBlast(Vector2(-size.x*.35, size.y/3),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour, this,
		data->spriteWeapon, 10 ,1));
	add(new KterbiIonBlast(Vector2(size.x*.35, size.y/3),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour, this,
		data->spriteWeapon, 10 ,1));
	return(TRUE);
}


int KterbiSaber::activate_special()
{
	STACKTRACE;
	if (boostlevel < 3) {
		boostlevel++;
		sound.stop(data->sampleExtra[1]);
		sound.play(data->sampleExtra[1]);
		if (boostlevel == TURBO_SPEED) {
			speed_max = normal_speed * 1.5;
			recharge_step = 0;
			recharge_amount = 0;
			accel_rate = normal_acc * 1.5;
			turn_rate = normal_turning * 1.33;
			weapon_rate = int(normal_rof / 1.33);
		}
		else if (boostlevel == SUPER_SPEED) {
			recharge_amount = -normal_recharge;
			recharge_step = 0;
			speed_max = normal_speed * 2;
			accel_rate = normal_acc * 2;
			turn_rate = normal_turning * 1/66;
			weapon_rate = int(normal_rof / 1.66);
		}
		else if (boostlevel == HYPER_SPEED) {
			recharge_rate = (normal_rate/2);
			speed_max = normal_speed * 2.5;
			recharge_step= 0;
			accel_rate = normal_acc * 2.5;
			turn_rate = normal_turning * 2;
			weapon_rate = int(normal_rof / 2);
		}
		return (TRUE);
	}
	else return (FALSE);
}


void KterbiSaber::calculate()
{
	STACKTRACE;
	if (((!fire_special) || ((crew == 1) && (batt == 0)) || (crew_max <= 2)) && (boostlevel != 0)) {
		boostlevel = 0;
		recharge_amount = normal_recharge;
		recharge_rate = normal_rate;
		speed_max = normal_speed;
		accel_rate = normal_acc;
		turn_rate = normal_turning;
		weapon_rate = normal_rof;
		recharge_step = 0;
	}

	if (boostlevel >= 1) {
		int chance;
		chance = random() % 10000;
		if (chance < (3 * frame_time)) {
			crew_max -= 2;
			if (crew > crew_max)
				crew = crew_max;
			blit(spritePanel->get_bitmap(7), spritePanel->get_bitmap(0),0,0,3,iround(50 - (crew_max)),9,4);
			ship->update_panel = TRUE;
			sound.stop(data->sampleExtra[0]);
			sound.play(data->sampleExtra[0]);

		}
	}

	Ship::calculate();

	if ((batt < 0) && (crew!=1)) {
		batt += batt_max;
		crew--;
	} else if (batt < 0)
	batt=0;

	if ((crew!=crew_max) && (batt >= batt_max)) {
		batt -= batt_max;
		crew++;
	}
}


int KterbiSaber::handle_speed_loss(SpaceLocation *source, int normal)
{
	STACKTRACE;
	speed_max = normal_speed;
	accel_rate = normal_acc;
	double r = Ship::handle_speed_loss(source, normal);
	normal_speed = speed_max;
	normal_acc = accel_rate;
	if (boostlevel == TURBO_SPEED) {
		speed_max = normal_speed * 1.5;
		accel_rate = normal_acc * 2;
	}
	else if (boostlevel == SUPER_SPEED) {
		speed_max = normal_speed * 2;
		accel_rate = normal_acc * 2;
	}
	else if (boostlevel == HYPER_SPEED) {
		speed_max = normal_speed * 2.5;
		accel_rate = normal_acc * 2.5;
	}
	return iround(r);
}


KterbiIonBlast::KterbiIonBlast(Vector2 opos, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship,
SpaceSprite *osprite, int ofcount, int ofsize) :
AnimatedShot(oship, opos, oangle, ov, odamage, orange, oarmour, oship,
osprite, ofcount, ofsize,1)
{
	STACKTRACE;
	min_damage=odamage;
}


void KterbiIonBlast::calculate()
{
	STACKTRACE;
	AnimatedShot::calculate();
	if (d < (range/2))
		damage_factor = min_damage;
	else
		damage_factor = min_damage * 2;
}


REGISTER_SHIP(KterbiSaber)
