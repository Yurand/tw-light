/* $Id: shpgerhe.cpp,v 1.18 2004/03/24 23:51:41 yurand Exp $ */
#include "ship.h"
REGISTER_FILE
#include "melee/mview.h"
#include "melee/mitems.h"
#include "melee/mcbodies.h"

/*

  GerHe

  Gerl Hero

  A big ship full of morons, following
  a small ship with a hero,

  The big ship can catapult the small craft,
  and supplies energy for the small craft,

  The small craft can attach itself onto a enemy and inflict damage by its main.
  (by maneouvering, the enemy can cost him valuable energy),

  Both fire and thrusting cost the small craft energy,
  It'll have to go back to the big ship to recharge, or
  wait until the big ship arrives to pick it up (slow ...)

  If a hero dies, a new one is chosen from the ranks of the
  morons. This costs 2 crew !

  The moron vessel is invulnerable to anything but the planet,
  the hero has 2 armor (see ini)

  If there's a "shock", the hero also releases the ship ...

  Pressing fire+special causes a suicide, but only when the hero
  is attached to the ship... causes 1 additional damage.

*/

class GerlHero;
class GerlMorons;

// Ok, this is not a real ship, but only a way to manage 2 vessels.

class GerlVirtualship: public Ship
{
	public:
		double weaponRange;
		double weaponVelocity;
		int    weaponDamage;
		int    weaponArmour;
		double accel_time;

		double  batt_recharge_range;

		GerlHero    *hero;
		GerlMorons  *morons;

		GerlVirtualship(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void calculate();
		virtual void animate(Frame *frame);

		virtual int activate_weapon();
		virtual int activate_special();

};

class GerlHero : public Ship
{
	public:
		Vector2         dvel, Vboarding;
		double          dangle;
		GerlVirtualship *mother;
		SpaceLocation   *moronbrother;
		int             activateyourweapon, activateyourspecial, attached,
			comeaboard, leavingship, movetotip;
		double          targetlastangle;
		double          armour;	 // similar to a weapon, it can absorb a little weapon fire
		double          boarding_atot, boarding_prevangle;
		SpaceObject     *boardingship;

		Vector2         lastenemyvel;

		GerlHero(GerlVirtualship *creator, SpaceLocation *brother, Vector2 opos,
			double oangle, SpaceSprite *osprite);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int  handle_damage(SpaceLocation *other, double normal, double direct = 0);

		void animateExplosion();
		void soundExplosion();

};

class GerlMorons : public Ship
{
	public:
		double          dangle, da_max;
		GerlVirtualship *mother;
		SpaceLocation   *herobrother;

