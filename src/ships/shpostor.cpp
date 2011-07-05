/* $Id: shpostor.cpp,v 1.1 2006/01/29 16:14:34 geomannl Exp $ */
#include "../ship.h"
#include "../melee/mview.h"
#include "../frame.h"
REGISTER_FILE

/** Copy of the Gerbillian Orion
 */

class OstokOrionWarhead;

class OstokOrion : public Ship
{
	public:
		double      weaponRange;
		double      weaponVelocity1;
		double      weaponMaxSpeed;
		int         weaponDamage1;
		int         weaponDamage2;
		int         weaponArmour1;
		int         weaponArmour2;
		double      weaponRelativity;
		int         weaponFramesToIgnition;
		int         weaponFramesOfThrust;
		int         weaponFramesOfCoasting;
		double      weaponAcceleration;
		double      weaponReleaseAngle1;
		double      weaponReleaseAngle2;
		double      weaponHotspotRate;
		double      weaponMass;
		double      weaponAngleIncrement;
		int         weaponAllLaunch;
		bool        bWiggle;

		int         bombLifetime;
		double      bombDamage, bombArmour;
		double      bombBlastRange, bombKick, bombVelocity, boost_max;
		double      bombExplodeDist;
		double      safeAngle;

		OstokOrionWarhead* bomb;

	public:
		OstokOrion(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
};

// ***********************************************

/* The Koanua Missle */
class OstokOrionMissile : public Missile
{
	public:
	private:
		bool isBurning;
		bool isCoasting;
		int side;
		double  period, rotatetime, accel;
		double offset;
		double hotspot_frame;

	public:
		OstokOrionMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, int side, bool bWiggle);

		Ship* creator;
		bool bWiggle;
		double hotspot_rate;
		double facingAngle;
		double framesToIgnition;
		double acceleration;
		double mass;
		double ignitionSpeed;
		double framesOfBurn;
		double framesOfCoasting;
		double maxSpeed;
		double damageAfterIgnition;
		double armourAfterIgnition;
		void calculate(void);
};

// ***********************************************

class OstokOrionWarheadExplosion : public Presence
{
	public:
	protected:
		Vector2 *xp, *xv;
		int     num, lifetime, life_counter, color;
	public:
		OstokOrionWarheadExplosion();
		OstokOrionWarheadExplosion(Vector2 opos, double ov, int onum, int olife, int ocolor);
		virtual void calculate();
		virtual void animate(Frame *space);
		virtual ~OstokOrionWarheadExplosion();
};

// ***********************************************

class OstokOrionWarheadExplosion2 : public OstokOrionWarheadExplosion
{
	public:
	public:
		OstokOrionWarheadExplosion2(Vector2 opos, double ov, int onum, int olife, int ocolor, SpaceObject* otarg);
};

// ***********************************************

class OstokOrionWarhead : public Missile
{
	public:
		double      blast_range, old_range;
		double      blast_damage, kick, lifetime, boost_max;
		double      safe_range;

		double      safeAngle;
		double      hotspot_rate;

		SpaceLocation* myShip;

		bool        active;

	public:
		OstokOrionWarhead (SpaceLocation *creator, Vector2 rpos, double ov, double oangle, double odamage,
			double oarmour, SpaceSprite *osprite, double oblast_range,
			int olifetime, double okick, double oblast_max, double osafeAngle, double osafe_range);
		virtual void calculate();
		virtual void animateExplosion();
};

// ***********************************************

