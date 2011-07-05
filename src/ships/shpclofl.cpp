/* $Id: shpclofl.cpp,v 1.17 2005/09/03 19:48:56 geomannl Exp $ */
#include "../ship.h"

REGISTER_FILE
#include "../frame.h"
#include "../melee/mcbodies.h"
#include "../melee/mview.h"

/*

  Clowan Flurry

*/

static double   gravity_mindist, gravity_range, gravity_force, gravity_power,
gravity_exist_time, gravity_shipaccscale, gravity_whip, gravity_whip2;

class GravShot;

class ClowanFlurry : public Ship
{
	public:

		double  weaponTime, specialRange, weaponTailTime;
		int     weaponDamage, specialDamage, weaponTailNum;
		int     weaponArmour, specialArmour, weaponPlanetBounce;
		double  weaponVelocity, specialVelocity, weaponAccelRate;
		double  weaponTurnrate;
		double  weaponWhipVel, weaponWhipTurn, weaponSlowDown;

		GravShot    *Gshot;

		ClowanFlurry(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
};

class MissileTrace : public Presence
{
	public:
		SpaceObject *mother;
		double  tail_time, add_tail_time;
		int     Npos;
		Vector2 *tracepos;
		int     *tracecol;

		MissileTrace(SpaceObject *creator, double add_tail_time, int N);
		virtual ~MissileTrace();

		virtual void calculate();
		void animate(Frame *space);
};

class MassShot : public SpaceObject
{
	public:
		Vector2 rel_startvel;
		int     acceleration, planet_bounces;
		double  v_default, t_default, max_speed;

		double  turn_rate, accel_rate;
		double  exist_time, exist_time_max;

		double  gravwhip_vel, gravwhip_turn, slowdown_factor;

		MassShot(ClowanFlurry *creator, Vector2 rpos, double oangle, double ov, double odamage,
			double orange, double oarmour, double otrate, SpaceLocation *opos,
			SpaceSprite *osprite, SpaceObject *target);

		int accelerate(SpaceLocation *source, double angle, double velocity, double max_speed);
		void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void inflict_damage(SpaceObject *other);
		//	void animate(Frame *space);

		void evade_planet();
		void turn_to_target(double damax, SpaceLocation *t);
};

class GravShot : public Shot
{
	public:
		double  existtime;

		// it'll move to some point, and create an (invisible) gravity well there
		GravShot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
			double odamage, double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, double relativity);

		virtual void calculate();
};

class GravWell : public SpaceLocation
{
	public:
		double gravity_time;
		Vector2 oldpos;

		GravWell(SpaceLocation *creator, Vector2 lpos, double langle, double multiplier);
		virtual void calculate();

		double  gravity_mindist, gravity_range, gravity_force, gravity_power,
			gravity_exist_time, gravity_weaponacc, gravity_whip, gravity_whip2;
};

class FallingParticles : public SpaceLocation
{
	public:

		struct part_dat_str
		{
			double  R, angle;
		};

		part_dat_str    *part_dat;
		int     Nparticles;
		double  Rmax;
		SpaceLocation   *mother;

		FallingParticles(SpaceLocation *creator, Vector2 lpos, double langle);
		virtual ~FallingParticles();
		virtual void calculate();
		virtual void animate(Frame *space);
		virtual int canCollide(SpaceLocation *other);

};

MissileTrace::MissileTrace(SpaceObject *creator, double oadd_tail_time, int N)
{
	STACKTRACE;
	mother = creator;

	Npos = N;
	add_tail_time = oadd_tail_time;

	tail_time = 0;

	tracepos = new Vector2 [Npos];
	tracecol = new int [Npos];

	int i;
	for ( i = 0; i < Npos; ++i ) {
		tracepos[i] = mother->pos;
		tracecol[i] = 0;
	}
}


MissileTrace::~MissileTrace()
{
	delete [] tracepos;
	delete [] tracecol;
}