		GerlMorons(GerlVirtualship *creator, SpaceLocation *brother, Vector2 opos, double oangle, SpaceSprite *osprite);
		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
		virtual void inflict_damage();
		int avoid_planet();
		void avoid_location(Vector2 pos, double minangle, double max_angle);
};

/*
class GerlHeroStun : public Presence
{
	Vector2	freeze_pos;
	double	duration, existtime;
	SpaceObject	*target;
public:
	GerlHeroStun(SpaceObject *otarget, double oduration);
	void calculate();
};
*/

GerlVirtualship::GerlVirtualship(Vector2 opos, double angle, ShipData *data, unsigned int code)
: Ship(opos, angle, data, code)
{
	STACKTRACE;

	//	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	//	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	//	weaponTurnrate = scale_turning(get_config_float("Weapon", "TurnRate", 0));

	accel_time = get_config_float("Weapon", "AccelTime", 1.0);

	morons = new GerlMorons(this, 0, opos, angle, this->data->spriteShip);
	game->add(morons);
	game->add(new WedgeIndicator(morons, 75, 7));
	// this doesn't add as a target

	hero = new GerlHero(this, morons, opos, angle, this->data->spriteExtra);
	morons->herobrother = hero;
	game->add(hero);
	hero->set_depth(DEPTH_SHIPS+0.1);
	//game->add_target(hero); no, cause they'll follow the virtual ship.

	game->add(new WedgeIndicator(morons, 50, makecol(0,0,200)) );

	batt_recharge_range = 200.0; // that's not very far ;)

	collide_flag_anyone = 0;	 // the virtual ship cannot collide !
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;

}


void GerlVirtualship::calculate()
{
	STACKTRACE;

	if ( crew == 0 ) {			 // this value is manipulated by the moronship and the heroship
		state = 0;
		return;
	}

	// and what if only the hero doesn't exist? in that case, it should be reborn! at
	// the expense of 2 crew:

	if ( (!hero || !hero->exists()) && (crew >= 2) ) {
		Vector2 opos;
		opos = morons->pos - 100.0 * unit_vector(trajectory_angle(target)/*morons->angle*/);
		hero = new GerlHero(this, morons, opos, angle, this->data->spriteExtra);
		morons->herobrother = hero;
		game->add(hero);
		//game->add_target(hero); no, cause the virtual ship = target already

		hero->set_depth(DEPTH_SHIPS+0.1);

		pos = opos;
		hero->pos = pos;
		batt = batt_max;		 // otherwise they keep dying of lack of juice !!
		vel = morons->vel;
		hero->vel = vel;

		crew -= 1;				 // 2 leave the moron ship, but only 1 is effectively sacrificed - one
		// is on the hero vessel.

		update_panel = 1;
	}

	if ( (!hero || !hero->exists()) && (crew <= 1) ) {
		crew = 0;				 // 1 crew is meaningless
		state = 0;
		return;
	}

	Vector2 oldvel;
	double  oldangle;
	double  oldbatt;

	oldvel = vel;
	oldangle = angle;
	oldbatt = batt;

	Ship::calculate();

	// the following case should not occur?
	if ( !(hero && hero->exists()) && !(morons && morons->exists()) ) {
		hero = 0;
		morons = 0;
		state = 0;
		return;
	}

	if ( hero && hero->exists() ) {

		// transfer commands to steer the hero-vessel
		hero->dvel = vel - oldvel;
		hero->dangle = angle - oldangle;
		batt = oldbatt;

		// steering ... the player steers the virtual ship; values are copied to the hero vessel

		// velocities are passed on to the hero vessel
		if ( thrust ) {			 // if the hero thrusts ...
								 // costs 1 fuel per second
			batt -= 1.0 * frame_time / 1000.0;
			if ( batt < 0.0 ) {
				batt = 0.0;
				//				turn_left = 0;		// isn't on the right spot here !!
				//				turn_right = 0;
				//				thrust = 0;
			}

			// might be important ... if you're attached, you can slightly control the enemy:
			// you can use this, to drag the enemy ship away from the "morons"
			if ( hero->attached ) {
				//double accel_time = 1.0;
				hero->boardingship->accelerate(hero->boardingship,
					hero->angle, hero->ship->speed_max * frame_time*1E-3/accel_time, hero->ship->speed_max);
				//						WELL, it's not that much fun :) can just as well when hitting the enemy
			}

		}

		if ( morons && morons->exists() ) {
			double R;

			// recharging the hero's battery, this occurs only when it is close enough
			// to its host vessel, and if the host vessel exists!

			R = hero->distance(morons);

			if ( R <= batt_recharge_range ) {
								 // 8 recharge per seconds
				batt += 8.0 * frame_time * 1E-3;
				if ( batt > batt_max )
					batt = batt_max;
			}

			// the morons slowly follow the hero.
			Vector2 dpos;
			double a1, a2, da;

			dpos = min_delta(hero->pos - morons->pos, map_size);

			a1 = morons->angle;
			a2 = atan(dpos);

			da = a2 - a1;

			while ( da >  PI )  da -= PI2;
			while ( da < -PI )  da += PI2;

			if ( da > morons->da_max )
				da = morons->da_max;
			if ( da < -morons->da_max )
				da = -morons->da_max;

			morons->dangle = da;

			// if the hero wants to be launched, double this speed.
			//			if ( hero->movetotip )
			//				morons->dangle += da;
			//			// well... too quirky and not that useful I guess.
		}

	} else {
		// watch where the (helpless) morons are going ...
		pos = morons->pos;
		vel = morons->vel;
	}

}


int GerlVirtualship::activate_weapon()
{
	STACKTRACE;
	if ( hero->attached ) {
								 // issue order to the hero that he activates his weapon.
		hero->activateyourweapon = 1;
		batt += weapon_drain;	 // prevent fuel drain
		return TRUE;
	} else
	return FALSE;
}


int GerlVirtualship::activate_special()
{
	STACKTRACE;
	hero->activateyourspecial = 1;
	batt += special_drain;		 // prevent fuel drain

	return true;
}


void GerlVirtualship::animate(Frame *frame)
{
	STACKTRACE;
	return;						 // do nothing, it's a virtual ship, and should be hidden from the game !!
}


/* ******************* HERO VESSEL ********************** */

GerlHero::GerlHero(GerlVirtualship *creator, SpaceLocation *brother, Vector2 opos,
double oangle, SpaceSprite *osprite)
: Ship(creator, opos, oangle, osprite)
{
	STACKTRACE;

	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_anyone = ALL_LAYERS;

	//	layer = LAYER_SHIPS;
	//	set_depth(DEPTH_SHIPS + 0.1);		// so that it'll be drawn last, on top of the ships.
	//	id = SPACE_SHIP;
	//	attributes |= ATTRIB_SHIP;

	ally_flag = creator->ally_flag;

	mass = 4;

	mother = creator;
	pos = mother->pos;

	moronbrother = brother;

	attached = 0;
	comeaboard = 0;
	activateyourweapon = 0;
	activateyourspecial = 0;
	boardingship = 0;
	leavingship = 0;
	movetotip = 0;				 // moving hero ship to launch position

	dvel = 0;
	dangle = 0;

	boarding_atot = 0;
	boarding_prevangle = 0;

	sprite_index = get_index(angle);

	armour = mother->weaponArmour;

	crew_max = armour;
	crew = armour;

	lastenemyvel = 0;
}


void GerlHero::calculate()
{
	STACKTRACE;

	SpaceObject::calculate();

	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;
		return;
	}

