/* $Id: shpbubbo.cpp,v 1.23 2005/08/28 20:34:07 geomannl Exp $ */
#include "../ship.h"
#include "../melee/mview.h"

// original by Tamaraw
// cleaned up by Varith and GeomanNL

//namespace BB01
//{

REGISTER_FILE

class BubalosBomber : public Ship
{
	public:
		int          shipCanReverseThrusters;

		int          explosionPercentChanceBigBoom;
		double       explosionRangeBigBoom;
		int          explosionDamageBigBoom;
		int          explosionPercentChanceShrapnel;
		int          explosionShrapnelNumber1;
		double       explosionShrapnelRange1;
		double       explosionShrapnelSpeed1;
		int          explosionShrapnelDamage1;
		int          explosionShrapnelArmour1;
		int          explosionShrapnelNumber2;
		double       explosionShrapnelRange2;
		double       explosionShrapnelSpeed2;
		int          explosionShrapnelDamage2;
		int          explosionShrapnelArmour2;

		double       weaponRange;
		double       weaponVelocity;
		double       weaponTurnRate;
		int          weaponDamage;
		int          weaponArmour;
		int          flame_frame;
		int          weaponArming;
		double       weaponMinSplitRadius;
		double       weaponSplitAngle;
		int          weapon_offset;
		bool         can_switch;

		double       mirvRange;
		double       mirvVelocity;
		int          mirvDamage;
		int          mirvArmour;
		double       mirvTurnRate;

		double specialRange;
		int    specialDamage;
		int    specialDDamage;
		double specialVelocity;

		int    specialArmour;
		int    specialDriftVelocity, specialDriftMaxVelocity;;
		double specialTurnRate;

		double EAS;
		double ExtraRange;
		int    ExtraDamage;

	public:
		BubalosBomber (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	protected:
		virtual void calculate();
		virtual int activate_weapon();
		virtual void calculate_hotspots();
		virtual int activate_special();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void death();

		virtual void calculate_index();
};

class BubalosBomberFlame : public PositionedAnimation
{
	public:
		int base_frame;
	public:
		BubalosBomberFlame (SpaceLocation *creator, double ox, double oy, SpaceSprite *osprite);
		virtual void calculate();
};

class BubalosMIRV : public Missile
{
	public:
		int          missileArming;
		int          missileActive;
		double       MinSplitRadius;

		double       mirvRange;
		double       mirvVelocity;
		int          mirvDamage;
		int          mirvArmour;
		double       mirvTurnRate;
		SpaceSprite  *mirvSprite;

	public:
		double       mirvSplitAngle;

		BubalosMIRV(double ox,double oy,double oangle, double ov,
			int odamage, double orange, int oarmour, int oarming,
			double oMinSplitRadius, Ship *oship, SpaceSprite *osprite,
			double mv, int mdamage, double mrange, int marmour, double mtrate,
			SpaceSprite *msprite);
		virtual void calculate();
};

class BubalosHMissile : public HomingMissile
{
	public:
		BubalosHMissile(double ox, double oy, double oangle, double ov,
			int odamage, double orange, int oarmour, double otrate,
			BubalosMIRV *opos, SpaceSprite *osprite);
};

class BubalosEMPSlug : public HomingMissile
{
	public:
		BubalosEMPSlug(double ox, double oy, double oangle, double ov,
			int odamage, int oddamage, double specialDriftVelocity,
			double specialDriftMaxVelocity, double otrate,
			double orange, int oarmour, Ship *oship,
			SpaceSprite *osprite);
		virtual void inflict_damage (SpaceObject *other);
		double kick, kick_max;
		int    EMPSlug_Damage;
};

class BubalosAccelLimiter : public Presence
{
	public:
		Ship *mother;
		double accel_rate_factor, timer, duration, default_accel;

