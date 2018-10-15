/* $Id: shphotsp.cpp,v 1.15 2005/08/28 20:34:08 geomannl Exp $ */
#include "ship.h"
REGISTER_FILE
#include "melee/mview.h"

/* NOTES:

Weapons which suck:
- Autofocus isn't nice as a special. It's what you can already do well enough by hand :)
- Extra strength of the spot isn't cool either, cause it's much too similar
  to the normal functioning of the weapon. Boring :)

- Ejecting the core should be ok - it adds a single unguided missile.

*/

class TheHotSpot;

class ShpHotSpot : public Ship
{
	public:
	public:
		double weaponRange, weaponVelocity, specialRange, specialVelocity, weaponVelocityStep;
		int    weaponDamage, weaponArmour, specialDamage, specialArmour;

		double weaponHeatingTime;

		//	double	xflame, yflame, xlens, ylens;
		Vector2 Vflame, Vlens;
		double  lens_loc, focus_loc, lens_step, lens_vel,
			lens_loc_min, lens_loc_max;

		double  drain_time, target_drain_time, extradrain;

		double  turn_rate_default;

		int     spr_lens_num, spr_flame_num;

		int         corepresent;
		double      corerespawntime, coremaketime;

		ShpHotSpot(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual void calculate();
		virtual void animate(Frame *frame);

		int activate_weapon();
		int activate_special();

		TheHotSpot *weaponhs;
};

class TheHotSpot : public Missile
{
	public:
	public:

		ShpHotSpot  *mother;
		double      damage_repeat_time, damage_time;
		int         hidefromview;

		TheHotSpot(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, ShpHotSpot *oship, SpaceSprite *osprite);

		virtual void calculate();

		virtual void inflict_damage(SpaceObject *other);
		virtual int  handle_damage(SpaceLocation *source, double normal, double direct = 0);
		virtual void animate(Frame *frame);

};

class CoreDump : public Shot
{
	public:
	public:

		int num_mirvs;

		CoreDump(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
			double odamage, double orange, double oarmour, SpaceLocation *opos,
			SpaceSprite *osprite, double relativity, int num);

