/* $Id: shpcrore.cpp,v 1.10 2005/08/28 20:34:07 geomannl Exp $ */
#include "../ship.h"

REGISTER_FILE

#define max_recoil 250
#define recoil_velocity scale_velocity(30)

class Crome  :  public Ship
{
	public:

		double        weaponRange, weaponBlastRange, weaponVelocity, weaponMaxCharge, weapon_charge;
		int           weaponDamage, weaponBlastDamage, weaponBlastDamageShots, weaponArmour;
		bool          weaponAutoExplode;
		bool          holding_fire;
		int           recoil_frame;
		int           recharge_extra;
		//  double        rtx, rty;
		Vector2       rtpos;

		double        specialRange, specialRepulse, specialTranslate;

	public:
		Crome      (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

		virtual  int  activate_weapon();
		virtual  int  activate_special();
		virtual void  calculate();
		virtual void  animate(Frame *space);
};

class CromeShot : public Shot
{
	public:
		bool exploded_already, auto_explode;
		int  blast_damage, blast_damage_shots;
		double blast_range;
		SpaceObject *direct_hit;

	public:
		CromeShot (SpaceLocation *creator, Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, SpaceSprite *osprite, double relativity,
			int oblast_damage, int oblast_damage_shots, double oblast_range, bool oauto_explode);