OstokOrion::OstokOrion(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	bombLifetime    = int(tw_get_config_float("Special", "Lifetime", 0) * 1000);
	bombDamage      = tw_get_config_float("Special", "Damage", 0);
	bombArmour      = tw_get_config_float("Special", "Armour", 0);
	bombBlastRange  = scale_range(tw_get_config_float("Special", "BlastRange", 0));
	bombKick        = scale_velocity(tw_get_config_float("Special", "Kick", 0));
	bombVelocity    = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	boost_max       = scale_velocity(tw_get_config_float("Special", "TopBoostSpeed", 0));
	bombExplodeDist = scale_range(tw_get_config_float("Special", "ExplodeDist",0));

	weaponAllLaunch = tw_get_config_int("Weapon", "OrderedIgnition", 0);
	weaponFramesOfCoasting = tw_get_config_int("Weapon", "FramesOfCoasting", 0);
	weaponVelocity1 = scale_velocity(tw_get_config_float("Weapon", "Velocity1", 0));
	weaponDamage1   = tw_get_config_int("Weapon", "Damage1", 0);
	weaponArmour1   = tw_get_config_int("Weapon", "Armour1", 0);
	weaponMaxSpeed  = scale_velocity(tw_get_config_float("Weapon", "MaxSpeed", 0));
	weaponDamage2   = tw_get_config_int("Weapon", "Damage2", 0);
	weaponArmour2   = tw_get_config_int("Weapon", "Armour2", 0);
	weaponHotspotRate = tw_get_config_float("Weapon", "HotspotRate",0);
	weaponFramesToIgnition = tw_get_config_int("Weapon", "FramesToIgnition", 0);
	weaponFramesOfThrust = tw_get_config_int("Weapon", "FramesOfThrust", 0);
	weaponReleaseAngle1 = tw_get_config_float("Weapon", "ReleaseAngle1", 0)*ANGLE_RATIO;
	weaponReleaseAngle2 = tw_get_config_float("Weapon", "ReleaseAngle2", 0)*ANGLE_RATIO;
	weaponAcceleration = scale_acceleration(tw_get_config_float("Weapon", "AccelRate",0), weaponHotspotRate);
	weaponMass = tw_get_config_float("Weapon", "Mass", 0);
	weaponAngleIncrement = tw_get_config_float("Weapon", "AngleIncrement", 0)*ANGLE_RATIO;
	bWiggle = bool(tw_get_config_int("Weapon", "Wiggle", 1));

	safeAngle = tw_get_config_float("Special", "SafeAngle", 0)*ANGLE_RATIO;

	bomb = NULL;
}


// ***********************************************

void OstokOrion::calculate()
{
	STACKTRACE;
	if (bomb && (bomb->state == 0 || !bomb->exists()))
		bomb = NULL;

	Ship::calculate();
}


// ***********************************************

int OstokOrion::activate_weapon()
{
	STACKTRACE;
	for(int i=1; i < 4; i++) {
		for(int j=-1; j < 2; j+=2) {
			OstokOrionMissile* K;

			double a;

			a = weaponAngleIncrement*i;
			if (j == 1) {
				a = -a;
				a += weaponReleaseAngle2;
			}
			else
				a += weaponReleaseAngle1;

			K = new OstokOrionMissile(0, turn_step+angle+ a, weaponVelocity1*i, weaponDamage1,
				scale_range(1000), weaponArmour1, this, data->spriteWeapon, j, bWiggle);

			K->hotspot_rate     = scale_frames(weaponHotspotRate);
			K->framesToIgnition = weaponFramesToIgnition * (weaponAllLaunch ? i : 1);
			K->framesOfBurn     = weaponFramesOfThrust;
			K->framesOfCoasting = weaponFramesOfCoasting;
			K->facingAngle      = angle+turn_step;
			K->creator          =this;
			K->damageAfterIgnition = weaponDamage2;
			K->armourAfterIgnition = weaponArmour2;
			K->acceleration     = weaponAcceleration;
			K->mass             = weaponMass;
			K->maxSpeed         = weaponMaxSpeed;

			game->add(K);
		}
	}

	return(TRUE);
}


// ***********************************************

int OstokOrion::activate_special()
{
	STACKTRACE;
	if (bomb)
		return false;

	add(bomb = new OstokOrionWarhead(this, Vector2(0, size.y / -2.0), bombVelocity, angle+PI, bombDamage, bombArmour, data->spriteSpecial,
		bombBlastRange, bombLifetime, bombKick, boost_max, safeAngle, bombExplodeDist));

	return true;
}


// ***********************************************

//		TAU BOMBER CODE

// ***********************************************

OstokOrionWarhead::OstokOrionWarhead (SpaceLocation *creator, Vector2 rpos, double ov, double oangle, double odamage,
double oarmour, SpaceSprite *osprite, double oblast_range,
int olifetime, double okick, double oboost_max, double osafeAngle, double osafe_range) :
Missile(creator, rpos, oangle, ov, 0, 1e40, oarmour, creator, osprite, 1.0),
blast_range(oblast_range), old_range(1e40), blast_damage(odamage), kick(okick),
lifetime(olifetime),
active(false)

