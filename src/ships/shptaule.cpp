/* $Id: shptaule.cpp,v 1.23 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"

REGISTER_FILE

//#include "../mview.h"

#define LEVIATHAN_SLIME_FOOD 0x12001457

//#define LEVIATHAN_JAMMER 0x2050

// allows other ships to affect control over a ship.
class OverrideControlLeviathan : public OverrideControl
{
	public:
		virtual void calculate(short *key);
};

class TauLeviathan : public Ship
{
	public:
		double      weaponRange, weaponVelocity, weaponDamage, weaponArmour;
		double      weaponSpread, weaponRelativity;

		double      specialRange, specialVelocity, specialDamage, specialArmour;
		double      specialTurnRate;

		int         extraTime;
		double      extraSpeed;
		bool        extraGreen;

		double      food2batt;
		double      healing_amount;
		int         healing_rate;
		int         healing_step;
		int         missile_side;

	public:

		TauLeviathan(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual Color crewPanelColor(int k = 0);
		//  virtual int battPanelColor();

		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void calculate_hotspots();
};

class LeviathanSlimeBall : public AnimatedShot

{
	public:

	protected:

		int l_time;

		double food_velocity;

		bool greenOnly;

	public:

		LeviathanSlimeBall(double ox, double oy, double oangle, double ov, double odamage, int otime, double ov2,

			double orange, double oarmour, SpaceLocation *creator, SpaceLocation *opos,

			SpaceSprite *oscprite, int ofcount, int ofsize, bool go, double rlt = 0.0);

		virtual void inflict_damage(SpaceObject *other);

};

class LeviathanSlimeFood : public AnimatedShot

{
	public:

	protected:

		int lifetime;

	public:

		LeviathanSlimeFood(double oangle, double ov, int otime,

			SpaceLocation *creator, SpaceLocation *opos,

			SpaceSprite *osprite, int ofcount, int ofsize);

		virtual void calculate();

		virtual int isAlly(SpaceLocation *other);

		virtual void inflict_damage(SpaceObject *other);

};

class LeviathanJammer : public FixedAnimation
{
	public:
	public:
		OverrideControlLeviathan *ocl;

		Ship    *victim;

		LeviathanJammer (SpaceLocation *creator, SpaceLocation *opos, SpaceSprite *osprite);
		virtual void calculate();
};

class LeviathanMissile : public HomingMissile

{
	public:

		SpaceSprite *expl_sprite;

		SAMPLE *extraSample;

	public:

		LeviathanMissile(double ox, double oy, double oangle, double ov, double odamage,

			double orange, double oarmour, double otrate, SpaceLocation *creator,

			SpaceSprite *osprite, SpaceSprite *esprite, SpaceObject *otarget);

		virtual void inflict_damage(SpaceObject *other);

};

LeviathanMissile::LeviathanMissile(double ox, double oy, double oangle, double ov, double odamage,

double orange, double oarmour, double otrate, SpaceLocation *creator,

SpaceSprite *osprite, SpaceSprite *esprite, SpaceObject *otarget) :

HomingMissile(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otrate, creator, osprite, otarget)

{
	STACKTRACE;

	expl_sprite = esprite;

	extraSample = data->sampleSpecial[1];

}


void LeviathanMissile::inflict_damage(SpaceObject *other)

{
	STACKTRACE;

	damage (other, damage_factor);

	//	if (!other->isShot()) {
	if (other->isblockingweapons) {

		state = 0;

		add(new LeviathanJammer(this, other, expl_sprite));

		play_sound(extraSample);
	} else {

		animateExplosion();

		soundExplosion();
	}

}


TauLeviathan::TauLeviathan(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :

Ship(opos, shipAngle, shipData, code)

{
	STACKTRACE;

	healing_amount  = tw_get_config_float("Ship", "HealingAmount", 0);

	healing_rate    = scale_frames(tw_get_config_int("Ship", "HealingRate", 0));

	healing_step    = healing_rate;

	food2batt       = tw_get_config_int("Ship", "Food2Batt", 0);

	weaponRange     = scale_range(tw_get_config_float("Weapon", "Range", 0));

	weaponVelocity  = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));

	weaponSpread    = tw_get_config_float("Weapon", "Spread", 0)*PI/180;

	weaponDamage    = tw_get_config_float("Weapon", "Damage", 0);

	weaponArmour    = tw_get_config_float("Weapon", "Armour", 0);

	weaponRelativity= tw_get_config_float("Weapon", "Relativity", 0);

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));

	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));

	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));

	specialDamage   = tw_get_config_int("Special", "Damage", 0);

	extraTime       = (int)(tw_get_config_float("Extra", "Lifetime", 1)*1000);

	extraSpeed      = scale_velocity(tw_get_config_float("Extra", "Velocity", 0));

	extraGreen      = (tw_get_config_int("Extra", "GreenOnly", 0) != 0);

	missile_side    = 1 - (tw_random() & 2);

}


int TauLeviathan::activate_weapon()

{
	STACKTRACE;
	add(new LeviathanSlimeBall(0.0, size.y/2.6, normalize(angle + tw_random(-weaponSpread, weaponSpread), PI2),
		weaponVelocity, weaponDamage, extraTime, extraSpeed, weaponRange,
		weaponArmour, this, this, data->spriteWeapon, 10, 70, extraGreen, weaponRelativity));
	return true;
}


int TauLeviathan::activate_special()
{
	STACKTRACE;

	add(new LeviathanMissile(22*missile_side, 5, angle + (PI/12)*missile_side,
		specialVelocity, specialDamage, specialRange, specialArmour,
		specialTurnRate, this, data->spriteSpecial, data->spriteSpecialExplosion,
		target));
	missile_side *= -1;

	return true;
}


void TauLeviathan::calculate()

{
	STACKTRACE;

	Ship::calculate();

	if (healing_step < 0) {

		healing_step += healing_rate;

		crew += healing_amount;

		if (crew > crew_max) crew = crew_max;
	}

	else  healing_step -= frame_time;

	Query q;

	for (q.begin(this, bit(LAYER_SHOTS), size.y*4.0); q.currento; q.next()) {
		if (!q.current->isObject())
			continue;
		if (q.currento->getID() == LEVIATHAN_SLIME_FOOD) {

			double r = distance(q.currento);

			if (r>size.y*2) r = scale_velocity(1)*frame_time*(size.y*4-r)/(size.y*2)/30.0;

			else       r = scale_velocity(1)*frame_time/30.0;

			q.currento->accelerate(this, normalize(PI+trajectory_angle(q.currento),PI2), r, MAX_SPEED);
		}
	}

	q.end();

}


int TauLeviathan::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	if (state == 0) return 0;

	if (source->getID() == LEVIATHAN_SLIME_FOOD) {
		damage(this, -1); source->state = 0;
		play_sound(data->sampleExtra[0]);
		return 0;
	}

	if (normal + direct < 0) {
		batt += food2batt * -(normal + direct);
		if (batt > batt_max) batt = batt_max;
	}

	return Ship::handle_damage(source, normal, direct);
}


void TauLeviathan::calculate_hotspots()

{
	STACKTRACE;

	if (!fire_special) Ship::calculate_hotspots();

}


Color TauLeviathan::crewPanelColor(int k)

{
	STACKTRACE;

	Color c = {159, 79, 0};

	return c;

}


/*int TauLeviathan::battPanelColor(){
	STACKTRACE;

	return tw_makecol(129,50,99);//palette_color[15];

}*/