	// the hero dies if he has no energy left.
	if ( mother->batt <= 0 ) {
		state = 0;
		return;
	}

	mother->pos = pos;
	mother->vel = vel;
	mother->angle = angle;

	sprite_index = get_index(angle, 0.5*PI);

	int beingbusy = attached || comeaboard || leavingship || boardingship;

								 // if it's destroyed !
	if ( beingbusy && !(boardingship && boardingship->exists()) ) {
		attached = 0;
		boardingship = 0;
		leavingship = 0;
		comeaboard = 0;
		beingbusy = 0;

		collide_flag_sameteam = ALL_LAYERS;
		collide_flag_sameship = ALL_LAYERS;
		collide_flag_anyone = ALL_LAYERS;
	}

	// check if the ship you're attached to has some sort of (serious) collision
	// but only if you're all aboard!!
	// (otherwise you've difficulty to board)
	if ( attached ) {
		if (boardingship->vel != lastenemyvel) {
			Vector2 dv = boardingship->vel - lastenemyvel;
			lastenemyvel = boardingship->vel;

			double arate;
			if (boardingship->isShip())
				arate = ((Ship*) boardingship)->accel_rate;
			else
				arate = 0;

			//			if ( dv.length() > arate * frame_time * 2 &&
			//				dv.length() > (arate + accel_rate) * frame_time * 2)
			if ( dv.length() > 0.5 ) {

				attached = 0;

				leavingship = 1;
			}
		}
	}

	// commands transferred from the virtual ship

