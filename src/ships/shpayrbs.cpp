/* $Id: shpayrbs.cpp,v 1.25 2005/08/28 20:34:07 geomannl Exp $ */

#include "../ship.h"
#include "../melee/mview.h"
REGISTER_FILE

#include <stdio.h>

#include "../frame.h"
#include "../other/shippart.h"
#include "../util/aastr.h"

class AyronBS;
class AutoGun;

// this is read separately from the layout file - to keep things organized ?
struct AyronShipPartInfo
{
	double  crewmax, crew, batt, dynamo, turning, thrusting;
	SpaceSprite *spr_crewed, *spr_uncrewed;
};

class AyronShipPart : public BigShipPart
{
	public:

		//double	crewmax, crew, batt, dynamo, turning, thrusting;

		AyronShipPart(AyronBS *aowner, Vector2 orelpos, int otype, AyronShipPartInfo **info);

		virtual void animate(Frame *space);
		//virtual void calculate();

		virtual int handle_damage(SpaceLocation *source, double normal, double direct=0);

		virtual bool isdisabled();
		virtual bool hascrew();
		virtual void recrew(int howmany);
};

class AutoGun : public BigShipPartDevice
{
	public:
		double shotrange, shotvel, shotdamage, shotarmour, shotdrain;
		double a_center, a_view, a_track;
		double shotbusy, shotperiod;
		SpaceSprite *sprshot;

	public:

		AutoGun(AyronShipPart *aowner, Vector2 orelpos, double centerangle, double moveangle,
			SpaceSprite *spr, SpaceSprite *osprshot);
		//virtual void animate(Frame *space);
		virtual void calculate();
		//	virtual int handle_damage(SpaceLocation *source, double normal, double direct=0);
};

class AyronBS : public BigShip
{
	public:
		AyronShipPart **ayronparts;
	public:
		int     Nx, Ny, Ntypes;
		double  w;

		int     nBSinfo;
		AyronShipPartInfo **BSinfo;

		double  recharge_rate_ref, accel_rate_ref, turn_rate_ref;

		double  weaponRange, weaponDamage;
		int     weaponColor;

