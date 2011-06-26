/* $Id: shptaubo.cpp,v 1.14 2004/03/27 10:06:21 geomannl Exp $ */
#include <assert.h>
#include "../ship.h"
#include "../melee/mview.h"
#include "../frame.h"

REGISTER_FILE

class TauBomber : public Ship
{
	int         bombLifetime;
	double      bombDamage, bombArmour;
	double      bombProximity, bombBlastRange, bombKick;

	int         decoyLifetime, side, decoyEffect;
	double      decoyRange, decoySlowdown, decoyVelocity;

	bool        can_launch_bomb;

	public:
		TauBomber   (Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		virtual int  activate_weapon();
		virtual int  activate_special();
		virtual void calculate();
		virtual void calculate_hotspots();
};

class TauBomberBomb : public Missile
{
	double      blast_range, proximity_range, old_range, kick;
	int     rotation_index;
	double      blast_damage, lifetime;
	double      rotation_angle;
	SpaceObject *tgt;
	bool        active;

	public:
		TauBomberBomb (SpaceLocation *creator, double ox, double oy, double oangle, double odamage,
			double oarmour, SpaceSprite *osprite, double oblast_range, double oproximity,
			int olifetime, double okick);
		virtual void calculate();
		virtual void animate(Frame *space);
		virtual void animateExplosion();
};

class TauBomberBombExplosion : public Presence
{
	Vector2 *xp, *xv;
	int     num, lifetime, life_counter, color;
	public:
		TauBomberBombExplosion(Vector2 opos, double ov, int onum, int olife, int ocolor);
		virtual void calculate();
		virtual void animate(Frame *space);
		virtual ~TauBomberBombExplosion();
};

								 //AnimatedShot
class TauBomberDecoy : public Shot
{
	int     lifetime, lifetime_max;
	double  slowdown;

	public:
		TauBomberDecoy (SpaceLocation *creator, double ox, double oy, double oangle, double ov,
			SpaceSprite *osprite, int olifetime, double er, double oslowdown, int effect);
		virtual void calculate();
};

class TauBomberJam : public SpaceLocation
{
	int lifetime;
	SpaceObject *host, *tgt;