void MissileTrace::calculate()
{
	STACKTRACE;
	// well ... nothing physical is done, really. It's just visuals :)

	if ( !(mother && mother->exists()) )
		mother = 0;				 // when you notice the dependent things has disappeared, reset pointer to it as well

	if ( !(mother && mother->exists()) && tracepos[Npos-1] == tracepos[0] ) {
		state = 0;
		return;
	}
}


void MissileTrace::animate(Frame *space)
{
	STACKTRACE;
	// add visual goodies ...

	int i;
	tail_time += frame_time * 1E-3;

	if ( tail_time > add_tail_time ) {
		tail_time -= add_tail_time;

		for ( i = 0; i < Npos-1; ++i ) {
			tracepos[i] = tracepos[i+1];
			tracecol[i] = tracecol[i+1];
		}

		if ( mother )
			tracepos[Npos-1] = mother->normal_pos();
		else
			tracepos[Npos-1] = tracepos[Npos-2];

		double v;
		if ( mother )
			v = mother->vel.magnitude();
		else
			v = -1.0;

		int r, g, b;

		if ( v >= 0 ) {
			r = int(255.0 * v);
			g = 50;
			b = int(255.0 - r);
		} else {
			r = 0;
			g = 0;
			b = 0;

		}

		if ( r > 255 )  r = 255;
		if ( b < 0 )    b = 0;

		tracecol[Npos-1] = tw_makecol(r, g, b);
	}

	for ( i = 1; i < Npos; ++i ) {
		Vector2 co1, co2;

		co1 = tracepos[i-1];
		co2 = tracepos[i];

		// convert to screen coordinates:

		co1 = corner(co1);		 // this normalizes so that points are always on the map
		co2 = corner(co2);

		// normalize the distance ;) so that points _may_ be off-map

		co2 = co1 + min_delta(co2, co1, map_size * space_zoom);

		int ix1, iy1, ix2, iy2;
		ix1 = int(co1.x);
		iy1 = int(co1.y);
		ix2 = int(co2.x);
		iy2 = int(co2.y);

		// also, make it darker towards the end:
		double a;
								 // note that point 0 is first created, and is last
		a = 0.5 + 0.5 * double(i)/Npos;

		int r, g, b, col;
		col = tracecol[i];

		r = tw_getr(col);
		g = tw_getg(col);
		b = tw_getb(col);

		r = int(a * r);
		g = int(a * g);
		b = int(a * b);

		col = tw_makecol(r,g,b);

		// draw a line ?!
		tw_line(space->surface, ix1, iy1, ix2, iy2, col);
		space->add_line(ix1, iy1, ix2, iy2);
	}
}


MassShot::MassShot(ClowanFlurry *creator, Vector2 rpos, double oangle, double ov, double odamage,
double orange, double oarmour, double otrate, SpaceLocation *opos,
SpaceSprite *osprite, SpaceObject *otarget)
:
SpaceObject( creator, opos->pos+rpos, oangle, osprite)
{
	STACKTRACE;
	layer = LAYER_SHOTS;

	isblockingweapons = false;

	target = otarget;

	mass = 1.0;

	acceleration = 0;

	v_default = creator->weaponVelocity;
	max_speed = v_default;		 // is changed by grav wells.

	pos = creator->pos + rpos;
	vel = creator->vel + v_default * unit_vector(angle);

	exist_time = 0;
	exist_time_max = creator->weaponTime;

	turn_rate = creator->weaponTurnrate;
	t_default = turn_rate;

	accel_rate = creator->weaponAccelRate;

	gravwhip_vel  = creator->weaponWhipVel;
	gravwhip_turn = creator->weaponWhipTurn;

	damage_factor = creator->weaponDamage;

	slowdown_factor = creator->weaponSlowDown;

	planet_bounces = creator->weaponPlanetBounce;

	game->add(new MissileTrace(this, creator->weaponTailTime, creator->weaponTailNum) );

}


