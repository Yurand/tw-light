/* $Id: shpnarlu.cpp,v 1.15 2004/03/26 17:55:49 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

#define NAROOL_POISON_ID     0x1122

#include "../util/aastr.h"
#include "../melee/mview.h"
#include "../frame.h"

#include <stdlib.h>

class NaroolPoison;

class NaroolLurker : public Ship
{
	double weaponRange;
	double weaponVelocity;
	double poison;
	int    weaponDamage;
	int    weaponArmour;
	int    weaponDuration;
	int    normalRechargeAmount;
	int    cloak;
	int    cloak_frame;

	public:

		static int cloak_color[3];

		NaroolLurker(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		BITMAP *lightningbmp;
		~NaroolLurker();

		virtual double isInvisible() const;
		virtual int activate_weapon();
		virtual void calculate_fire_special();
		virtual void calculate_hotspots();
		virtual void calculate();
		virtual void animate(Frame *space);

		void calc_lightning();
		double maxsparktime, sparktime, Rmax;
		Vector2 sparkpos;
		virtual void inflict_damage(SpaceObject *other);

};
class NaroolGas: public AnimatedShot
{

	int hitShip;
	int duration;
	float poison;

	public:

		NaroolGas(double ox,double oy,double oangle,double ov, int odamage, int duration,
			double orange, int oarmour, float poison, NaroolLurker *oship, SpaceSprite *osprite, double relativity = 0);

		virtual void inflict_damage(SpaceObject *other);
		virtual void soundExplosion();
		virtual void animateExplosion();
};

class NaroolPoison : public SpaceObject
{
	public:
		Ship *oship;
		double poison;
		int duration;
		int start;

		NaroolPoison(NaroolGas *gas, int duration, float poison, Ship *kill_it, SpaceSprite *osprite);
		virtual void calculate();
		virtual void animate(Frame *space);
};

int NaroolLurker::cloak_color[3] = { 15, 10, 2 };

NaroolLurker::NaroolLurker(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponDuration = get_config_int("Weapon", "Duration", 0);
	poison         = get_config_float("Weapon", "poison", 0);

	normalRechargeAmount = recharge_amount;

	cloak = FALSE;
	cloak_frame = 0;

	BITMAP *shpbmp = sprite->get_bitmap(0);
	int bpp = bitmap_color_depth(shpbmp);
	lightningbmp = create_bitmap_ex(bpp, shpbmp->w, shpbmp->h);
	clear_to_color(lightningbmp, makeacol(0,0,0,255));
	//maxsparktime = 2000;
	maxsparktime = get_config_float("Quirk", "maxsparktime", 2000);
	sparktime = maxsparktime;
	sparkpos = 0.5 * Vector2(lightningbmp->w,lightningbmp->h);

	Rmax = get_config_float("Quirk", "Rmax", 1);
}


NaroolLurker::~NaroolLurker()
{
	if (lightningbmp)
		destroy_bitmap(lightningbmp);
}


double NaroolLurker::isInvisible() const
{
	if (cloak_frame >= 300
		&& sparktime <= 0) return(1);
	return 0;
}


int NaroolLurker::activate_weapon()
{
	STACKTRACE;
	double a, relativity = game->shot_relativity;
	if (target) {
		a = intercept_angle2(pos, vel * relativity, weaponVelocity,
			target->normal_pos(), target->get_vel());
	}
	else a = angle;
	add(new NaroolGas(0.0, 0.0,
		a , weaponVelocity, weaponDamage, weaponDuration, weaponRange, weaponArmour,
		poison, this, data->spriteWeapon, relativity));
	return(TRUE);
}


void NaroolLurker::calculate_fire_special()
{
	STACKTRACE;
	special_low = FALSE;

	if (fire_special) {
		if ((batt < special_drain) && (!cloak)) {
			special_low = TRUE;
			return;
		}

		if (special_recharge > 0)
			return;

		if (cloak) {
			cloak = FALSE;

			play_sound2(data->sampleSpecial[1]);
			recharge_amount = normalRechargeAmount;
		} else {
			cloak = TRUE;
			play_sound2(data->sampleSpecial[0]);
			//batt -= special_drain;
			//recharge_amount = 1;
			recharge_amount = 0;
		}

		special_recharge = special_rate;
	}
}


void NaroolLurker::calculate_hotspots()
{
	STACKTRACE;
	if (!cloak)
		Ship::calculate_hotspots();
}


void NaroolLurker::calculate()
{
	STACKTRACE;
	if ((cloak) && (cloak_frame < 300))
		cloak_frame+= frame_time;
	if ((!cloak) && (cloak_frame > 0))
		cloak_frame-= frame_time;

	Ship::calculate();

	if (sparktime > 0)
		sparktime -= frame_time;
	else
		sparktime = 0;
}


void NaroolLurker::animate(Frame *space)
{
	STACKTRACE;
	if ((cloak_frame > 0) && (cloak_frame < 300))
		sprite->animate_character( pos,
			sprite_index, pallete_color[cloak_color[(int)(cloak_frame / 100)]], space);
	else
	if ((cloak_frame >= 300))
		sprite->animate_character( pos,
				sprite_index, pallete_color[0], space);
	else
		Ship::animate(space);

	//Vector2 lightningrelpos = 0.5 * Vector2(lightningbmp->w,lightningbmp->h);
	calc_lightning();

	//	aa_set_mode(find_aa_mode(general_attributes));

	Vector2 P, S;
	S = sprite->size(0) * ::space_zoom;
	P = corner(pos, sprite->size(0));

	int ix, iy, iw, ih;
	// target position
	ix = iround(P.x);
	iy = iround(P.y);
	// target size
	iw = iround(S.x);
	ih = iround(S.y);

	//int a;
	//a = aa_get_trans();
	//aa_set_trans(128);

	int a;
	a = aa_get_mode();
	aa_set_mode(a | AA_ALPHA);
	aa_stretch_blit(lightningbmp, space->surface,
		0, 0,lightningbmp->w,lightningbmp->h,
		ix, iy, iw, ih);
	aa_set_mode(a);

	space->add_box(ix, iy, iw, ih);
}


NaroolGas::NaroolGas(double ox, double oy,double oangle,double ov,int odamage, int oduration,
double orange,int oarmour, float poison, NaroolLurker *oship, SpaceSprite *osprite, double relativity) :
AnimatedShot(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship,
osprite, 50, 21, relativity)

{
	STACKTRACE;
	duration = oduration;
	hitShip = FALSE;
	this->poison = poison;
	collide_flag_anyone = bit(LAYER_SHIPS) | bit(LAYER_CBODIES);
}


void NaroolGas::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	play_sound2(data->sampleExtra[1]);

	if (other->isShip()) {
		add( new NaroolPoison(this, duration, poison, (Ship*)other, data->spriteWeapon));
		add(new FixedAnimation(this, other, data->spriteWeaponExplosion, 0, 8, 150, LAYER_EXPLOSIONS));
	} else {
		add(new Animation(this, pos, data->spriteWeaponExplosion, 0, 8, 150, LAYER_EXPLOSIONS));
	}
	AnimatedShot::inflict_damage(other);
	return;
}


void NaroolGas::animateExplosion()
{
	STACKTRACE;
	return;
}


void NaroolGas::soundExplosion()
{
	STACKTRACE;
	if (!hitShip) {
		play_sound2(data->sampleExtra[1]);
	}
	return;
}


NaroolPoison::NaroolPoison(NaroolGas *gas, int nduration, float poison, Ship *nship, SpaceSprite *osprite) :
SpaceObject (gas, nship->normal_pos(), 0.0, osprite),
oship(nship),
poison(poison),
duration(nduration)
{
	STACKTRACE;
	target = oship;
	id |= NAROOL_POISON_ID;
	layer = LAYER_HOTSPOTS;
	start = TRUE;
	collide_flag_anyone = 0;
	Query q;
	for (q.begin(oship, bit(LAYER_HOTSPOTS), 10); q.current; q.next()) {
		if ((q.current->getID() == getID()) && (((NaroolPoison*)q.current)->oship == oship)) {
			((NaroolPoison*)q.current)->duration = duration;
			state = 0;
		}
	}
	q.end();
}


void NaroolPoison::calculate()
{
	STACKTRACE;
	int chance;
	SpaceObject::calculate();

	pos = oship->normal_pos();
	vel = oship->get_vel();
	duration -= frame_time;
	if (duration < 0) {
		if (oship->spritePanel) {
			blit(oship->data->spritePanel->get_bitmap(0), oship->spritePanel->get_bitmap(0),
				16 , 18, 16, 18, 32, 30);
			oship->update_panel = TRUE;
		}
		state = 0;
		return;
	}
	if (!(oship && oship->exists())) {
		oship = 0;
		state = 0;
	} else {
		if (start) {
			start = FALSE;
			if (oship->spritePanel) {
				data->spriteExtra->draw(16, 18, 0, oship->spritePanel->get_bitmap(0));
				//			draw_sprite(oship->spritePanel->get_bitmap(0), data->spriteExtra->get_bitmap(0),16,18);
				oship->update_panel = TRUE;
			}
		}
		chance = iround(random(1000.0));
		if (chance < frame_time * poison) {
			play_sound(data->sampleExtra[0]);
			damage(oship, 0, 1);
		}
	}
	return;
}


void NaroolPoison::animate(Frame *space)
{
	STACKTRACE;
	return;
}


void NaroolLurker::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	Ship::inflict_damage(other);

	//you've hit something; activate sparks.
	sparktime = maxsparktime;

	// but, where did you hit it ?
	// place the source somewhere... at the edge .. how ?
	double a, R;
	a = trajectory_angle(other);
	R = 40;
	sparkpos = R * unit_vector(a);
	sparkpos += 0.5 * Vector2(lightningbmp->w,lightningbmp->h);
}


void NaroolLurker::calc_lightning()
{
	STACKTRACE;

	Vector2 P;
	P = sparkpos;

	//return;
	//clear_to_color(lightningbmp, makeacol(0,0,0,255));
	//circlefill(lightningbmp, P.x, P.y, 10, makeacol(255,0,0,200));

	// first, let the image grow fainter
	BITMAP *b;
	b = lightningbmp;
	int ix, iy;
	int iw, ih;
	iw = b->w;
	ih = b->h;

	BITMAP *shpbmp;
	shpbmp = sprite->get_bitmap(sprite_index);

	// assume it's a 32 bit image ...
	unsigned char *a;

	for ( iy = 0; iy < ih; ++iy ) {
		a = (unsigned char*) b->line[iy];
		a += 0;					 // red = the 3rd color (in my case).

		for ( ix = 0; ix < iw; ++ix ) {
			// reduce colors
			int i;
			for ( i = 0; i < 3; ++i ) {
				if (*a > 5)
					(*a) -= 5;
				else
					(*a) = 0;

				++a;
			}

			++a;
		}
	}

	if (sparktime > 0) {
		// create some kind of lightning now ? How ?
		int i, N;

		N = 5 + rand() % 5;		 //graphics

		double ang, R;
								 //graphics
		ang = (rand() % 360) * PI/180;

		for ( i = 0; i < N; ++i ) {
			double dx, dy;
			//dx = (random(double(iw)) - 0.5*iw) / N;
			//dy = (random(double(ih)) - 0.5*ih) / N;

								 //graphics
			ang += (rand() % 180 - 90) * PI/180;
								 //graphics
			R = (rand() % int(Rmax+1)) / N;
			dx = R * cos(ang);
			dy = R * sin(ang);

			int j, M;
			M = 10;
			for ( j = 0; j < M; ++j ) {
				P.x += dx / M;
				P.y += dy / M;

				int col;
				int re, gr, bl;

				col = getpixel(shpbmp, iround(P.x), iround(P.y));

				re = getr(col);
				gr = getg(col);
				bl = getb(col);

				if ( !(re == 255 && gr == 0 && bl == 255)) {
					int k;
					k = iround(128*(0.5 + 0.5*sparktime/maxsparktime));

					//	re = 0;
					//	bl = 0;
					//	if ( k > 96)
					//		re = k + 127 / (i+1);
					//	else
					//		bl = k + 127 / (i+1);

					double c, f;
					//f = (0.5 + (0.5*sparktime)/maxsparktime);
					f = sparktime / maxsparktime;
					c = 255;	 // / (i+1);
					re = iround(f * c);
					bl = iround((1-f) * c);

					putpixel(b, iround(P.x), iround(P.y), makeacol(re,0,bl,255));
				}
			}

		}
	}

}


REGISTER_SHIP(NaroolLurker)