LeviathanSlimeBall::LeviathanSlimeBall(double ox, double oy, double oangle, double ov, double odamage,

int otime, double ov2, double orange, double oarmour, SpaceLocation *creator, SpaceLocation *opos,

SpaceSprite *osprite, int ofcount, int ofsize, bool go, double rlt) :

AnimatedShot(creator, Vector2(ox,oy), oangle , ov, odamage, orange, oarmour, opos, osprite, ofcount, ofsize, rlt)

{
	STACKTRACE;

	explosionSprite     = data->spriteWeaponExplosion;

	explosionFrameCount = 16;

	explosionFrameSize  = 50;

	l_time = otime;

	food_velocity = ov2;

	greenOnly = go;

}


void LeviathanSlimeBall::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->isShip()) {
		int c = iround(((Ship*)other)->getCrew());
		AnimatedShot::inflict_damage(other);
		c -= iround(((Ship*)other)->getCrew());
		///RGB col = ((Ship*)other)->crewPanelColor();
		int i;
		for (i=0; i<c; i++)
			add(new LeviathanSlimeFood(random(361), food_velocity*(1-0.90*random(1.01)),
				(int)(l_time * (1-0.00*(random(1.0)))), this, this, data->spriteExtra, 10, 70));
	}
	else
		AnimatedShot::inflict_damage(other);
};