/*
int MassShot::accelerate(SpaceLocation *source, double angle, double velocity, double max_speed)
{
	STACKTRACE;
	HomingMissile::accelerate(source, angle, velocity, max_speed);

	// only special sources can accelerate a projectile
//	if (!source->isPlanet())
//	{
		//vel *= 1 + 0.3 * frame_time*1E-3;	// uhm, a bit excessive ;)
		double a;

		//vel += gravity_weaponacc * frame_time*1E-3 * (vel / vel.magnitude());
//		v += gravity_weaponacc * frame_time*1E-3;
//		turn_rate += gravity_weaponacc*0.01 * frame_time*1E-3;

		double dt = frame_time * 1E-3;
		v += 50 * velocity * dt;
		turn_rate += 0.5 * velocity * dt;

		acceleration = 1;
//	}

	return false;
}

void MassShot::calculate()
{
	STACKTRACE;

	if (!(target && target->exists()))
	{
		state = 0;
		return;
	}

	HomingMissile::calculate();

	vel += rel_startvel;
	rel_startvel *= 1 - frame_time*1E-3;	// slowly forgets about the starting velocity

	// also, slowly slow down a shot
	if (!acceleration)
	{
		double dt = frame_time * 1E-3;
		double a = (1 - dt);

		// slowly restore original properties, when the shots leave the grav field.
		v = v_default + (v - v_default) * a;
		turn_rate = t_default + (turn_rate - t_default) * a;

	} else {
		acceleration = 0;	// reset acc. switch for next round

		// inside a grav well, shots exist forever ?!
		d -= v * frame_time;	// subtract the distance travelled.
	}

	// change the angle of flight to evade the planet
	evade_planet();
}

int MassShot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int i;

	// ok .. if the shot hits a planet, it wont be damaged, but greatly accelerated
	// this should hardly ever happen, cause shots first try to evade the planet !
	if ( source->isPlanet() )
	{
		state = 1;
		double a, b, da;
		a = atan(min_delta(pos - source->pos, map_size));
		b = atan(-vel);
		da = a - b;
		angle = b + 2*da;
		v *= 1.5;
		i = 0;
	} else
		i = HomingMissile::handle_damage(source, normal, direct);

	return i;
}

void MassShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if ( !other->isPlanet() )
		HomingMissile::inflict_damage(other);
}

// copy some stuff from Orz-marines:
// should be added to SpaceLocation I think !
void MassShot::evade_planet()
{
	STACKTRACE;

	Planet *spacePlanet = nearest_planet();
	if (spacePlanet!=NULL)
	{
		double r = distance(spacePlanet);
		if (r < 1.1*spacePlanet->get_sprite()->size().x)
		{
			double t_a = trajectory_angle(spacePlanet);
			double d_a = normalize(t_a - angle, PI2);
			if (d_a > PI)
				d_a -= PI2;
			//                                double p_a = normalize(atan3(1.9*spacePlanet->getSprite()->width()/2.0, r), PI2);
			//                                p_a = p_a - fabs(d_a);
			//                                if (p_a > 0) {
			if (fabs(d_a)<PI/2)
			{
				if (d_a > 0)
					angle = normalize(t_a - PI/2, PI2);
				else
					angle = normalize(t_a + PI/2, PI2);
			}
		}
	}

}

void MassShot::animate(Frame *space)
{
	STACKTRACE;
	HomingMissile::animate(space);

	// and, add visual goodies ...

	int i;
	tail_time += frame_time * 1E-3;

	if ( tail_time > add_tail_time )
	{
		tail_time -= add_tail_time;

		for ( i = 0; i < Npos-1; ++i )
			tracepos[i] = tracepos[i+1];

		tracepos[Npos-1] = normal_pos();
	}

	for ( i = 1; i < Npos; ++i )
	{
		Vector2 co1, co2;

		co1 = tracepos[i-1];
		co2 = tracepos[i];

		// convert to screen coordinates:

		co1 = corner(co1);	// this normalizes so that points are always on the map
		co2 = corner(co2);

		// normalize the distance ;) so that points _may_ be off-map

		co2 = co1 + min_delta(co2, co1, map_size * space_zoom);

		int r, g, b, col;

		r = int(255.0 * v);
		g = 50;
		b = int(255.0 - r);

		if ( r > 255 )	r = 255;
		if ( b < 0 )	b = 0;

		// also, make it darker towards the end:
		double a;
		a = 0.5 + 0.5 * double(i)/Npos;	// note that point 0 is first created, and is last
		r = int(a * r);
		g = int(a * g);
		b = int(a * b);

		col = tw_makecol(r, g, b);

		int ix1, iy1, ix2, iy2;
		ix1 = int(co1.x);
		iy1 = int(co1.y);
		ix2 = int(co2.x);
		iy2 = int(co2.y);

		// draw a line ?!
		line(space->surface, ix1, iy1, ix2, iy2, col);
		space->add_line(ix1, iy1, ix2, iy2);
	}
}
*/