		BubalosAccelLimiter(Ship *target, double duration, double accel_rate_factor);
		virtual void calculate();
};

BubalosBomber::BubalosBomber(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	shipCanReverseThrusters = tw_get_config_int("Ship", "CanReverseThrusters", 0);
	weaponRange       = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity    = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage      = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour      = tw_get_config_int("Weapon", "Armour", 0);
	weaponArming      = scale_frames(tw_get_config_float("Weapon", "Arming", 0));
	weaponMinSplitRadius  = scale_range(tw_get_config_float("Weapon", "MinSplitRadius", 0));
	weaponSplitAngle = tw_get_config_float("Weapon", "SplitAngle", 0);
	weaponTurnRate = scale_turning(tw_get_config_float("Weapon", "TurnRate", 0));
	weapon_offset     = 0;

	mirvRange     = scale_range(tw_get_config_float("Weapon", "MIRVRange", 0));
	mirvVelocity  = scale_velocity(tw_get_config_float("Weapon", "MIRVVelocity", 0));
	mirvDamage    = tw_get_config_int("Weapon", "MIRVDamage", 0);
	mirvArmour    = tw_get_config_int("Weapon", "MIRVArmour", 0);
	mirvTurnRate  = scale_turning(tw_get_config_float("Weapon", "MIRVTurnRate", 0));

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialDDamage  = tw_get_config_int("Special", "DDamage", 0);
	specialArmour    = tw_get_config_int("Special","Armour",0);
	specialDriftVelocity = tw_get_config_int("Special", "DriftVelocity", 0);
	specialDriftMaxVelocity = iround(scale_velocity(tw_get_config_float("Special", "DriftMaxVelocity", 0)));
	specialTurnRate = scale_turning(tw_get_config_float("Special","TurnRate",0));

	explosionPercentChanceBigBoom = tw_get_config_int("Explosion", "PercentChanceBigBoom", 0);
	explosionRangeBigBoom = scale_range(tw_get_config_float("Explosion", "RangeBigBoom", 0));
	explosionDamageBigBoom = tw_get_config_int("Explosion", "DamageBigBoom", 0);
	explosionPercentChanceShrapnel = tw_get_config_int("Explosion", "PercentChanceShrapnel", 0);
	explosionShrapnelNumber1 = tw_get_config_int("Explosion", "ShrapnelNumber1", 0);
	explosionShrapnelSpeed1 = scale_velocity(tw_get_config_float("Explosion", "ShrapnelSpeed1", 0));
	explosionShrapnelDamage1 = tw_get_config_int("Explosion", "ShrapnelDamage1", 0);
	explosionShrapnelArmour1 = tw_get_config_int("Explosion", "ShrapnelArmour1", 0);
	explosionShrapnelRange1 = scale_range(tw_get_config_float("Explosion", "ShrapnelRange1", 0));

	explosionShrapnelNumber2 = tw_get_config_int("Explosion", "ShrapnelNumber2", 0);
	explosionShrapnelSpeed2 = scale_velocity(tw_get_config_float("Explosion", "ShrapnelSpeed2", 0));
	explosionShrapnelDamage2 = tw_get_config_int("Explosion", "ShrapnelDamage2", 0);
	explosionShrapnelArmour2 = tw_get_config_int("Explosion", "ShrapnelArmour2", 0);
	explosionShrapnelRange2 = scale_range(tw_get_config_float("Explosion", "ShrapnelRange2", 0));

	flame_frame = 0;
	can_switch = true;

								 //Enhanced Absorber shield
	EAS  = tw_get_config_float("Extra", "EAS", 0);
	ExtraRange      = scale_range(tw_get_config_float("Extra", "Range", 0));
	ExtraDamage     = tw_get_config_int("Extra", "Damage",0);

	sprite_index = sprite_index % 32;

	attributes &= ~ATTRIB_STANDARD_INDEX;
}


int BubalosBomber::activate_weapon()
{
	STACKTRACE;
	if (fire_special && shipCanReverseThrusters) return false;

	BubalosMIRV* BMIRV;
	if (shipCanReverseThrusters)
		if (fire_special) return false;

	double FireAngle, Direction, DeltaA, DetonateRange;
	double r1, r;
	if (target) r = distance(target);
	else r = 1e40;
	SpaceObject *o, *t = NULL;
	SpaceObject *PrevTarget = target;
	Query a;
	for (a.begin(this,bit(LAYER_SPECIAL),ExtraRange); a.current; a.next()) {
		if (!a.current->isObject())
			continue;
		o = a.currento;  r1 = distance(o);
		if (r1 > size.y)
			if ((!o->sameTeam(this)) && (r1 < r) &&
			canCollide(o) && o->exists() && (o->ally_flag != (unsigned int)-1))
				{ t = o; r = r1;  }
	}

	if (t != NULL) target = t;
	if (target && target->exists() && (!target->isInvisible())) {
		DeltaA = fabs(angle - trajectory_angle(target));
		if (DeltaA > PI*3/2) DeltaA = 0;
	}
	//                DetonateRange = distance(target); } else { DeltaA = 0; DetonateRange = weaponRange; }

	DetonateRange = weaponRange;
	if (DeltaA <= PI/2)
		{ FireAngle = angle; Direction = 1; } else {
		FireAngle = angle + PI; Direction = -1;
	}

	//  int wo = (weapon_offset>3)?(6-weapon_offset):weapon_offset;

	// remove the alternating fire
	/* weapon_offset = (weapon_offset + 1) % 6;

	  double dx;
		switch (wo) {
		  case 0: dx = -12; break;
		case 1: dx = -8; break;
		case 2: dx = 8; break;
		case 3: dx = 12;  break;
		default: tw_error("Unexpected value of weapon_offset"); dx=0;break;
		}
	*/
	double dx;
	dx = 0;

	BMIRV = new BubalosMIRV(dx, 54 * Direction, FireAngle,weaponVelocity,
		weaponDamage,DetonateRange,weaponArmour,
		weaponArming,weaponMinSplitRadius,this, data->spriteWeapon,
		mirvVelocity,mirvDamage,mirvRange,mirvArmour,mirvTurnRate,data->spriteWeapon);
	BMIRV->mirvSplitAngle = weaponSplitAngle;
	game->add(BMIRV);
	target = PrevTarget;
	return(TRUE);
}


BubalosMIRV::BubalosMIRV(double ox,double oy,double oangle, double ov,
int odamage, double orange, int oarmour, int oarming,
double oMinSplitRadius, Ship *oship, SpaceSprite *osprite,
double mv, int mdamage, double mrange, int marmour, double mtrate,
SpaceSprite *msprite) :
Missile(oship, Vector2(ox,oy), oangle, ov,
odamage, orange, oarmour, oship, osprite)
{
	STACKTRACE;
	missileArming = oarming;
	missileActive = FALSE;
	MinSplitRadius = oMinSplitRadius;

	mirvRange    = mrange;
	mirvVelocity = mv;
	mirvDamage   = mdamage;
	mirvArmour   = marmour;
	mirvSprite   = msprite;
	mirvTurnRate = mtrate;

	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;

}


void BubalosMIRV::calculate()
{
	STACKTRACE;
	double SplitRad = 0;
	Missile::calculate();

	if (!missileActive) {
		missileArming -= frame_time;
		if (missileArming <= 0)
			missileActive = TRUE;
	}
	else if (target && target->exists()) {
		if ((distance(target) < MinSplitRadius) || (d >= range)) {
			SplitRad = this->mirvSplitAngle * ANGLE_RATIO;
			BubalosHMissile *BHMissile;
			BHMissile = (new BubalosHMissile(0,0,this->angle+SplitRad ,mirvVelocity,
				mirvDamage, mirvRange, mirvArmour,mirvTurnRate,
				this,mirvSprite));
			game->add(BHMissile);
			BHMissile = (new BubalosHMissile(0,0,this->angle-SplitRad,mirvVelocity,
				mirvDamage, mirvRange, mirvArmour,mirvTurnRate,
				this,mirvSprite));
			game->add(BHMissile);
			this->state = 0;
		}
	}

	return;
}


void BubalosBomber::death()
{
	STACKTRACE;

	int lastHurrah;
	double radInc;
	int i;
	Missile* M;
	Vector2 CenterPoint;
	CenterPoint.x = 0; CenterPoint.y = 0;

	if (tw_random(100) < explosionPercentChanceBigBoom) {
		Query q;
		//message.print(4500,9,"BOOM!");
		for (q.begin(this,OBJECT_LAYERS, explosionRangeBigBoom); q.currento; q.next()) {
			if (!q.current->isObject())
				continue;
			lastHurrah = (int)ceil((explosionRangeBigBoom - distance(q.currento)) /
				explosionRangeBigBoom * explosionDamageBigBoom);
			damage( q.currento, 0, lastHurrah);
		}						 //for
		add(new Animation(this, CenterPoint, data->spriteExtra, 0, data->spriteExtra->frames(),
			50, LAYER_EXPLOSIONS));
	}
	if (random(100) < explosionPercentChanceShrapnel) {
		radInc = PI2 / explosionShrapnelNumber1;
		for(i=0; i<explosionShrapnelNumber1; i++) {
			M = new Missile(this, CenterPoint, radInc * i, explosionShrapnelSpeed1,
				explosionShrapnelDamage1, explosionShrapnelRange1, explosionShrapnelArmour1,
				this, this->data->spriteExtraExplosion, 0);
			game->add(M);
		}
		radInc = PI2 / explosionShrapnelNumber2;
		for(i=0; i<explosionShrapnelNumber2; i++) {
			M = new Missile(this, CenterPoint, radInc * i, explosionShrapnelSpeed2,
				explosionShrapnelDamage2, explosionShrapnelRange2, explosionShrapnelArmour2,
				this, this->data->spriteExtraExplosion, 0);
			game->add(M);
		}
	}
	return;
}


int BubalosBomber::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	if (normal > 0) {
		//normal = int(normal * EAS);
		normal *= EAS;			 // don't use INT here !!
		//if (normal == 0) normal = 1;
		//batt += normal;	?!?!?!??! this sucks imo.
		//if (batt > batt_max) batt = batt_max;
	}							 //if
	return Ship::handle_damage(source, normal, direct);
}								 // handle_damage


