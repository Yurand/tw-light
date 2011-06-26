/* $Id: shpfrebo.cpp,v 1.1 2006/01/29 16:14:34 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

#include "../melee/mmain.h"

#define SMALL_BOOMERANG 1
#define MEDIUM_BOOMERANG 2
#define LARGE_BOOMERANG 3

/* Copy of the Kahr Boomerang
 */

class FreinBoomerang;
class FreinSmall;
class FreinMedium;
class FreinLarge;

class FreinBoomerang : public Ship
{
	public:

		int          weaponChoice;

		double       weapon1Range;
		double       weapon1Turn;
		double       weapon1Velocity;
		int          weapon1Drain;
		int          weapon1Damage;
		int          weapon1Armour;
		int          weapon1Rate;

		double       weapon2Range;
		double       weapon2Turn;
		double       weapon2Velocity;
		int          weapon2Drain;
		int          weapon2Damage;
		int          weapon2Armour;
		int          weapon2Rate;

		double       weapon3Range;
		double       weapon3Turn;
		double       weapon3Velocity;
		int          weapon3Drain;
		int          weapon3Damage;
		int          weapon3Armour;
		int          weapon3Rate;

		FreinLarge    *boomerangL;

	public:

		int                 num_medium_boomerangs;

		FreinBoomerang(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void calculate_fire_special();
};

class FreinSmall : public Shot
{
	public:
	public:
		FreinSmall(double ox,double oy,double oangle, double ov, double oturn,
			int odamage, double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, int ofcount, int ofsize);

		int frame;
		int frame_step;
		int frame_size;
		int frame_count;
		double maxspeed;
		double srange;
		int returning;
		double turning;
		int turned;

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

class FreinMedium : public Shot
{
	public:
	public:
		FreinMedium(double ox,double oy,double oangle, double ov, double oturn,
			int odamage, double orange, int oarmour, FreinBoomerang *oship,
			SpaceSprite *osprite, int ofcount, int ofsize);
		virtual void death();

		int frame;
		int frame_step;
		int frame_size;
		int frame_count;
		double maxspeed;
		double srange;
		int returning;
		double turning;
		int turned;

		FreinBoomerang        *Freinship;

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

class FreinLarge : public Shot
{
	public:
	public:
		FreinLarge(double ox,double oy,double oangle, double ov, double oturn,
			int odamage, double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite, int ofcount, int ofsize);