GravShot::GravShot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
double odamage, double orange, double oarmour, SpaceLocation *opos,
SpaceSprite *osprite, double relativity)
:
Shot(creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite, relativity)
{
	STACKTRACE;
	// do nothing
	existtime = 0;
	mass = 0;
}


void GravShot::calculate()
{
	STACKTRACE;

	Shot::calculate();
	// this can also make state = 0, if range is exceeded.

	if ( !(ship && ship->exists()) ) {
		ship = 0;
		state = 0;
		return;
	}

								 // check if key is released
	if ( !(ship->nextkeys & keyflag::special) )
		state = 0;

	if ( state == 0 )
		add(new GravWell(this, this->pos, 0.0, 1/*+existtime*/));   // best to keep the force constant

		existtime += frame_time * 1E-3;
}


FallingParticles::FallingParticles(SpaceLocation *creator, Vector2 lpos, double langle)
:
SpaceLocation(creator, lpos, langle)
{
	STACKTRACE;
	layer = LAYER_HOTSPOTS;
	set_depth(DEPTH_HOTSPOTS);

	Nparticles = 50;
	Rmax = 250.0;

	mother = creator;

	part_dat = new part_dat_str [Nparticles];

	int i;
	for ( i = 0; i < Nparticles; ++i ) {
		part_dat[i].R = tw_random(Rmax);
		part_dat[i].angle = tw_random(PI2);
	}

}


FallingParticles::~FallingParticles()
{
	delete [] part_dat;
}


void FallingParticles::calculate()
{
	STACKTRACE;
	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;
		return;
	}

	SpaceLocation::calculate();

	vel = 0;
	pos = mother->pos;

	int i;

	double dt;
	dt = frame_time * 1E-3;

	for ( i = 0; i < Nparticles; ++i ) {
		part_dat[i].angle += 10*PI2 * dt / part_dat[i].R;
		part_dat[i].R -= 10.0 * dt;

		if ( part_dat[i].R < 5.0 ) {
			part_dat[i].R = tw_random(Rmax);
			part_dat[i].angle = tw_random(PI2);
		}
	}
}


void FallingParticles::animate(Frame *space)
{
	STACKTRACE;
	// inefficient - this draws pixels even if they're off-screen
	// if (!drawability_box(pos, Rmax)) return;	// if no part of the draw area is on-screen...

	int i;
	for ( i = 0; i < Nparticles; ++i ) {
		int col;
		Vector2 posi, co;

		col = tw_makecol(int(255 * (Rmax - part_dat[i].R) / Rmax), 0, 0);

		posi = pos + part_dat[i].R * unit_vector(part_dat[i].angle );
		co = corner(posi);

		int ix, iy;
		ix = int(co.x);
		iy = int(co.y);

		tw_putpixel(space->surface, ix, iy, col);
		space->add_pixel(ix, iy);
	}
}


int FallingParticles::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	return FALSE;
}


