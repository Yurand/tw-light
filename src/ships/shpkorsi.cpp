/* $Id: shpkorsi.cpp,v 1.3 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

#include "../melee/mview.h"
#include "../melee/mcbodies.h"

#define WEAPON_MASS .25

class KorvianSidekick : public Ship
{
	public:
	public:

		double absorption;		 //added for gob

		double       Sidekick_turn_step;

		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;

		double       specialAccelRate;
		double       specialSpeedMax;
		int          specialHotspotRate;
		int          specialArmour;

		double       SidekickAngle;

		int          recoil;
		int          recoil_rate;
		int          recoil_range;
		double       Sidekick_turn_rate;

		SpaceObject* tugger;
		double tuggerDistance;
		double maxTuggerSpeed;
		double k;

		KorvianSidekick(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual void calculate();
		virtual void animate(Frame *space);

		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate_hotspots();
};

class SidekickLaser : public PointLaser
{
	public:
	public:
		SidekickLaser(SpaceLocation *creator,
			SpaceLocation *lsource, SpaceObject *ltarget, Vector2 rel_pos = Vector2(0,0)) ;
		virtual void inflict_damage(SpaceObject *other);
};

SidekickLaser::SidekickLaser(SpaceLocation *creator,
SpaceLocation *lsource, SpaceObject *ltarget, Vector2 rel_pos) :
PointLaser(creator, tw_get_palete_color(15), 0, 1, lsource, ltarget, rel_pos)
{
	STACKTRACE;
}


void SidekickLaser::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

}


class SidekickMissile : public Missile
{
	public:
	public:
		SidekickMissile(double oangle, double ov, int odamage, double orange,
			int oarmour, Ship *oship, SpaceSprite *osprite);
		virtual void inflict_damage(SpaceObject *other);
};

KorvianSidekick::KorvianSidekick(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	absorption = 0;

	collide_flag_sameship = bit(LAYER_SPECIAL);
	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);

	specialSpeedMax    = scale_velocity(tw_get_config_float("Special", "SpeedMax", 0));
	double raw_specialHotspotRate = tw_get_config_int("Special", "HotspotRate", 0);
	specialHotspotRate = scale_frames(raw_specialHotspotRate);
	specialAccelRate   = scale_acceleration(tw_get_config_float("Special", "AccelRate", 0), raw_specialHotspotRate);
	specialArmour      = tw_get_config_int("Special", "Armour", 0);

	SidekickAngle = 0.0;
	recoil = 0;

	recoil_rate = scale_frames(tw_get_config_float("Turret", "RecoilRate",0));
	if (recoil_rate > weapon_rate) recoil_rate = weapon_rate;
	recoil_range = tw_get_config_int("Turret", "Recoil", 0);
	if (recoil_range < 0) recoil_range = 0;
	Sidekick_turn_rate = scale_turning(tw_get_config_float("Turret", "TurnRate", 0));

	Sidekick_turn_step = 0;

	tugger = NULL;
	tuggerDistance = scale_range(tw_get_config_float("Special", "TowDistance", 0));
	maxTuggerSpeed = specialSpeedMax;
	k = .01;					 //'springiness' constant of the Korvian's special
}


int KorvianSidekick::activate_weapon()
{
	STACKTRACE;
	if (fire_special)
		return(FALSE);
	add(new SidekickMissile(
		SidekickAngle, weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon));

	if (mass > 0)
		accelerate(this, SidekickAngle, -WEAPON_MASS / mass * weaponVelocity, MAX_SPEED);
	recoil += recoil_rate;

	return(TRUE);
}


int KorvianSidekick::activate_special()
{
	STACKTRACE;
	SpaceObject *o;
	double closestDistance = tuggerDistance, d;
	int foundTugger = FALSE;
	Query a;
	if (tugger == NULL) {
		for (a.begin(this, bit(LAYER_SHIPS) | bit(LAYER_SHOTS) | bit(LAYER_SPECIAL) | bit(LAYER_CBODIES),
		tuggerDistance); a.current; a.next()) {
			if (!a.current->isObject())
				continue;
			o = a.currento;
			if ( (!o->isInvisible()) && (o->collide_flag_anyone & bit(LAYER_LINES))
			&& abs(get_vel() - o->get_vel()) < maxTuggerSpeed) {
				d = distance(o);
				if (d < closestDistance) {
					foundTugger = TRUE;
					closestDistance = d;
					tugger = o;
				}
			}
		}
		if (tugger == NULL)
			return FALSE;
		//play_sound2(data->sampleExtra[0]);
	} else {
		tugger = NULL;
		//play_sound2(data->sampleExtra[0]);
	}
	return TRUE;
	/*  if (turn_left && (recoil<=0))
		Sidekick_turn_step -= frame_time * Sidekick_turn_rate;
	  if (turn_right && (recoil<=0))
		Sidekick_turn_step += frame_time * Sidekick_turn_rate;

	  while (fabs(Sidekick_turn_step) > (PI2/64)/2) {
		if (Sidekick_turn_step < 0.0 ) {
		  SidekickAngle -= (PI2/64);
		  Sidekick_turn_step += (PI2/64); } else {
		  SidekickAngle += (PI2/64);
		  Sidekick_turn_step -= (PI2/64); }
	  }
	  SidekickAngle = normalize(SidekickAngle, PI2);

	  return(FALSE);*/
}


