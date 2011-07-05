/* $Id: shpsefna.cpp,v 1.1 2006/02/20 23:05:12 geomannl Exp $ */
#include "../ship.h"
#include "../frame.h"
REGISTER_FILE

/*

  A ship with a harpoon, and a short-range laser.

*/

class Hook;

class SefyNautilus : public Ship
{
	public:

		int     weaponColor;
		double  weaponRange;
		int     weaponFrames;
		int     weaponDamage;

		double  TurnRateFactor, HookDamage;

	public:

		Hook    *hook;
		double  unrolltime;

		double  specialRange, specialDelay, specialOscFreq, specialRelVelocity,
			specialSegLength, specialArmour, specialSprConst, specialLifeTime;

		SefyNautilus(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual int handle_damage(SpaceLocation *src, double normal, double direct=0);
};

static const int maxnodes = 50;

class Hook : public SpaceObject
{
	public:
		double  armour;

		struct ropenode
		{
			Vector2     pos, vel, acc;
			int         col;
			double      dL;
		} ropenode[maxnodes];

		int     Nnodes, hooklocked;
		double  L, Lmax, ropestart, roll_time, exist_time, life_time,
			oscperiod, ejvel, ropeseglen,
			hookfixdist, hookfixangle, hookfixorientation, hooksize, springconst;
		SpaceLocation   *hooktarget;

		double turnrate_factor;

		Hook(SefyNautilus *creator, Vector2 orelpos, double TurnRateFactor, double HookDamage, SpaceSprite *osprite);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *src, double normal, double direct=0);
		virtual void animate ( Frame *space );
		void        animate_ropeseg( Frame *space, Vector2 pos1, Vector2 pos2, int ropecol );

		void unroll();
};

SefyNautilus::SefyNautilus(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	weaponColor  = tw_get_config_int("Weapon", "Color", 0);
	weaponFrames = tw_get_config_int("Weapon", "Frames", 0);
	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);

	// time for retracting 1 piece of rope, in seconds
	specialLifeTime = tw_get_config_float("Special", "LifeTime", 60.0);
	// time for retracting 1 piece of rope, in seconds
	specialDelay    = tw_get_config_float("Special", "Delay", 0.5);
	// osc freq. of the rope when it's released
	specialOscFreq = tw_get_config_float("Special", "OscFreq", 5.0);
	// extra velocity of the rope when release
	specialRelVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	// default segment length of a piece of rope (in pixels)
	specialSegLength = tw_get_config_float("Special", "SegLength", 2.0);
	// armour of the hook
	specialArmour = tw_get_config_float("Special", "Armour", 5.0);
	// spring constant: higher value, faster oscillations
	specialSprConst = tw_get_config_float("Special", "SprConst", 250.0);

	TurnRateFactor = tw_get_config_float("Special", "TurnRateFactor", 250.0);
	HookDamage = tw_get_config_float("Special", "Damage", 250.0);

	hook = 0;
	unrolltime = 0;
}


int SefyNautilus::activate_weapon()
{
	STACKTRACE;

	if ( this->nextkeys & keyflag::special )
		return FALSE;

	const int N = 2;
	Vector2 Laserpositions[N] = { Vector2(-10, 50), Vector2(10, 50) };
	double  angles[N] = { -0.1, 0.1 };

	for ( int i = 0; i < N; ++i ) {
		game->add(new Laser(this, get_angle()+angles[i], pallete_color[weaponColor],
								 // synching=true
			weaponRange, weaponDamage, weaponFrames, this, Laserpositions[i], true ));
	}

	return TRUE;
}


int SefyNautilus::activate_special()
{
	STACKTRACE;

	unrolltime += frame_time * 1E-3;

	if ( hook && hook->exists() ) {

		if (
			!(hook->hooktarget &&
		hook->hooktarget->exists()) ) {
			// the hook hasn't attached yet - cancel this shot.
			hook->state = 0;
			return FALSE;
		}

		if  (unrolltime > specialDelay) {
			hook->unroll();
			//hook->hooklocked = 1;		// stop unrolling any further
			//--hook->Nnodes;
			unrolltime -= specialDelay;
		}

		return FALSE;
	}

	hook = new Hook(this, 100*unit_vector(angle), TurnRateFactor, HookDamage, data->spriteSpecial);
	game->add( hook );

	return TRUE;
}