GravWell::GravWell(SpaceLocation *creator, Vector2 lpos, double langle, double multiplier)
:
SpaceLocation(creator, lpos, langle)
{
	STACKTRACE;
	collide_flag_anyone = 0;	 // not possible to collide with anything
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;

	gravity_time = 0;

	oldpos = pos;

	gravity_mindist = ::gravity_mindist;
	gravity_range = ::gravity_range;
	gravity_force = ::gravity_force * multiplier;
	gravity_power = ::gravity_power;
	gravity_exist_time = ::gravity_exist_time;
	gravity_whip  = ::gravity_whip;
	gravity_whip2 = ::gravity_whip2;

	// also, add a visual thingy - particles falling in
	game->add( new FallingParticles(this, this->pos, 0.0) );
}


void GravWell::calculate()
{
	STACKTRACE;
	SpaceLocation::calculate();

	vel = 0;
	pos = oldpos;

	gravity_time += frame_time * 1E-3;
	if ( gravity_time > gravity_exist_time ) {
		state = 0;
		return;
	}

	// apply gravity whip/force to everything in range
	// same code (exactly) as in class Planet::calculate

	SpaceObject *o;
	Query a;
	a.begin(this, OBJECT_LAYERS, gravity_range);
	for (;a.currento;a.next()) {
		if (!a.current->isObject())
			continue;

		o = a.currento;

		// do not affect other planets, that's weird ;)
		if (o->isPlanet())
			continue;

		//		if (o->mass > 0) {
		double r = distance(o);
		if (r < gravity_mindist) r = gravity_mindist;
		double sr = 1;
		double vadd;
		//gravity_power rounded up here
		if (gravity_power < 0) {
			r /= 40 * 5;
			for (int i = 0; i < -gravity_power; i += 1) sr *= r;
			vadd = frame_time * gravity_force / sr;
		} else {
			r = 1 - r/gravity_range;
			for (int i = 0; i < gravity_power; i += 1) sr *= r;
			vadd = frame_time * gravity_force * sr;
		}

		// normal acceleration for all objects and ships in direction of
		// the gravity well.

		double tmp = distance(o) / this->gravity_range;
		o->accelerate(this, trajectory_angle(o)+PI, vadd, MAX_SPEED * (this->gravity_whip * tmp + 1) + tmp * this->gravity_whip2);

		// extra (big) acceleration for the creator of the grav. well
		// this is in addition to the normal thrust
		if ( o == ship && o->ship->thrust )
			o->accelerate(this, o->angle, o->ship->accel_rate*frame_time*gravity_shipaccscale, MAX_SPEED * (this->gravity_whip * tmp + 1) + tmp * this->gravity_whip2);
		// not very neat, I'm using a global variable here ;)
		//			}
		//		}
	}

}


ClowanFlurry::ClowanFlurry(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

								 // in seconds
	weaponTime     = tw_get_config_float("Weapon", "Time", 0);
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponTurnrate = scale_turning(tw_get_config_float("Weapon", "TurnRate", 0));
	weaponTailNum  = tw_get_config_int("Weapon", "TailNum", 0);
	weaponTailTime = tw_get_config_float("Weapon", "TailTime", 0);
	weaponAccelRate   = scale_acceleration(tw_get_config_float("Weapon", "AccelRate", 0), 0);
								 // vel incr. per second
	weaponWhipVel   = scale_velocity(tw_get_config_float("Weapon", "WhipVel", 0));
								 // turnr. incr. per second
	weaponWhipTurn  = scale_turning(tw_get_config_float("Weapon", "WhipTurn", 0));
	weaponSlowDown  = tw_get_config_float("Weapon", "SlowDownFactor", 0);
	weaponPlanetBounce = tw_get_config_int("Weapon", "PlanetBounce", 0);

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialArmour   = tw_get_config_int("Special", "Armour", 0);

	gravity_mindist = scale_range(tw_get_config_float("GravWell", "GravityMinDist", 0));
	gravity_range = scale_range(tw_get_config_float("GravWell", "GravityRange", 0));
	gravity_power = tw_get_config_float("GravWell", "GravityPower", 0);
	// vel. increase per second for the special weapon:
	gravity_force = scale_acceleration(tw_get_config_float("GravWell", "GravityForce", 0), 0);
	gravity_exist_time = tw_get_config_float("GravWell", "ExistTime", 0);
	gravity_whip  = tw_get_config_float("Planet", "GravityWhip", 0);
	gravity_whip2 = tw_get_config_float("Planet", "GravityWhip2", 0);

	gravity_shipaccscale = tw_get_config_float("GravWell", "GravityShipAccScale", 0);

	Gshot = 0;
}


