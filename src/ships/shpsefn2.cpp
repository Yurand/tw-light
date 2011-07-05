/* $Id: shpsefn2.cpp,v 1.6 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
#include "../frame.h"
#include "../melee/mview.h"
REGISTER_FILE

/*

  A ship with a harpoon, and a short-range laser.

*/

class ShipPart2;
class ShipPart2Manager;

class Hook2;

class SefyNautilus2 : public Ship
{
	public:

		bool    arm_movement, specialHitRoids;
		double  arm_angle, arm_time, arm_time_last, arm_period, arm_period2, arm_maxangle;
		double  arm_maxdamage, arm_mindamage;

		double  sound_recharge;
		ShipPart2       *ArmLeft, *ArmRight;
		ShipPart2Manager    *ManageParts;

	public:

		Hook2   *hook;
		double  unrolltime;

		double  specialRange, specialDelay, specialOscFreq, specialRelVelocity,
			specialSegLength, specialArmour, specialSprConst, specialLifeTime;

		SefyNautilus2(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void animate(Frame *space);
		virtual void calculate();
		virtual int activate_weapon();
		virtual int activate_special();
		virtual int handle_damage(SpaceLocation *src, double normal, double direct=0);
};

// *********************************************************************************

static const int maxnodes = 50;

class Hook2 : public SpaceObject
{
	public:
		double  armour;
		bool bHitAsteroids;

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

		Hook2(SefyNautilus2 *creator, Vector2 orelpos, SpaceSprite *osprite, bool bHit);

		virtual void calculate();
		virtual int handle_damage(SpaceLocation *src, double normal, double direct=0);
		virtual void animate ( Frame *space );
		void        animate_ropeseg( Frame *space, Vector2 pos1, Vector2 pos2, int ropecol );

		void unroll();
};

// *********************************************************************************

class ShipPart2 : public SpaceObject
{
	void calc_angle();
								 // calculates it for this angle
	void calc_pos(Vector2 refpos);

	public:

		Ship    *mother;
		int     hascollided;
		int     ship_rotations;
		double  offset_angle;
		Vector2 offset_pos, pivot_point;
		Vector2 change_pos, change_vel;

		ShipPart2(Ship *creator, SpaceSprite *osprite,
			double oangle, Vector2 orelpos, Vector2 opivot, double omass);

		virtual void inflict_damage(SpaceObject *other);
		virtual void calculate();// should be empty
		virtual void animate(Frame *space);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);

								 // add minor detection part
		virtual void collide(SpaceObject *other);
		int collide_SpaceObject(SpaceObject *other);

		void calculate_manager(Vector2 refpos, Vector2 refvel);
		// should do the real calculations to allow syncing with the mothership
};

// *********************************************************************************

static const int MaxParts = 100;
class ShipPart2Manager : public Presence
{
	ShipPart2   *partlist[MaxParts];
	int         Nparts;
	Ship        *mother;
	public:
		Vector2     oldpos, oldvel;

		ShipPart2Manager(Ship *creator);

		void add_part(ShipPart2 *newpart);

								 // empty
		virtual void calculate() {
		};

		void calculate_manager();// this:
		// should be called immediately following Ship::calculate function of the mother,
		// since it could add additional changes to pos, vel based on "experiences" of
		// the ship parts (read: collisions !).

};

// *********************************************************************************

ShipPart2Manager::ShipPart2Manager(Ship *creator)
{
	STACKTRACE;
	int i;
	for ( i = 0; i < MaxParts; ++i )
		partlist[i] = 0;
	Nparts = 0;

	mother = creator;
	oldpos = mother->pos;
	oldvel = mother->vel;
}


// *********************************************************************************

void ShipPart2Manager::add_part(ShipPart2 *newpart)
{
	STACKTRACE;
	partlist[Nparts] = newpart;
	++Nparts;
}


// *********************************************************************************

void ShipPart2Manager::calculate_manager()
{
	STACKTRACE;
	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;

		int i;
		for ( i = 0; i < Nparts; ++i )
			partlist[i]->calculate_manager(oldpos, oldvel);

		return;
	}

	int i;
	for ( i = 0; i < Nparts; ++i ) {

		partlist[i]->calculate_manager(oldpos, oldvel);

								 // important - this should override pos/vel settings
		if ( partlist[i]->hascollided ) {
			mother->pos = oldpos + partlist[i]->change_pos;
			mother->vel = oldvel + partlist[i]->change_vel;
		}

		// reset collision information BUT note, that this is handled separately by the physics...
		// (I suppose that's for synching reasons, so that all objects are stationary while
		// they're being checked). That's why I reset it here, just after it's been used.
		// I guess it's not very neat, but well, it's done by the manager responsible for it :)

		partlist[i]->hascollided = 0;
		partlist[i]->change_pos = 0;
		partlist[i]->change_vel = 0;
	}

	//	oldpos = mother->pos;
	//	oldvel = mother->vel;

}


