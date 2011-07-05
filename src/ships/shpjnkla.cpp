/* $Id: shpjnkla.cpp,v 1.16 2004/03/27 10:06:08 geomannl Exp $ */
#include "../ship.h"
REGISTER_FILE

#define jnk_marker_id 0x2071

class JnkdwoLazelCrystal;

class JnkdwoLazel : public Ship
{

	friend class JnkdwoLazelCrystal;

	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       specialRange;
	int          specialDamage;
	double       specialAngle;
	int          specialFrames;
	double       extraVisibility;
	double       extraInvisibility;
	double       extraRangeFactor;
	bool         extraShipsOnly;
	double       specialAim;
	double       specialSpread;

	JnkdwoLazelCrystal* crystal[8];

	public:
		JnkdwoLazel (Vector2 opos, double angle, ShipData *data, unsigned int code);

		virtual int activate_weapon();
		virtual void calculate_fire_special();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void materialize();

		virtual void calculate();
};

class JnkdwoLazelCrystal : public SpaceLocation
{
	JnkdwoLazel *ship;
	double rx, ry, ra;
	int recharge;

	public:
		JnkdwoLazelCrystal (JnkdwoLazel *oship, double ox, double oy, double ora);
		virtual void calculate();
};

class JnkdwoLazelMarker : public SpaceLocation
{
	public:
		SpaceObject *o;
		JnkdwoLazelMarker (SpaceLocation *creator, SpaceObject *oo);
		virtual void calculate();
};

class JnkdwoLazelMissile : public Missile
{
	public:
		JnkdwoLazelMissile(SpaceLocation *creator, double ox, double oy, double oangle, double ov, int odamage, double orange,
			int oarmour, SpaceSprite *osprite);
		virtual void inflict_damage (SpaceObject *other);
};

class JnkdwoLazelLaser : public Laser
{
	int    power_left;
	double base_length;
	double aim, max_angle, lng, d_a;
	SpaceObject *tgt;