		virtual void calculate();
		virtual void animateExplosion();
		virtual void inflict_damage(SpaceObject *other);
};

Crome::Crome (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	//        hotspot_position = 0.22;

	weaponRange             = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponDamage            = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour            = tw_get_config_int("Weapon", "Armour", 0);
	weaponVelocity          = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponBlastDamage       = tw_get_config_int("Weapon", "BlastDamage", 0);
	weaponBlastDamageShots  = tw_get_config_int("Weapon", "BlastDamageShots", 0);
	weaponBlastRange        = scale_range(tw_get_config_float("Weapon", "BlastRange", 0));
	weaponAutoExplode       = (tw_get_config_int("Weapon", "AutoExplode", 0) != 0);
	weaponMaxCharge         = tw_get_config_int("Weapon", "MaxChargeTime", 0);

	specialRange            = scale_range(tw_get_config_float("Special", "Range", 0));
	specialRepulse          = scale_velocity(tw_get_config_float("Special", "RepulseAway", 0))/1000.0;
	specialTranslate        = scale_range(tw_get_config_float("Special", "TranslateAway", 0))/1000.0;

	holding_fire            = false;
	recoil_frame            = 0;
	recharge_extra          = recharge_amount;
}


int Crome::activate_weapon()
{
	STACKTRACE;
	if ((holding_fire) || (recoil_frame > 0)) return false;
	holding_fire = true;
	weapon_charge = 0;
	recharge_amount = 0;
	return true;
}


int Crome::activate_special()
{
	STACKTRACE;
	return true;
}


void Crome::calculate()
{
	STACKTRACE;
	if ((holding_fire) && (weapon_charge < weaponMaxCharge)) {
		if ( ((weapon_charge += frame_time) >= weaponMaxCharge)
		|| (!fire_weapon)) {
			game->add(new CromeShot (this, Vector2(0, 10), angle, weaponVelocity, weaponDamage,
				(weaponRange*weapon_charge)/weaponMaxCharge, weaponArmour, data->spriteWeapon, 1, weaponBlastDamage,
				weaponBlastDamageShots, weaponBlastRange, weaponAutoExplode));

			play_sound2(data->sampleWeapon[0],0);
			play_sound2(data->sampleWeapon[1]);
			recoil_frame = max_recoil;
			//                        rtx = cos(angle*ANGLE_RATIO);
			//                        rty = sin(angle*ANGLE_RATIO);
			rtpos = unit_vector(angle);
			recharge_amount = recharge_extra;
			weapon_charge = weaponMaxCharge;
		}
	}

	if (!fire_weapon) {
		holding_fire = false;
		recharge_amount = recharge_extra;
	}

	Ship::calculate();

	if (recoil_frame > 0) {
		double xxx = frame_time * recoil_frame * recoil_velocity / max_recoil;
		//                x -= rtx * xxx;
		//                y -= rty * xxx;
		pos -= rtpos * xxx;
		recoil_frame -= frame_time;
	}

	if (special_recharge > 0) {
		Query q;
		double r, p, t, xp, xt, ov, nv, ta;
		Vector2 nvel, ovel;

		for (q.begin(this, OBJECT_LAYERS, specialRange); q.currento; q.next()) {
			if (!q.current->isObject())
				continue;

			if (((q.currento->mass <= 0) && !q.currento->isShot()) ) continue;
			xt = 0; xp = 0;
			r = distance(q.currento)/specialRange;
			//                        r *= r;

			r = sqrt(sqrt(r));
			p = specialRepulse * (1 - r) * frame_time;
			t = specialTranslate * (1 - r) * frame_time;

			xp = p;
			xt = t;

			if (q.currento->isPlanet()) {
				if (mass >= 1) {
					xp = p / sqrt(mass);
					xt = t / sqrt(mass);
				}
			} else {
				if (q.currento->mass > 1 && mass > 1) {
					xp = (p / sqrt(mass)) * (1-1/sqrt(q.currento->mass));
					xt = (t / sqrt(mass)) * (1-1/sqrt(q.currento->mass));
					p /= sqrt(q.currento->mass);
					t /= sqrt(q.currento->mass);
				}
			}

			//                        ovx = q.currento->get_vx();
			//                        ovy = q.currento->get_vy();
			ovel = q.currento->get_vel();
								 //sqrt(ovx*ovx + ovy*ovy);
			ov  = ovel.magnitude();
			ta = trajectory_angle(q.currento);
			accelerate(this, ta+PI, xp, MAX_SPEED);

			if (!q.currento->isPlanet())
				q.currento->accelerate(this, ta, p, MAX_SPEED);

			ta *=-1;			 // -ANGLE_RATIO;
								 //(-cos(ta)*xt, sin(ta)*xt);
			translate(xt * -unit_vector(-ta));
			if (q.currento->isPlanet()) continue;
								 //(cos(ta)*t, -sin(ta)*t);
			q.currento->translate(unit_vector(-ta));

			if (q.currento->isShot()) {
				//                                ovx = q.currento->get_vx();
				//                                ovy = q.currento->get_vy();
				ovel = q.currento->get_vel();
								 //atan3(ovy, ovx)*180/PI
				((Shot*)q.currento)->changeDirection(ovel.atan());
				//                                nvx = q.currento->get_vx();
				//                                nvy = q.currento->get_vy();
				nvel = q.currento->get_vel();
								 //sqrt(nvx*nvx + nvy*nvy);
				nv  = nvel.magnitude();

				if (fabs(nv-ov) > 0.2*(ov+nv)/2) {
					//                                        q.currento->vx = ovx;
					//                                        q.currento->vy = ovy;
					q.currento->set_vel ( ovel );
				}
			}
		}
	}
}


void Crome::animate(Frame* space)
{
	STACKTRACE;
	if (special_recharge > 0)
		data->spriteSpecial->animate(pos, sprite_index, space);
	else    sprite->animate(pos, sprite_index, space);
}


CromeShot::CromeShot (SpaceLocation *creator, Vector2 opos, double oangle, double ov, int odamage,
double orange, int oarmour, SpaceSprite *osprite, double relativity,
int oblast_damage, int oblast_damage_shots, double oblast_range, bool oauto_explode) :

Shot(creator, opos, oangle, ov, odamage, orange, oarmour, creator, osprite, relativity)
{
	exploded_already = false;
	blast_range = oblast_range;
	blast_damage = oblast_damage;
	auto_explode = oauto_explode;
	direct_hit = 0;
	blast_damage_shots = oblast_damage_shots;
	explosionSample = data->sampleWeapon[2];

	attributes &= ~ATTRIB_STANDARD_INDEX;
}


void CromeShot::calculate()
{
	STACKTRACE;
	Shot::calculate();

	if ((state == 0) && (!exploded_already) && auto_explode) {
		soundExplosion();
		animateExplosion();
		return;
	}

	sprite_index = int(24*d/range);
}


void CromeShot::animateExplosion()
{
	STACKTRACE;
	// this is part of physics (although the name suggests otherwise)

	if (exploded_already) return;
	exploded_already = true;

	game->add(new Animation(this, pos, data->spriteWeaponExplosion, 0, 8, 50, LAYER_EXPLOSIONS));
	game->add(new Animation(this, pos, data->spriteExtraExplosion, 0, 12, 50, LAYER_HOTSPOTS));

	Query q;

	for (q.begin(this, OBJECT_LAYERS, blast_range); q.currento; q.next()) {
		if (!q.current->isObject())
			continue;

		if (q.currento != direct_hit) {
			// damage everything in range...
			double r;
			r = distance(q.currento) / blast_range;

			if (q.currento->sameShip(this))
				r *= 2;			 // reduced damage for your own ship ??

			if (r > 1) continue; // out of damage range

			if (q.currento->isShot())
				damage(q.currento, (int)ceil(blast_damage_shots * (1 - r*r*r*r)));
			else
				damage(q.currento, (int)ceil(blast_damage * (1 - r*r*r*r)));
		}
	}

}


void CromeShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	direct_hit = other;
	Shot::inflict_damage(other);
}


REGISTER_SHIP(Crome)