// *********************************************************************************

void ShipPart2::calc_angle()
{
	STACKTRACE;
	angle = mother->angle + offset_angle;
	while (angle < 0  ) angle += PI2;
	while (angle > PI2) angle -= PI2;
	// the angle should be between 0 and 2PI
}


// *********************************************************************************

void ShipPart2::calc_pos(Vector2 refpos)
{
	STACKTRACE;
	pos = refpos + rotate(offset_pos, mother->angle) - rotate(pivot_point, angle);
}


// *********************************************************************************

ShipPart2::ShipPart2(Ship *creator, SpaceSprite *osprite,
double oangle, Vector2 orelpos, Vector2 opivot, double omass)
:
SpaceObject(creator, creator->pos, oangle, osprite),
mother(creator)
{
	STACKTRACE;
	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHOTS);

	// angle relative to the mother ship (sprite angle = mother angle + rel. angle)
	offset_angle = oangle;

	// the point relative to center of this sprite, rotation is around this point
	// The pi/2 rotation is needed, since the vectors are declared along y as if angle=0 there,
	// instead angle=0 along x.
	pivot_point = rotate(opivot, PI/2);

	// position of the pivot point relative to mothership central position
	offset_pos = rotate(orelpos, PI/2);

	// just here so that you can override this, if needed.
	ship_rotations = 64;

	// mass, should be non-zero to allow for collisions; the bigger the mass, the less
	// it'll "move" out of position due to collisions
	//	mass = omass;
	mass = ship->mass;			 // might be better ?

	// ok, override earlier settings now
	calc_angle();
	calc_pos(mother->pos);

	collide_flag_anyone = mother->collide_flag_anyone;
	//collide_flag_anyone = ALL_LAYERS;
	//collide_flag_anyone = 1<<LAYER_SHIPS - 1<<LAYER_SHOTS;
	//collide_flag_sameteam = mother->collide_flag_sameteam;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;

	hascollided = 0;
	change_pos = 0;
	change_vel = 0;
}


// *********************************************************************************

void ShipPart2::calculate()
{
	STACKTRACE;
	// this should be checked here, too, cause the manager can die as well; if the
	// parts aren't dead by then, the check will never be made otherwise !!
	if (!(mother && mother->exists())) {
		mother = 0;
		state = 0;
		return;
	}
}


// *********************************************************************************

void ShipPart2::calculate_manager(Vector2 refpos, Vector2 refvel)
{
	STACKTRACE;

	if (!(mother && mother->exists())) {
		mother = 0;
		state = 0;
		return;
	}

	calc_angle();				 // update (angle) using mother settings
	calc_pos(refpos);			 // update (pos) using manager settings
	vel = refvel;

	sprite_index = get_index(angle);

	SpaceObject::calculate();

}


// *********************************************************************************

int ShipPart2::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	if (mother && mother->exists()) {
		state = 1;
		if (damage_factor > 0)
			return 0;
		return mother->handle_damage(source, normal, direct);
	} else {
		mother = 0;
		state = 0;
		return 0;
	}
}


// *********************************************************************************

void ShipPart2::animate(Frame *space)
{
	STACKTRACE;

	SpaceObject::animate(space);
}


// *********************************************************************************

