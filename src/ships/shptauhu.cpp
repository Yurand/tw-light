/* $Id: shptauhu.cpp,v 1.10 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE

class TauHunterPortal;

class TauHunter  :  public Ship
{
	double      weaponRange, weaponVelocity, weaponRelativity, weaponSpread;
	double      weaponLength, weaponDamage;
	int         weaponNumber;
	bool        weaponAlternating;

	double      specialRange, specialVelocity;
	double      specialDamage, specialArmour;

	double      extraVelocity, extraDrain, extraExitVelocityFactor;
	int         extraFrameTime;

	double      residual_drain;
	int         weapon_side, weapon_angle;
	int         ship_recharge_amount;
	int         exit_countdown;
	Vector2     ee, vv;
	double      s_a;

	int         batt_delay, weapon_delay, special_delay, extra_delay;

	public:

		double      extraCriticalRange;
		double      extraCriticalAngle;
		bool        holding_spec;
		bool        in_jump;
		bool        just_exited;

		TauHunter    (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		void  jump_in(TauHunterPortal *portal);

		virtual  int activate_weapon();
		virtual void calculate_fire_special();
		virtual void calculate();
		virtual int  handle_damage(SpaceLocation* source, double normal, double direct);
		virtual int  handle_fuel_sap(SpaceLocation* source, double normal);
		virtual double handle_speed_loss(SpaceLocation* source, double normal);
		virtual int  canCollide(SpaceLocation *other);
		virtual int  translate(Vector2 rel_pos);
		virtual int  accelerate(SpaceLocation *source, double angle, double vel, double max_speed);
		virtual void animate(Frame *space);
		virtual double  isProtected() const;
		virtual double  isInvisible() const;
		virtual void calculate_hotspots();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual Color  battPanelColor();
};

class TauHunterLaser : public SpaceLine
{
	double        range, d, v;

	public:

		TauHunterLaser   (double ox, double oy, double oangle, double ov, double orange, double olength, SpaceLocation *creator, double relativity, double odamage);
		virtual void  calculate();
		virtual void  inflict_damage(SpaceObject *other);
};

class TauHunterPortal : public Animation
{
	TauHunter    *ship;

	public:
		TauHunterPortal  (SpaceLocation *creator, double ox, double oy, SpaceSprite *osprite, int oft, TauHunter *oship);
		virtual void  calculate();
};

class TauHunterShot : public Shot
{
	int              extraFrameTime;
	SpaceSprite     *expl_sprite, *portal_sprite;
	TauHunter       *ship;

	public:

		TauHunterShot(double ox, double oy, double oangle, double ov, double odamage, double orange, double oarmour, TauHunter *creator,
			SpaceSprite *osprite, SpaceSprite *esprite, SpaceSprite *psprite, int oft);
		virtual void  calculate();
};

TauHunter::TauHunter (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	weapon_delay            = scale_frames(get_config_float("Weapon", "RechargeDelay", 0));
	special_delay           = scale_frames(get_config_float("Special", "RechargeDelay", 0));
	extra_delay             = scale_frames(get_config_float("Extra", "RechargeDelay", 0));

	weaponRange             = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity          = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponLength            = get_config_int("Weapon", "Length", 0);
	weaponSpread            = get_config_float("Weapon", "Spread", 0)*PI/180;
	weaponAlternating       = (get_config_int("Weapon", "Alternating", 0) != 0);
	weaponNumber            = get_config_int("Weapon", "Number", 1);
	weaponDamage            = get_config_float("Weapon", "Damage", 0);
	weaponRelativity        = get_config_float("Weapon", "Relativity", 0.0);

	specialRange            = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity         = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage           = get_config_int("Special", "Damage", 0);
	specialArmour           = get_config_int("Special", "Armour", 0);

	extraVelocity           = scale_velocity(get_config_float("Extra", "Velocity", 0));
	extraDrain              = get_config_float("Extra", "Drain", 0) / 1000.0;
	extraExitVelocityFactor = get_config_float("Extra", "ExitVelocityFactor", 1.0);
	extraCriticalAngle      = get_config_float("Extra", "CriticalAngle", 60)*PI/180;
	extraCriticalRange      = get_config_float("Extra", "CriticalRange", 0);
	extraFrameTime          = get_config_int("Extra", "FrameTime", 50);

	weapon_side             = random()%3;
	weapon_angle            = random()%6;
	in_jump                 = false;
	ship_recharge_amount    = recharge_amount;
	exit_countdown          = 0;
	holding_spec            = false;
	just_exited             = false;
	batt_delay              = 0;
}


int TauHunter::activate_weapon()
{
	STACKTRACE;
	#define R1 27.5
	#define R2 5.0

	if (in_jump) return false;

	double wx, wy = 10;

	double  a, b, c;
	int     i, k;

	for ( k=0; k<3; k++ ) {

		for ( i=0; i<weaponNumber; i++ ) {
			wx = R1 * cos((angle + weapon_side*PI2/3)) +
				-R2 * sin((weapon_angle + i) * PI/3);
			a = (1 - (random()%201)/100.0);
			b = (1 - 0.1*(random()%201)/100.0);
			c = (1 - 0.1*(random()%201)/100.0);
			SpaceLocation* s =  new TauHunterLaser(wx, wy, angle + weaponSpread * a,
				weaponVelocity * c, weaponRange * (1 - 0.2*fabs(a)) * b,
				weaponLength, this, weaponRelativity, weaponDamage);
			add(s);

			if ( sin((angle + weapon_side*PI2/3)) < 0)
				s->set_depth(get_depth() + 1);
			else
				s->set_depth(get_depth() - 1);
		}

		weapon_side += 1;
		if ( weapon_side == 3 ) {
			weapon_side = 0;
			weapon_angle += weaponNumber;
			if ( weapon_angle >= 6 )
				weapon_angle -= 6;
		}

		if (weaponAlternating)
			break;

	}

	if (batt_delay < weapon_delay)
		batt_delay = weapon_delay;
	recharge_amount = 0;
	return true;
}


void TauHunter::jump_in(TauHunterPortal *portal)
{
	STACKTRACE;
	double a = normalize(atan(vel) - angle, PI2);
	if (a > PI) a -= PI2;
	if (fabs(a) > extraCriticalAngle) return;
	in_jump = true;
	batt = batt_max;
	update_panel = true;
	residual_drain  = 0;
	recharge_amount = 0;
	vv = extraVelocity * unit_vector(angle);
	vel = 0;

	// not targetable
	//	int i;
	//	for(i=0; game->target[i] != this; i++);
	//	game->num_targets--;
	//	game->target[i] = game->target[game->num_targets];
	targets->rem(this);
}


void TauHunter::calculate_fire_special()
{
	STACKTRACE;
	special_low = false;
	if ( !fire_special ) {
		holding_spec = false;
		if ( in_jump && ( exit_countdown <= 0 ) ) {
			exit_countdown = extraFrameTime * 16;
			ee = normalize(pos + vv * extraFrameTime * 16, map_size);
			SpaceLocation *s = new Animation(this, ee, data->spriteSpecial, 0, 40, extraFrameTime, LAYER_HOTSPOTS);
			add(s);
			s->play_sound(data->sampleExtra[0]);
		}
	} else {
		if ( ( special_recharge > 0 ) || in_jump )
			return;
		if ( batt < special_drain ) {
			special_low = true; return;
		}
		batt -= special_drain;
		special_recharge += special_rate;
		SpaceLocation *s = new  TauHunterShot(0, 16, angle, specialVelocity, specialDamage, specialRange, specialArmour, this,
			data->spriteWeapon, data->spriteWeaponExplosion, data->spriteSpecial, extraFrameTime);
		add(s);
		if ( batt_delay < special_delay )
			batt_delay = special_delay;
		recharge_amount = 0;
		s->play_sound2(data->sampleSpecial[0]);
		holding_spec = true;
	}
}


void TauHunter::calculate()
{
	STACKTRACE;
	just_exited = false;

	if ( batt_delay > 0 ) batt_delay -= frame_time;

	else    if ( !in_jump )
		recharge_amount = ship_recharge_amount;

	Ship::calculate();

	if ( in_jump ) {
		//		vx = 0; vy = 0;
		vel = 0;
		if ( exit_countdown > 0 ) {
			exit_countdown -= frame_time;
			if ( exit_countdown <= 0 ) {
				in_jump = false;
				just_exited = true;
				recharge_amount = 0;
				//restore target
				targets->add(this);
				pos = ee;
				batt = 0;
				if (batt_delay < extra_delay)
					batt_delay = extra_delay;
				special_recharge = special_rate;
				vel = speed_max * extraExitVelocityFactor * vv;

			} else {
				pos = normalize(ee - vv * exit_countdown, map_size);
			}
		} else {
			vv = extraVelocity * unit_vector(angle);
			pos = normalize(pos + vv * frame_time, map_size);
			if (batt <= extraDrain*extraFrameTime*16) {
				exit_countdown = extraFrameTime * 16;
				ee = normalize(pos + vv * extraFrameTime * 16, map_size);
				SpaceLocation *s = new Animation(this, ee, data->spriteSpecial, 0, 40, extraFrameTime, LAYER_HOTSPOTS);
				add(s);
				s->play_sound(data->sampleExtra[0]);
			}
		}
		residual_drain += extraDrain * frame_time;
		batt -= (int)floor(residual_drain);
		if ( batt < 0 ) batt = 0;
		residual_drain -= floor(residual_drain);
	}
}


int TauHunter::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if ( in_jump )
		return 0;
	if ( just_exited && source->isPlanet() )
		normal = crew+1;
	return Ship::handle_damage(source, normal, direct);
}


int TauHunter::handle_fuel_sap(SpaceLocation *source, double normal)
{
	STACKTRACE;
	if ( in_jump )
		return 0;
	return Ship::handle_fuel_sap(source, normal);
}


double TauHunter::handle_speed_loss(SpaceLocation *source, double normal)
{
	STACKTRACE;
	if ( in_jump )
		return 0;
	return Ship::handle_speed_loss(source, normal);
}


int TauHunter::canCollide(SpaceLocation *other)
{
	STACKTRACE;
	if ( in_jump )
		return false;
	else
		return Ship::canCollide(other);
}


int TauHunter::translate(Vector2 rel_pos)
{
	STACKTRACE;
	if ( !in_jump ) return Ship::translate(rel_pos);
	return false;
}


int TauHunter::accelerate(SpaceLocation *source, double oangle, double vel, double omax_speed)
{
	STACKTRACE;
	if ( !in_jump ) return Ship::accelerate(source, oangle, vel, omax_speed);
	return false;
}


void TauHunter::animate(Frame* space)
{
	STACKTRACE;
	if ( !in_jump ) Ship::animate(space);
}


double TauHunter::isProtected() const
{
	if ( in_jump )
		return 1.0;
	else
		return Ship::isProtected();
}


double TauHunter::isInvisible() const
{
	if ( in_jump )
		return 1.0;
	else
		return Ship::isInvisible();
}


void TauHunter::calculate_hotspots()
{
	STACKTRACE;
	if ( !in_jump ) Ship::calculate_hotspots();
}


void TauHunter::calculate_turn_left()
{
	STACKTRACE;
	if ( exit_countdown <= 0 ) Ship::calculate_turn_left();
}


void TauHunter::calculate_turn_right()
{
	STACKTRACE;
	if ( exit_countdown <= 0 ) Ship::calculate_turn_right();
}


Color TauHunter::battPanelColor()
{
	STACKTRACE;
	Color c = {50,50,170};
	if ( in_jump ) return c;
	return Ship::battPanelColor();
}


TauHunterLaser::TauHunterLaser (double ox, double oy, double oangle, double ov, double orange, double olength, SpaceLocation *creator, double relativity, double odamage) :
SpaceLine(creator, creator->normal_pos(), oangle, olength, makecol(255,255,255)),
range(orange), d(0), v(ov)
{

	pos = normalize(pos + rotate(Vector2(-ox, oy), -PI/2+creator->get_angle()));

	damage_factor = odamage;
	vel = unit_vector(angle) + creator->get_vel() * relativity;
}


void TauHunterLaser::calculate()
{
	STACKTRACE;
	double r = (d) / range;
	double r2 = r*r;
	double r3 = r2*r;
	int g = (int)floor(225*(1 - 2*r2 + r3));
	if (g < 0) g = 0;
	int b = (int)floor(255*(1 - 6*r2+5*r3));
	if (b < 0) b = 0;
	color = makecol(int(235/(0.7*r+1)), g, b);
	SpaceLine::calculate();
	d += v * frame_time;
	if (d > range) state = 0;
}


void TauHunterLaser::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (d >= range) return;
	damage_factor *= pow((1-d/range), 0.2);
	SpaceLine::inflict_damage(other);
	state = 0;
}


TauHunterPortal::TauHunterPortal(SpaceLocation *creator, double ox, double oy, SpaceSprite *osprite, int oft, TauHunter *oship) :
Animation(creator, Vector2(ox, oy), osprite, 0, 40, oft, LAYER_HOTSPOTS), ship(oship)
{
	STACKTRACE;
}


void TauHunterPortal::calculate()
{
	STACKTRACE;
	Animation::calculate();
	if ( ship ) {
		if ( (!ship->exists()) || (!ship->holding_spec) || ship->in_jump ) {
			ship = NULL; return;
		}
		if ( (sprite_index >= 10) && (sprite_index <= 25) && (distance(ship) < ship->extraCriticalRange ) )
			ship->jump_in(this);
	}
}


TauHunterShot::TauHunterShot(double ox, double oy, double oangle, double ov, double odamage, double orange, double oarmour, TauHunter *creator,
SpaceSprite *osprite, SpaceSprite *esprite, SpaceSprite *psprite, int oft) :
Shot(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, creator, osprite, 1.0)
{
	STACKTRACE;
	expl_sprite             = esprite;
	portal_sprite           = psprite;
	ship                    = creator;
	extraFrameTime          = oft;
	/*	collide_flag_anyone		= 0;
		collide_flag_sameteam	= 0;
		collide_flag_sameship	= 0;*/
}


void TauHunterShot::calculate()
{
	STACKTRACE;
	if ( state == 0 ) return;
	sprite_index = (int)floor(20 * d*d / (range*range));
	if ( ship ) if ( (!ship->exists()) || (!ship->holding_spec) || ship->in_jump )
		ship = NULL;
	Shot::calculate();
	if ( d >= range )
		add(new TauHunterPortal(this, pos.x, pos.y, portal_sprite, extraFrameTime, ship));
}


REGISTER_SHIP(TauHunter)