int ClowanFlurry::activate_weapon()
{
	STACKTRACE;

	if (! (nextkeys & keyflag::special) ) {

		if ( !(target && target->exists()) )
			return FALSE;

		const int Nangles = 5;
		double extraangles[Nangles] = { -PI*0.6, -PI*0.3, 0.0, PI*0.3, PI*0.6 };
		double R = 22.0;
		double Offs = 25.0;

		Vector2 startrmid, startrpos;

		startrmid = Offs * Vector2(0.0, 1.0);

		int i;
		for ( i = 0; i < Nangles; ++i ) {
			double a;
								 // 0.0 is relative
			a = 0.0 + extraangles[i];
								 // +PI/2 cause 0 angle points up (duh)
			startrpos = startrmid + R * unit_vector(angle + a);

			MassShot *newshot;
			if ( target && target->exists() ) {
				newshot = new MassShot(this, startrpos, angle+a, weaponVelocity,
					weaponDamage, weaponTime, weaponArmour, weaponTurnrate,
					this, this->data->spriteWeapon, target);
				game->add( newshot );
			}

		}

	} else {

		// this is a special, defensive mode :)

		const int Nangles = 5;
		double extraangles[Nangles] = { -PI*0.6, -PI*0.3, 0.0, PI*0.3, PI*0.6 };
		double R = 22.0;
		double Offs = 25.0;

		Vector2 startrmid, startrpos;

		startrmid = Offs * Vector2(0.0, 1.0);

		int i;
		for ( i = 0; i < Nangles; ++i ) {
			double a;
								 // 0.0 is relative
			a = 0.0 + extraangles[i];
								 // +PI/2 cause 0 angle points up (duh)
			startrpos = startrmid + R * unit_vector(a+PI/2);

			Shot *newshot;
			if ( target && target->exists() ) {
				newshot = new Shot(this, startrpos, angle-a, weaponVelocity,
					weaponDamage, 200*weaponTime, weaponArmour,
								 // no relativity, otherwise it's too useful
					this, this->data->spriteWeapon, 0.0);
				newshot->mass = 1.0;
				game->add( newshot );
			}

		}

		// this mode takes less energy and recharge time:

		batt += (weapon_drain - 1.0);
		// weapon_recharge += weapon_rate;
		weapon_recharge -= iround(0.75 * weapon_rate);
	}

	return TRUE;
}


int ClowanFlurry::activate_special()
{
	STACKTRACE;
	if (nextkeys & keyflag::fire )
		return FALSE;

								 // check if you've already fired
	if ( !(Gshot && Gshot->exists()) ) {
		Gshot = new GravShot(this, Vector2(0.0, 50.0), angle, specialVelocity,
			specialDamage, specialRange, specialArmour, this, this->data->spriteSpecial, 1.0);

		game->add( Gshot );
		return TRUE;

	}

	return FALSE;
}


void ClowanFlurry::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if ( !(Gshot && Gshot->exists()) )
		Gshot = 0;				 // detect that it disappears, otherwise you point to some empty memory space.

	if ( !(target && target->exists()) )
		target = 0;				 // detect that it disappears, otherwise you point to some empty memory space.
}


int MassShot::accelerate(SpaceLocation *source, double angle, double velocity, double m_speed)
{
	STACKTRACE;
	double dt = frame_time * 1E-3;
	max_speed += gravwhip_vel * dt;
	m_speed = max_speed;

	turn_rate += gravwhip_turn * dt;

	SpaceLocation::accelerate(source, angle, velocity, m_speed);

	acceleration = 1;

	return false;
}