// an exact copy, with a small modification: it also return a boolean value
int ShipPart2::collide_SpaceObject(SpaceObject *other)
{
	STACKTRACE;
	//	double dx, dy;
	//	double dvx, dvy;
	double tmp;

	//	int x1, y1;
	//	int x2, y2;

	if (this == other) {
		tw_error("SpaceObject::collide - self!");
	}
	if ((!canCollide(other)) || (!other->canCollide(this)))
		return 0;
	if (!exists() || !other->exists())
		return 0;

	pos = normal_pos();

	Vector2 p1, p2, dp, dv;

	p1 = pos;
	p2 = other->normal_pos();
	dp.x = min_delta(p1.x, p2.x, map_size.x);
	dp.y = min_delta(p1.y, p2.y, map_size.y);
	p2 = p1 - dp - other->size / 2;
	p1 = p1 - size / 2;

	/*	x1 = (int)(normal_x() - (w / 2.0));
		y1 = (int)(normal_y() - (h / 2.0));
		dx = min_delta(normal_x(), other->normal_x(), map_size.x);
		dy = min_delta(normal_y(), other->normal_y(), Y_MAX);
		x2 = (int)(normal_x() - dx - ((other->w) / 2.0));
		y2 = (int)(normal_y() - dy - ((other->h) / 2.0));*/

	if (!sprite->collide(iround(p1.x), iround(p1.y), sprite_index, iround(p2.x), iround(p2.y),
		other->get_sprite_index(), other->get_sprite() ))
		return 0;
	//sprite->collide(x1, y1, sprite_index, x2, y2, other->sprite_index, other->sprite);

	inflict_damage(other);
	other->inflict_damage(this);

	if (!mass || !other->mass)
		return 0;

	dv = vel - other->vel;

	p1 = pos;
	p2 = other->normal_pos();
	dp.x = min_delta(p1.x, p2.x, map_size.x);
	dp.y = min_delta(p1.y, p2.y, map_size.y);
	p2 = p1 - dp - other->size / 2;
	p1 = p1 - size / 2;

	/*x1 = (int)(normal_x() - (w / 2.0));
	y1 = (int)(normal_y() - (h / 2.0));
	dx = min_delta(normal_x(), other->normal_x(), map_size.x);
	dy = min_delta(normal_y(), other->normal_y(), Y_MAX);
	x2 = (int)(normal_x() - dx - ((other->w) / 2.0));
	y2 = (int)(normal_y() - dy - ((other->h) / 2.0));*/

	while ((dp.x == 0) && (dp.y == 0)) {
		dp.x = (tw_random(5) - 2) / 99.0;
		dp.y = (tw_random(5) - 2) / 99.0;
	}

	Vector2 _dp = unit_vector(dp);
	tmp = dot_product(dv, _dp);
	tmp = ( -2 * tmp );
	tmp = tmp * (mass * other->mass) / (mass + other->mass);
	if (tmp >= 0) {
		vel += _dp * tmp / mass;
		other->vel -= _dp * tmp / other->mass;
	}

	Vector2 nd;
	nd = unit_vector(dp);
	nd /= (mass + other->mass);
	while (sprite->collide(iround(p1.x), iround(p1.y), sprite_index, iround(p2.x), iround(p2.y),
	other->get_sprite_index(), other->get_sprite() )) {
		pos = normalize(pos + nd * other->mass);
		other->pos = normalize(other->pos - nd * mass);

		p1 = pos;
		p2 = other->normal_pos();
		dp.x = min_delta(p1.x, p2.x, map_size.x);
		dp.y = min_delta(p1.y, p2.y, map_size.y);
		p2 = p1 - dp - other->size / 2;
		p1 = p1 - size / 2;
	}

	return 1;
}


// *********************************************************************************

void ShipPart2::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	SpaceObject::inflict_damage(other);

	//damage_factor = 0;
}


// *********************************************************************************

void ShipPart2::collide(SpaceObject *other)
{
	STACKTRACE;
	Vector2 oldpos, oldvel;

	oldpos = pos;
	oldvel = vel;

	// collision changes velocity and position
	int i;

	i = collide_SpaceObject(other);

	if (i) {
		if ((pos - oldpos).magnitude() > change_pos.magnitude()) {
			hascollided = 1;
			change_pos = pos - oldpos;
			change_vel = vel - oldvel;
		}
	}

}


// *********************************************************************************