void SefyNautilus::calculate()
{
	STACKTRACE;

	Ship::calculate();

	// release the hook:
	if ( (this->nextkeys & keyflag::special) && (this->nextkeys & keyflag::fire ) )
		if ( hook && hook->exists() )
			hook->state = 0;

	// detect when the hook disappears - reset the pointer to 0 in that case
	if ( !(hook && hook->exists()) )
		hook = 0;
}


int SefyNautilus::handle_damage(SpaceLocation *src, double normal, double direct)
{
	STACKTRACE;
	Ship::handle_damage(src, normal, direct);

	return iround(normal + direct);
}


Hook::Hook(SefyNautilus *creator, Vector2 orelpos, double TurnRateFactor, double HookDamage, SpaceSprite *osprite)
:
SpaceObject(creator, creator->pos+orelpos, creator->angle, osprite)
{
	STACKTRACE;
	//	double	specialRange, specialDelay
	//			;

	damage_factor = HookDamage;
	turnrate_factor = TurnRateFactor;

	sprite_index = get_index(angle);

								 // velocity of the ejected hook (0.5).
	ejvel = creator->specialRelVelocity;
								 // this is in ms
	vel = ship->vel + ejvel * unit_vector(ship->angle);

	armour = creator->specialArmour;
	Nnodes = 0;

	roll_time = 0;
	exist_time = 0;
	life_time = creator->specialLifeTime;
	oscperiod = creator->specialOscFreq;

	ropestart = 50.0;
								 // minimum pixels length of a rope segment, approx.
	ropeseglen = creator->specialSegLength;

	hooktarget = 0;
	hooklocked = 0;

	springconst = creator->specialSprConst;

	hooksize = 9;				 // from center to the eye for the rope.

	layer = LAYER_SHOTS;
	set_depth(DEPTH_SHOTS);

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	isblockingweapons = false;
}