	public:
		TauBomberJam (SpaceObject *creator, SpaceObject *ohost, int olifetime);
		virtual void calculate();
};

TauBomber::TauBomber(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	bombLifetime    = int(get_config_float("Bomb", "Lifetime", 0) * 1000);
	bombDamage      = get_config_float("Bomb", "Damage", 0);
	bombArmour      = get_config_float("Bomb", "Armour", 0);
	bombProximity   = scale_range(get_config_float("Bomb", "Proximity", 0));
	bombBlastRange  = scale_range(get_config_float("Bomb", "BlastRange", 0));
	bombKick        = scale_velocity(get_config_float("Bomb", "Kick", 0));

	decoyLifetime   = int(get_config_float("Decoy", "Lifetime", 0) * 1000);
	decoyRange      = scale_range(get_config_float("Decoy", "Range", 0));
	decoyEffect     = get_config_int("Decoy", "Effect", 50);
	if (decoyEffect < 0) decoyEffect = 0;
	else if (decoyEffect > 100) decoyEffect = 100;
	decoySlowdown   = get_config_float("Decoy", "Slowdown", 0) / 10000.0;
	decoyVelocity   = scale_velocity(get_config_float("Decoy", "Velocity", 0));

	can_launch_bomb = true;

	side            = -1;

}


int TauBomber::activate_weapon()
{
	STACKTRACE;
	if (!can_launch_bomb) return false;
	add(new TauBomberBomb(this, 0, 0, angle, bombDamage, bombArmour, data->spriteWeapon,
		bombBlastRange, bombProximity, bombLifetime, bombKick));
	can_launch_bomb = false;
	return true;
}


int TauBomber::activate_special()
{
	STACKTRACE;
	add(new TauBomberDecoy(this, side*5, -10, angle + side*PI*11/12.0, decoyVelocity,
		data->spriteSpecial, decoyLifetime, decoyRange, decoySlowdown, decoyEffect));
	side *= -1;
	return true;
}


void TauBomber::calculate()
{
	STACKTRACE;
	if (!fire_weapon) can_launch_bomb = true;
	Ship::calculate();
}


void TauBomber::calculate_hotspots()
{
	STACKTRACE;
	if ((thrust) && (hotspot_frame <= 0)) {
		game->add(new Animation(this, pos - 17*unit_vector(angle),
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0)
		hotspot_frame -= frame_time;
	return;
}


TauBomberBomb::TauBomberBomb (SpaceLocation *creator, double ox, double oy, double oangle, double odamage,
double oarmour, SpaceSprite *osprite, double oblast_range, double oproximity,
int olifetime, double okick) :
Missile(creator, Vector2(ox, oy), oangle, 0, 0, 1e40, oarmour, creator, osprite, 1.0),
blast_range(oblast_range), proximity_range(oproximity), old_range(1e40), kick(okick), blast_damage(odamage),  lifetime(olifetime),
tgt(NULL), active(false)

{
	id = SPACE_OBJECT;
	collide_flag_sameteam = 0;
	mass = 0.01;

	rotation_angle = 0;
	//	attributes &= ~ATTRIB_SHOT;

}


void TauBomberBomb::calculate()
{
	STACKTRACE;
	if (ship && !active)
	if ((!ship->exists()) || (!ship->fire_weapon)) {
		ship = NULL;
		active = true;
	}

	double d_a = normalize(atan(vel) - angle, PI2);
	if (d_a > PI) d_a -= PI2;
	d_a *= 1 - exp(-0.004*magnitude(vel)*frame_time);
	angle = normalize(angle + d_a, PI2);

	sprite_index = iround(angle / (PI2/64)) + 16;
	sprite_index &= 63;

	if (lifetime > 0) lifetime -= frame_time;

	SpaceObject *old_tgt = tgt;
	tgt = NULL;

	double r0 = 1e40;
	Query q;
	for (q.begin(this, OBJECT_LAYERS, proximity_range); q.currento; q.next())
		if (q.currento->isShip() && q.currento->exists()
		&& (!q.currento->isInvisible()) && (!q.currento->sameTeam(this))) {

			double r;
		r = distance(q.currento);

		if (r < r0) {
			r0 = r;
			tgt = q.currento;
		}
	}
	q.end();

	if ((old_tgt?old_tgt->exists():false) && (tgt?(r0>old_range):true) && active) {
		damage(this, 9999); return;
	}
	old_range = r0;
	if (lifetime <= 0) {
		if ((old_tgt?old_tgt->exists():false) && active) {
			damage(this, 999);
		}
		else    state = 0;
	}

	rotation_angle += 1.1 * magnitude(vel) * frame_time * PI/180;
	rotation_angle = normalize(rotation_angle, PI2);
	rotation_index = iround(rotation_angle / (PI2/64));
	rotation_index &= 15;
}


void TauBomberBomb::animate(Frame *space)
{
	STACKTRACE;
	sprite->animate(pos,sprite_index+rotation_index*64,space);
}


void TauBomberBomb::animateExplosion()
{
	STACKTRACE;
	if (active) {

		explosionSample = data->sampleWeapon[1];
		explosionSprite = data->spriteWeaponExplosion;
		explosionFrameCount = 10;
		explosionFrameSize = 50;

		Query q;
		double r;
		int d;
		for (q.begin(this, OBJECT_LAYERS, blast_range); q.currento; q.next()) {
			r = distance(q.currento);
			if (r > blast_range) continue;
			r = (blast_range - distance(q.currento)) / blast_range;
			if (r > 0.5)
				d = (int)ceil(blast_damage * (r - 0.5));
			else    d = 0;
			damage(q.currento, (int)ceil(r * blast_damage) - d, d);
			if ((q.currento->mass > 0) && (!q.currento->isPlanet()))
				q.currento->accelerate(this, trajectory_angle(q.currento), kick * r / ((q.currento->mass > 1)?sqrt(q.currento->mass):1), MAX_SPEED);
		}
		add(new TauBomberBombExplosion(pos, scale_velocity(70), 150, 450, makecol(255,240,140)));
	}

	Missile::animateExplosion();
}


TauBomberBombExplosion::TauBomberBombExplosion(Vector2 opos, double ov, int onum, int olife, int ocolor) :
Presence(), num(onum), lifetime(olife), life_counter(0), color(ocolor)
{
	STACKTRACE;
	if (onum <= 0) {
		state = 0; return;
	}
	set_depth(DEPTH_EXPLOSIONS);
	xp = new Vector2[num];
	xv = new Vector2[num];
	int i;
	for (i=0; i<num; i++) {
		xp[i] = opos;
		xv[i] = ov * (0.5+sqrt(sqrt((random()%1000000001)/1000000000.0))) * unit_vector(PI2 * (random()%1000000)/1000000.0);
	}
}


void TauBomberBombExplosion::calculate()
{
	STACKTRACE;
	life_counter += frame_time;
	if (life_counter >= lifetime) {
		state = 0; return;
	}
	int i;
	for (i=0; i<num; i++)
		xp[i] += xv[i] * frame_time;
}


void TauBomberBombExplosion::animate(Frame *space)
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
				set_trans_blender(0, 0, 0, iround(1* 255 * t * (4-j) / 4.0));
			xi = iround(x0 - dx * j);
			yi = iround(y0 - dy * j);
			putpixel(space->surface, xi, yi, color);
			space->add_pixel(xi, yi);
		}
	}
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


TauBomberBombExplosion::~TauBomberBombExplosion()
{
	if (num > 0) {
		delete xp;
		delete xv;
	}
}


TauBomberDecoy::TauBomberDecoy (SpaceLocation *creator, double ox, double oy, double oangle, double ov,
SpaceSprite *osprite, int olifetime, double er, double oslowdown, int effect) :
Shot(creator, Vector2(ox, oy), oangle, ov, 1, 1e40, 1, creator, osprite, 1.0),
lifetime(olifetime), lifetime_max(olifetime), slowdown(oslowdown)
{
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;

	Query q;
	for (q.begin(this, ALL_LAYERS, er); q.current; q.next())
		if ((q.current->target == creator) && (tw_random()%100 < effect))
			add(new TauBomberJam(this, q.currento, lifetime));

	sprite_index = 0;
}


void TauBomberDecoy::calculate()
{
	STACKTRACE;
	d = 0.1;
	Shot::calculate();

	lifetime -= frame_time;

	sprite_index = iround(floor(40 * (1 - lifetime/(double)lifetime_max)));
	if (sprite_index >= 40) {
		state =0; return;
	}
	sprite_index = sprite_index % 40;

	double a = exp(-slowdown*frame_time);
	vel *= a;

	damage_factor = lifetime/(double)lifetime_max;
}


TauBomberJam::TauBomberJam (SpaceObject *creator, SpaceObject *ohost, int olifetime) :
SpaceLocation(creator, ohost->normal_pos(), 0), tgt(creator)
{
	lifetime = olifetime;
	host = ohost;
	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	host->target = tgt;
	if (host->isShip())
		((Ship*)host)->control->target = tgt;
}


void TauBomberJam::calculate()
{
	STACKTRACE;
	if ((lifetime -= frame_time) <= 0) {
		state = 0; return;
	}
	if ( (!(host && host->exists())) || (!(tgt && tgt->exists())) ) {
		state = 0;
		host = 0;
		tgt = 0;
		return;
	}
	pos = host->normal_pos();
	host->target = tgt;
	if (host->isShip())
		((Ship*)host)->control->target = tgt;
}


REGISTER_SHIP(TauBomber)