SefyNautilus2::SefyNautilus2(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	// time for retracting 1 piece of rope, in seconds
	specialLifeTime = get_config_float("Special", "LifeTime", 60.0);
	// time for retracting 1 piece of rope, in seconds
	specialDelay    = get_config_float("Special", "Delay", 0.5);
	// osc freq. of the rope when it's released
	specialOscFreq = get_config_float("Special", "OscFreq", 5.0);
	// extra velocity of the rope when release
	specialRelVelocity = get_config_float("Special", "RelVelocity", 0);
	// default segment length of a piece of rope (in pixels)
	specialSegLength = get_config_float("Special", "SegLength", 2.0);
	// armour of the hook
	specialArmour = get_config_float("Special", "Armour", 5.0);
	// spring constant: higher value, faster oscillations
	specialSprConst = get_config_float("Special", "SprConst", 250.0);
	specialHitRoids = bool(get_config_int("Special", "SnagAsteroids",0));

	hook = 0;
	unrolltime = 0;

	// below lifted from the Ducly Lanternjaws
	arm_angle = 0;

	double m = this->mass;
	// -dy is up, +dy is down !
	ArmLeft  = new ShipPart2(this, this->data->more_sprites[0], arm_angle, Vector2(-12,-21), Vector2(0,5), m);

	ArmRight = new ShipPart2(this, this->data->more_sprites[1], arm_angle, Vector2( 12,-21), Vector2(0,5), m);

	add(ArmLeft);
	add(ArmRight);

	arm_movement = false;
	arm_time = 0;
	//  arm_period = 10.0;
	//  arm_maxangle = 100 * ANGLE_RATIO;
	//  arc_corr = 30.0 * ANGLE_RATIO;
	arm_period = get_config_float("Arms", "OpenArmTime", 0);
	arm_period2 = get_config_float("Arms", "SnapArmTime", 0);
	arm_maxangle = get_config_float("Arms", "ArmMaxAngle", 0) * ANGLE_RATIO;
	arm_maxdamage = get_config_float("Weapon", "MaxDamage",0);
	arm_mindamage = get_config_float("Weapon", "MinDamage",0);

	sound_recharge = 0;

	ManageParts = new ShipPart2Manager(this);
	game->add(ManageParts);

	ManageParts->add_part(ArmLeft);
	ManageParts->add_part(ArmRight);
}


// *********************************************************************************

int SefyNautilus2::activate_weapon()
{
	STACKTRACE;

	if (!arm_movement) {
		arm_movement = true;
		play_sound2(data->sampleWeapon[0],256, iround(1000 * (data->sampleWeapon[0]->len / arm_period)));
		weapon_recharge += weapon_rate;
	}

	return FALSE;
}


// *********************************************************************************

int SefyNautilus2::activate_special()
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
			game->play_sound(data->sampleExtra[tw_random(3)],this,255,900+tw_random(100));

			angle = trajectory_angle(hook->hooktarget);
		}

		return FALSE;
	}

	hook = new Hook2(this, 100*unit_vector(angle), data->spriteSpecial, specialHitRoids);
	game->add( hook );

	return TRUE;
}


// *********************************************************************************

void SefyNautilus2::animate(Frame *space)
{
	STACKTRACE;
	/*
	if (ArmLeft)
		ArmLeft->SpaceObject::animate(space);
	if (ArmRight)
		ArmRight->SpaceObject::animate(space);
	*/
	Ship::animate(space);
}


// *********************************************************************************

void SefyNautilus2::calculate()
{
	STACKTRACE;

	ManageParts->oldpos = pos;
	ManageParts->oldvel = vel;

	Ship::calculate();

								 // this can apply extra changes to this ship pos,vel !
	ManageParts->calculate_manager();

	if ( arm_movement ) {
		if (fire_weapon && batt > 0 ) {
			if (arm_time <= arm_period) {
				arm_time += frame_time * 1E-3;
				arm_time_last = arm_angle;
				arm_angle = (0.5 - 0.5*cos(arm_time * PI / arm_period)) * arm_maxangle;

				// update the arms:

				if (arm_angle <= arm_maxangle) {
								 // anti-cl.w.
					ArmLeft->offset_angle  = -arm_angle;
								 // clockwise
					ArmRight->offset_angle =  arm_angle;
				}
			}
			if (weapon_recharge <= 0) {
				weapon_recharge += weapon_rate;
				batt -= weapon_drain;
			}
			if (weapon_recharge > 0)
				weapon_recharge -= frame_time;
		}

		// unleash the fury!!
		else if (arm_time > 0) {

			if (arm_time_last < arm_period2)
				arm_time_last = arm_time_last;

			double temp;

			temp = (arm_time_last / arm_maxangle) * arm_maxdamage;
			if (temp < arm_mindamage)
				temp = arm_mindamage;

			play_sound2(data->sampleWeapon[0],5);

			if (sound_recharge <= 0) {
				play_sound2(data->sampleWeapon[1]);

				sound_recharge += arm_period2 / arm_period;
				ArmLeft->damage_factor = temp;
				ArmRight->damage_factor = temp;
			}
			if (sound_recharge > 0)
				sound_recharge -= frame_time * 1E-3;

			//ArmLeft->collide_flag_anyone = ALL_LAYERS;
			//ArmRight->collide_flag_anyone = ALL_LAYERS;

			//if (arm_angle <= 0.05)
			{
				//	message.out("Bob!");

			}

			arm_time -= frame_time * 1E-3;

			arm_angle = (0.5 - 0.5*cos(arm_time * PI / arm_period2)) * arm_maxangle;

			ArmLeft->offset_angle  = -arm_angle;
			ArmRight->offset_angle = arm_angle;
		} else {
			arm_movement = false;
								 // anti-cl.w.
			ArmLeft->offset_angle  = 0;
								 // clockwise
			ArmRight->offset_angle = 0;

			ArmLeft->damage_factor = 0;
			ArmRight->damage_factor = 0;

			//ArmLeft->collide_flag_anyone = ArmLeft->mother->collide_flag_anyone;
			//ArmRight->collide_flag_anyone = ArmRight->mother->collide_flag_anyone;
		}

	}

	// release the hook:
	/*
	if ( (this->nextkeys & keyflag::special) && (this->nextkeys & keyflag::fire ) )
		if ( hook && hook->exists() )
			hook->state = 0;
	*/
	// detect when the hook disappears - reset the pointer to 0 in that case
	if ( !(hook && hook->exists()) )
		hook = 0;
}