		AyronBS(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual void calculate();

		// you can overload this to load a different type of ship-part into the AyronBS.
		virtual AyronShipPart *init_part(Vector2 orelpos, int otype, AyronShipPartInfo **info);

		virtual int activate_weapon();
		virtual int activate_special();

};

AyronShipPart *AyronBS::init_part(Vector2 orelpos, int otype, AyronShipPartInfo **info)
{
	STACKTRACE;

	if (otype > 0)
		return new AyronShipPart(this, orelpos, otype, info);
	else
		return 0;
}


AyronBS::AyronBS(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code)
:
BigShip(opos, shipAngle, shipData, code | SpaceSprite::NO_AA)
{
	STACKTRACE;

	int i;
	int ix, iy;

	// read the layout
	FILE *inp;

	inp = fopen("ships/shpayrbs_layout.txt", "rt");
	if (!inp) {tw_error("couldn't find the AyronBS layout");}

	fscanf(inp, "%i\n", &nBSinfo);

	Ntypes = nBSinfo;

	BSinfo = new AyronShipPartInfo* [nBSinfo];

	for ( i = 0; i < nBSinfo; ++i ) {
		BSinfo[i] = new AyronShipPartInfo();

		fscanf(inp, "%*10c %lf %lf %lf %lf %lf",
			&BSinfo[i]->crew, &BSinfo[i]->batt, &BSinfo[i]->dynamo,
			&BSinfo[i]->turning, &BSinfo[i]->thrusting);

		BSinfo[i]->spr_crewed = data->more_sprites[i];
		BSinfo[i]->spr_uncrewed = data->more_sprites[i + Ntypes];
	}

	fscanf(inp, "%i %i\n", &Nx, &Ny);
	Nparts = Nx * Ny;

	//	ayronparts = new (AyronShipPart*) [N];
	ayronparts = new AyronShipPart* [Nparts];
	parts = (BigShipPart**) ayronparts;

	int *itype, *iweapon;
	double *anglecenter, *moveangle;

	itype = new int [Nparts];
	iweapon = new int [Nparts];
	anglecenter = new double [Nparts];
	moveangle = new double [Nparts];

	// the ship hull sprites
	for ( i = 0; i < Nparts; ++i ) {
		fscanf(inp, "%i",
			&itype[i]);
	}

	// the ship weapons
	for ( i = 0; i < Nparts; ++i ) {
		fscanf(inp, "%i %lf %lf", &iweapon[i], &anglecenter[i], &moveangle[i]);
		anglecenter[i] *= PI / 180.0;
		moveangle[i] *= PI / 180.0;
	}

	fclose(inp);

	// the dimension of a single block of the ship (on a regular grid).
	// note that each block should be square, and of each grid-block is
	// of the same size.
	w = data->more_sprites[0]->size(0).x;

	// the orig picture has 0.5 extra room to allow for rotation; this extra room
	// is empty.
	w /= 1.5;

	// initialize the ship ayronparts.
	for ( iy = 0; iy < Ny; ++iy ) {
		for ( ix = 0; ix < Nx; ++ix ) {
			int k;
			k = iy*Nx + ix;

			Vector2 relpos;

			relpos = w * Vector2(0.5*(Ny-1) - iy, ix - 0.5*(Nx-1));

			ayronparts[k] = init_part(relpos, itype[k], BSinfo);
			if (ayronparts[k])
				physics->add(ayronparts[k]);

			//	AutoGun(AyronBS *aowner, Vector2 orelpos, int otype, SpaceSprite *spr);
			if (iweapon[k] == 1)
				game->add(new AutoGun(ayronparts[k], relpos,
					anglecenter[k], moveangle[k],
					data->spriteWeapon, data->spriteWeaponExplosion));
		}
	}

	delete[] itype;
	delete[] iweapon;
	delete[] anglecenter;
	delete[] moveangle;

	for ( i = 0; i < nBSinfo; ++i )
		delete BSinfo[i];
	delete[] BSinfo;

	weaponColor  = tw_get_config_int("Weapon", "Color", 0);
	weaponRange  = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponDamage = tw_get_config_int("Weapon", "Damage", 0);

	recharge_rate_ref = recharge_rate;
	turn_rate_ref = turn_rate;
	accel_rate_ref = accel_rate;

	// remove this from the physics interaction
	//mass = 0; this value is needed by the ship ayronparts
	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	attributes |= ATTRIB_UNDETECTABLE;

	// also, add the ship's ayronparts to the physics target list
	//	for (i = 0; i < N; ++i )
	//		if (ayronparts[i])
	//			game->add_target(ayronparts[i]);
	// update: no need to this here anymore, it's done in the part-constructor.

	crew_max = 0;
	for ( i = 0; i < Nparts; ++i )
		if (ayronparts[i])
			crew_max += ayronparts[i]->crew_max;
}


void AyronBS::calculate()
{
	STACKTRACE;
	int i;

	// re-calculate the total ship stats from all of its little ayronparts

	crew = 0;
	batt_max = 0;
	recharge_rate = 0;
	turn_rate = 0;
	accel_rate = 0;

	double Nrecharge = 0;

	for ( i = 0; i < Nparts; ++i ) {
		if (!ayronparts[i])
			continue;

		if (!ayronparts[i]->exists()) {
			ayronparts[i] = 0;
			continue;
		}

		// NOTE:
		// this should not occur.
		// the ship ayronparts should always be "there",
		// except that they'll go inert (and grey) if they're "dead"
		// (unless they were never added in the first place of course)

		// add the stats of this part to the total stats
		if (ayronparts[i]->hascrew()) {
			crew += ayronparts[i]->crew;
			batt_max += ayronparts[i]->batt;
			//recharge_rate += ayronparts[i]->dynamo * recharge_rate_ref;
			Nrecharge += ayronparts[i]->recharge_rate;
			turn_rate += ayronparts[i]->turn_rate * turn_rate_ref;
			accel_rate += ayronparts[i]->accel_rate * accel_rate_ref;
		}
	}

	// this is the delay time between recharges.
	if (Nrecharge > 0)
		recharge_rate = iround(recharge_rate_ref / Nrecharge);

	// detect your death ... weeeeeh :()
	if (!crew) {
		state = 0;
		return;
	}

	BigShip::calculate();

}


//AyronShipPart::AyronShipPart(AyronBS *aowner, Vector2 orelpos,
//				double acrew, double abatt, double adynamo,
//				double aturning, double athrusting,
//				SpaceSprite *spr, SpaceSprite *spr_uncrewed)

AyronShipPart::AyronShipPart(AyronBS *aowner, Vector2 orelpos, int otype, AyronShipPartInfo **info)
:
BigShipPart(aowner, orelpos, 0.0, info[otype-1]->spr_crewed, info[otype-1]->spr_uncrewed)
{
	STACKTRACE;

	//crew      = info[otype-1]->crew;
	//crewmax   = info[otype-1]->crew;
	//batt      = info[otype-1]->batt;
	//dynamo    = info[otype-1]->dynamo;
	//turning   = info[otype-1]->turning;
	//thrusting = info[otype-1]->thrusting;

	crew = info[otype-1]->crew;;
	crew_max = crew;
	batt = info[otype-1]->batt;
	batt_max = batt;
	recharge_rate = iround(info[otype-1]->dynamo);

	recharge_step = recharge_rate;
	recharge_amount = 1;
	turn_rate = info[otype-1]->turning;
	accel_rate = info[otype-1]->thrusting;

}


int AyronShipPart::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	// transmit damage to the ship owner ...
	//return owner->handle_damage(source, normal, direct);
	// no ...
	// handle damage locally. The big ship can test all ayronparts to calculate its global
	// parameters (like total crew left, total thrust, etc)...