		virtual void calculate();
};

ShpHotSpot::ShpHotSpot(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponRange         = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour        = tw_get_config_int("Weapon", "Armour", 0);
	weaponHeatingTime   = tw_get_config_float("Weapon", "WeaponHeatingTime", 0);
	weaponVelocityStep  = tw_get_config_float("Weapon", "VelocityStep", 0);

	specialRange        = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity     = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialDamage       = tw_get_config_int("Special", "Damage", 0);
	specialArmour       = tw_get_config_int("Special", "Armour", 0);

	turn_rate_default = turn_rate;

	// for this ship, we need additional graphic for the lens, and the source flame:

	spr_lens_num = 0;			 // = new SpaceSprite(this->ship->data->spriteExtra[0]);

	spr_flame_num = 1;			 // = new SpaceSprite(this->ship->data->spriteExtra[1]);

	lens_loc = 0.0;
	lens_vel = 0.0;
	//	lens_step = 0.0000003;
	lens_step = weaponVelocityStep * 1E-6;
	focus_loc = 80.0;

	lens_loc_min = 0.0;
	lens_loc_max = 0.9;

	drain_time = 0;
	target_drain_time = 2000.0;	 // default, it takes 2 seconds for the next fuel drain.

	weaponhs = 0;

	corepresent = 1;
	corerespawntime = 0.0;		 //10.0;		// it takes 10 seconds. --
	// or rather, don't worry; the batt is already drained anyway. It's nicer to play if
	// you can fire again, even though you've not mucho battery

	coremaketime = 0.0;
}


void ShpHotSpot::calculate()
{
	STACKTRACE;

	double a, L;

	if ( !(weaponhs && weaponhs->exists()) )
		weaponhs = 0;

	//Vector2 U = vel;
	a = angle;
	double prev_batt = batt;
	extradrain = 0;

	// the turn_rate depends somewhat on the distance of the flame ... far away, then the
	// turning is slower:
	//double Lrel = ( lens_loc - lens_loc_min ) / ( lens_loc_max - lens_loc_min );
	turn_rate = turn_rate_default / (1.0 + 0.25*lens_loc*weaponRange/80.0);
	// thus, you've default turn-rate at standard distance; and decreasing rate
	// further away.

	int fire_weapon_prev, fire_special_prev;
	fire_weapon_prev = fire_weapon;
	fire_special_prev = fire_special;

	Ship::calculate();
	// place the lens and the flame

	// refine the angle ... (doh - I know, this violates sc2 physics :( but it is
	//						necessary to make this ship anything near playable)
	angle += turn_step;
	turn_step = 0.0;

	// it takes awhile before there's a new nuclear core created.
	if ( !corepresent ) {
								 // in seconds
		coremaketime += frame_time * 1E-3;
		if ( coremaketime > corerespawntime )
			corepresent = 1;	 // now you can use the weapon again, which activates the core
	} else
	coremaketime = 0;

	//	if ( fire_weapon )
	//vel = U;	// avoid that the ship thrusts (if you use thrust/backthrust keys)
	//		angle = a;	// avoid that the ship turns (if you use left/right keys)

	if ( weaponhs ) {
		batt = prev_batt;		 // override the recharging.
		batt -= extradrain;		 // include drain from the special
	} else
	lens_loc = 0;

	sprite_index = get_index(angle);
	//a = sprite_index * PI / 32 + 0.5*PI;
	a = angle;

	// fixed position for the candle light

	L = 0.95 * 0.5*get_sprite()->size().y;

	Vflame = L * unit_vector(angle);

	// variable position for the focusing lens ... can use autofocus, or not !

	if ( weaponhs ) {			 // && !weaponhs->autofocus )
		if (fire_weapon && !fire_weapon_prev)
			lens_vel += lens_step * frame_time;

		if (fire_special && !fire_special_prev)
			lens_vel -= lens_step * frame_time;
	}
	/* else {
		// very slow (?) update of position:
		double a = 0.25 * lens_step;		// the acceleration value of the lens
										// scale by weaponRange for acc. of the hotspot
		if ( target && target->exists() )
		{
			double decel_time;
			if ( a > 0 )
				decel_time = lens_vel / a;	// fastest case is constant decelleration.
			else
				decel_time = 0.0;	// it cannot move at all ??!!

			// note, decel_time should be a signed value, otherwise it won't always
			// be opposing the velocity direction .... sort of.

			double decel_dist;
			double R = distance(target);

			// 2 cases, there is already an overshoot, and you've to turn back, and
			// there's no overshoot yet, and you've to slow down.

			if ( fabs(lens_vel) < a * frame_time )	// case 0, stationary point - initiate movement
			{
				if ( R > focus_loc )
					lens_vel += a * frame_time;
				else
					lens_vel -= a * frame_time;
			} else
			if ( (focus_loc - R) * lens_vel > 0 )	// case 1, already overshoot:
			{
				if ( focus_loc < R )
					lens_vel += a * frame_time;
				else
					lens_vel -= a * frame_time;
			} else {

				// case 2, moving towards target as planned

				if ( R < focus_loc )
					a *= -1;		// you require that the focus moves back !! thus, neg. acc.

				// what happens if you'd keep this desired? acceleration
				decel_dist = (lens_vel - 0.5 * a * decel_time) * decel_time * weaponRange;
				// this is the distance travelled if you'd decellerate continuously -
				// for the hotspot, not the lens

				if ( (R - (focus_loc + decel_dist)) * lens_vel > 0 )		// NOT a possible overshoot detected
					lens_vel += a * frame_time;		// accelerate
				else
					lens_vel -= a * frame_time;		// decellerate - overshoot, thus move in the opposite direction of what you'd expect
			}

		}
	}
	*/

	lens_loc += lens_vel * frame_time;

	if (lens_loc < lens_loc_min) {
		lens_loc = lens_loc_min;
		lens_vel = 0.0;
	}

	if (lens_loc > lens_loc_max) {
		lens_loc = lens_loc_max;
		lens_vel = 0.0;
	}

	L = (0.8-2*lens_loc) * 0.5*get_sprite()->size().y;

	Vlens = L * unit_vector(angle);

								 //1000.0;
	focus_loc = 80.0 + lens_loc * weaponRange;

	if ( weaponhs ) {
		//if ( weaponhs->autofocus )
		drain_time += frame_time;
		//else
		//	drain_time += 2*frame_time;

		if ( drain_time >= target_drain_time ) {
			drain_time -= target_drain_time;

			batt -= weapon_drain;

			if ( batt < 0 )
				batt = 0;
		}
	} else {
		drain_time = 0;
	}
}


void ShpHotSpot::animate(Frame *frame)
{
	STACKTRACE;
	// first, draw the lens and the flame.

	data->spriteWeapon->animate(pos-Vlens, sprite_index, frame);

	if ( weaponhs )
		data->spriteWeapon->animate(pos-Vflame, sprite_index+64, frame);

	// then, draw (animate?) the ship.

	Ship::animate(frame);
}


int ShpHotSpot::activate_weapon()
{
	STACKTRACE;
	if (!(weaponhs && weaponhs->exists()) && corepresent ) {
		weaponhs = new TheHotSpot(
			Vector2(0.0, 0.5*get_size().y), angle, weaponVelocity, weaponDamage, weaponRange,
			weaponArmour, this, data->spriteSpecial);

		game->add( weaponhs );
		return TRUE;

	} else
	return FALSE;
}


int ShpHotSpot::activate_special()
{
	STACKTRACE;
	if (! (nextkeys & keyflag::fire) )
		return FALSE;			 // you should be pressing a combination of them

								 // you can only drop a "cool" core if you've enough juice
	if ( corepresent && batt > 0.75*batt_max ) {
		corepresent = 0;

		if ( weaponhs && weaponhs->exists() )
			weaponhs->state = 0; // destroy the hotspot.

		batt = 0;
		update_panel = 1;

		// the core has a short range ;) but can do a lot of damage (6)
		game->add( new CoreDump(this, Vector2(0,-75), angle+PI, specialVelocity,
			specialDamage, specialRange, specialArmour,
			this, this->data->spriteExtra, 1.0, 0) );
	}

	return TRUE;				 // it was activated !!
}


TheHotSpot::TheHotSpot(Vector2 opos, double oangle, double ov,
int odamage, double orange, int oarmour, ShpHotSpot *oship, SpaceSprite *osprite)
:
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship,osprite)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 20;
	explosionFrameSize  = 50;

	mother = oship;

								 // about 0.5 second interval for damage.
	damage_time = mother->weaponHeatingTime;
	damage_repeat_time = 0;

	this->collide_flag_sameship = 0;

}