void MassShot::turn_to_target(double damax, SpaceLocation *t)
{
	STACKTRACE;
	double da;

	if (  (!(t && t->exists())
		|| t->isInvisible() ))
		return;

	da = intercept_angle2(pos, vel, vel.magnitude(), t->normal_pos(), t->get_vel()) - angle;
	//	da = trajectory_angle(t) - angle;

	while (da > PI)     da -= PI2;
	while (da < -PI)    da += PI2;

	if ( da > damax )   da = damax;
	if ( da < -damax )  da = -damax;

	angle += da;
}


void MassShot::calculate()
{
	STACKTRACE;

	if (!(target && target->exists()) || exist_time > exist_time_max) {
		target = 0;
		state = 0;
		return;
	}

	// the following changes the angle in direction of the enemy:
	turn_to_target(turn_rate * frame_time, target);

	// change the angle of flight to evade the planet (overrides angle setting)
	evade_planet();

	// ok ... desired angles are known now ...

	// can also use the planet to accelerate :)
	Planet *p = nearest_planet();

	if (p && distance(p) < p->gravity_range ) {
		double r, vadd;
		r = distance(p);
		r = 1 - r / p->gravity_range;
		vadd = frame_time * p->gravity_force * r;

		accelerate(this, trajectory_angle(p)+PI, vadd, max_speed);

		acceleration = 1;
	}
	// changes vel around the planet.
	// hmm ..
	// but in the wrong way ! the wrong value is used ...

	// and now for the acceleration in direction of the target:
	SpaceObject::accelerate(this, angle, accel_rate * frame_time, max_speed);

	// also, slowly slow down a shot
	if (!acceleration) {
		double dt = frame_time * 1E-3;
		if ( dt > 1)    dt = 1;
		double a = (1 - slowdown_factor * dt);

		// slowly restore original properties, when the shots leave the grav field.
		max_speed = v_default + (max_speed - v_default) * a;

		turn_rate = t_default + (turn_rate - t_default) * a;

		// also _force_ velocity to go down, since the normal routine
		// maintains momentum... :

		double v;
		v = vel.magnitude();
		if ( v > max_speed )
			vel *= max_speed / v;
		// (ok :) that works)

		exist_time += dt;

	} else {
		acceleration = 0;		 // reset acc. switch for next round
		//no exist_time updated ... inside a grav well, the shots exist forever.
	}

	SpaceObject::calculate();

}


int MassShot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int i = 0;

	// ok .. if the shot hits a planet, it wont be damaged, but greatly accelerated
	// this should hardly ever happen, cause shots first try to evade the planet !
	if ( source->isPlanet() && planet_bounces ) {
		state = 1;
		double a, b, da;
		a = atan(min_delta(pos - source->pos, map_size));
		b = atan(-vel);
		da = a - b;
		angle = b + 2*da;
		vel *= 1.5;
		i = 0;
	} else
	state = 0;
	//		i = SpaceObject::handle_damage(source, normal, direct); // this doesn't do anything

	return i;
}


void MassShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if ( !other->isPlanet() ) {
		SpaceObject::inflict_damage(other);
		state = 0;
	}
}


// copy some stuff from Orz-marines:
// should be added to SpaceLocation I think !
void MassShot::evade_planet()
{
	STACKTRACE;

	Planet *spacePlanet = nearest_planet();
	if (spacePlanet!=NULL) {
		double r = distance(spacePlanet);
		if (r < 1.1*spacePlanet->get_sprite()->size().x) {
			double t_a = trajectory_angle(spacePlanet);
			double d_a = normalize(t_a - angle, PI2);
			if (d_a > PI)
				d_a -= PI2;
			//                                double p_a = normalize(atan3(1.9*spacePlanet->getSprite()->width()/2.0, r), PI2);
			//                                p_a = p_a - fabs(d_a);
			//                                if (p_a > 0) {
			if (fabs(d_a)<PI/2) {
				if (d_a > 0)
					angle = normalize(t_a - PI/2, PI2);
				else
					angle = normalize(t_a + PI/2, PI2);
			}
		}
	}

}


REGISTER_SHIP(ClowanFlurry)