	if (!hascrew())
		return 0;

	double t;
	t = normal + direct;
	crew -= t;

	if (crew <= 0) {
		t = -crew;

		// set crew to 0 (false)
		crew = 0;

		// make this ship part inert.
		// (thus exposing other, perhaps more vulnerable, ayronparts of the ship)
		collide_flag_anyone = 0;
		collide_flag_sameship = 0;
		collide_flag_sameteam = 0;
		mass = 0;
		attributes |= ATTRIB_UNDETECTABLE;

		// as soon as this is done, it won't have to handle damage anymore ?

		// remove this thing from the game target list.
		//removefromtargetlist(this);
		targets->rem(this);

	} else
	t = 0;						 // all damage absorbed.

	//return normal+direct - t;	// returns true or false I guess...
	return 1;
}


bool AyronShipPart::hascrew()
{
	STACKTRACE;
	return crew != 0;
}


bool AyronShipPart::isdisabled()
{
	STACKTRACE;
	return !hascrew();
}


void AyronShipPart::recrew(int howmany)
{
	STACKTRACE;
	if (crew == 0 && howmany > 0) {
		// restore physical interaction
		collide_flag_anyone = ALL_LAYERS;
		collide_flag_sameteam = ALL_LAYERS;

		mass = owner->mass;

		attributes &= ~ATTRIB_UNDETECTABLE;

		// add it back to the target list of the game
		if (!targets->isintargetlist(this))
			targets->add(this);
	}

	crew += howmany;
	if (crew > crew_max)
		crew = crew_max;
}


AutoGun::AutoGun(AyronShipPart *aowner, Vector2 orelpos, double centerangle, double moveangle,
SpaceSprite *spr, SpaceSprite *osprshot)
:
BigShipPartDevice(aowner, spr)
{
	STACKTRACE;
	sprshot = osprshot;

	a_center = centerangle;
	a_view = moveangle;

	shotvel = 1.0;
	shotdamage = 1;
	shotrange = 1000;
	shotarmour = 1;
	shotdrain = 1;
	shotperiod = 0.5;			 // reload time in seconds

	a_track = 0;
	shotbusy = 0;

	attributes &= ~ATTRIB_STANDARD_INDEX;

	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;
	collide_flag_anyone = 0;
}


//Shot::Shot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
//	double odamage, double orange, double oarmour, SpaceLocation *opos,
//	SpaceSprite *osprite, double relativity)

