/* $Id: shpbatde.cpp,v 1.21 2005/08/28 20:34:07 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

#include "../frame.h"
#include "../util/aastr.h"

/*

  quirk: anti-grav field. Its strength depends on the battery.
  battery drains when the ship moves.

  weapon: plasma missiles

  special: gas cloud, each confuses one of the existing homing missiles, which
			have the Deviant as their target.

*/

class SpriteDrawListItem : public Presence
{
	public:
		SpriteDrawListItem  *prev, *next;

		Vector2             pos;
		SpaceSprite         **sprites;
		int                 sprite_index, sprite_array_index;

		SpriteDrawListItem(SpriteDrawListItem *s, SpaceSprite **osprites);

		void animate(Frame *frame);
		void init(int oindex, Vector2 opos);
};

class SpriteDrawList : public Presence
{
	public:
		SpriteDrawListItem  *firstitem, *lastitem;
		SpaceObject         *mother;
		int                 Nsprites;
		double              existtime, delaytime;

		SpriteDrawList(SpaceObject *creator, int N, SpaceSprite **osprites, double odelaytime);
		virtual ~SpriteDrawList();

		void calculate();

		void animate(Frame *frame);
};

class   BathaMissile;
class   BathaCloud;

class BathaDeviant : public Ship
{
	public:
		double weaponRange, weaponVelocity, weaponTurnRate, weaponTailDelay, weaponMass;
		int    weaponDamage, weaponArmour;

		double  cloudLifeTime, cloudDamage;

		double  gravforce, gravforce_default, gravRange;

		double well_size;
		double whipfactor;
		double whipacc_max;

		double drain_travel_distance, drain_distance;

		int Nleakingsprites, Ntailsprites;
		SpaceSprite **leakingsprites, **tailsprites;

	public:

		int     CrewAsteroidHit, CrewShipHit, CrewPlanetHit;

		BathaDeviant(Vector2 opos, double angle, ShipData *data, unsigned int code);

		int activate_weapon();
		int activate_special();
		int accelerate_gravwhip( SpaceLocation *source, double angle, double velocity, double max_speed );

		virtual void calculate();

		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);

		virtual void inflict_damage(SpaceObject *other);

		void animate(Frame *frame);
};

class BathaMissile : public HomingMissile
{
	public:
		SpaceSprite *tailsprite;
	public:
		BathaMissile(SpaceLocation *creator, Vector2 rpos,
			double oangle, double ov, double odamage, double orange, double oarmour,
			double otrate, double omass, SpaceLocation *opos, SpaceSprite *osprite,
			SpaceObject *otarget, SpaceSprite **tailsprites, int Ntailsprites, double odelaytime);

		void animate(Frame *frame);
};

class BathaCloud : public SpaceObject
{
	public:

		double existtime, lifetime, spr_changetime, spr_time;

		BathaCloud(SpaceLocation *creator, Vector2 opos, double oangle,
			SpaceSprite *osprite, double olifetime, double odamage);

		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
};

BathaDeviant::BathaDeviant(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponRange         = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour        = tw_get_config_int("Weapon", "Armour", 0);
	weaponTurnRate      = scale_turning(tw_get_config_float("Weapon", "TurnRate", 0));
	weaponMass          = tw_get_config_float("Weapon", "Mass", 0);
	weaponTailDelay     = tw_get_config_float("Weapon", "TailDelay", 0);

	cloudLifeTime = tw_get_config_float("Special", "LifeTime", 0);
	cloudDamage = tw_get_config_float("Special", "Damage", 0);

	// properties of this ship are :

	gravforce_default = scale_acceleration(tw_get_config_float("Quirk", "GravityForce", 0), 0);
	whipfactor = tw_get_config_float("Special", "Quirk", 0);
	whipacc_max = tw_get_config_float("Special", "Quirk", 0);
	well_size = scale_range(tw_get_config_float("Quirk", "WellSize", 0));
	drain_distance = tw_get_config_float("Quirk", "DrainDistance", 0);
	drain_travel_distance = 0;

	// how much crew do you lose when you hit an enemy ship or an asteroid?

	CrewAsteroidHit = tw_get_config_int("Quirk", "DieAsteroid", 0);
								 // 10
	CrewShipHit = tw_get_config_int("Quirk", "DieShip", 0);
	CrewPlanetHit = tw_get_config_int("Quirk", "DiePlanet", 0);

	gravRange = scale_range( tw_get_config_float("Quirk", "GravRange", 0) );

	Ntailsprites = 8;
	tailsprites = &(data->more_sprites[0]);

	Nleakingsprites = 4;
	leakingsprites = &(data->more_sprites[Ntailsprites]);
}


int BathaDeviant::activate_weapon()
{
	STACKTRACE;
	BathaMissile *bm;
	bm = new BathaMissile(
		this, Vector2(0.0, 0.5*get_size().y), angle, weaponVelocity, weaponDamage,
		weaponRange, weaponArmour, weaponTurnRate, weaponMass, this, data->spriteWeapon,
		target, tailsprites, Ntailsprites, weaponTailDelay);
	game->add( bm );

	return TRUE;
}


