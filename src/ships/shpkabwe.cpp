/* $Id: shpkabwe.cpp,v 1.21 2005/09/27 22:00:49 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"
#include "../frame.h"
#include "../util/aastr.h"

/* How does this ship work

  The ship has a main pea-shooter that does 1 damage by default.

  It also fires mines, which create a damaging field around their host when
  they attach themselves.

  The damaging field increases ANY damage that is being taken by 1.0 and is permanent.

  Still to do:
  graphics.

*/

// Based on the Kzer-Za dreadnought and numerous other
// fragments of timewarp/ allegro code.

class KaboHaze : public SpaceLocation
{
	public:
	public:
		KaboHaze    *next, *prev;
		Ship        *host;		 //*mother,
		double      power, basepower;
		//	double		decay_time;
		int         oldcrew, newcrew;
		int         *edge_left, *edge_right;
		int         sprite_index, shield_sprite_index;

		BITMAP      *shield_bmp[64];

		KaboHaze(SpaceLocation *creator, Ship *ohost, double obasepower);
		virtual void calculate();
		virtual void animate(Frame *space);
		virtual void collide(SpaceObject *other);
		//virtual void death();
		virtual ~KaboHaze();
};

// This keeps track of all hazes fired by all ships, and is located
// static in memory.

static KaboHaze *KaboHazeFirst = 0;

class KaboPod;

class KaboWeakener : public Ship
{
	public:
	public:
		double weaponRange;
		double weaponVelocity;
		int    weaponDamage;
		int    weaponArmour;

		double mineLifeTime, mineHostileTime;
		double mineVelocity;
		double mineRange;
		int    mineArmour;
		double mineTurnRate;

		double  podVelocity, podRange, podArmour, podDamage;
		int     podNmines;

								 //, Haze_decaytime;
		double  Mine_RangeAcc, Haze_basepower;

		KaboPod *pod;

		KaboWeakener(Vector2 opos, double angle, ShipData *data, unsigned int code);

		int activate_weapon();
		int activate_special();
};

// an intermediate - flies a while, then releases the mines
class KaboPod : public Missile
{
	public:

		KaboWeakener *mother;
		int ididdamage;
		double spriteindextime;

	public:
		KaboPod(Vector2 opos, double oangle, double ov,
			double orange, double oarmour, double odamage,
			KaboWeakener *oship, SpaceSprite *osprite);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

class KaboMine : public SpaceObject
{
	public:
		double lifetime, existtime, hostiletime;
		double spriteindextime;
		double Haze_basepower;	 //, Haze_decaytime;

		double  velocity, turn_rate;

		SpaceObject *thetarget;

	public:
		KaboMine(Vector2 opos, double oangle, double ov, double oturn_rate,
			double orange, double oarmour, double olifetime, double ohostiletime,
			double orangeacc,  double obasepower,
			SpaceLocation *ocreator, SpaceSprite *osprite);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

KaboWeakener::KaboWeakener(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;

	// for the pea shooter:
	weaponRange         = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour        = tw_get_config_int("Weapon", "Armour", 0);

	// for the KaboMine:
	mineVelocity         = scale_velocity(tw_get_config_float("Mine", "Velocity", 0));
	mineArmour           = tw_get_config_int("Mine", "Armour", 0);
	Mine_RangeAcc        = scale_range(tw_get_config_float("Mine", "Range", 0));
	mineTurnRate         = scale_turning(tw_get_config_float("Mine", "TurnRate", 0));
	mineLifeTime         = tw_get_config_float("Mine", "LifeTime", 0);
	mineHostileTime      = tw_get_config_float("Mine", "HostileTime", 0);

	// for the Pod:
	podVelocity        = scale_velocity(tw_get_config_float("Pod", "Velocity", 0));
	podArmour          = tw_get_config_int("Pod", "Armour", 0);
	podRange           = scale_range(tw_get_config_float("Pod", "Range", 0));
	podDamage          = tw_get_config_float("Pod", "Damage", 0);
	podNmines          = tw_get_config_int("Pod", "Nmines", 0);

	// for the haze:
	Haze_basepower =    tw_get_config_float("Haze", "Power", 1.0);
	//	Haze_decaytime =	tw_get_config_float("Haze", "DecayTime", 10000.0);

	pod = 0;

}


int KaboWeakener::activate_weapon()
{
	STACKTRACE;

	Missile *m;
	m = new Missile(this, Vector2(0.0, 0.5*get_size().y),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon);
	m->isblockingweapons = false;
	add(m);
	return(TRUE);
}


int KaboWeakener::activate_special()
{
	STACKTRACE;
	if (!pod) {
		pod = new KaboPod(Vector2(0.0, -0.5*get_size().y),
			angle+PI, podVelocity, podRange, podArmour, podDamage,
			this, data->spriteSpecial);

		game->add(pod);
		return TRUE;
	}

	return FALSE;
}


KaboMine::KaboMine (Vector2 opos, double oangle, double ov, double oturnrate,
double orange, double oarmour, double olifetime, double ohostiletime, double orangeacc, double obasepower,
SpaceLocation *ocreator, SpaceSprite *osprite)
:
SpaceObject(ocreator, ocreator->pos+rotate(opos,oangle-PI/2), oangle, osprite),
lifetime(olifetime),
hostiletime(ohostiletime),
Haze_basepower(obasepower)
{
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);