BubalosEMPSlug::BubalosEMPSlug(double ox, double oy, double oangle, double ov,
int odamage, int oddamage, double specialDriftVelocity, double specialDriftMaxVelocity, double otrate,
double orange, int oarmour, Ship *oship, SpaceSprite *osprite) :
HomingMissile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otrate,
oship, osprite, oship->target),

kick(specialDriftVelocity),
kick_max(specialDriftMaxVelocity),
EMPSlug_Damage(oddamage)
{
	STACKTRACE;
	collide_flag_sameteam  = bit(LAYER_SHIPS);
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
}


BubalosHMissile::BubalosHMissile(double ox, double oy, double oangle,double ov,
int odamage, double orange, int oarmour, double otrate,
BubalosMIRV *opos, SpaceSprite *osprite) :
HomingMissile(opos, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otrate,
opos, osprite, opos->target)
{
	STACKTRACE;
	collide_flag_sameteam   = bit(LAYER_SHIPS);
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
}


int BubalosBomber::activate_special()
{
	STACKTRACE;

	if ( (!fire_weapon) && shipCanReverseThrusters) return false;

	double FireAngle;
	double Direction;
	double DeltaA;

	if (target && target->exists() && (!target->isInvisible())) {
		DeltaA = fabs(angle - trajectory_angle(target));
		if (DeltaA > PI*3/2) DeltaA = 0;
	} else DeltaA = 0;

	if (DeltaA <= PI/2) {
		FireAngle = angle;
		Direction = 1;
	} else {
		FireAngle = angle + PI;
		Direction = -1;
	}

	add(new BubalosEMPSlug(
		0.0, 55 * Direction, FireAngle, specialVelocity,
		specialDamage, specialDDamage, specialDriftVelocity,
		specialDriftMaxVelocity, specialTurnRate,
		specialRange, specialArmour, this, data->spriteSpecial));

	return(TRUE);

}