// *********************************************************************************

int SefyNautilus2::handle_damage(SpaceLocation *src, double normal, double direct)
{
	STACKTRACE;
	Ship::handle_damage(src, normal, direct);

	return iround(normal + direct);
}


// *********************************************************************************

Hook2::Hook2(SefyNautilus2 *creator, Vector2 orelpos, SpaceSprite *osprite, bool bHit)
:
SpaceObject(creator, creator->pos+orelpos, creator->angle, osprite)
{
	STACKTRACE;
	//	double	specialRange, specialDelay
	//			;

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

	ropestart = 20.0;
								 // minimum pixels length of a rope segment, approx.
	ropeseglen = creator->specialSegLength;

	hooktarget = 0;
	hooklocked = 0;
	bHitAsteroids = bHit;

	springconst = creator->specialSprConst;

	hooksize = 9;				 // from center to the eye for the rope.

	layer = LAYER_SHOTS;
	set_depth(DEPTH_SHOTS);

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	isblockingweapons = false;
}


// *********************************************************************************

void Hook2::calculate()
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
	}
	else if (hooklocked) {
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

	}
	else
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


// *********************************************************************************

int Hook2::handle_damage(SpaceLocation *src, double normal, double direct)
{
	STACKTRACE;
	if ( src == hooktarget )
		return 0;

	if ( src->isShip() || (bHitAsteroids && src->isAsteroid())) {
		if ( hooktarget )		 // if it's already attached to another ship
			return 0;

		if ( src == ship )
			return 0;			 // if it is the ship itself (doh)

		hooktarget = src;

		game->play_sound(data->sampleSpecial[1],this);

		hookfixdist = magnitude(min_delta(pos, src->pos, map_size));
		hookfixorientation = atan(min_delta(pos, src->pos, map_size)) - src->angle;
		hookfixangle = angle - src->angle;

		collide_flag_anyone = 0;
		collide_flag_sameteam = 0;
		collide_flag_sameship = 0;

		hooklocked = 1;			 // stop unrolling any further

		if (src->isShip()) {
			src->ship->turn_rate *= 0.80;
			damage(src, 1.0, 0.0);
		}

		return 0;

	} else {

		SpaceObject::handle_damage(src, normal, direct);

		armour -= normal + direct;
		if ( armour <= 0 )
			state = 0;

		return iround(normal + direct);
	}
}


// *********************************************************************************

void Hook2::animate_ropeseg( Frame *space, Vector2 pos1, Vector2 pos2, int ropecol )
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
	a = atan2((float)dy,(float)dx);
	colscale = fabs(sin(a));	 // flat lying = less visible.

	int col, r, g, b;
	col = ropecol;
	r = iround(getr(col) * colscale);
	g = iround(getg(col) * colscale);
	b = iround(getb(col) * colscale);
	col = makecol(r, g, b);

	// draw the line

	line(space->surface, ix1, iy1, ix2, iy2, col);
	space->add_line(ix1, iy1, ix2, iy2);

}


// *********************************************************************************

void Hook2::animate( Frame *space )
{
	STACKTRACE;
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


// *********************************************************************************

void Hook2::unroll()
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


// *********************************************************************************

REGISTER_SHIP(SefyNautilus2)