LeviathanSlimeFood::LeviathanSlimeFood(double oangle, double ov, int otime,
SpaceLocation *creator, SpaceLocation *opos,
SpaceSprite *osprite, int ofcount, int ofsize) :
AnimatedShot(creator, 0, oangle , ov, 0, 99999999, 1, creator, osprite, ofcount, ofsize)
{
	STACKTRACE;
	id = LEVIATHAN_SLIME_FOOD;

	layer = LAYER_SHOTS;
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameship = bit(LAYER_SHIPS);
	collide_flag_sameteam = bit(LAYER_SHIPS);

	lifetime = otime;
	//        mass = 0.001;
};

void LeviathanSlimeFood::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	other->handle_damage(this, 0);
};

void LeviathanSlimeFood::calculate()
{
	STACKTRACE;

	AnimatedShot::calculate();
	vel *= exp(-0.0018*frame_time);
	d = 0;
	lifetime -= frame_time;

	if (lifetime <= 0) state = 0;
}


int LeviathanSlimeFood::isAlly(SpaceLocation *other)
{
	STACKTRACE;
	return true;
}


void OverrideControlLeviathan::calculate(short *key)
{
	STACKTRACE;
	*key &= ~(keyflag::left + keyflag::right + keyflag::thrust);
}


LeviathanJammer::LeviathanJammer (SpaceLocation *creator, SpaceLocation *opos, SpaceSprite *osprite) :
FixedAnimation(creator, opos, osprite, 0, 20, 50, LAYER_EXPLOSIONS)
{
	//id = LEVIATHAN_JAMMER;

	if (opos->isShip()) {
		victim = (Ship*)opos;
		/* obsolete: override controls is handled by a list; resetting the victim pointer externally like
		this will make the list management fail.
		Query q;

		for (q.begin(victim, bit(LAYER_EXPLOSIONS), 1); q.current; q.next())
		{

			if (q.currento->getID() == LEVIATHAN_JAMMER)
			{
				if (((LeviathanJammer*)q.current)->victim == victim)
					((LeviathanJammer*)q.current)->victim = 0; }
		}
		*/
	} else {
		victim = 0;
	}

	if (victim) {
		ocl = new OverrideControlLeviathan();
		victim->set_override_control(ocl);
	} else
	ocl = 0;
};

void LeviathanJammer::calculate()

{
	STACKTRACE;

	FixedAnimation::calculate();

	if (victim && !victim->exists()) {
		//victim->del_override_control(ocl);
		victim = 0;
		state = 0;
		//return;
	}

	if (!(ship && ship->exists()) ) {
		die();
	}

	// perform this test here, cause there's a return later on...
	if (!exists()) {
		if (victim)
			victim->del_override_control(ocl);
	}

	if (exists() && victim) {
		//((Ship*)victim)->nextkeys &= ~(keyflag::left + keyflag::right + keyflag::thrust);
		Vector2 vv = victim->get_vel();
		double vvv = magnitude(vv);

		if (vvv < 1e-5)
			return;

		vvv = (vvv - ((Ship*)victim)->accel_rate * frame_time) / vvv;
		if (vvv < 0) vvv = 0;
		vv *= (vvv - 1);
		vvv = magnitude(vv);
		victim->accelerate(victim, atan(vv), vvv, MAX_SPEED);
	}

}


REGISTER_SHIP(TauLeviathan)