int BathaDeviant::activate_special()
{
	STACKTRACE;

	// come slowly a halt.
	vel *= (1 - 5*frame_time*1E-3);

	// and also put up a single extra smoke cloud
	BathaCloud *bc;
	Vector2 D;

	double R = tw_random(50, 100);
	D = R * unit_vector(tw_random(PI2));

	bc = new BathaCloud(this, pos+D, angle, data->spriteSpecial, cloudLifeTime, cloudDamage);
	game->add(bc);

	return(true);
}


void BathaDeviant::calculate ()
{

	Ship::calculate();

	// if the Batha flies happily around, its battery slowly drains
	drain_travel_distance += vel.magnitude() * frame_time;
	if ( drain_travel_distance > drain_distance ) {
		if (batt >= 1) {
			batt -= 1;
			update_panel = 1;
		} else
		batt = 0;

		drain_travel_distance -= drain_distance;
	}

	// the anti-grav force depends on the battery,
	// but there is some minimum default level (to aid in escaping)

	//gravforce = gravforce_default * (0.5 + 0.5 * batt / batt_max);
	gravforce = gravforce_default * (0.05 + 0.95 * batt / batt_max);

	int layers = int(bit(LAYER_CBODIES)) | int(bit(LAYER_SHOTS)) |
		int(bit(LAYER_SHIPS)) | int(bit(LAYER_SPECIAL));

	//	double passiveRange = 1000.0;	// outside this area, gravity doesn't do anything

	Query a;
	for (a.begin(this, layers, gravRange); a.current; a.next()) {
		SpaceObject *o = a.currento;
		if (!o->isObject())
			continue;
		if (o->mass != 0 && o != ship ) {

			Vector2 Vd = min_delta(pos - o->pos, map_size);
			double R = magnitude(Vd);
			if (R < 1)
				continue;

			double bb;
			double Rscaled = R / well_size;

			if ( Rscaled > 1.0 )
				// outside the gravity well
				bb = 1.0 / (Rscaled * sqrt(Rscaled));
			// not R*R gravity, but slightly less for better feel (R*root(R))
			// also adds an extra velocity increase.
			else
				//bb = sin(Rscaled*0.5*PI);				// inside the gravity well
				// this sine function is stable, probably because it's smooth
				// at R=0 (has a derivative). It works good, much better than bb=const
				// or bb=Rscaled.
				bb = 1.0;

			bb *= frame_time;

			// directional tweak

			double ang;
			ang = trajectory_angle(o);
			ang = angle - ang;
			while (ang < -PI) ang += PI2;
			while (ang >  PI) ang -= PI2;

			bb *= fabs( sin(0.5 * ang) );

			// accelerate towards (or away from) the source

			Vector2 Vacc = -(Vd/R) * gravforce*bb;

			if (!o->isPlanet())
				o->change_vel ( Vacc );
			else
				change_vel ( -Vacc );

			change_vel (-Vacc);
			if (vel.length() > speed_max)
				vel *= speed_max / vel.length();

		}
	}

	if (((game->game_time) & 128) != ((game->game_time+frame_time) & 128)) {
		int i;
		i = tw_random(Nleakingsprites);

		Vector2 P;

		P.x = tw_random(-20,20);
		P.y = -20 - tw_random(20);

		int duration = iround(1000*fabs(1 - fabs(P.x/100)));
		P = rotate(P, angle+PI + PI/2);

		game->add(new Animation(this, pos+P, leakingsprites[i], sprite_index, 1,
			duration, DEPTH_SHIPS, 1.0));
	}

}


int BathaDeviant::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	// hitting a planet with its enormous mass is fatal
	// also creates an extra asteroid ;)
	if (source->isPlanet())
		normal += CrewPlanetHit;

	// hitting an asteroid deals damage
	if (source->isAsteroid())
		normal += CrewAsteroidHit;

	// hitting a ship causes major trauma as well
	if (source->isShip())
		normal += CrewShipHit;

	Ship::handle_damage(source, normal, direct);
	return iround(normal+direct);
}


void BathaDeviant::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	// the main body of the ship hits the enemy and does damage ....
	if (other->isShip()) {
		damage_factor = CrewShipHit;
		SpaceObject::inflict_damage(other);
	}

	if (other->isAsteroid()) {
		damage_factor = CrewAsteroidHit;
		SpaceObject::inflict_damage(other);
	}

}


//Animation::Animation(SpaceLocation *creator, Vector2 opos,
//	SpaceSprite *osprite, int first_frame, int num_frames, int frame_length,
//	double depth, double _scale)

void BathaDeviant::animate(Frame *frame)
{
	STACKTRACE;
	Ship::animate(frame);

}


int BathaDeviant::accelerate_gravwhip( SpaceLocation *source, double angle, double velocity, double max_speed )
{
	STACKTRACE;

	//Planet *p = nearest_other_planet();
	//if ( !p )
	return SpaceLocation::accelerate( source, angle, velocity, max_speed );
	//double tmp;
	//tmp = distance( p ) / p->gravity_range;
	//if ( tmp > 1 ) return SpaceLocation::accelerate( source, angle, velocity, max_speed );
	//return SpaceLocation::accelerate( source, angle, velocity,
	//		max_speed * (p->gravity_whip * tmp + 1) );
}


