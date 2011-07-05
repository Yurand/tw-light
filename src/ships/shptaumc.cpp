/* $Id: shptaumc.cpp,v 1.13 2006/03/16 22:30:32 geomannl Exp $ */
#include "../ship.h"
#include "../melee/mview.h"
#include "../frame.h"
#include "../util/aastr.h"
#include "../melee.h"
REGISTER_FILE

#define blinker_rate_idle 1000
#define blinker_rate_aim 500
#define blinker_dark_rate 0.55

class TauMC : public Ship
{
	public:
		double        weaponRange;
		double        weaponVelocity;
		double        weaponTurnRate;
		int           weaponDamage, weaponBlastDamage;
		int           weaponArmour;
		int           weaponRechargeRate;
		double        weaponBlastRange;

		bool          torpedo_ready[2];
		int           torpedo_recharge[2];
		int           current_torpedo;

		int           lock_ticks;
		int           lock_count;
		int           blinker_recharge, light_recharge;
		int           blinker_rate;
		int           current_blinker;
		double        lock_angle;

		double        danger_range;

		double        specialRange;
		double        specialVelocity;
		double        specialTurnRate;
		int           specialDamage;
		int           specialArmour;
		int           specialRechargeRate;
		int           special_recharge_count;
		double        specialTrackAngle;
		double        weight_ship, weight_shot;
		int           ammo;
		int           current_barrel;

		double        turretTurnRate;
		double        turret_angle;

		SpaceObject  *old_target;