	public:
		JnkdwoLazelLaser (SpaceLocation *creator, double langle, double mangle, int lcolor, double lrange, int ldamage, int lfcount,
			SpaceLocation *opos, SpaceObject *otgt, double oaim, double spread);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

JnkdwoLazel::JnkdwoLazel(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialDamage  = get_config_int("Special", "Damage", 0);
	specialRange   = scale_range(get_config_float("Special", "Range", 0));
	specialAngle   = get_config_float("Special", "Angle", 90) * ANGLE_RATIO;
	specialFrames  = get_config_int("Special", "Frames", 100);
	specialAim     = get_config_float("Special", "Aim", 0) * ANGLE_RATIO / 1000.0;
	specialSpread  = get_config_float("Special", "Spread", 0) * ANGLE_RATIO;

	extraVisibility   = get_config_float("Extra", "VisibilityPoints", 0);
	extraInvisibility = get_config_float("Extra", "InvisibilityFactor", 0);
	extraRangeFactor  = get_config_float("Extra", "RangeFactor", 0);
	extraShipsOnly    = !(get_config_int("Extra", "ShipsOnly", 0) == 0);

	special_low    = false;

}


int JnkdwoLazel::activate_weapon()
{
	STACKTRACE;

	add(new JnkdwoLazelMissile(this, -6.0, 45.0, angle , weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, data->spriteWeapon));
	add(new JnkdwoLazelMissile(this, +6.0, 45.0, angle , weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, data->spriteWeapon));
	return(TRUE);
}


void JnkdwoLazel::calculate_fire_special()
{
	STACKTRACE;

	if ((!fire_special) || (batt >= special_drain)) special_low = false;
}


void JnkdwoLazel::materialize()
{
	STACKTRACE;
	Ship::materialize();
	for (int i = 0; i <4; i++) {
		crystal[i]=new JnkdwoLazelCrystal(this,-16.0, 1+(i-1.5)*12, -PI/2);
		add(crystal[i]);
		crystal[i+4]=new JnkdwoLazelCrystal(this,+16.0, 1+(i-1.5)*12, +PI/2);
		add(crystal[i+4]);
		//                game->addItem(new JnkdwoLazelCrystal(this,-16.0, 1+(i-1.5)*12, -PI/2));
		//                game->addItem(new JnkdwoLazelCrystal(this,+16.0, 1+(i-1.5)*12, +PI/2));
	}
}


JnkdwoLazelCrystal::JnkdwoLazelCrystal (JnkdwoLazel *oship, double ox, double oy, double ora) :
SpaceLocation(oship, 0, 0), ship(oship), rx(ox), ry(oy), ra(ora)
{

	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;
	recharge = 1;
	calculate();
}


void JnkdwoLazelCrystal::calculate()
{
	STACKTRACE;

	if (state == 0) return;

	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	angle = normalize(ship->angle + ra, PI2);

	double alpha = ship->angle ;
	double tx = cos(alpha);
	double ty = sin(alpha);

	pos.x = normalize(ship->pos.x + ry * tx - rx * ty, map_size.x);
	pos.y = normalize(ship->pos.y + ry * ty + rx * tx, map_size.y);

	if (recharge > 0) recharge -= frame_time;
	else if (ship->fire_special) {

		if (ship->batt < ship->special_drain) {
			ship->special_low = true;
			return;
		};

		Query q;
		SpaceObject *t = NULL;
		double r1, r = 0;
		double marks;
		for (q.begin(this, OBJECT_LAYERS, ship->specialRange); q.currento; q.next())
		if ((!q.currento->sameTeam(this)) && (q.currento->collide_flag_anyone & bit(LAYER_LINES)) && (!q.currento->isPlanet())) {
			if (ship->extraShipsOnly && q.currento->isShip()) continue;
			alpha = trajectory_angle(q.currento);
			double d_a = normalize(alpha - angle, PI2);
			if (d_a > PI) d_a -= PI2;
			if (fabs(d_a) > ship->specialAngle) continue;
			marks = 0;
			Query qm;
			for (qm.begin(q.currento, bit(LAYER_SPECIAL), 10); qm.current; qm.next())
				if (qm.current->getID() == jnk_marker_id)
					if (((JnkdwoLazelMarker*)qm.current)->o == q.currento)
						marks += 1;
			if (!q.currento->isInvisible())
				marks += ship->extraVisibility;
			else    marks *= ship->extraInvisibility;

			r1 = distance(q.currento);

			if (r1 >= 10.0)
				marks *= (1 + ship->extraRangeFactor * 10.0 / r1);

			if (marks > r) {
				r = marks;  t = q.currento;
			}
		}

		if (t) {
			int c = random()%80;
			c = makecol(160+c, 120+c, 175+c);
			//                        game->addItem(new PointLaser(c, ship->specialDamage, ship->specialRange, ship->specialFrames, this, t, 0, 0));
			add(new JnkdwoLazelLaser (this, trajectory_angle(t), ship->specialAngle, c, ship->specialRange, ship->specialDamage,
				ship->specialFrames, this, t, ship->specialAim * r, ship->specialSpread));
			//play_sound(ship->data->sampleSpecial[0]);
			play_sound(data->sampleSpecial[0]);
			recharge += ship->special_rate;
			ship->batt -= ship->special_drain;
		}
	}
}


int JnkdwoLazel::handle_damage (SpaceLocation *source, double normal, double direct)
{

	int d = Ship::handle_damage(source, normal, direct);

	if (state == 0)
		for (int i =0; i < 8; i++)
			crystal[i]->state = 0;

	return d;
}


void JnkdwoLazel::calculate()
{
	STACKTRACE;

	Ship::calculate();

	// just to be sure I guess .. check if the crystal aren't dead.
	// dunno if this can ever happen (geo)?
	for (int i =0; i < 8; i++) {
		if (!crystal[i]->exists()) {
			tw_error("A crystal died - should not happen !!");
		}
	}
}


JnkdwoLazelMissile::JnkdwoLazelMissile(SpaceLocation *creator, double ox, double oy, double oangle, double ov, int odamage, double orange,
int oarmour, SpaceSprite *osprite) :
Missile(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, creator, osprite)
{
	STACKTRACE;

	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
}


void JnkdwoLazelMissile::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