void KorvianSidekick::calculate_turn_left()
{
	STACKTRACE;
	if (turn_left /*&& (recoil<=0)*/)
		Sidekick_turn_step -= frame_time * Sidekick_turn_rate;
	//Ship::calculate_turn_left();
}


void KorvianSidekick::calculate_turn_right()
{
	STACKTRACE;
	if (turn_right /*&& (recoil<=0)*/)
		Sidekick_turn_step += frame_time * Sidekick_turn_rate;
	//Ship::calculate_turn_right();
}


void KorvianSidekick::calculate()
{
	STACKTRACE;
	if (tugger != NULL && !tugger->exists()) tugger = NULL;
	if (tugger != NULL && tugger->exists()) {
		double x = distance(tugger)-tuggerDistance;
		//Vector2 v = tugger->pos-pos;
		if (x > 0) {
			if (mass > 0)
				accelerate(tugger, trajectory_angle(tugger), k*x / mass, abs(tugger->get_vel()));

			if (tugger->mass > 0)
				tugger->accelerate(this, tugger->trajectory_angle(this), k*x / tugger->mass, MAX_SPEED);
		}
	}
	if (tugger != NULL && tugger->exists() && !tugger->isInvisible()) {
		game->add(new SidekickLaser(this, this, tugger));
	}
	turn_right = TRUE;
	Ship::calculate_turn_right();
	while (fabs(Sidekick_turn_step) > (PI2/64)/2) {
		if (Sidekick_turn_step < 0.0 ) {
			SidekickAngle -= (PI2/64);
			Sidekick_turn_step += (PI2/64);
		} else {
			SidekickAngle += (PI2/64);
			Sidekick_turn_step -= (PI2/64);
		}
	}
	SidekickAngle = normalize(SidekickAngle, PI2);

	if (recoil > 0) {
		recoil -= frame_time;
		if (recoil < 0) recoil = 0;
	}
	Ship::calculate();
}


void KorvianSidekick::animate(Frame *space)
{
	STACKTRACE;
	double rec;
	int Sidekick_index;

	/*
	ra = normalize(angle + SidekickAngle, PI2);
	Sidekick_index = get_index(ra);
	bmp = data->spriteShip->get_bitmap(64);
	clear_to_color( bmp, tw_makecol(255,0,255));
	sprite->draw(0, 0, sprite_index, bmp);
	rec = (double)recoil/recoil_rate;
	rec *= rec * recoil_range;
	data->spriteExtra->draw( -cos(ra)*rec, -sin(ra)*rec, Sidekick_index, bmp);
	data->spriteShip->animate(x,y,64, space);
	*/

	Ship::animate(space);

	Sidekick_index = get_index(SidekickAngle);
	rec = (double)recoil/recoil_rate;
	rec *= rec * recoil_range;
								 //  x - cos(angle+SidekickAngle)*rec, y - sin(angle+SidekickAngle)*rec
	data->spriteExtra->animate( pos - rec*unit_vector(SidekickAngle),
		Sidekick_index, space);

	return;
}


void KorvianSidekick::calculate_hotspots()
{
	STACKTRACE;

}


SidekickMissile::SidekickMissile(double oangle, double ov, int odamage, double orange,
int oarmour, Ship *oship, SpaceSprite *osprite) :
Missile(oship, Vector2(0.0, 0.0), oangle, ov, odamage, orange, oarmour, oship,
osprite)
{
	STACKTRACE;
	//  x += cos(angle) * 30.0;
	//  y += sin(angle) * 30.0;
	pos += 30.0 * unit_vector(angle);

	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
	/*add(new Animation(this, pos,
	  data->spriteExtraExplosion, 0, 10, 30, DEPTH_EXPLOSIONS));*/
}


void SidekickMissile::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	if (other->mass > 0)
		other->accelerate (this, this->angle, (WEAPON_MASS / other->mass) * abs(this->get_vel()), MAX_SPEED);
	Missile::inflict_damage(other);
}


REGISTER_SHIP(KorvianSidekick)