void Hook::calculate()
{
	STACKTRACE;
	exist_time += frame_time * 1E-3;
	if ( exist_time > life_time )
		state = 0;

	// if it's completely unrolled ...
	if ( hooklocked && Nnodes <= 0 )
		state = 0;

	// or if the host ship has gone
	if ( !(ship && ship->exists()) ) {
		ship = 0;
		state = 0;
	}

	if ( hooktarget && hooktarget->exists() ) {
		vel = hooktarget->vel;
		angle = hooktarget->angle + hookfixangle;
		sprite_index = get_index(angle);
		pos = hooktarget->pos + hookfixdist * unit_vector(hooktarget->angle + hookfixorientation);
	} else if (hooklocked)
	{
		state = 0;
		hooktarget = 0;
	}

	if ( state == 0 )
		return;

	SpaceObject::calculate();

	if ( !hooklocked)
		roll_time += frame_time * 1E-3;
	else						 // otherwise it keeps pumping energy into the line
		roll_time = 0.0;

	Vector2 hookendpos;
	hookendpos = pos - hooksize * unit_vector(angle);

	Vector2 ejpos;
	if (ship && ship->exists() ) {
		ejpos = ship->pos + ropestart * unit_vector(ship->angle) +
			15 * sin(oscperiod*roll_time) * unit_vector(ship->angle + 0.5*PI);

	} else
	ejpos = 0;

	if (ship && ship->exists() && !hooklocked ) {

		// extend the elastic rope slowly

		double dL;
		if ( Nnodes > 0 )
			dL = magnitude(min_delta(ropenode[Nnodes-1].pos, ejpos, map_size));
		else
			dL = magnitude(min_delta(hookendpos, ejpos, map_size));

		if ( dL > ropeseglen && Nnodes < maxnodes ) {

			ropenode[Nnodes].pos = ejpos;
								 // *1E+3 because from now we do calculations in seconds instead of ms.
			ropenode[Nnodes].vel = (ship->vel + ejvel * unit_vector(ship->angle)) * 1E+3;
								 // changes color every 4 pieces ?
			ropenode[Nnodes].col = pallete_color[11 + (Nnodes/4) % 3];
			dL = ropeseglen;
								 // relaxed length of the segment between nodes i and i-1.
			ropenode[Nnodes].dL = dL;

			++ Nnodes;

			if ( Nnodes == maxnodes )
				state = 0;		 // no target found ... poor thing ;)

		}

	}

	// calculate the nodes on the elastic rope (except the end-points, which are fixed:

	double k = springconst;		 // 250.0

	int i;

	Vector2 D;
	double  R;

	//	ropenode[0].acc = ..;
	//	ropenode[Nnodes-1].acc = ..;

	int Ninterpol, iinterpol;

	// this may be needed if values of the spring constant are high
	Ninterpol = 10;
	double dt = frame_time * 1E-3 / Ninterpol;

	for ( iinterpol = 0; iinterpol < Ninterpol; ++iinterpol ) {

		// reset forces
		for ( i = 0; i < Nnodes; ++i )
			ropenode[i].acc = 0;

		// forces between rope nodes
		for ( i = 0; i < Nnodes; ++i ) {
			if ( i > 0 ) {
				D = min_delta(ropenode[i-1].pos - ropenode[i].pos, map_size);
				R = D.length();
				L = ropenode[i].dL;

				if ( R != 0 )
					ropenode[i].acc += k * (R - L) * D / R;
			}

			if ( i < Nnodes-1 ) {
				D = min_delta(ropenode[i+1].pos - ropenode[i].pos, map_size);
				R = D.length();
				L = ropenode[i+1].dL;

				if ( R != 0 )
					ropenode[i].acc += k * (R - L) * D / R;
			}
		}

		// attached to the hook
		Vector2 hookacc;
		hookacc = 0;

		if ( Nnodes > 0 ) {
			i = 0;

			D = min_delta(hookendpos - ropenode[i].pos, map_size);
			R = D.length();
			L = ropenode[i].dL;

			hookacc = k * (R - L) * D / R;

			if ( R != 0 )
				ropenode[i].acc += hookacc;
		}

		// attached to the ship, if it still exists...
		if ( ship && ship->exists() && Nnodes > 0 ) {
			i = Nnodes-1;

			D = min_delta(ejpos - ropenode[i].pos, map_size);
			R = D.length();
			L = ropeseglen;		 // should be i+1, but that doesn't exist
			if ( R != 0 )
				ropenode[i].acc += k * (R - L) * D / R;;
		}

		// slow down a little, for stability
		for ( i = 0; i < Nnodes; ++i )
			ropenode[i].acc -= 10.0 * ropenode[i].vel * dt;

		// apply accelerations (also to the end points)
		for ( i = 0; i < Nnodes; ++i ) {
			ropenode[i].vel += ropenode[i].acc * dt;
			ropenode[i].pos += ropenode[i].vel * dt;
		}

		// apply acceleration to the target:
		if ( hooktarget && hooktarget->exists() ) {
			hooktarget->vel -= 0.1 * 1E-3 * hookacc * dt;

			// and also some deceleration ...
			double a = 1 - 0.1*dt;
			if ( a < 0 ) a = 0;
			if ( a > 1 ) a = 1;
			hooktarget->vel *= a;

			// influence the angle of the enemy ship:

			a = atan(hookendpos - hooktarget->pos);

			double b, da, rotacc;
			b = atan(hookacc);

			da = b - a;

			rotacc = magnitude(hookacc) * sin(da);

			hooktarget->angle -= 0.25 * 1E-3 * rotacc * dt;

			// angle must never exceed half PI with the end of the rope...

			b = atan(ropenode[0].pos - hookendpos);

			da = b - a;

			while ( da >  PI )  da -= PI2;
			while ( da < -PI )  da += PI2;

			if ( da < -0.5*PI || da > 0.5*PI ) {
				double dacorr = fabs(da) - 0.5*PI;
				if ( dacorr > 5*PI2*dt )
					dacorr = 5*PI2*dt;

				if ( da < 0.0 )
					hooktarget->angle -= dacorr;
				else
					hooktarget->angle += dacorr;
			}

			// to stabilize, make sure neither vessels' speed ever exceeds the maximum:

			double V1, V2;

			if ( ship && ship->exists() ) {
				V1 = magnitude(ship->vel);
				V2 = 2 * ship->speed_max;
				if ( V1 > V2 )
					ship->vel *= V2 / V1;
			}

			if ( hooktarget->isShip() ) {
				V1 = magnitude(hooktarget->vel);
				V2 = 2 * ((Ship*) hooktarget)->speed_max;
				if ( V1 > V2 )
					hooktarget->vel *= V2 / V1;
			}

		}

	}

}