	Missile::inflict_damage(other);

	if (other && other->exists())
		add(new JnkdwoLazelMarker(this, other));
}


JnkdwoLazelMarker::JnkdwoLazelMarker(SpaceLocation *creator, SpaceObject *oo) :
SpaceLocation(creator, 0, 0), o(oo)
{
	STACKTRACE;
	id = jnk_marker_id;
	layer = LAYER_SPECIAL;
	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;
	calculate();
}


void JnkdwoLazelMarker::calculate()
{
	STACKTRACE;

	if (state == 0) return;

	if (!(o && o->exists())) {
		o = 0;
		state = 0;
		return;
	}

	pos = o->normal_pos();

	// dunno if the absence of this could cause something strange ... perhaps it does?
	// maybe "some" routine needs to access its "ship" pointer sometime ?
	SpaceLocation::calculate();
}


JnkdwoLazelLaser::JnkdwoLazelLaser (SpaceLocation *creator, double langle, double mangle, int lcolor, double lrange, int ldamage, int lfcount,
SpaceLocation *opos, SpaceObject *otgt, double oaim, double spread)
:
Laser(creator, langle, lcolor, lrange, 1, lfcount/ldamage, opos, 0, 0)
{
	//        collide_flag_sameteam = 0;
	//        collide_flag_sameship = 0;

	target = NULL; ship = NULL;
	power_left = ldamage-1;
	tgt = otgt; aim = oaim;
	max_angle = mangle; lng = length;
	base_length = length;
	angle = normalize(angle + spread * (1000 - (random()%2001))/1000.0, PI2);

	d_a = 0;
}


void JnkdwoLazelLaser::calculate()
{
	STACKTRACE;

	if (!(lpos && lpos->exists())) {
		lpos = 0;
		state = 0;
	}
	if (state == 0) return;

	if (frame < frame_count) {
		frame += frame_time;
	} else {

		if (!(tgt && tgt->exists())) {
			tgt = 0;
			state = 0;
			return;
		}
		frame = 0;
		if (power_left > 0) {
			damage_factor = 1; power_left--;
		}
		else    tgt = NULL;
	}

	double alpha;

	pos = lpos->normal_pos() + rotate(rel_pos, lpos->get_angle() - PI/2);
	vel = lpos->get_vel();

	if (sinc_angle) angle = normalize(lpos->get_angle() + relative_angle, PI2);

	if (tgt) { if (!tgt->exists()) tgt = NULL;}
	else tgt = NULL;

	if (tgt)
		d_a = normalize(trajectory_angle(tgt) - angle, PI2);
	else {
		base_length = lng * (frame_count - frame) / frame_count;
	}

	length = base_length;
	SpaceLine::calculate();

	if (d_a > PI) d_a -= PI2;
	alpha = aim * frame_time;
	if (fabs(d_a) < alpha) alpha = fabs(d_a);
	if (d_a > 0) angle += alpha;
	else         angle -= alpha;

	d_a = normalize(lpos->get_angle() - angle, PI2);
	if (d_a > PI) d_a -= PI2;
	if (fabs(d_a) > max_angle)
		tgt = NULL;
	//                angle = pos->get_angle() - max_angle * d_a / fabs(d_a);
}


void JnkdwoLazelLaser::inflict_damage(SpaceObject *other)
{
	STACKTRACE;

	//  return;

	if (damage_factor < 0) return;
	int i;
	i = iround(damage_factor / 2);
	if (i >= BOOM_SAMPLES)
		i = BOOM_SAMPLES - 1;
	play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	damage(other, damage_factor, 0);
	damage_factor = -1;
	//	collide_flag_anyone = collide_flag_sameship = collide_flag_sameteam = 0;

	//        return;

	add(new Animation( this,
		normal_pos() + edge(), meleedata.sparkSprite, 0,
		SPARK_FRAMES, 50, LAYER_EXPLOSIONS));
	return;
}


REGISTER_SHIP(JnkdwoLazel)