	spriteindextime = 0;

	velocity = ov;
	turn_rate = oturnrate;

	existtime = 0;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = 0;

	isblockingweapons = false;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void KaboMine::calculate()
{
	STACKTRACE;

	existtime += frame_time * 1E-3;

	if ( existtime > lifetime ) {
		state = 0;
		return;
	}

	if ( existtime > hostiletime ) {
		collide_flag_sameship = ALL_LAYERS;
	}

	// make the sprite/mines rotate around their axes
	spriteindextime += frame_time / 25.0;
	sprite_index = (int)(spriteindextime);
	sprite_index &= 63;

	// move around aimlessly

	double t = turn_rate * frame_time;
	angle += tw_random(-t, t);
	vel = velocity * unit_vector(angle);

	SpaceObject::calculate();
}


int KaboMine::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	//	state = 0;	// is already done by inflict_damage.
	return 0;
}


void KaboMine::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

								 // only attack ships !
	if ( !(other && other->isShip()) ) {
		die();
		return;
	}

	if (other == ship) {
		// if the object is the mother-ship, then don't inflict damage.
		die();
		return;
	}

	// first check, if a haze already exists,
	// attached to the same "enemy"

	KaboHaze *haze;
	for ( haze = KaboHazeFirst ; haze != 0 ; haze = haze->next) {
		if ( other == haze->host )
			break;				 // hey ! this ship is already targeted !!
	}

	if ( haze != 0 ) {			 // hey ! this ship is already targeted !!
		haze->power += haze->basepower;
	} else {
		// it's the first time a haze is applied to this ship ...
		haze = new KaboHaze( this, (Ship*) other, Haze_basepower);

		game->add(haze);
	}

	// the seed mine has done it's job well, it can rest now ...
	state = 0;

	return;
}


void KaboHaze::collide(SpaceObject *other)
{
	STACKTRACE;
	return;						 // never collide with anything.
}


void KaboHaze::calculate()
{
	STACKTRACE;
	SpaceLocation::calculate();	 // this sets ship=0 or target=0 if needed.

	if (!(host && host->exists()) ) {
		host = 0;
		state = 0;
		return;
	}

	// the following is used for drawing, really ; but has to be calculated here
	// because it's a globally accessible/ used variable.

								 // + 16;
	shield_sprite_index = get_index(host->angle, 0.5*PI);
	shield_sprite_index &= 63;

	// decrease strength over time ... and when strength = 0, then it should be removed
	/*
		if ( power <= 0.01 )	// too weak, the field loses integrity
			state = 0;
		else
			power *= exp( - frame_time / decay_time );
	*/
	// change: the damaging shield is permanent, now.

	// check damage taken this turn ... increase that damage !

	oldcrew = newcrew;
	newcrew = iround(host->getCrew());
	if ( newcrew < oldcrew  ) {
		// add extra damage !!
		//... but how ... fractional, or what ??

		//int extradeaths = int( (oldcrew - newcrew) * ( power - 1 ) );	// this would be a damage amplifier
								 // simple, an additional damage on top of other damage, each frame
		int extradeaths = iround(power);

		if ( extradeaths > 0 ) {
			//this->damage_factor = extradeaths;
			//this->inflict_damage(host);		// that poor host !
			this->damage(host, extradeaths);
		}

		newcrew = iround(host->getCrew());

		//power *= 0.66;		// big powerdrain from the shield !
		// change: the damage shield is permanent.

	}

}


void blit_singlecolor(BITMAP *src, BITMAP *dest, int copycolor)
{
	STACKTRACE;
	if ( src->w != dest->w || src->h != dest->h )
		tw_error("error in copying color in shpysr2");

	for ( int iy = 0; iy < src->h; ++iy ) {
		for ( int ix = 0; ix < src->w; ++ix ) {
			int color = getpixel(src, ix, iy);
			if ( color == copycolor )
				tw_putpixel(dest, ix, iy, color);
		}
	}

}