{
	myShip = ship;
	id = SPACE_SHOT;
	collide_flag_sameteam = 0;
	mass = 0.01;
	boost_max = oboost_max;
	safeAngle = osafeAngle;
	safe_range = osafe_range;
}


// ***********************************************

void OstokOrionWarhead::calculate()
{
	STACKTRACE;
	sprite_index++;
	sprite_index &= 63;

	if (ship && distance(ship) > safe_range && (!ship->exists() || !ship->fire_special)) {
		ship = NULL;
		active = true;
		damage(this, 999);
	}

	Missile::calculate();
}


// ***********************************************

void OstokOrionWarhead::animateExplosion()
{
	STACKTRACE;
	explosionSample = data->sampleSpecial[1];
	explosionSprite = data->spriteSpecialExplosion;
	explosionFrameCount = 10;
	explosionFrameSize = 50;

	Query q;
	double r;
	int d;
	Vector2 stuff;
	for (q.begin(this, OBJECT_LAYERS, blast_range); q.currento; q.next()) {
		if (!q.current->isObject())
			continue;
		r = distance(q.currento);
		if (r > blast_range)
			continue;
		r = (blast_range - distance(q.currento)) / blast_range;

		if (r > 0.5)
			d = (int)ceil(blast_damage * (r - 0.5));
		else
			d = 0;

		stuff = q.currento->normal_pos();

		if (q.currento != myShip || (q.currento == myShip && abs(iround(trajectory_angle(q.currento) - q.currento->get_angle())) > safeAngle)) {
			//stuff = q.currento->normal_pos()/* + (unit_vector(q.currento->trajectory_angle(this)) * q.currento->get_size()/2.5)*/;

			add(new OstokOrionWarheadExplosion2(stuff, scale_velocity(tw_random(2)+5), 100, 600, tw_makecol(255,128,0), this));
			damage(q.currento, (int)ceil(r * blast_damage) - d, d);

			int i = iround_down(((int)ceil(r * blast_damage)) / 2);
			if (i >= BOOM_SAMPLES)
				i = BOOM_SAMPLES - 1;
			play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat), iround(36*r + 220));
		}
		else if (q.currento == myShip) {
			/* + (unit_vector(q.currento->trajectory_angle(this)) * q.currento->get_size()/4)*/;

			add(new OstokOrionWarheadExplosion(stuff, scale_velocity(tw_random(2)+5), 100, 1000, tw_makecol(128,128,255)));
		}

		if ((q.currento->mass > 0) && (!q.currento->isPlanet()))
			q.currento->accelerate(this, trajectory_angle(q.currento), kick * r / ((q.currento->mass > 1)?sqrt(q.currento->mass):1), boost_max);
	}
	add(new OstokOrionWarheadExplosion(pos, scale_velocity(tw_random(20)+25), 150, 450, tw_makecol(255,128,0)));

	Missile::animateExplosion();
}


// ***********************************************

OstokOrionWarheadExplosion::OstokOrionWarheadExplosion()
{
	STACKTRACE;
}


OstokOrionWarheadExplosion::OstokOrionWarheadExplosion(Vector2 opos, double ov, int onum, int olife, int ocolor) :
Presence(), num(onum), lifetime(olife), life_counter(0), color(ocolor)
{
	STACKTRACE;
	if (onum <= 0) {
		state = 0;
		return;
	}
	set_depth(DEPTH_EXPLOSIONS);
	xp = new Vector2[num];
	xv = new Vector2[num];
	int i;
	for (i=0; i<num; i++) {
		xp[i] = opos;
		xv[i] = ov * (0.5+sqrt(sqrt(tw_random(1.0)))) * unit_vector(tw_random(PI2));
	}
}


// ***********************************************

void OstokOrionWarheadExplosion::calculate()
{
	STACKTRACE;
	life_counter += frame_time;
	if (life_counter >= lifetime) {
		state = 0;
		return;
	}
	int i;
	for (i=0; i<num; i++)
		xp[i] += xv[i] * frame_time;
}


// ***********************************************