	// order a suicide:
	// must be ordered before the release order (since it'll override that release order)

	if ( mother->fire_special && mother->fire_weapon && attached ) {
		activateyourspecial = 0;
		activateyourweapon = 0;

		state = 0;

								 // it's the (virtual) mother that does the damage
		mother->damage(boardingship, 1, 0);

		int n = mother->data->spriteWeaponExplosion->frames();
		game->add( new Animation(this, pos, mother->data->spriteWeaponExplosion, 0,
			n, time_ratio, LAYER_EXPLOSIONS) );
	}

	if (!beingbusy ) {
		vel += dvel;
		angle += dangle;
	} else {
		vel = 0;
		angle += 0.5 * dangle;	 // more difficult to turn, when you're attached?!
	}

	// crawl aboard the enemy ship
	if ( comeaboard && !attached ) {

		Vector2 Vd;

		pos += min_delta(boardingship->pos, Vboarding, map_size);
		Vboarding = boardingship->pos;

		Vd = min_delta(boardingship->pos, pos, map_size);

		if ( Vd.x < -2 )
			Vd.x = -2;
		if ( Vd.x > 2 )
			Vd.x = 2;
		if ( Vd.y < -2 )
			Vd.y = -2;
		if ( Vd.y > 2 )
			Vd.y = 2;

		pos += Vd;

		if ( fabs(pos.x - boardingship->pos.x) < 1E-3 &&
		fabs(pos.y - boardingship->pos.y) < 1E-3 ) {
			comeaboard = 0;
			attached = 1;

								 // initialize this check value
			lastenemyvel = boardingship->vel;

			boarding_atot = 0.0;
			boarding_prevangle = boardingship->angle;
		}

		//vel = boardingship->vel; this doesn't work ...
		vel = 0;
	}