void blit_resize( BITMAP* *bmpold, int wnew, int hnew)
{
	STACKTRACE;
	//int wold = (*bmpold)->w;
	//int hold = (*bmpold)->h;

	BITMAP *bmpnew = create_bitmap(wnew, hnew);

	stretch_blit(*bmpold, bmpnew, 0, 0, (*bmpold)->w, (*bmpold)->h, 0, 0, bmpnew->w, bmpnew->h);

	destroy_bitmap(*bmpold);

	*bmpold = bmpnew;			 // point to the scaled bitmap !
}


// WHY DOES THIS TAKE SO LONG ?!?!
// an image of 100x100 pixels means,
// there's 10,000 calculations involved !
// no wonder.
void blit_blur(BITMAP *src, int R)
{
	STACKTRACE;
	int *blurmap;

	if ( R <= 2 )
		return;

	// first, create a sinusoid blur template

	blurmap = new int [R*R];

	double radius = R / 2.0;
	double xmid = radius;
	double ymid = radius;

	int ix, iy, k;

	k = 0;
	for ( iy = 0; iy < R; ++iy ) {
		for ( ix = 0; ix < R; ++ix ) {
								 // 0.5, so that you're in the center of the pixel
			double x = ix+0.5 - xmid;
			double y = iy+0.5 - ymid;
			double range = sqrt(x*x + y*y);
			if (range > radius) range = radius;
			blurmap[k] = int(255 * cos(0.5*PI * range/R));
			++k;
		}
	}

	// next, create a copy of the picture to which we write the result

	BITMAP *temp_bmp = create_bitmap(src->w, src->h);

	// next, apply the blur

	for ( iy = 0; iy < src->h; ++iy ) {
		for ( ix = 0; ix < src->w; ++ix ) {

			// create a new pixel at this position:

			int xblur, yblur;
			int newcolorR = 0;
			int newcolorG = 0;
			int newcolorB = 0;
			int totweight = 0;

			k = 0;
			for ( yblur = 0; yblur < R; ++yblur ) {
				for ( xblur = 0; xblur < R; ++xblur ) {
					int color = getpixel(src, ix+xblur-R/2, iy+yblur-R/2);
					if ( color != -1 ) {

						int weight = blurmap[k];
						++k;

						newcolorR += getr(color) * weight;
						newcolorG += getg(color) * weight;
						newcolorB += getb(color) * weight;
						totweight += weight;
					}
				}
			}
			newcolorR /= totweight;
			newcolorG /= totweight;
			newcolorB /= totweight;

			int newcolor = tw_makecol(newcolorR, newcolorG, newcolorB);
			tw_putpixel(temp_bmp, ix, iy, newcolor);

		}
	}

	// and then, copy the blurred image on top of the old one, so that becomes blurred.

	blit(temp_bmp, src, 0, 0, 0, 0, src->w, src->h);

	destroy_bitmap(temp_bmp);

	delete[] blurmap;
}


//void KaboHaze::death()
KaboHaze::~KaboHaze()
{
	// update the pointers, remove this item from the list
	KaboHaze *p1, *p2;
	p1 = this->prev;
	p2 = this->next;

	if (p1)
		p1->next = p2;

	if (p2)
		p2->prev = p1;

	if ( KaboHazeFirst == this )
		KaboHazeFirst = p2;

	for ( int i = 0; i < 64; ++i )
		if ( shield_bmp[i] )
			destroy_bitmap(shield_bmp[i]);

	delete[] edge_left;
	delete[] edge_right;

	// other destructive stuff ?
	//	SpaceObject::~SpaceObject();

}