void OstokOrionWarheadExplosion::animate(Frame *space)
{
	STACKTRACE;
	if (state == 0)
		return;
	int i, j;
	double t = 1 - life_counter/(double)lifetime;
	double  x0, y0, dx, dy;
	int xi, yi;
	Vector2 p0;
	drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	for (i=0; i<num; i++) {
		p0 = corner(xp[i]);
		x0 = p0.x;
		y0 = p0.y;
		p0 = unit_vector(xv[i]) * 3 * space_zoom;
		dx = p0.x;
		dy = p0.y;
		for (j=3; j>=0; j--) {
			if (space_zoom <= 1)
				set_trans_blender(0, 0, 0, iround(space_zoom * 255 * t * (4-j) / 4.0));
			else
				set_trans_blender(0, 0, 0, iround(1 * 255 * t * (4-j) / 4.0));
			xi = iround(x0 - dx * j);
			yi = iround(y0 - dy * j);
			tw_putpixel(space->surface, xi, yi, color);
			space->add_pixel(xi, yi);
		}
	}
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


// ***********************************************

OstokOrionWarheadExplosion::~OstokOrionWarheadExplosion()
{
	if (num > 0) {
		delete xp;
		delete xv;
	}
}


// ***********************************************

OstokOrionWarheadExplosion2::OstokOrionWarheadExplosion2(Vector2 opos, double ov, int onum, int olife, int ocolor, SpaceObject* otarg)
{
	STACKTRACE;
	num = onum;
	lifetime = olife;
	life_counter = 0;
	color = ocolor;

	if (onum <= 0) {
		state = 0;
		return;
	}
	set_depth(DEPTH_EXPLOSIONS);
	xp = new Vector2[num];
	xv = new Vector2[num];
	int i;

	for (i=0; i<num; i++) {
		xp[i] = opos;
		xv[i] = ov * ((0.5+sqrt(sqrt(random(1.0)))) * unit_vector(trajectory_angle(opos, otarg->pos) - PI/2 + (random(PI))));
	}
}


// ***********************************************

//		KOANUA+VIOGEN MISSILE CODE

// ***********************************************

OstokOrionMissile::OstokOrionMissile(Vector2 opos, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite, int oside, bool obWiggle)
: Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship,osprite, 1.0)
{
	STACKTRACE;
	bWiggle = obWiggle;
	side = oside;
	explosionSprite = data->spriteWeaponExplosion;
	isBurning = false;
	isCoasting = false;
	facingAngle = oangle;

	hotspot_frame = 0;
	period = 1.0;
	rotatetime = 0;

	accel = scale_acceleration(7);
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


// ***********************************************

void OstokOrionMissile::calculate()
{
	STACKTRACE;
	if (!isBurning && !isCoasting)
	if (framesToIgnition>=0) {
		framesToIgnition -= frame_time;
	} else {
		framesToIgnition = 0;
		isBurning = true;
		vel = unit_vector(angle + (PI/2 * side)) / 2;
		angle = facingAngle;
		damage_factor = damageAfterIgnition;
		armour = armourAfterIgnition;
		range = 99999;
		play_sound2(data->sampleWeapon[1]);

		offset = distance(ship);
	} else {
		Vector2 normal;
		normal = Vector2(-vel.y, vel.x);
		normalize(normal);

		rotatetime += frame_time * 1E-3;

		if (bWiggle) {
			double a;
			a = sin(PI2 * rotatetime / period + (PI/2 * side));

			vel += (accel*frame_time) * a * normal;
			angle = vel.atan();
		}

		if (hotspot_frame > 0)
			hotspot_frame -= frame_time;

		if (hotspot_frame <= 0) {
			// release some smoke trail.
			Vector2 relpos;

			relpos = Vector2( random(10.0)-5.0, -40.0 + random(10.0) );

			Animation *anim;
			anim = new Animation(this, pos + rotate(relpos, angle-0.5*PI),
				data->spriteExtra, 0, data->spriteExtra->frames(),
				100, LAYER_HOTSPOTS);
			add(anim);
			hotspot_frame += hotspot_rate;
		}
	}

	if (isBurning && !isCoasting) {
		if (framesOfBurn>=0) {
			framesOfBurn -= frame_time;
			accelerate_gravwhip (this, facingAngle, /*acceleration / mass*/ acceleration * frame_time, maxSpeed);
		} else {
			framesOfBurn = 0;
			isBurning = false;
			isCoasting = true;
		}
	}
	else if (isCoasting) {
		framesOfCoasting -= frame_time;
		if (framesOfCoasting<=0)
			state=0;
	}

	Missile::calculate();

	sprite_index = (get_index(facingAngle) + (int(isBurning) * 64));
}


REGISTER_SHIP(OstokOrion)