	// place the virtual ship on top of the hero, so that the view is focused on the
	// little ship.
	if ( attached ) {
		pos = boardingship->pos;
		vel = boardingship->vel;
		//vel = 0;

		// when you're attached, you have to let go cause the enemy moves too much
		// otherwise you'll die because of lack of fuel .
		//		int forcerelease;

		// should be d_angle, not angle itself !!
		double da;

		da = boardingship->angle - boarding_prevangle;
		boarding_prevangle = boardingship->angle;
		if ( da >  PI ) da -= PI2;
		if ( da < -PI ) da += PI2;

		boarding_atot += fabs(da) * 0.2 * (1.0 + boardingship->vel.length()) *frame_time;

								 // staying attached uses up fuel
		if (boarding_atot > 1 * PI2) {
			mother->batt -= 1;
			mother->update_panel = 1;
			boarding_atot = 0;
		}
		//			forcerelease = 1;
		//		else
		//			forcerelease = 0;

		// he can choose to release, when he uses his special, while attached
		if ( activateyourspecial /*|| forcerelease*/ ) {
		leavingship = 1;
		comeaboard = 0;
		attached = 0;
		// command received - hero retreating from his attack position
	}

}


if ( leavingship ) {
	// slowly move away !!

	double a = angle;
	vel = boardingship->vel + 0.1 * unit_vector(a);

	double R;
	Vector2 Vd;
	Vd = pos - boardingship->pos;
	R = magnitude(Vd);

	// check if you're far away enough from the ship:
	if ( R > 0.5*(boardingship->get_size().x + get_size().x) ) {
		attached = 0;
		comeaboard = 0;
		leavingship = 0;
		boardingship = 0;

		collide_flag_sameteam = ALL_LAYERS;
		collide_flag_sameship = ALL_LAYERS;
		collide_flag_anyone = ALL_LAYERS;

	}

}


//	double iweaponDrain = 4;
if ( activateyourweapon && attached && mother->ship->batt >= mother->weapon_drain ) {
	activateyourweapon = 0;
								 // it's the (virtual) mother that does the damage
	mother->damage(boardingship, mother->weaponDamage, 0);

	int i = mother->data->spriteWeapon->frames();
	game->add( new Animation(this, pos + Vector2(tw_random(10), tw_random(10)), mother->data->spriteWeapon, 0,
		i, time_ratio, LAYER_EXPLOSIONS) );

	// take some energy
	mother->handle_fuel_sap(this, mother->weapon_drain);

	// also, stun the enemy for a few seconds:
	//game->add( new GerlHeroStun(boardingship, 5.0) );
	// disabled: it's a bit worthless, and not nice.

	// he can fall off when he uses his weapon
	/*
	double targetdangle = target->angle - targetlastangle;
	targetlastangle = target->angle;
	if ( fabs(targetdangle*100) > tw_random()%100 )
	{
		attached = 0;
		angle = target->angle + 90 * sign(targetdangle);
		heroV = 1.0;
		double a = angle*PI/180;
		vx = heroV * cos(a);
		vy = heroV * sin(a);
	}
	*/
}


if ( (activateyourspecial && !attached && !leavingship) || movetotip ) {
	if (activateyourspecial && !attached)
		if (movetotip == 1)
			movetotip = 0;		 // toggle the move command off

	if ( !(moronbrother && moronbrother->exists()) ) {
		movetotip = 0;
	} else {

		double R, R0;
		Vector2 Vd, Vtip;

		//a = moronbrother->angle;
		R0 = 100.0;

		Vtip = moronbrother->pos + R0 * unit_vector(moronbrother->angle);

		// move automatically towards the tip:

		double a1, a2, da, R1, R2;

		Vd = pos - moronbrother->pos;
		a1 = atan(Vd);
		R1 = magnitude(Vd);

		// only move automatically, if the hero is close enough to the moron ship
		double autoguide_range = 200.0;

		if ( R1 < autoguide_range )
			movetotip = 1;
		else
			movetotip = 0;

		if ( movetotip ) {

			a2 = moronbrother->angle;
			R2 = R0;			 // distance you want to achieve

			double dda, davel;

			// the angle remaining
			dda = a2 - a1;
			while ( dda >   PI )    dda -= 2*PI;
			while ( dda <= -PI )    dda += 2*PI;

			// always move at max angular velocity in some direction
								 // max. 20 degree per second
			davel = 20.0 * ANGLE_RATIO;
			if ( dda > 0 )
				da =  davel;
			else
				da = -davel;

			da *= frame_time / 1000.0;

			// check if you don't overshoot the optimal point:
			if ( da >  fabs(dda) )      da =  fabs(dda);
			if ( da < -fabs(dda) )      da = -fabs(dda);

			if ( fabs(da) > 1E-3 )
				R = R1 + (R2 - R1) * da / dda;
			else
				R = R2;			 // should be the target distance.

			double dR = 50.0 * frame_time * 1E-3;
								 // don't move too fast though ;)
			if ( R - R1 > dR )
				R = R1 + dR;
			if ( R - R1 < -dR )
				R = R1 - dR;

			double a;
			a = a1 + da;

			Vector2 oldpos;
			oldpos = pos;

			pos = moronbrother->pos + R * unit_vector(a);
								 // match velocity with the moron's ship
			vel = moronbrother->vel;

			// also, re-orientate the hero vessel till it's aligned with the moron ship
			// the target angle
			dda = moronbrother->angle - angle;
			while ( dda >=  PI )    dda -= 2*PI;
			while ( dda <  -PI )    dda += 2*PI;

			// the angular change
			davel = 180.0 * ANGLE_RATIO;
			if ( dda > 0 )
				da =  davel;
			else
				da = -davel;

			da *= frame_time / 1000.0;

			// check if you don't overshoot the optimal point:
			if ( da >  fabs(dda) )      da =  fabs(dda);
			if ( da < -fabs(dda) )      da = -fabs(dda);

			angle += da;
		}

		Vd = pos - Vtip;
		R = magnitude(Vd);

		da = angle - moronbrother->angle;
		while ( da >=  PI ) da -= 2*PI;
		while ( da <  -PI ) da += 2*PI;

		if ( R < 1.0 && fabs(da) < 1.0 * ANGLE_RATIO ) {

			// launch !
								 // very fast ?!
			double heroV = 1.25;
								 // launched in the orientation of the moron ship!
			vel = moronbrother->vel + heroV * unit_vector(angle);

			// rebound of the moron ship upon launch ... does this work?
			moronbrother->vel -= 0.5 * heroV * unit_vector(angle);

			movetotip = 0;

		}
	}
}


activateyourweapon = 0;
activateyourspecial = 0;

if ( attached )					 // set your priorites right correctly
	movetotip = 0;

return;
}