void BubalosEMPSlug::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (other->mass) {
		Vector2 D;
		double veff, dv, a;

		// after some testing, given physics of timewarp, it's most effective to push ships
		// back from the direction they're pointing to.

		D = unit_vector(other->angle+PI);
		D /= D.magnitude();

		veff = other->vel.dot(D);// effective vel. pointing away from slug-effect; this
		// shouldn't exceed kick_max.

		if ( veff < kick_max ) {
			a = D.atan();		 // direction opposite to ship movement

			dv = kick / other->mass;

			if ( dv+veff > kick_max )
				dv = kick_max - veff;

			other->accelerate (this, a, dv, MAX_SPEED);

			// also, reduce accel_rate of the enemy for a short while, so
			// that also ships with extremely high accel rate are affected
			// for a few seconds:

			double t = 3.0;
			if ( other->isShip())
				game->add(new BubalosAccelLimiter((Ship*) other, t, 0.1));
		}
	}
	other->damage (this, 0, EMPSlug_Damage);
	Missile::inflict_damage(other);
}


void BubalosBomber::calculate_hotspots()
{
	STACKTRACE;

	Ship::calculate_hotspots();

	if ((thrust) && (flame_frame <= 0)) {
		add(new BubalosBomberFlame(this, -28, 42, data->spriteExtraExplosion));
		add(new BubalosBomberFlame(this, -28, -42, data->spriteExtraExplosion));
		flame_frame += 100;
	}
	if (flame_frame > 0) flame_frame -= frame_time;

}


