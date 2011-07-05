/* $Id: shpqlore.cpp,v 1.12 2004/03/26 17:55:49 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

class QlonRedeemer : public Ship
{
	int    weaponColor;
	double weaponRange;
	int    weaponDamage;
	int    weaponFrames;

	double specialRange;
	double specialVelocity;
	double specialSlowdown;
	int    specialArmour;

	public:
		QlonRedeemer(Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual int activate_special();
		virtual void animate(Frame *space);
};

class QlonLimpet : public AnimatedShot
{
	double slowdown_factor;

	public:
		QlonLimpet(Vector2 opos, double ov, double slowdown, double orange,
			int oarmour, Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};
QlonRedeemer::QlonRedeemer(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)

{
	STACKTRACE;
	weaponColor  = get_config_int("Weapon", "Color", 0);
	weaponRange  = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage = get_config_int("Weapon", "Damage", 0);
	weaponFrames = get_config_int("Weapon", "Frames", 0);

	specialRange    = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialSlowdown = get_config_float("Special", "Slowdown", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);
}


int QlonRedeemer::activate_weapon()
{
	STACKTRACE;
	int fire = FALSE;
	SpaceObject *o;

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
	bit(LAYER_CBODIES), weaponRange); a.current; a.next()) {
		o = a.currento;
		if ( (!o->isInvisible()) && !o->sameTeam(this) && (o->collide_flag_anyone & bit(LAYER_LINES))) {
			SpaceLocation *l = new PointLaser(this, pallete_color[weaponColor], 1,
				weaponFrames, this, o, Vector2(0.0, 10.0));
			add(l);
			if (l->exists()) fire = TRUE;
		}
	}
	if (fire) play_sound((SAMPLE *)(melee[MELEE_BOOM + 0].dat));
	return(fire);
}


int QlonRedeemer::activate_special()
{
	STACKTRACE;
	add(new QlonLimpet(Vector2(0, get_size().y / 1.0),
		specialVelocity, specialSlowdown, specialRange, specialArmour, this,
		data->spriteSpecial, 100, 5));
	return(TRUE);
}


QlonLimpet::QlonLimpet(Vector2 opos, double ov, double slowdown,
double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
int ofsize, int ofcount) :
AnimatedShot(oship, opos, 0.0, ov, 0, orange, oarmour, oship, osprite,
ofcount, ofsize),
slowdown_factor(slowdown)
{
	STACKTRACE;
	if ((ship->target) && (!ship->target->isInvisible()))
		angle = ship->get_angle() +0;
	else
		angle = ship->get_angle() -0;

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
	}
}


void QlonLimpet::calculate()
{
	STACKTRACE;
	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	if (ship && (ship->target) && (!ship->target->isInvisible())) {
		angle = ship->get_angle() -0;
		//		vx = v * cos(angle );
		//		vy = v * sin(angle );
		vel = v * unit_vector(angle );
	}
	AnimatedShot::calculate();
}


void QlonLimpet::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!other->isShip()) {
		if (other->damage_factor || other->mass) state = 0;
		return;
	}

	Ship *target = (Ship *) other;

	play_sound(data->sampleSpecial[1]);

	//MYCODE begin
	int tries, col;
	Vector2 H;

	if ( target->spritePanel != 0 ) {
		BITMAP *bmp = target->spritePanel->get_bitmap(0);
		//find a random spot on the target ship where it "exist"
		tries = 0;
		while (tries < 30) {
								 //graphics
			H.x = 18 + (rand() % 27);
								 //graphics
			H.y = 15 + (rand() % 36);
			tries++;
			col = getpixel(bmp,iround(H.x),iround(H.y));
			if (col == bitmap_mask_color(bmp)) continue;
			if (col == 0) continue;
			if (col == -1) continue;
			if (col == palette_color[8]) continue;
			break;
		}

		// draw the Limplet on the ship panel
		//	BITMAP *s = sprite->get_bitmap(sprite_index);
		//	stretch_sprite(bmp, s, hx - s->w/4, hy - s->h/4,	s->w/2, s->h/2 );
		sprite->draw(H - sprite->size()/4,
			sprite->size()/2,
			sprite_index, bmp);
		target->update_panel = TRUE;
	}
	target->handle_speed_loss(this, slowdown_factor);
	state = 0;
}


void QlonRedeemer::animate(Frame *space)
{
	STACKTRACE;

	double back_x=get_size().x/3.60, back_y=-get_size().y/2.33,
		frnt_x=get_size().x/5.55, frnt_y=+get_size().y/17.01,
		back_y_1=-get_size().x/2.06;

	double tx = cos(angle);
	double ty = sin(angle);
	Vector2 t = unit_vector(angle + 0.5*PI);
	int s_index = get_index(angle);

	// I think the following rotation code is wrong (comment: GeomanNL)

	if (turn_right)
		data->spriteWeapon->animate(Vector2(pos.x + frnt_y * tx - frnt_x * ty,
								 //graphics
			pos.y + frnt_y * ty + frnt_x * tx), s_index + ((rand()%3) << 6), space);
	if (turn_left)
		data->spriteWeapon->animate(Vector2(pos.x + frnt_y * tx + frnt_x * ty,
			pos.y + frnt_y * ty - frnt_x * tx),
								 //graphics
			s_index + ((rand()%3) << 6), space);

	s_index += 32; s_index &= 63;

	if (thrust) {
		data->spriteExtra->animate(Vector2(pos.x + back_y_1 * tx - back_x * ty,
								 //graphics
			pos.y + back_y_1 * ty + back_x * tx), s_index + ((rand()%3) << 6), space);
		data->spriteExtra->animate(Vector2(pos.x + back_y_1 * tx + back_x * ty,
								 //graphics
			pos.y + back_y_1 * ty - back_x * tx), s_index + ((rand()%3) << 6), space);
	} else {
		if (turn_left)
			data->spriteWeapon->animate(Vector2(pos.x + back_y * tx - back_x * ty,
								 //graphics
				pos.y + back_y * ty + back_x * tx), s_index + ((rand()%3) << 6), space);
		if (turn_right)
			data->spriteWeapon->animate(Vector2(pos.x + back_y * tx + back_x * ty,
								 //graphics
				pos.y + back_y * ty - back_x * tx), s_index + ((rand()%3) << 6), space);
	}

	Ship::animate(space);
};

REGISTER_SHIP(QlonRedeemer)