// This is used to detect that the Hero collides with some object in space,
// then, it'll crawl on top of it.
// (I could've use handle_damage instead).
void GerlHero::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!state)
		return;

	if ( other->isShip() || other->isAsteroid() ) {

		// but only if it's a ship or an asteroid
		// (it certainly must not be a weapon or planet)

		if ( !attached && (other/*->get_serial()*/ != moronbrother/*->get_serial()*/) ) {
		comeaboard = 1;
		boardingship = other;

		collide_flag_sameship = 0;
		collide_flag_sameteam = 0;
		collide_flag_anyone = 0;

		Vboarding = boardingship->pos;

		// give the enemy a small amount of impact velocity ;)
		boardingship->vel = 0.5 * (boardingship->vel + vel);
		//if (boardingship->vel

	}

}


// important: isn't detected by handle_damage ?!
if ( other->isPlanet() )
	state = 0;

// is already done by handle_damage:
if ( state == 0 )
	mother->ship->crew -= 1;

}


void GerlHero::animateExplosion()
{
	STACKTRACE;
	//  add( new Animation(this, pos,
	//    data->spriteWeaponExplosion, (charge_phase * 20), 20, 25,
	//    DEPTH_EXPLOSIONS));
}


void GerlHero::soundExplosion()
{
	STACKTRACE;
	//	play_sound2(data->sampleExtra[2]);
}


int GerlHero::handle_damage(SpaceLocation *other, double normal, double direct)
{
	STACKTRACE;
	if (!state)
		return 0;

	armour -= normal + direct;

	if ( armour <= 0 )
		state = 0;

	if ( other->isPlanet() )
		state = 0;

	if ( state == 0 ) {
		mother->ship->crew -= 1;

		animateExplosion();
		soundExplosion();
	}

	if ( !state ) {
		// this ship dies :(
		play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
		game->add(new Animation(this, pos, meleedata.kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
	}

	return iround(normal + direct);
}


/* ******************* MORONS VESSEL ********************** */

GerlMorons::GerlMorons(GerlVirtualship *creator, SpaceLocation *brother, Vector2 opos,
double oangle, SpaceSprite *osprite)
: Ship((SpaceLocation*)creator, opos, oangle, osprite)
{
	STACKTRACE;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	//	layer = LAYER_SHIPS;
	//	set_depth(DEPTH_SHIPS);
	//	id = SPACE_SHIP;
	//	attributes |= ATTRIB_SHIP;

	mass  = 15;

	mother = creator;

	dangle = 0.0;

	herobrother = brother;

	pos.x = mother->pos.x;
	pos.y = mother->pos.y + 60.0;

	da_max = 45 * ANGLE_RATIO * frame_time/1000.0;

	sprite_index = get_index(angle);

	crew_max = creator->crew_max;
	crew = creator->crew;

}


void GerlMorons::calculate()
{
	STACKTRACE;
	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;
		return;
	}

	double speed_max  = 80 * 1E-3;
	double accel_rate =  0.1 * 1E-3;

	// set/ overrule (!) the angle in order to avoid the planet!
	if ( !avoid_planet() )		 // if true, this changes the angle by itself already
		angle += dangle;

	// acceleration:
	accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);

	double vsqr = magnitude_sqr(vel);
	if ( vsqr > speed_max*speed_max ) {
		double a;

		a = fabs(1.0 - 0.1 * frame_time / 1000.0 );

		if ( a > 1.0 )
			a = 0.999;

		vel *= a;
	}

	SpaceObject::calculate();
	sprite_index = get_index(angle, 0.5*PI);

	return;
}