BubalosBomberFlame::BubalosBomberFlame(SpaceLocation *creator, double ox, double oy, SpaceSprite *osprite) : PositionedAnimation(creator, creator, Vector2(ox,oy), osprite, 0, 1, 100, LAYER_SHOTS)
{
	STACKTRACE;
	base_frame = 64*(random(4));
	sprite_index = base_frame + get_index(follow->get_angle());
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void BubalosBomberFlame::calculate()
{
	STACKTRACE;
	PositionedAnimation::calculate();
	if (exists())
		sprite_index = base_frame + get_index(follow->get_angle());
}


void BubalosBomber::calculate()
{
	STACKTRACE;
	if (shipCanReverseThrusters) {
		if (fire_special && thrust) {
			if (can_switch) {
				can_switch = false;
				angle = normalize (angle+PI, PI2);
			}
		}
		else
			can_switch = true;
	}

	Ship::calculate();
	//sprite_index = sprite_index & 31;
}


void BubalosBomber::calculate_index()
{
	STACKTRACE;
	// this makes use of symmetry of the ship.
	sprite_index = get_index(angle) & 31;
}


BubalosAccelLimiter::BubalosAccelLimiter(Ship *otarget, double oduration, double oaccel_rate_factor)
{
	STACKTRACE;
	mother = otarget;
	duration = oduration;

	accel_rate_factor = oaccel_rate_factor;

	timer = 0;
}


void BubalosAccelLimiter::calculate()
{
	STACKTRACE;
	timer += frame_time * 1E-3;

	if ( !(mother && mother->exists()) || ( timer > duration ) ) {
		mother = 0;
		state = 0;
		return;
	}

	if ( mother->thrust ) {
		double f = 1 - timer/duration;
		mother->accelerate(mother, mother->angle, -f * mother->accel_rate * frame_time, mother->speed_max);
	}

}


REGISTER_SHIP(BubalosBomber)