KaboHaze::KaboHaze(SpaceLocation *creator, Ship *ohost, double obasepower)
:
SpaceLocation(creator, Vector2(0.0, 0.0), 0.0)
{
	STACKTRACE;
	prev = 0;
	next = 0;

	// update the list (insert at the start)
	if (KaboHazeFirst)
		KaboHazeFirst->prev = this;
	next = KaboHazeFirst;
	prev = 0;
	KaboHazeFirst = this;

	//mother = omother;
	host = ohost;
	basepower = obasepower;

	power = basepower;

	//	decay_time = odecaytime;	// in milliseconds

	newcrew = iround(host->getCrew());
	oldcrew = newcrew;

	// this "haze" is passive, of course:
	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;

	// I'll make 64 rotated versions in total, if needed, which I'll store
	// in memory:

	for ( int i = 0; i < 64; ++i ) {
		shield_bmp[i] = 0;
	}
	sprite_index = 0;			 // no rotation.
	shield_sprite_index = 0;

	// this item cannot collide

	collide_flag_anyone = 0;

	// this is probably important ... otherwise a thing like a flipping wedge indicator
	// can avoid a shield from being drawn ?!?!

	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS + 0.1);
	// The +0.1 places this presence at a higher level, so that this routine is
	// always done after the ship was drawn! Thnx Orz, for telling.

	// Graphics init stuff for the shield !

	// copy the ship sprite (yes, a COPY because we need to do some operations on it !)

	SpaceSprite *ship_spr;
	ship_spr = host->get_sprite();
	if (!ship_spr) {
		state = 0;
		return;
	}

	//BITMAP *ship_bmp;
	int wship = ship_spr->width();
	int hship = ship_spr->height();

	BITMAP *ship_bmp = create_bitmap(wship, hship);
	clear_to_color(ship_bmp, 0); // important otherwise it contains artefacts

	int index = 0;
	ship_spr->draw(Vector2(0, 0), Vector2(wship, hship), index, ship_bmp);
	// this does a (masked?) blit

	// create a blurred image from this:
	int R = 3;
	blit_blur(ship_bmp, R);		 // a complex and costly funtion ! Inefficiently programmed as well of course (by me).

	// now, create a masked shield - only the area that covers the
	// blurred image of the ship:

	shield_bmp[sprite_index] = create_bitmap(wship, hship);
								 // important otherwise it contains artefacts
	clear_to_color(shield_bmp[sprite_index], 0);

	// scale/draw a shield:

	/*
	// the raw shield image
	BITMAP *raw_bmp = this->sprite->get_bitmap_readonly(0);

	int wraw = raw_bmp->w;
	int hraw = raw_bmp->h;

	stretch_blit(raw_bmp, shield_bmp[sprite_index], 0, 0, wraw, hraw, 0, 0, wship, hship );
	*/
								 // a uniform green glow
	clear_to_color(shield_bmp[sprite_index], tw_makecol(0,255,0));

	// mask out the areas outside the ship, so that the shield only covers
	// the ship.
	blit_singlecolor(ship_bmp, shield_bmp[sprite_index], tw_makecol(0,0,0));

	destroy_bitmap(ship_bmp);

	// ok ! this is what we need - only things left are
	// resize
	// rotate
	// trans-draw.
	// which we've to do repeatedly.

	// we may also cache the rotated images !

	// flashes can occur within the edges of the shield ... check where the
	// edges are !! (assuming here, it's a closed shape).

	edge_left = new int [hship];
	edge_right = new int [hship];

	for ( int j = 0; j < hship; ++j ) {
		edge_left[j] = -1;
		edge_right[j] = -1;
		for ( int i = 0; i < wship; ++i ) {
			int color = getpixel(shield_bmp[0], i, j);

			if ( color != 0 ) {
				if ( edge_left[j] == -1 )
					edge_left[j] = i;

				edge_right[j] = i;
			}

		}
	}

	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void KaboHaze::animate(Frame *space)
{
	STACKTRACE;
	// IMPORTANT: physics must not be changed, that is, NON-LOCAL variable must not
	// be changed in this subroutine !
	// Reason: the TimeWarp engine can decide to skip animations in case of low
	// frame-rate, thus, leading to a desynch between computers!

	/*

	  Aaaaah, lots of work to do for a simple job, namely, create a transparent overlay
	  of the shield onto the host:

	  - Init (see constructor):
	  Take the host-ship picture.
	  Create a (blurred) mask image of it.
	  Draw the shield and
	  Overlay the mask.

	  - Animate:
	  Scale and draw the masked shield,
	  Rotate and
	  Draw transparent onto the screen.

	*/

	if (!host)
		return;

	//if ( state == 0 )
	//	return;

	// the host can die in-between calculate and animate, therefore I use this; it's
	// not allowed to change state of this presence though; that's done only in
	// calculate().

	// Create a rotated copy of the shield sprite ... but only, if such a thing
	// does not exist, yet ! The purpose of this is, to spread the amount of
	// calculations over different frames, and to limit them to when they're
	// needed.

	int wshield = shield_bmp[0]->w;
	int hshield = shield_bmp[0]->h;

	if ( !shield_bmp[shield_sprite_index] ) {
		shield_bmp[shield_sprite_index] = create_bitmap(wshield, hshield);
								 // important otherwise it contains artefacts
		clear_to_color(shield_bmp[shield_sprite_index], 0);
		rotate_sprite(shield_bmp[shield_sprite_index], shield_bmp[0], 0, 0, iround((1<<24)*(host->angle + 0.5*PI)/PI2) );
		// result is in sprite_bmp[sprite_index]   ( nice conventions, huh !)
	}

	//sprite_index = 0;	// also needed - for collision detection??
	// note, I've turned the collision of, since collide_flag_anyone = 0, but
	// otherwise, the collision detector would use this sprite_index to access
	// a ship sprite that doesnt exist !!

	// next, animate ...

	// first, reserve space for the target image, but ... how big should it be?
	// well, as big as the ship_bmp, but then, zoomed in space:

	int wfinal = int(wshield * space_zoom);
	int hfinal = int(hshield * space_zoom);

	BITMAP *final_bmp;
	final_bmp = create_bitmap(wfinal, hfinal);

	// scale/draw a shield:

	stretch_blit(shield_bmp[shield_sprite_index], final_bmp, 0, 0, wshield, hshield, 0, 0, wfinal, hfinal );
	// result is in final_bmp

	// I need to calculate screen coordinates (using the original bmp size).
	//	double xhost = host->normal_pos().x;
	//	double yhost = host->normal_pos().y;
	Vector2 Vcorner;
	Vcorner = corner(host->normal_pos(), Vector2(wshield, hshield) );

								 //wcorner(xhost, wshield);
	int xplot = iround(Vcorner.x);
								 //hcorner(yhost, hshield);
	int yplot = iround(Vcorner.y);
	// these routines are the standard way to calculate screen coordinates !

								 // local
	double power_scaled = power / 10.0;
	if ( power_scaled > 1.0 )
		power_scaled = 1.0;		 // max brightness.

	// change the brightness of the shield:
	int brightness = int(255 * power_scaled);

	set_add_blender(0, 0, 0, brightness);

	draw_trans_sprite(space->surface, final_bmp, xplot, yplot);
	space->add_box(xplot, yplot, wshield, hshield);

	// also, draw a (few) flashes, at twice the brightness:

	brightness = 255;

	for ( int i = 0; i < int(power); ++i ) {
		int dx, dy;

		dx = wshield;
		dy = hshield;

		int icheck = 0;
		for (;;) {
			++icheck;
			if (icheck > 100)
				break;			 // too bad !!

								 //graphics
			dy = rand() % hshield;
			if (edge_left[dy] == -1)
				continue;

			if ( !(rand() % 2) ) //graphics
				dx = edge_left[dy];
			else
				dx = edge_right[dy];
		}

		dx -= wshield / 2;
		dy -= hshield / 2;

		double a = host->angle + 0.5*PI;
								 // rotated around the center
		int dx2 = iround(wshield/2 + dx * cos(a) - dy * sin(a));
		int dy2 = iround(hshield/2 + dy * cos(a) + dx * sin(a));

		dx = iround( dx2 * space_zoom);
		dy = iround( dy2 * space_zoom);

		int x = xplot + dx;
		int y = yplot + dy;

		tw_putpixel(space->surface, x, y, tw_makecol(brightness,brightness,0) );

		space->add_pixel(x, y);

	}

	// release the temporary bitmap:
	destroy_bitmap(final_bmp);
}