		int frame;
		int frame_step;
		int frame_size;
		int frame_count;
		double srange;
		int returning;
		double turning;
		double maxspeed;
		int turned;

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

FreinBoomerang::FreinBoomerang(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weapon1Range    = scale_range(tw_get_config_float("WeaponS", "Range", 0));
	weapon1Turn     = scale_turning(tw_get_config_float("WeaponS","Turn", 0));
	weapon1Velocity = scale_velocity(tw_get_config_float("WeaponS", "Velocity", 0));
	weapon1Drain    = tw_get_config_int("WeaponS", "Drain", 0);
	weapon1Damage   = tw_get_config_int("WeaponS", "Damage", 0);
	weapon1Armour   = tw_get_config_int("WeaponS", "Armour", 0);
	weapon1Rate     = scale_frames(tw_get_config_float("WeaponS", "Rate", 0));

	weapon2Range    = scale_range(tw_get_config_float("WeaponM", "Range", 0));
	weapon2Turn     = scale_turning(tw_get_config_float("WeaponM","Turn", 0));
	weapon2Velocity = scale_velocity(tw_get_config_float("WeaponM", "Velocity", 0));
	weapon2Drain    = tw_get_config_int("WeaponM", "Drain", 0);
	weapon2Damage   = tw_get_config_int("WeaponM", "Damage", 0);
	weapon2Armour   = tw_get_config_int("WeaponM", "Armour", 0);
	weapon2Rate     = scale_frames(tw_get_config_float("WeaponM", "Rate", 0));

	weapon3Range    = scale_range(tw_get_config_float("WeaponL", "Range", 0));
	weapon3Turn     = scale_turning(tw_get_config_float("WeaponL","Turn", 0));
	weapon3Velocity = scale_velocity(tw_get_config_float("WeaponL", "Velocity", 0));
	weapon3Drain    = tw_get_config_int("WeaponL", "Drain", 0);
	weapon3Damage   = tw_get_config_int("WeaponL", "Damage", 0);
	weapon3Armour   = tw_get_config_int("WeaponL", "Armour", 0);
	weapon3Rate     = scale_frames(tw_get_config_float("WeaponL", "Rate", 0));

	weapon_rate    = weapon1Rate;
	weapon_drain   = weapon1Drain;
	weaponChoice   = SMALL_BOOMERANG;
	num_medium_boomerangs = 0;
	boomerangL    = NULL;
}


int FreinBoomerang::activate_weapon()
{
	STACKTRACE;
	int chance;
	int answer = FALSE;
	if (weaponChoice == SMALL_BOOMERANG) {
		add(new FreinSmall( 0.0, 0.0 , angle,
			weapon1Velocity, weapon1Turn, weapon1Damage, weapon1Range,
			weapon1Armour, this, data->spriteWeapon, 64, 3));
		weapon1Turn = (weapon1Turn * -1.0);
		answer = TRUE;
	}
	if (weaponChoice == MEDIUM_BOOMERANG) {
		if (num_medium_boomerangs < 4) {
			add(new FreinMedium( 0.0, 0.0, angle, weapon2Velocity,
				(weapon2Turn*-1.0), weapon2Damage, weapon2Range, weapon2Armour,
				this, data->spriteSpecial, 32, 4));
			num_medium_boomerangs += 1;
			answer = TRUE;
		} else answer = FALSE;
	}
	if (weaponChoice == LARGE_BOOMERANG) {
		chance = tw_random(2);
		if (chance) weapon3Turn = weapon3Turn * -1;
		if (boomerangL == NULL) {
			boomerangL = new FreinLarge(0.0, 0.0, angle, weapon3Velocity,
				weapon3Turn, weapon3Damage, weapon3Range, weapon3Armour,
				this, data->spriteExtra, 64, 3);
			add(boomerangL);
			answer = TRUE;
		} else answer = FALSE;
	}
	return answer;
}


int FreinBoomerang::activate_special()
{
	STACKTRACE;
	int answer;
	if (weaponChoice == SMALL_BOOMERANG) {
		weapon_drain = weapon2Drain;
		weapon_rate = weapon2Rate;
		weaponChoice = MEDIUM_BOOMERANG;
		answer = TRUE;
	}
	else if (weaponChoice == MEDIUM_BOOMERANG) {
		weapon_drain = weapon3Drain;
		weapon_rate = weapon3Rate;
		weaponChoice = LARGE_BOOMERANG;
		answer = TRUE;
	}
	else if (weaponChoice == LARGE_BOOMERANG) {
		weapon_drain = weapon1Drain;
		weapon_rate = weapon1Rate;
		weaponChoice = SMALL_BOOMERANG;
		answer = TRUE;
	} else answer = FALSE;
	return (answer);
}


void FreinBoomerang::calculate()
{
	STACKTRACE;
	if ((boomerangL != NULL) && (!boomerangL->exists()))
		boomerangL=NULL;
	Ship::calculate();
	if ((!fire_weapon) && ((boomerangL != NULL))
	&& (!(boomerangL->returning))) {
		boomerangL->returning = TRUE;
		if ((turn_left) && (boomerangL->turning > 0.0))
			boomerangL->turning = ((boomerangL->turning) * -1.0);
		else if ((turn_right) && (boomerangL->turning < 0.0))
			boomerangL->turning = ((boomerangL->turning) * -1.0);
	}
}


void FreinBoomerang::calculate_fire_special()
{
	STACKTRACE;
	if (weaponChoice == SMALL_BOOMERANG) {
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(1), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(2), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(3), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(4), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(5), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(7), this->spritePanel->get_bitmap(6), 36, 0, 36, 0, 19, 30);
	}
	else if (weaponChoice == MEDIUM_BOOMERANG) {
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(1), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(2), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(3), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(4), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(5), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(8), this->spritePanel->get_bitmap(6), 36, 0, 36, 0, 19, 30);
	}
	else if (weaponChoice == LARGE_BOOMERANG) {
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(1), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(2), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(3), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(4), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(5), 36, 0, 36, 0, 19, 30);
		tw_blit(this->spritePanel->get_bitmap(9), this->spritePanel->get_bitmap(6), 36, 0, 36, 0, 19, 30);
	}
	Ship::calculate_fire_special();
}


FreinSmall::FreinSmall(double ox,double oy,double oangle, double ov,
double oturn, int odamage, double orange, int oarmour, Ship *oship,
SpaceSprite *osprite, int ofcount, int ofsize) :
Shot(oship, Vector2(ox,oy), oangle, ov, odamage, -1.0 , oarmour, oship, osprite)

{
	STACKTRACE;
	mass = 0.25;
	frame = 0;
	frame_step = 0;
	frame_size = ofsize;
	frame_count = ofcount;
	maxspeed = ov;
	srange = orange;
	returning = FALSE;
	turning = oturn;
	turned = FALSE;
}


void FreinSmall::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		state = 0;
		ship = 0;
		return;
	}
	if (distance(ship) > srange)
		returning = TRUE;
	if (returning) {
		collide_flag_sameship |= bit(LAYER_SHIPS);
		if (normalize(normalize(trajectory_angle(ship), PI2) - normalize(angle,PI2),PI2) > (15 * ANGLE_RATIO))
			angle += turning * frame_time;
		else turned = TRUE;
		if (turned) angle = trajectory_angle(ship);
		double v, alpha;
		alpha = atan(vel);
		alpha = alpha + PI;
		alpha = normalize(alpha, PI2);

		v = maxspeed;

		if ((fabs(min_delta(alpha, angle, PI2)) > 0.5))
			vel = unit_vector(angle) * v;
	}
	Shot::calculate();
	frame_step += frame_time;
	if (frame_step >= frame_size) {
		frame_step -= frame_size;
		if (turning > 0) frame--;
		else frame++;
		if (frame == frame_count)
			frame = 0;
		if (frame == -1)
			frame = (frame_count - 1);
		sprite_index=frame;
	}
}