	public:
		TauMC(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual   int activate_weapon();
		virtual   int activate_special();
		virtual  void animate(Frame *space);
		virtual  void calculate_turn_left();
		virtual  void calculate_turn_right();
		virtual  void calculate();
		virtual  void calculate_hotspots();
		virtual   Color battPanelColor(int k = 0);
		//virtual  void handle_damage(SpaceLocation *source);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

class TauMCMissile : public HomingMissile
{
	public:
		double track_angle, w_ship, w_shot;

	public:
		TauMCMissile (Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, double otrate, SpaceLocation *creator,
			SpaceSprite *osprite, int side, double ta, double wsp, double wst);
		virtual  void calculate();
		double get_aim(SpaceObject *tgt);
};

class TauMCTorpedo : public HomingMissile
{
	public:
		double blast_range;
		int    blast_damage;
		double old_range;
		bool exploded;

	public:
		TauMCTorpedo (Vector2 opos, double oangle, double ov, int odamage, int oblast_damage,
			double orange, int oarmour, double otrate, SpaceLocation *creator,
			SpaceSprite *osprite, SpaceObject *otarget, double oblast_range);
		virtual  void calculate();
		virtual  void animateExplosion();
		virtual  void soundExplosion();
};

TauMC::TauMC(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;
	weaponRange     = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity  = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponTurnRate  = scale_turning(tw_get_config_float("Weapon", "TurnRate", 0));
	weaponDamage    = tw_get_config_int("Weapon", "Damage", 0);
	weaponBlastDamage = tw_get_config_int("Weapon", "BlastDamage", 0);
	weaponBlastRange= tw_get_config_float("Weapon", "BlastRange", 1);
	weaponArmour    = tw_get_config_int("Weapon", "Armour", 0);
	weaponRechargeRate = scale_frames(tw_get_config_float("Weapon", "Rate", 0));
	lock_angle      = tw_get_config_float("Weapon", "LockAngle", 30.0);
	lock_ticks      = tw_get_config_int("Weapon", "LockCount", 1)+1;

	specialRange    = scale_range(tw_get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(tw_get_config_float("Special", "Velocity", 0));
	specialTurnRate = scale_turning(tw_get_config_float("Special", "TurnRate", 0));
	specialDamage   = tw_get_config_int("Special", "Damage", 0);
	specialArmour   = tw_get_config_int("Special", "Armour", 0);
	specialRechargeRate = scale_frames(tw_get_config_float("Special", "Rate", 0));
	specialTrackAngle = tw_get_config_float("Special", "TrackAngle", 90);
	weight_ship     = tw_get_config_float("Special", "Ship", 0);
	weight_shot     = tw_get_config_float("Special", "Shot", 0);

	turretTurnRate = scale_turning(tw_get_config_float("Extra", "TurnRate", 0));
	turret_angle = 0;

	special_recharge_count = specialRechargeRate;

	ammo = 4;
	current_barrel = 0;

	blinker_rate = blinker_rate_idle;
	blinker_recharge = blinker_rate;
	lock_count = 0;
	current_blinker = -1;

	current_torpedo = 0;
	torpedo_ready[0] = true;
	torpedo_ready[1] = true;
}


int TauMC::activate_weapon()
{
	STACKTRACE;
	if ((fire_special) || (!torpedo_ready[current_torpedo]) || (lock_count < lock_ticks))
		return false;
	add(new TauMCTorpedo(Vector2(20*(2*current_torpedo-1), 25), angle,
		weaponVelocity, weaponDamage, weaponBlastDamage,
		weaponRange, weaponArmour, weaponTurnRate, this,
		data->spriteWeapon, target, weaponBlastRange));
	torpedo_ready[current_torpedo] = false;
	torpedo_recharge[current_torpedo] = weaponRechargeRate;
	current_torpedo = (current_torpedo + 1) % 2;
	return true;
}


int TauMC::activate_special()
{
	STACKTRACE;
	if ((!fire_weapon) || (ammo == 0))
		return false;

	double oa = angle;
	//angle = (int(normalize(angle + turret_angle, 360)/5.625))*5.625;
	angle = normalize(angle + turret_angle, PI2);
	add(new TauMCMissile(Vector2((current_barrel - 1.5)*4.6, 23), angle,
		specialVelocity, specialDamage, specialRange, specialArmour,
		specialTurnRate, this, data->spriteSpecial, 0, specialTrackAngle, weight_ship, weight_shot));
	add(new TauMCMissile(Vector2(((current_barrel + 2)%4 - 1.5)*4.6, 23), angle,
		specialVelocity, specialDamage, specialRange, specialArmour,
		specialTurnRate, this, data->spriteSpecial, 1, specialTrackAngle, weight_ship, weight_shot));
	angle = oa;
	ammo--;
	current_barrel = (current_barrel + 1) % 4;

	return true;
}


void TauMC::animate(Frame *space)
{
	STACKTRACE;

	double ra;
	int turret_index;
	BITMAP *bmp;
	ra = normalize(turret_angle + angle, PI2);
	turret_index = get_index(ra);//((int)(ra / 5.625) + 16) & 63;
	//ra *= ANGLE_RATIO;
	bmp = data->spriteShip->get_bitmap(64);
	clear_to_color(bmp, tw_makecol(255,0,255));
	sprite->draw(0, 0, sprite_index, bmp);
	data->spriteExtra->draw(31, 31, turret_index, bmp);
	double tx, ty;

	int j, k = ammo;
	ra = angle;					 // * ANGLE_RATIO;
	tx = cos(ra);
	ty = sin(ra);

	if (torpedo_ready[0])
		data->spriteExtraExplosion->draw(iround(44.5 + 16*tx + 20*ty), iround(44.5 + 16*ty - 20*tx), 1, bmp);
	else    data->spriteExtraExplosion->draw(iround(44.5 + 16*tx + 20*ty), iround(44.5 + 16*ty - 20*tx), 0, bmp);
	if (torpedo_ready[1])
		data->spriteExtraExplosion->draw(iround(44.5 + 16*tx - 20*ty), iround(44.5 + 16*ty + 20*tx), 1, bmp);
	else    data->spriteExtraExplosion->draw(iround(44.5 + 16*tx - 20*ty), iround(44.5 + 16*ty + 20*tx), 0, bmp);

	int l_i = 1;

	if (lock_count == 0)
		data->spriteExtraExplosion->draw(iround(44.5 + 38*tx - current_blinker*12*ty), iround(44.5 + 38*ty + current_blinker*12*tx), l_i, bmp);
	else
	if (((lock_count < lock_ticks) && (blinker_recharge > blinker_rate*blinker_dark_rate))
	|| (lock_count >= lock_ticks)) {
		if (target && (lock_count >= lock_ticks)) if (distance(target) < weaponBlastRange * 2) l_i = 0;
		data->spriteExtraExplosion->draw(iround(44.5 + 38*tx - 12*ty), iround(44.5 + 38*ty + 12*tx), l_i, bmp);
		data->spriteExtraExplosion->draw(iround(44.5 + 38*tx + 12*ty), iround(44.5 + 38*ty - 12*tx), l_i, bmp);
	}

	for (int i=0; i<4; i++) {
		j = (current_barrel + i) % 4;
		switch (j) {
			case 0: if (k > 0) data->spriteSpecial->draw(38, 38, turret_index+64*1+64, bmp);
			else       data->spriteSpecial->draw(38, 38, turret_index+64*0+64, bmp);
			break;
			case 1: if (k > 0) data->spriteSpecial->draw(38, 38, turret_index+64*3+64, bmp);
			else       data->spriteSpecial->draw(38, 38, turret_index+64*2+64, bmp);
			break;
			case 2: if (k > 0) data->spriteSpecial->draw(38, 38, (turret_index+32)%64+64*3+64, bmp);
			else       data->spriteSpecial->draw(38, 38, (turret_index+32)%64+64*2+64, bmp);
			break;
			case 3: if (k > 0) data->spriteSpecial->draw(38, 38, (turret_index+32)%64+64*1+64, bmp);
			else       data->spriteSpecial->draw(38, 38, (turret_index+32)%64+64*0+64, bmp);
		}
		k--;
	}

	animate_bmp(bmp, pos, space);
}


void TauMC::calculate_turn_left()
{
	STACKTRACE;
	if (turn_left) {
		if (fire_special)
			turret_angle -= turretTurnRate * frame_time;
		else    turn_step -= turn_rate * frame_time;
	}
}


void TauMC::calculate_turn_right()
{
	STACKTRACE;
	if (turn_right) {
		if (fire_special)
			turret_angle += turretTurnRate * frame_time;
		else    turn_step += turn_rate * frame_time;
	}
}


void TauMC::calculate()
{
	STACKTRACE;
	Ship::calculate();

	if (target?(!target->isInvisible()):false) {
		double d_a = normalize(trajectory_angle(target) - angle, 360);
		if (d_a > 180) d_a -= 360;
		d_a = fabs(d_a);
		if ((d_a < lock_angle) && (distance(target) < weaponRange) && (target == old_target)) {
			if (lock_count == 0) {
				blinker_rate = blinker_rate_aim;
				blinker_recharge = 0;
				lock_count = 1;
			}
		}
		else    if (lock_count > 0) {
			blinker_rate = blinker_rate_idle;
			blinker_recharge = 0;
			lock_count = 0;
		}
	}
	else    if (lock_count > 0) {
		blinker_rate = blinker_rate_idle;
		blinker_recharge = 0;
		lock_count = 0;
	}
	old_target = target;

	if (blinker_recharge > 0)
		blinker_recharge -= frame_time;
	else {
		blinker_recharge += blinker_rate;
		current_blinker *= -1;
		if ((lock_count > 0) && (lock_count <= lock_ticks))
			lock_count++;
	}

	for (int i=0; i<2; i++)
	if (!torpedo_ready[i]) {
		if (torpedo_recharge[i] > 0) torpedo_recharge[i] -= frame_time;
		else    torpedo_ready[i] = true;
	}

	if (ammo < 4) {
		if (special_recharge_count > 0)
			special_recharge_count -= frame_time;
		else {
			special_recharge_count = specialRechargeRate;
			ammo++;
		}
	}
}


void TauMC::calculate_hotspots()
{
	STACKTRACE;
	if ((thrust) && (hotspot_frame <= 0)) {
		add(new Animation(this,
								 //normal_x() - (cos(angle * ANGLE_RATIO) * w / 2.0),
								 //normal_y() - (sin(angle * ANGLE_RATIO) * h / 2.0),
			normal_pos() - 0.5 * sprite->size().x * unit_vector(angle),
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, LAYER_HOTSPOTS));
		hotspot_frame += hotspot_rate;
	}
	if (hotspot_frame > 0) hotspot_frame -= frame_time;
}


Color TauMC::battPanelColor(int k)
{
	STACKTRACE;
	//     return tw_makecol(50, 50, 180);
	Color c = {50, 50, 180};
	return c;
}


int TauMC::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	double d;

	d = normal + direct;

	batt -= d;
	if (batt < 0) {
		d = -batt;
		batt = 0;
	}
	else
		d = 0;

	Ship::handle_damage(source, d, 0);

	return 0;
}


TauMCMissile::TauMCMissile(Vector2 opos, double oangle, double ov, int odamage,
double orange, int oarmour, double otrate, SpaceLocation *creator,
SpaceSprite *osprite, int side, double ta, double wsp, double wst) :
HomingMissile(creator, opos, oangle, ov, odamage, orange, oarmour, otrate, creator, osprite, NULL)
{
	STACKTRACE;
	if (side == 0) {
		layer = LAYER_EXPLOSIONS;
		//                depth = 65535*layer;
	}

	track_angle = ta;
	w_ship = wsp;
	w_shot = wst;

	explosionSprite     = data->spriteSpecialExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
}


double TauMCMissile::get_aim(SpaceObject *tgt)
{
	STACKTRACE;
	Vector2 tv = tgt->get_vel();
	//double tvy = tgt->get_vy();
	//double rx  = min_delta(tgt->normal_x(), normal_x(), X_MAX);
	//double ry  = min_delta(tgt->normal_y(), normal_y(), Y_MAX);
	Vector2 r = min_delta(tgt->normal_pos(), normal_pos());
								 //rx*rx + ry*ry;
	double r2  = magnitude_sqr(r);
	double u2  = v * v;
								 //(tvx*tvx + tvy*tvy);
	double d2v = u2 - magnitude_sqr(tv);
	double t = r.dot(tv);		 //(rx*tvx + ry*tvy);
	double q, p;
	if (fabs(d2v/u2) > 0.01 ) {
		q = t*t + r2*d2v;
		if (q > 0) q = sqrt(q);
		else    return (1e10);
		p = (t+q)/d2v;
		q = (t-q)/d2v;
		if (p > 0) t = p;
		else       t = q;
		if (t < 0) return (1e10);
	} else {
		if (fabs(t)<1e-6) return (1e10);
		else    t = - 0.5 * r2 / t;
		if (t < 0) return (1e10);
	}
	t = normalize(atan3(tv.y*t + r.y, tv.x*t + r.x) - angle, PI2);
	if (t > PI) t -= PI2;
	return (t);
}


void TauMCMissile::calculate()
{
	STACKTRACE;
	Missile::calculate();
	if (state == 0) return;
	if (target)
		if ((!target->exists()) || (fabs(get_aim(target)) > track_angle))
			target = NULL;

	if (!target) {
		Query q;
		double a_a, a0 = -1e20;
		for (q.begin(this, OBJECT_LAYERS, range - d); q.currento; q.next()) {
			if (!q.current->isObject())
				continue;
			if ( (!q.currento->sameTeam(this)) && (q.currento->collide_flag_anyone&bit(layer))
			&& !q.currento->isPlanet() ) {
				a_a = fabs(get_aim(q.currento));
				if (a_a < 0) continue;
				if (a_a < track_angle) {
					a_a = (track_angle - a_a) * ((range - d) - distance(q.currento)) *
						(1 + (q.currento->isShip()?w_ship:0) + (q.currento->isShot()?w_shot:0) );
					if (a_a > a0) {
						target = q.currento;
						a0 = a_a;
					}
				}
			}
		}
	}

	if (target && !target->isInvisible()) {
		double d_a = get_aim(target);
		double ta = turn_rate * frame_time;
		if (fabs(d_a) < ta) ta = fabs(d_a);
		if (d_a > 0) turn_step += ta;
		else turn_step -= ta;
		while(fabs(turn_step) > 5.625/2) {
			if (turn_step < 0.0) {
				angle -= 5.625;
				turn_step += 5.625;
			}
			else if (turn_step > 0.0) {
				angle += 5.625;
				turn_step -= 5.625;
			}
		}
		angle = normalize(angle, 360);
	}

	// ?
	//	sprite_index = (int)(angle / 5.625) + 16;
	//	sprite_index &= 63;
	sprite_index = get_index(angle);

	//vx = v * cos(angle * ANGLE_RATIO);
	//vy = v * sin(angle * ANGLE_RATIO);
	vel = v * unit_vector(angle);
}


TauMCTorpedo::TauMCTorpedo (Vector2 opos, double oangle, double ov, int odamage, int oblast_damage,
double orange, int oarmour, double otrate, SpaceLocation *creator,
SpaceSprite *osprite, SpaceObject *otarget, double oblast_range)
:
HomingMissile(creator, opos, oangle, ov, odamage, orange, oarmour, otrate, creator, osprite, otarget)
{
	blast_range = oblast_range;
	blast_damage = oblast_damage;
	old_range = 2*blast_range;
	exploded = false;
}


void TauMCTorpedo::calculate()
{
	STACKTRACE;
	HomingMissile::calculate();
	if (target) {
		double r = distance(target);
		if ((old_range < blast_range) && (r > old_range)) {
			state = 0;
			animateExplosion();
			soundExplosion();
		}
		else    old_range = r;
	}
}


void TauMCTorpedo::soundExplosion()
{
	STACKTRACE;
	if (old_range < blast_range)
		damage_factor = blast_damage;
	HomingMissile::soundExplosion();
}


void TauMCTorpedo::animateExplosion()
{
	STACKTRACE;
	if (exploded) return;
	exploded = true;
	if (old_range < blast_range) {
		Query q;
		double r, d;
		for (q.begin(this, OBJECT_LAYERS, blast_range); q.currento; q.next()) {
			if (!q.current->isObject())
				continue;
			r = distance(q.currento) / blast_range;
			if (r > 1)
				r = 1;
			d = blast_damage * (1 - r*r);
			//r = sqrt(r);
			//dam = (int)ceil(d * (1-r));
			//ddam = (int)ceil(d * r);
			//q.currento->handle_damage(this, dam, ddam);
			q.currento->handle_damage(this, d, 0);
		}
		explosionSprite     = data->spriteWeaponExplosion;
		damage_factor = blast_damage;
		explosionFrameCount = 10;
		explosionFrameSize  = 50;
	}
	HomingMissile::animateExplosion();
}


REGISTER_SHIP(TauMC)