void AutoGun::calculate()
{
	STACKTRACE;
	BigShipPartDevice::calculate();

	if (!(ownerpart && !ownerpart->isdisabled()))
		return;

	// find the closest target
	Query a;
	SpaceObject *oclosest;
	double R;

	oclosest = 0;
	R = 1E99;

	double da_track;
	da_track = -a_track;		 // forces to go back into default position

	for (a.begin(this, OBJECT_LAYERS, 0.75*shotrange); a.current != 0; a.next()) {
		SpaceObject *o;
		o = a.currento;

		if (!o->isObject())
			continue;

		if (o->sameShip(this) || o->sameTeam(this))
			continue;

		if (o->isPlanet())
			continue;

		if (o->isInvisible())
			continue;

		double L = 0;
		L = distance(o);

		if (!oclosest || L < R) {
			// this is a possible candidate but, is it also within the view angle ?

			double a, b, da;
			a = trajectory_angle(o);
			// check relative to the gun's neutral position
			b = ownerpart->get_angle() + a_center;

			da = a - b;
			while (da < -PI)    da += PI2;
			while (da >  PI)    da -= PI2;

			if (da > -a_view && da < a_view) {
				// okay! it's close, within dist-range, and it's within angle-range
				oclosest = o;
				R = L;

				// required movement is relative to the gun's current position
				// note that at this moment, you cannot use the value of "angle"
				// which is overridden by the call to AyronShipPart::calculate; so,
				// use "b" instead.
				b += a_track;
				da_track = a - b;
				while (da_track < -PI)  da_track += PI2;
				while (da_track >  PI)  da_track -= PI2;
			}

		}

	}

	// track the target (or go back to neutral position)
	// max tracking speed per second (0.1 circles / second).
	// this tracking angle is relative to the neutral angle of the gun.

	double da;
	da = 100 * PI2 * frame_time * 1E-3;
	if (da_track > da)
		da_track = da;
	if (da_track < -da)
		da_track = -da;

	a_track += da_track;

	if (a_track < -a_view)
		a_track = -a_view;
	if (a_track > a_view)
		a_track = a_view;

	// absolute angle of the gun
	angle = ownerpart->get_angle() + a_center + a_track;
	sprite_index = get_index(angle);

	// fire the weapon ?
	if (oclosest && ownerpart->ship && ownerpart->ship->batt > shotdrain && !shotbusy) {

		Shot *s;
		s = new Shot(this, Vector2(0,0), angle, shotvel,
			shotdamage, shotrange, shotarmour, this,
			sprshot, 0);

		s->set_depth(DEPTH_SHIPS + 0.1);
		game->add( s );

		shotbusy = shotperiod;

		ownerpart->ship->handle_fuel_sap(this, shotdrain);
	}

	if (shotbusy > 0)
		shotbusy -= frame_time * 1E-3;
	else
		shotbusy = 0;

}


void AyronShipPart::animate(Frame *space)
{
	STACKTRACE;
	SpaceSprite *spr;

	if (!hascrew())
		spr = sprite_uncrewed;
	else
		spr = sprite;

	// the aa_stretch_blit scales down too much (dunno why).
	// use the usual stretch_blit instead ?

	Vector2 P, S, W;

	S = spr->size(sprite_index);
	P = corner(pos, S );
	W = S * space_zoom;

	BITMAP *b;
	b = spr->get_bitmap(sprite_index);

	masked_stretch_blit(b, space->surface, 0, 0, b->w, b->h, iround(P.x), iround(P.y), iround(W.x), iround(W.y));
	//aa_stretch_sprite(b, space->surface, iround(P.x), iround(P.y), iround(W.x), iround(W.y));
	space->add_box(P.x, P.y, W.x, W.y);
}


int AyronBS::activate_weapon()
{
	STACKTRACE;
	double dx, dy;

	dx = 75;
	dy = Ny/2 * w - 25;

	// a big laser ?
	if (ayronparts[0] && ayronparts[0]->hascrew())
		add(new Laser(this, angle,
			pallete_color[weaponColor], weaponRange, weaponDamage, weapon_rate,
			this, Vector2(-dx, dy), true));

	if (ayronparts[Nx-1] && ayronparts[Nx-1]->hascrew())
		add(new Laser(this, angle,
			pallete_color[weaponColor], weaponRange, weaponDamage, weapon_rate,
			this, Vector2(dx, dy), true));

	return true;
}


int AyronBS::activate_special()
{
	STACKTRACE;
	// add to the crew of one of the damaged compartments.

	// first, check how many are "damaged"
	int i, Ndead;

	Ndead = 0;
	for ( i = 0; i < Nparts; ++i )
		if (ayronparts[i] && ayronparts[i]->crew < ayronparts[i]->crew_max)
			++Ndead;

	if (!Ndead)
		return false;

	// pick a random one from the dead ones.
	int k;

	k = tw_random(Ndead) + 1;

	for ( i = 0; i < Nparts; ++i ) {
		if (ayronparts[i] && ayronparts[i]->crew < ayronparts[i]->crew_max) {
			--k;
			if (k <= 0)
				break;
		}
	}

	if (i < Nparts) {
		ayronparts[i]->recrew(1);
	}

	return true;
}


REGISTER_SHIP(AyronBS)