void Hook::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	SpaceObject::inflict_damage(other);
	damage_factor = 0;
}


int Hook::handle_damage(SpaceLocation *src, double normal, double direct)
{
	STACKTRACE;
	if ( src == hooktarget )
		return 0;

	if ( src->isShip() ) {
		if ( hooktarget )		 // if it's already attached to another ship
			return 0;

		if ( src == ship )
			return 0;			 // if it is the ship itself (doh)

		hooktarget = src;

		hookfixdist = magnitude(min_delta(pos, src->pos, map_size));

		// it makes a different, where you hit the enemy. In the side is better than head-on.
		// where it is attacked.
		hookfixorientation = atan(min_delta(pos, src->pos, map_size)) - src->angle + 0.2*PI*(random(2.0)-1.0);
		hookfixangle = angle - src->angle;

		collide_flag_anyone = 0;
		collide_flag_sameteam = 0;
		collide_flag_sameship = 0;

		// this is done by inflict-damage... but that applies it all the time, so let's handle that here, instead.
		//damage(src, damage_factor, 0.0);
		//damage_factor = 0;
		// then, reset the damage to zero...

		hooklocked = 1;			 // stop unrolling any further

		src->ship->turn_rate *= turnrate_factor;

		return 0;

	} else {

		SpaceObject::handle_damage(src, normal, direct);

		armour -= normal + direct;
		if ( armour <= 0 )
			state = 0;

		return iround(normal + direct);
	}

}


void Hook::animate_ropeseg( Frame *space, Vector2 pos1, Vector2 pos2, int ropecol )
{
	STACKTRACE;
	int ix1, iy1, ix2, iy2;

	Vector2 co;

	co = corner(pos1);
	ix1 = int(co.x);
	iy1 = int(co.y);

	co = corner(pos2);
	ix2 = int(co.x);
	iy2 = int(co.y);

	int dx, dy;
	dx = iround(min_delta(ix2, ix1, map_size.x));
	dy = iround(min_delta(iy2, iy1, map_size.y));
	ix2 = ix1 + dx;
	iy2 = iy1 + dy;

	// simulate visibility of a glittering line in the sun ...
	double a, colscale;
	a = atan2((double)dy,(double)dx);
	colscale = fabs(sin(a));	 // flat lying = less visible.

	int col, r, g, b;
	col = ropecol;
	r = iround(getr(col) * colscale);
	g = iround(getg(col) * colscale);
	b = iround(getb(col) * colscale);
	col = tw_makecol(r, g, b);

	// draw the line

	line(space->surface, ix1, iy1, ix2, iy2, col);
	space->add_line(ix1, iy1, ix2, iy2);

}


void Hook::animate ( Frame *space )
{
	SpaceObject::animate(space);

	// also, animate the rope, these are points in space

	int i;
	for ( i = 0; i < Nnodes-1; ++i ) {
		animate_ropeseg(space, ropenode[i].pos, ropenode[i+1].pos, ropenode[i+1].col);
	}

	// extra part, leading to the end of the hook:
	Vector2 hookendpos;
	hookendpos = pos - hooksize * unit_vector(angle);

	i = 0;
	animate_ropeseg(space, hookendpos, ropenode[i].pos, ropenode[i].col);

}


void Hook::unroll()
{
	STACKTRACE;
	hooklocked = 1;

	if ( Nnodes > 1 )
		--Nnodes;

	//	if ( Nnodes == 0 )
	//		return;

	/*
	Vector2 pos1old = ropenode[0].pos;
	Vector2 pos2old = ropenode[Nnodes].pos;
	Vector2 pos1new = pos1old;
	Vector2 pos2new = ropenode[Nnodes-1].pos;

	// spread the disappeared distance a little:

	int i;
	for ( i = Nnodes-1; i >= 0; --i )
	{
		ropenode[i] = ropenode[i-1];
	}
	*/

}


REGISTER_SHIP(SefyNautilus)