BathaMissile::BathaMissile(SpaceLocation *creator, Vector2 rpos,
double oangle, double ov, double odamage, double orange, double oarmour,
double otrate, double omass, SpaceLocation *opos, SpaceSprite *osprite,
SpaceObject *otarget, SpaceSprite **tailsprites, int Ntailsprites,
double otaildelay)
:
HomingMissile(creator, rpos, oangle, ov, odamage, orange, oarmour, otrate, opos, osprite, otarget)
{
	STACKTRACE;
	game->add(new SpriteDrawList(this, Ntailsprites, tailsprites, otaildelay) );
	mass = omass;
}


void BathaMissile::animate(Frame *frame)
{
	STACKTRACE;
	// animate the sphere.
	HomingMissile::animate(frame);
}


SpriteDrawListItem::SpriteDrawListItem(SpriteDrawListItem *s, SpaceSprite **osprites)
{
	STACKTRACE;
	prev = s;
	next = 0;

	sprites = osprites;

	sprite_index = 0;
	sprite_array_index = 0;
}


void SpriteDrawListItem::init(int oindex, Vector2 opos)
{
	STACKTRACE;
	sprite_index = oindex;
	pos = opos;

	sprite_array_index = 0;
}


void SpriteDrawListItem::animate(Frame *frame)
{
	STACKTRACE;
	SpaceSprite *spr;
	spr = sprites[sprite_array_index];

	Vector2 C, S;
	S = spr->size();
	C = corner(pos, S);

	spr->draw(C, S*space_zoom, sprite_index, frame);
}


SpriteDrawList::SpriteDrawList(SpaceObject *creator, int N, SpaceSprite **osprites, double odelaytime)
{
	STACKTRACE;
	mother = creator;

	SpriteDrawListItem *s;

	Nsprites = N;

	s = 0;

	int i;
	for ( i = 0; i < N; ++i ) {
		s = new SpriteDrawListItem(s, osprites);
		s->init(mother->get_sprite_index(), mother->pos);

		if (i == 0)
			firstitem = s;
	}

	lastitem = s;

	delaytime = odelaytime;
	existtime = 0;
}


SpriteDrawList::~SpriteDrawList()
{
	SpriteDrawListItem *s, *t;

	t = 0;

	s = firstitem;
	while ( s != 0 ) {
		t = s->next;
		delete s;

		s = t;
	}
}


void SpriteDrawList::calculate()
{
	STACKTRACE;
	if ( !(mother && mother->exists()) ) {
		mother = 0;
		state = 0;
		return;
	}

	existtime += frame_time * 1E-3;
	if ( existtime < delaytime )
		return;
	else
		existtime -= delaytime;

	SpriteDrawListItem *s;
	//	Vector2 *P;

	for ( s = firstitem; s != 0; s = s->next)
		if (s->sprite_array_index < Nsprites-1)
			++ s->sprite_array_index;

	// reset the last in the list, bring it to the front and init its values:
	s = lastitem;
	lastitem = s->prev;
	lastitem->next = 0;

	s->next = firstitem;
	s->prev = 0;
	firstitem->prev = s;

	firstitem = s;

	s->init(mother->get_sprite_index(), mother->pos);
}


void SpriteDrawList::animate(Frame *frame)
{
	STACKTRACE;
	SpriteDrawListItem *s;
	for ( s = lastitem; s != 0; s = s->prev)
		s->animate(frame);
}


BathaCloud::BathaCloud(SpaceLocation *creator, Vector2 opos, double oangle,
SpaceSprite *osprite, double olifetime, double odamage)
:
SpaceObject(creator, opos, oangle, osprite)
{
	STACKTRACE;
	layer = LAYER_SHOTS;
	vel = 0;
	isblockingweapons = false;	 //true;  don't block; just deal damage.

	damage_factor = odamage;

	lifetime = olifetime;
	existtime = 0;

	// check if one of the homing missiles has the creator as target - change
	// its target:

	int layers = bit(LAYER_SHOTS);
	double passiveRange = 1000.0;// outside this area, don't do anything

	Query a;
	for (a.begin(this, layers, passiveRange); a.current; a.next()) {
		SpaceObject *o = a.currento;
		if (!o->isObject())
			continue;
		if ( o->isShot() && ((Shot*)o)->isHomingMissile() && o->target == ship ) {
			o->target = this;
			break;
		}
	}

	spr_changetime = 0.3;
	spr_time = 0;
	sprite_index = 0;

	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void BathaCloud::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();

	existtime += frame_time * 1E-3;
	if (existtime > lifetime) {
		state = 0;
		return;
	}

	spr_time += frame_time * 1E-3;
	if ( spr_time > spr_changetime ) {
		spr_time -= spr_changetime;
		++sprite_index;
		if (sprite_index >= sprite->frames())
			sprite_index = 0;
	}
}


int BathaCloud::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	state = 0;					 // just disappear.
	return true;
}


REGISTER_SHIP(BathaDeviant)