void FreinSmall::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other == ship) {
		state = 0;
		mass = 0;
	}
	else if (other->isShip()) {
		angle += PI;
		returning = TRUE;
		turned = FALSE;
		SpaceObject::inflict_damage(other);
	} else Shot::inflict_damage(other);
}


FreinMedium::FreinMedium(double ox,double oy,double oangle, double ov,
double oturn, int odamage, double orange, int oarmour, FreinBoomerang *oship,
SpaceSprite *osprite, int ofcount, int ofsize) :
Shot(oship, Vector2(ox,oy), oangle, ov, odamage, -1.0 , oarmour, oship, osprite)

{
	STACKTRACE;
	mass = 3.0;
	frame = 0;
	frame_step = 0;
	frame_size = ofsize;
	frame_count = ofcount;
	maxspeed = ov;
	srange = orange;
	returning = FALSE;
	turning = oturn;
	turned = FALSE;

	Freinship = oship;
}


void FreinMedium::death()
{
	STACKTRACE;
	if (Freinship) (Freinship)->num_medium_boomerangs -= 1;
	Shot::death();
}


void FreinMedium::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		state = 0;
		ship = 0;
		return;
	}
	if (!(Freinship && Freinship->exists()))
		Freinship = 0;

	if (distance(ship) > srange)
		returning = TRUE;
	if (returning) {
		collide_flag_sameship |= bit(LAYER_SHIPS);
		if (normalize(normalize(trajectory_angle(ship), PI2) - normalize(angle,PI2),PI2) > (15*ANGLE_RATIO))
			angle += turning * frame_time;
		else turned = TRUE;
		if (turned) angle = trajectory_angle(ship);
		double v, alpha;
		alpha = atan(vel);
		alpha = alpha + PI;
		alpha = normalize(alpha, PI2);

		v = maxspeed;

		if ((fabs(min_delta(alpha, angle, PI2)) > 0.5))
			vel = unit_vector(angle) * v;
	}
	Shot::calculate();
	frame_step += frame_time;
	if (frame_step >= frame_size) {
		frame_step -= frame_size;
		if (turning > 0) frame--;
		else frame++;
		if (frame == frame_count)
			frame = 0;
		if (frame == -1)
			frame = (frame_count - 1);
		sprite_index=frame;
	}
}


void FreinMedium::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other == ship) {
		state = 0;
		mass = 0;
	}
	else if (other->isShip()) {
		angle += PI;
		returning = TRUE;
		turned = FALSE;
		SpaceObject::inflict_damage(other);
	} else Shot::inflict_damage(other);
}


FreinLarge::FreinLarge(double ox,double oy,double oangle, double ov,
double oturn, int odamage, double orange, int oarmour, Ship *oship,
SpaceSprite *osprite, int ofcount, int ofsize) :
Shot(oship, Vector2(ox,oy), oangle, ov, odamage, -1.0 , oarmour, oship, osprite)

{
	STACKTRACE;
	mass =5.0;
	frame = 0;
	frame_step = 0;
	frame_size = ofsize;
	frame_count = ofcount;
	maxspeed = ov;
	srange = orange;
	returning = FALSE;
	turning = oturn;
	turned = FALSE;
}


void FreinLarge::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		state = 0;
		ship = 0;
		return;
	}
	if (returning) {
		collide_flag_sameship |= bit(LAYER_SHIPS);
		if (normalize(normalize(trajectory_angle(ship), PI2) - normalize(angle,PI2),PI2) > (15 * ANGLE_RATIO))
			angle += turning * frame_time;
		else turned = TRUE;
		if (turned) angle = trajectory_angle(ship);
		double v, alpha;
		alpha = atan(vel);
		alpha = alpha + PI;
		alpha = normalize(alpha, PI2);

		v = maxspeed;

		if ((fabs(min_delta(alpha, angle, PI2)) > 0.5 * ANGLE_RATIO))
			vel = unit_vector(angle) * v;
	}
	Shot::calculate();
	frame_step += frame_time;
	if (frame_step >= frame_size) {
		frame_step -= frame_size;
		if (turning > 0) frame--;
		else frame++;
		if (frame == frame_count)
			frame = 0;
		if (frame == -1)
			frame = (frame_count - 1);
		sprite_index=frame;
	}
}


void FreinLarge::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other == ship) {
		state = 0;
		mass = 0;
	}
	else if (other->mass > 0) {
		angle += PI;
		returning = TRUE;
		turned = FALSE;
		SpaceObject::inflict_damage(other);
	} else Shot::inflict_damage(other);
}


int FreinLarge::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	return Shot::handle_damage(source, 0, 0);
}


REGISTER_SHIP(FreinBoomerang)