void TheHotSpot::calculate()
{
	STACKTRACE;

	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;

		return;
	}

	if ( mother->batt <= 0 ) {
		state = 0;
		return;
	}

	double a;
	//a = mother->get_sprite_index() * PI / 32 - 0.5*PI;
	a = mother->get_angle();

	pos = mother->pos + mother->focus_loc * unit_vector(a);
	vel = mother->vel;

	// Check all objects in-between the focus and the source, to see if the "light"
	// is not "blocked" ; also, it it's hitting a planet, relocate the spot to the
	// planet surface facing the ship.

	int blockedsight = 0;
	int hitplanet = 0;

	int layers = bit(LAYER_SHIPS) + bit(LAYER_CBODIES);
	double passiveRange = mother->focus_loc;

	Query q;
	for (q.begin(this, layers, passiveRange); q.current; q.next()) {
		if (!q.current->isObject())
			continue;

		SpaceObject *o = q.currento;

		if (o == mother || o == this)
			continue;

		// check, if this thing is in-between the ship and the hotspot focus

		// the size of this thing is ?
		double wi;
		wi = 0.5 * (o->get_size().x + o->get_size().y);

		// closest distance to the line of sight to the object is ...
		double R, D, a2;
		Vector2 Vd;
		Vd = o->normal_pos() - mother->normal_pos();
		R = magnitude( Vd );
		a2 = atan( Vd );
		D = fabs(R * sin(a-a2));

								 // does the query search work in a square region ?!
		if ( R > mother->focus_loc )
			continue;

		if ( D < wi )			 // a bit simplistic ; gross approximation !!
								 // make sure you CAN hit a ship or asteroid
			if ( fabs(R - mother->focus_loc) > wi )
				blockedsight = 1;

		// check if it hits ... a planet
		if (o->isPlanet() && D < wi && fabs(R - mother->focus_loc) < wi )
			hitplanet = 1;
	}

	// re-calculate the position
	if ( hitplanet || blockedsight )
		pos = mother->pos;

	if ( hitplanet || blockedsight )
		hidefromview = 1;
	else
		hidefromview = 0;

	// call this for pointer cleanup.
	SpaceObject::calculate();

	// check which graphic to use:
	//if ( normalhotspot )
	//	if ( autofocus )
	//		sprite_index = 0;		// a small hotspot
	//	else
	sprite_index = 64;			 // a big hotspot
}


void TheHotSpot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	damage_repeat_time += frame_time;

	if (damage_repeat_time > damage_time ) {
		damage_repeat_time -= damage_time;

		if (!hidefromview)
			damage(other, 0, damage_factor);
	}
}


int TheHotSpot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	// what happens when it hits the planet? it dies !!
	// why ... I dunno !?
	// answer: planet sets state=0 for 0-mass objects on impact.

	if (!state) {
		state = 1;
		hidefromview = true;
	}

	// don't do anything - it exists, until the source flame dies.
	return true;
}


void TheHotSpot::animate(Frame *frame)
{
	STACKTRACE;
	if (!hidefromview)
		Missile::animate(frame);
}


CoreDump::CoreDump(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
double odamage, double orange, double oarmour, SpaceLocation *opos,
SpaceSprite *osprite, double relativity, int num)
:
Shot(creator, rpos, oangle, ov,
odamage, orange, oarmour, opos,
osprite, relativity)
{
	STACKTRACE;
	num_mirvs = num;
	sprite_index = num_mirvs;	 // each mirv has a different sprite
}


void CoreDump::calculate()
{
	STACKTRACE;
	Shot::calculate();

	if (!state && num_mirvs < sprite->frames()-1) {
		++num_mirvs;
		// spawn 2 new coredumps, with half the damage, and at different angles

		double da = 45 * ANGLE_RATIO;

		game->add( new CoreDump(this, Vector2(0,0), angle+da, v,
			damage_factor/2, range, armour/2,
			this, sprite, 1.0, num_mirvs) );

		game->add( new CoreDump(this, Vector2(0,0), angle-da, v,
			damage_factor/2, range, armour/2,
			this, sprite, 1.0, num_mirvs) );

	}
}


REGISTER_SHIP(ShpHotSpot)