KaboPod::KaboPod(Vector2 opos, double oangle, double ov,
double orange, double oarmour, double odamage,
KaboWeakener *oship, SpaceSprite *osprite)
:
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite)
{
	STACKTRACE;
	mother = oship;
	ididdamage = 0;
	spriteindextime = 0;
	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void KaboPod::calculate()
{
	STACKTRACE;
	if (! (mother && mother->exists()) ) {
		mother = 0;
		state = 0;
		return;
	}

	Missile::calculate();

	// preliminary destruction, if special is released:
	if (!mother->fire_special)
		state = 0;

	if (!state)
		mother->pod = 0;

	if (!state && !ididdamage) {
		// release several spores, in random directions

		int     i;
		double  a;

		for ( i = 0; i < mother->podNmines; ++i ) {
			a = tw_random(PI2);

			game->add( new KaboMine(Vector2(0.0, 40.0),
				a, mother->mineVelocity, mother->mineTurnRate,
				mother->mineRange, mother->mineArmour, mother->mineLifeTime,
				mother->mineHostileTime,
				mother->Mine_RangeAcc, mother->Haze_basepower,
				this, mother->data->spriteExtra));
		}

	}

	// make the sprite/mines rotate around their axes
	spriteindextime += frame_time / 25.0;
	sprite_index = (int)(spriteindextime);
	sprite_index &= 63;

	ididdamage = 0;
}


void KaboPod::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	Missile::inflict_damage(other);
	ididdamage = 1;

	state = 0;
	if ( mother && mother->exists() )
		mother->pod = 0;
}


REGISTER_SHIP(KaboWeakener)