int GerlMorons::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	//int totdamage = normal + direct;

	state = 1;					 // just in case it's been changed by the planet !

	// experimental:
	double t = 0;

	if (!source->isPlanet()) {
		// t = normal + direct; a bit too powerful?
		t = 0;					 // perfect armor/ shielding
		if (t >= 0) {
			mother->crew += t;
			if ( mother->crew > mother->crew_max )
				mother->crew = mother->crew_max;
			mother->update_panel = true;
		}

	}
	return iround(t);

	/*
		// most of the crew is on the moron vessel. One is on the hero vessel. This complicates
		// things ... there should always be 1 crew left on either the moron or the hero
		// vessel, and thus, also on the moron ship...

		if (totdamage >= mother->ship->crew)
		{

			mother->ship->crew -= totdamage;

			if ( herobrother && herobrother->exists() )
			{
				if (mother->ship->crew < 1 )
					mother->ship->crew = 1;

				if (mother->ship->crew <= 1 )	// the only remaining survivor is the hero; the morons are all dead
				{
					state = 0;
	//				return totdamage;
				}
			}

			if ( mother->ship->crew <= 0 )
			{
				state = 0;
	//			return totdamage;
			}

		} else
			mother->ship->crew -= totdamage;

		if ( !state )
		{
			// this ship dies :(
			play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
			game->add(new Animation(this, pos, game->kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
		}

		return totdamage;
	*/
}


void GerlMorons::inflict_damage()
{
	STACKTRACE;
	// do nothing
}


int GerlMorons::avoid_planet()
{
	STACKTRACE;

	Planet *p;
	p = nearest_planet();		 // this searches within 1600 range
	if ( !p )
		return 0;

	double d;
								 //crash?!
	d = distance((SpaceLocation*) p);

	if ( d < 4 * p->size.x ) {	 // stay away for a long distance !
		avoid_location(p->pos, 0.5*PI, 0.7*PI);
		return 1;
	} else
	return 0;
}


// minangle - maxangle is the range of angles used to avoid this position
void GerlMorons::avoid_location(Vector2 pos, double min_angle, double max_angle)
{
	STACKTRACE;
	double a, b, da;

	a = angle;					 //atan(vel);
	b = atan(min_delta(pos - this->pos, map_size));

	da = b - a;
	while (da < -PI)    da += PI2;
	while (da >  PI)    da -= PI2;

	if ( fabs(da) >= min_angle && fabs(da) <= max_angle )
		return;

	if ( fabs(da) < min_angle) {
		if ( da > 0 )
			angle -= da_max;	 //calculate_turn_right();
		if ( da < 0 )
			angle += da_max;	 //calculate_turn_left();
	} else {
		if ( da > 0 )
			angle += da_max;
		if ( da < 0 )
			angle -= da_max;
	}

}


/*
GerlHeroStun::GerlHeroStun(SpaceObject *otarget, double oduration)
{
	STACKTRACE;
	target = otarget;
	duration = oduration;		// in seconds
	freeze_pos = target->pos;

	existtime = 0;
}

void GerlHeroStun::calculate()
{
	STACKTRACE;
	existtime += frame_time * 1E-3;

	if ( !(target && target->exists()) || existtime > duration )
	{
		state = 0;
		return;
	}

	// make sure, the target does not move from its position !
	target->pos = freeze_pos;
	target->vel = 0;
}
*/

REGISTER_SHIP(GerlVirtualship)
