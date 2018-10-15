/* $Id: shpastba.cpp,v 1.4 2004/03/24 23:51:40 yurand Exp $ */
#include <stdio.h>
#include "../ship.h"
#include "../melee/mview.h"
#include "../util/aastr.h"
#include "../frame.h"
#include "../melee.h"
//#include "Seg.h"
REGISTER_FILE

#define BASILISK_POISON_ID     0x1122

inline double sqr(double x)
{
	return x*x;
}


// ***************************************************************************

class BChain;

// ***************************************************************************

class BasiliskPoison;

class BasiliskAimline;

// ***************************************************************************

class BasiliskAreaHurt : public Presence
{
	Vector2 *xp, *xv;
	int     num, lifetime, life_counter, color;
	public:
		BasiliskAreaHurt(Vector2 opos, double ov, int onum, int olife, int ocolor);
		virtual void calculate();
		virtual void animate(Frame *space);
		virtual ~BasiliskAreaHurt();
};

// ***************************************************************************

class AstromorphBasilisk : public Ship
{
	int     weaponColor;
	double  weaponRange;
	int     weaponDamage;
	int     weaponLife;
	double  weaponVelocity;
	double  weaponArmour;

	double  aimlineRange;
	double  aimlineAngleRange;
	double  aimBonus;

	double  specialMass;
	int     specialSegs;
	double  specialSpacing;
	double  specialHealth;

	double  slitherTime;
	double slitherTick;
	double  slitherAmount;
	double  slitherFrame;
	double  slitherAccel;
	double slitherFriction;
	bool    bOtherWay;

	double  cruise_max;
	double  whip_speed;

	double  poison;

	double  regrowTime;
	double  regrow_delay;
	double  regrowDrain;

	double  hurt_damage;
	double  max_hurt_flash_time;

	BasiliskAimline* aimline;

	public:

		BChain *Head;

		double hurty_time;
		int numSegs;

		AstromorphBasilisk(Vector2 opos, double angle, ShipData *data, unsigned int code);
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void animate(Frame *space);
		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void calculate_thrust();
};

// ***************************************************************************

class BasiliskAimline : public Laser
{
	private:
		double angle_range;
		int oncolor, offcolor;

	protected:
		AstromorphBasilisk* ship;

	public:
		virtual void calculate();
		virtual void collide(SpaceObject *o);

		BasiliskAimline(AstromorphBasilisk *creator, double langle,
			int ooffcolor, int ooncolor, double lrange, SpaceLocation *opos,
			Vector2 rpos, double oangle_range);
};

// ***************************************************************************

BasiliskAimline::BasiliskAimline(AstromorphBasilisk *creator, double langle,
int ooffcolor, int ooncolor, double lrange, SpaceLocation *opos,
Vector2 rpos, double oangle_range)
: Laser(creator, langle, ooffcolor, lrange, 0, 0, opos, rpos, true)
{
	STACKTRACE;
	angle_range = oangle_range;
	oncolor = ooncolor;
	offcolor = ooffcolor;

	ship = creator;

	target = NULL;
}


// ***************************************************************************

void BasiliskAimline::calculate()
{
	STACKTRACE;
	if ((lpos->exists() && ship->fire_weapon)) {
		pos = lpos->normal_pos() + rotate(rel_pos, lpos->get_angle() - PI/2);
		vel = lpos->get_vel();

		angle = normalize(lpos->get_angle() + relative_angle, PI2);

		if (target) {
			double temp_angle;

			temp_angle = trajectory_angle(target);
			if (ship->distance(target) <= length &&
			fabs(temp_angle-angle)  <= angle_range) {
				angle = temp_angle;
				color = oncolor;
			} else {
				target = NULL;
				color = offcolor;
			}
		}
		else
			color = offcolor;

		SpaceLine::calculate();
	}
	else
		state = 0;

	return;
}


// ***************************************************************************

void BasiliskAimline::collide(SpaceObject *o)
{
	STACKTRACE;

	if ((!canCollide(o)) || (!o->canCollide(this)))
		return;

	if (target == NULL && o->isShip())
		target = o;

	return;
}


// ***************************************************************************

//This is the base class for BasiLink and BChain.

class Seg : public SpaceObject
{
	protected:
		double max_health;
		double max_hurt_flash_time, hurt_damage;
	public:
		double hurty_time;
		double health;
		double friction;

		Seg(SpaceLocation *creator, Vector2 opos, double oangle,
			SpaceSprite *osprite, SpaceObject *Prev_Object,
			SpaceObject *Next_Object);
		Seg *Next_Seg;
		Seg *Prev_Seg;

};

// ***************************************************************************

Seg::Seg(SpaceLocation *creator, Vector2 opos, double oangle,
SpaceSprite *osprite, SpaceObject *Prev_Object,
SpaceObject *Next_Object):SpaceObject(creator,
opos,oangle,osprite)
{
	STACKTRACE;
	hurty_time = 0;
	Prev_Seg=(Seg *)Prev_Object;
	Next_Seg=(Seg *)Next_Object;
}


// ***************************************************************************

class BChain : public Seg
{
	void BChainPhysics(SpaceObject *first, SpaceObject *second);
	void BChainRecur(Seg *other, int num);
	virtual void calculate();
	virtual void animate(Frame* space);
	double Seg_Distance;

	public:
		BChain(SpaceLocation *creator, Vector2 opos, double oangle,
			SpaceSprite *osprite, int oSegs, double ospacing, double omass, double ohealth,
			double om_h_f_t, double oh_d, double of);

		void ouchify(double amount, double direction, double whip_speed, int segs);
		bool regrow(int maxSegs);

		virtual void inflict_damage(SpaceObject *other);
		int handle_damage(SpaceLocation *source, double normal, double direct);
		void Pull_Last_Seg();
};

// ***************************************************************************

//A BasiLink is pretty much the same, except it rotates and has mass.
//Needs 64 images.
class BasiLink : public Seg
{
	protected:
		virtual void animate(Frame* space);
		virtual void calculate();

	public:
		BasiLink(SpaceLocation *creator, Vector2 opos, double oangle,
			SpaceSprite *osprite, SpaceObject *Prev_Object,
			SpaceObject *Next_Object, double omass, double ohealth, double omax_health,
			double om_h_f_t, double oh_d, double of);

		virtual void inflict_damage(SpaceObject *other);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

// ***************************************************************************

class BasiliskGas: public AnimatedShot
{

	int hitShip;
	int duration;
	float poison;

	public:

		BasiliskGas(double ox,double oy,double oangle,double ov, int odamage, int duration,
			double orange, int oarmour, float poison, AstromorphBasilisk *oship, SpaceSprite *osprite, double relativity = 0);

		virtual void inflict_damage(SpaceObject *other);
		virtual void soundExplosion();
		virtual void animateExplosion();
};

// ***************************************************************************

class BasiliskPoison : public SpaceObject
{
	public:

		double poison;
		int duration;
		int start;
		Ship *oship;

		BasiliskPoison(BasiliskGas *gas, int duration, float poison, Ship *kill_it, SpaceSprite *osprite);
		virtual void calculate();
		virtual void animate(Frame *space);
};

// ***************************************************************************

BasiLink::BasiLink(SpaceLocation *creator, Vector2 opos, double oangle,
SpaceSprite *osprite, SpaceObject *Prev_Object,
SpaceObject *Next_Object,double omass, double ohealth, double omax_health, double om_h_f_t, double oh_d, double of)
:
Seg(creator,opos,oangle,osprite,Prev_Object,Next_Object)
{
	STACKTRACE;
	max_hurt_flash_time = om_h_f_t;
	hurt_damage = oh_d;
	hurty_time = 0;
	friction = of;

	mass=omass;
	health=ohealth;
	max_health = omax_health;
	layer = LAYER_SPECIAL;
}


// ***************************************************************************

int BasiLink::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	int  totalDamage = int( normal+direct + 0.5);

	if (totalDamage == 0)
		return 0;

	//If there are no Segs, the Seg is destroyed.
	if (!Prev_Seg && !Next_Seg) {
		state=0;
		return totalDamage;
	}

	if (hurty_time > 0)
		return 0;

	//If a next Seg exists, the damage is passed to it.
	//directDamage is used so that we don't play the damage sound
	//again.

	if (Next_Seg)
		return damage(Next_Seg, 0, totalDamage);
	else {
		if (totalDamage > health) {
			Prev_Seg->Next_Seg=NULL;
			damage(Prev_Seg, 0, totalDamage - health);
			play_sound2(data->sampleExtra[0]);
			die();
			((AstromorphBasilisk*)ship)->numSegs--;
		}
		else
			health -= totalDamage;
	}
	return 1;
}


// ***************************************************************************

void BasiLink::calculate()
{
	STACKTRACE;

	Seg::calculate();

	if ( !Prev_Seg && !Next_Seg) {
								 //If the mothership is killed, the BChain dies.
		state=0;
		ship = 0;
		return;
	}

	if (hurty_time <= 0)
		vel *= 1 - friction * frame_time;;

	if (Prev_Seg && !Prev_Seg->exists())
		Prev_Seg = NULL;
	if (Next_Seg && !Next_Seg->exists())
		Next_Seg = NULL;

	if (!Next_Seg && Prev_Seg)
		sprite_index = get_index(angle)+64;
	else
		sprite_index = get_index(angle);

	if (hurty_time >= 0)
		hurty_time -= frame_time * 1E-3;
	if (hurty_time < 0)
		hurty_time = 0;
}


// ***************************************************************************

void BasiLink::animate(Frame *space)
{
	STACKTRACE;
	if ( hurty_time == 0 ) {
		int _old_trans = aa_get_trans();
		aa_set_trans ( iround(255 * ((max_health-health) / (max_health + 1))) );
		sprite->animate(pos, sprite_index, space);
		aa_set_trans(_old_trans);
	} else {
		int col = makecol(150,40,0);
		sprite->animate_character(pos, sprite_index, col, space);

		int _old_trans = aa_get_trans();
		aa_set_trans ( iround(255 * (hurty_time / max_hurt_flash_time)));
		sprite->animate(pos,sprite_index, space);
		aa_set_trans(_old_trans);
	}
}


// ***************************************************************************

void BasiLink::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (hurty_time > 0) {
		add(new BasiliskAreaHurt(pos, scale_velocity(20), 75, 450, makecol(255,240,140)));
		play_sound(data->sampleSpecial[1]);
		damage(other, hurt_damage);
		hurty_time = 0;
	}

	Seg::inflict_damage(other);
}


// ***************************************************************************

BChain::BChain(SpaceLocation *creator, Vector2 opos, double oangle,
SpaceSprite *osprite, int oSegs,double ospacing, double omass,
double ohealth, double om_h_f_t, double oh_d, double of)
:
Seg(creator,opos, oangle,osprite,NULL,NULL)
{
	STACKTRACE;

	health = ohealth;
	max_health = ohealth;
	friction = of;

	max_hurt_flash_time = om_h_f_t;
	hurt_damage = oh_d;
	hurty_time = 0;

	Prev_Seg=(Seg *)creator;	 //Attach the first Seg to the ship
	mass=omass;

	Seg *Cur_Seg=this;			 //Move this pointer to the first Seg
	Seg_Distance=ospacing;

	//Each new Seg is placed (ospacing) far away from the previous,
	//forming an angled line.
	Vector2 dd = - Seg_Distance * unit_vector(oangle);

	//Loop that creates all the other Segs
	for(int num=1; num<oSegs; num++) {
		//Calculate position of new Seg
		Vector2 ppos = dd*num + opos;

		//Create a new Seg that knows that it's attached to Cur_Seg
		Cur_Seg->Next_Seg = new BasiLink(creator,ppos,oangle,osprite,
			Cur_Seg,NULL,omass, ohealth, ohealth, om_h_f_t, oh_d, of);

		//Add it to the game
		add(Cur_Seg->Next_Seg);

		//Move the pointer to the new Seg
		Cur_Seg=Cur_Seg->Next_Seg;
	}

}


// ***************************************************************************

void BChain::Pull_Last_Seg()
{
	STACKTRACE;
	Seg *l=this;

	//Seek to the end of the list
	while(l->Next_Seg!=NULL)
		l=l->Next_Seg;

	//decelerate the last Seg
	l->vel *= 0.5;
	//l->angle = Prev_Seg->angle;
}


// ***************************************************************************

int BChain::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	if (normal+direct == 0)
		return 0;

	if (hurty_time > 0)
		return 0;

	if (Next_Seg)
		damage(Next_Seg, 0, normal+direct);
	else
		damage(ship, 0, normal+direct);
	return 1;
}


// ***************************************************************************

bool BChain::regrow(int maxSegs)
{
	STACKTRACE;
	int segCount = 0;

	Seg* cur = this;
	Seg* last = NULL;

	// increment down the list, looking for hurt sections
	while(cur != NULL && ((cur->health == max_health) && (segCount < maxSegs))) {
		last = cur;
		cur = cur->Next_Seg;
		segCount++;
	}

	if (cur != NULL && (cur->health < max_health)) {
		play_sound2(data->sampleExtra[1]);
		cur->health++;

		return true;
	}
	else if (cur == NULL && last != NULL && segCount < maxSegs) {
		Vector2 dd = - Seg_Distance * unit_vector(last->angle);

		Vector2 ppos = dd + last->pos;

		//Create a new Seg that knows that it's attached to Cur_Seg
		last->Next_Seg = new BasiLink(ship,ppos,last->angle, sprite,
			last, NULL, mass, 1, max_health, max_hurt_flash_time, hurt_damage, friction);

		//Add it to the game
		add(last->Next_Seg);
		((AstromorphBasilisk*)ship)->numSegs++;
		play_sound2(data->sampleExtra[1]);

		return true;
	}

	return false;
}


// ***************************************************************************

//A recursive function, the most efficient way to go thru a linked list
//backwards.
void BChain::BChainRecur(Seg *other,int num)
{
	STACKTRACE;

	//Call BChainRecur for the next Seg UNLESS you're at the last Seg

	if (other->Next_Seg!=NULL)
		BChainRecur(other->Next_Seg,num+1);

	//The current Seg is jerked to a halt by the previous Seg.
	//If the first Seg ain't budging, CHECK THE MASS!

	BChainPhysics(other->Prev_Seg,other);

}


// ***************************************************************************

void BChain::ouchify(double amount, double direction, double whip_speed, int segs)
{
	STACKTRACE;
	Seg* cur;
	int seg_count = segs;

	cur=this;

	while(cur) {
		cur->hurty_time = amount;
		/*
				if (cur->Next_Seg == NULL && direction != 0)
				{
				//if (direction != 0)
					cur->accelerate_gravwhip(ship, cur->angle+(PI/2)*direction, whip_speed, MAX_SPEED);
				}
				*/
		cur->accelerate_gravwhip(ship, cur->angle+((PI/2)*direction), whip_speed, MAX_SPEED);
		whip_speed *= 0.75;
		seg_count--;

		cur = cur->Next_Seg;
	}
};

// ***************************************************************************

void BChain::calculate()
{
	STACKTRACE;
	if ( !(ship && ship->exists()) ) {
								 //If the mothership is killed, the BChain dies.
		state=0;
		ship = 0;
		return;
	}

	BChainRecur(this,0);		 //Do the physics for the BChain

	if (!Next_Seg && Prev_Seg)
		sprite_index = get_index(angle)+64;
	else
		sprite_index = get_index(angle);

	if (hurty_time <= 0)
		vel *= 1 - friction * frame_time;

	SpaceObject::calculate();	 //Let the base class do it's stuff

	if (hurty_time >= 0)
		hurty_time -= frame_time * 1E-3;
	if (hurty_time < 0)
		hurty_time = 0;
}


// ***************************************************************************

void BChain::animate(Frame *space)
{
	STACKTRACE;
	if ( hurty_time == 0 ) {
		int _old_trans = aa_get_trans();
		aa_set_trans ( iround(255 * ((max_health-health) / (max_health + 1)) ));
		sprite->animate(pos, sprite_index, space);
		aa_set_trans(_old_trans);
	} else {
		int col = makecol(150,40,0);
		sprite->animate_character(pos, sprite_index, col, space);

		int _old_trans = aa_get_trans();
		aa_set_trans ( iround(255 * (hurty_time / max_hurt_flash_time)));
		sprite->animate(pos,sprite_index, space);
		aa_set_trans(_old_trans);
	}
}


// ***************************************************************************

void BChain::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (hurty_time > 0) {
		play_sound(data->sampleSpecial[1]);
		add(new BasiliskAreaHurt(pos, scale_velocity(20), 75, 450, makecol(255,240,140)));
		damage(other, hurt_damage);
		hurty_time = 0;
	}

	Seg::inflict_damage(other);
}


// ***************************************************************************

void BChain::BChainPhysics(SpaceObject *first, SpaceObject *second)
{
	STACKTRACE;

	// first = towards the head, second = towards the tail

	if ((first==NULL)||(second==NULL))   return;

								 // arg - class
	double a = second->trajectory_angle((SpaceLocation *)first);
	double b = first->angle;	 //trajectory_angle((SpaceLocation *)second);

	double da = b - a;
	if (da >  PI) da -= PI2;
	if (da < -PI) da += PI2;

	double maxangle = 30 * ANGLE_RATIO;

	if ( da > maxangle )
		da -= maxangle;			 // overshoot correction
	else
	if ( da < -maxangle )
		da += maxangle;
	else
		da = 0;

	a += da * 0.1;				 // small correction for overshoot, *0.1 to make it _much_ more smooth

	angle = a;

	//Jerking the Seg back into place if it's gone too far
	//	first->pos = second->pos - Seg_Distance*unit_vector(a);

	if (first->isShip())
		first->pos = second->pos + 10*unit_vector(a);
	else
		first->pos = second->pos + Seg_Distance*unit_vector(a);

	//second->angle = a;
	second->angle = a - 0.5 * da;
	if (!first->isShip())
		first->angle = b - 0.5 * da;

	// the nodes are already updated at the back (second) - and values are updated
	// forward.
	// Yeah! This is a great source for weird behaviour :)

	// if such a "limit" occurs, you've extra "force" on the ends ...
	// accelerations ?! velocities ?!

	/*
	//Calculate final velocity of both using the Law of
	//Conservation of Momentum,
	//vfinal=(m1*v1+m2*v2)/(m1+m2)
	Vector2 v_f = (first->mass*first->vel + second->mass*second->vel)/(first->mass+second->mass);

	  first->vel = v_f;
	  second->vel = v_f;
	  // this is wrong, since it won't allow rotation
	//*/

	// right - this method doesn't treat freedom of rotation properly.
	// but is needed to make it look more real. The velocities in the direction of
	// the connection should be set equal.

	Vector2  L;
	L = unit_vector(a);

	double v1, v2;
	v1 = L.dot(first->vel);
	v2 = L.dot(second->vel);
	// the velocities projected onto the bar

	//Conservation of Momentum,
	// as above, but now only in the direction of the bar:
	double vfinal;
	vfinal = ( first->mass * v1 + second->mass * v2) / (first->mass + second->mass);

	first->vel += (vfinal - v1) * L;
	second->vel += (vfinal - v2) * L;

	// accelerations of "second" due to centripetal force on either side :

	Vector2 accel;
	double v;

	accel = 0;

								 // the perpendicular component
	v = magnitude( first->vel - L*first->vel.dot(L) );
	accel += L * (first->mass / second->mass) * sqr(v) / Seg_Distance;

	SpaceObject *third;

	third = this->Next_Seg;
	//third = 0;
	if ( third != 0 ) {
		Vector2 L2;
		L2 = unit_vector(third->angle);

								 // the perpendicular component
		v = magnitude( third->vel - L2*third->vel.dot(L2) );
		accel -= L2 * (third->mass / second->mass) * sqr(v) / Seg_Distance;
	}

	// apply these accelerations:

	second->vel += 1E-3 * accel * frame_time;

	// By default, let the element become neutral in rotational velocity by
	// including drag in the rotation direction.

	second->vel -= 0.5 * frame_time*1E-3 * (second->vel - L*second->vel.dot(L));

}


// ***************************************************************************

AstromorphBasilisk::AstromorphBasilisk(Vector2 opos, double angle, ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)

{
	STACKTRACE;
	slitherTime         = get_config_float("Ship", "SlitherTime",0);
	slitherAmount       = get_config_float("Ship", "SlitherAmount",0);
	slitherAccel        = scale_acceleration(get_config_float("Ship", "SlitherRate", 0), 0);
	slitherTick         = 1.0;
	slitherFriction     = get_config_float("Ship", "SlitherFriction", 0.0006);
	cruise_max          = scale_velocity(get_config_float("Ship", "CruiseMax",0));
	whip_speed          = scale_velocity(get_config_float("Special", "WhipSpeed",0));

	hurt_damage         = get_config_float("Special", "FlashDamage", 0);
	max_hurt_flash_time = get_config_float("Special", "FlashTime", 0);
	hurty_time          = 0;

	regrowTime          = get_config_float("Ship", "RegrowTime",0);
	regrow_delay        = regrowTime;
	regrowDrain         = get_config_int("Ship", "RegrowDrain",0);

	bOtherWay           = false;
	slitherFrame        = 0;

	weaponColor         =  get_config_int("Weapon", "Color", 0);

	poison              = get_config_float("Weapon", "Poison", 0);
	weaponVelocity      = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponRange         = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage        = get_config_int("Weapon", "Damage", 0);
	weaponLife          = get_config_int("Weapon", "PoisonDuration",0);
	weaponArmour        = get_config_int("Weapon", "Armor",0);

	aimlineRange        = scale_range(get_config_float("Weapon", "AimRange",0));
	aimlineAngleRange   = get_config_float("Weapon", "AimAngleMax",0) * ANGLE_RATIO;
	aimBonus            = get_config_float("Weapon", "TargetedSpeedBonus",1);
	aimline             = NULL;

	specialSegs         =  get_config_int("Special","Links",0);
	numSegs             = specialSegs;
	specialMass         =  get_config_float("Special","LinkMass",0);
	specialSpacing      = get_config_float("Special","LinkDistance",0);
	specialHealth       = get_config_float("Special", "LinkHealth",0);

	add(Head = new BChain(this, pos, angle, data->spriteSpecial, specialSegs,
		specialSpacing, specialMass, specialHealth, max_hurt_flash_time, hurt_damage, slitherFriction));

	hashotspots = false;
}


// ***************************************************************************

int AstromorphBasilisk::activate_weapon()
{
	STACKTRACE;

	if (aimline == NULL) {
		add(aimline = new BasiliskAimline(this, angle, tw_color(0, 96, 0), tw_color(0, 240, 0),
			aimlineRange, this, Vector2(0, (get_size().y / 2.07)), aimlineAngleRange));
		play_sound2(data->sampleWeapon[0]);
	}

	return(FALSE);
}


// ***************************************************************************

int AstromorphBasilisk::activate_special()
{
	STACKTRACE;
	/*
		if (Head==NULL)
			return(FALSE);
	*/
	//add(new BasiliskAreaHurt(pos, scale_velocity(20), 75, 450, makecol(255,240,140)));
	hurty_time = max_hurt_flash_time;
	play_sound(data->sampleSpecial[0]);

	if (Head) {
		double direction;

		if (turn_left && !turn_right)
			direction = 0.25;
		else if (turn_right && !turn_left)
			direction = -0.25;
		else
			direction = 0;

		//this->accelerate_gravwhip(this, angle+(PI/2)*(-direction), whip_speed, MAX_SPEED);
		Head->ouchify(max_hurt_flash_time, direction, whip_speed, specialSegs);
		//vel *= 0.5;
	}
	regrow_delay = regrowTime;
	return(TRUE);
}


// ***************************************************************************

void AstromorphBasilisk::animate(Frame *space)
{
	STACKTRACE;
	if ( hurty_time == 0 )
		sprite->animate(pos, sprite_index, space);
	else {
		//sprite->animate_character(pos, sprite_index, pallete_color[hot_color[((int)((hurty_time / max_hurt_flash_time) * 10)) % HOT_COLORS]], space);
		sprite->animate(pos, sprite_index, space);
		int col = makecol(150,40,0);
		sprite->animate_character(pos, sprite_index, col, space);

		int _old_trans = aa_get_trans();
		aa_set_trans ( iround(255 * (hurty_time / max_hurt_flash_time) ));
		sprite->animate(pos,sprite_index, space);
		aa_set_trans(_old_trans);
	}
};

// ***************************************************************************

void AstromorphBasilisk::calculate()
{
	STACKTRACE;

	// firing the weapon

	//	vel*=0.99;
	if (aimline && !fire_weapon && batt - weapon_drain >= 0 && weapon_recharge <= 0) {
		play_sound2(data->sampleWeapon[1]);

		batt -= weapon_drain;
		if (recharge_amount > 1)
			recharge_step = recharge_rate;
		weapon_recharge += weapon_rate;

		regrow_delay = regrowTime;

		double a;

		if (aimline->target != NULL) {
			SpaceObject* targ = aimline->target;

			a = intercept_angle2(pos, vel * game->shot_relativity, weaponVelocity*aimBonus,
				targ->normal_pos(), targ->get_vel());
		}
		else
			a = angle;

		add(new BasiliskGas(0.0, 0.0,
			a, (aimline->target != NULL) ? weaponVelocity*aimBonus : weaponVelocity,
			weaponDamage, weaponLife, weaponRange, iround(weaponArmour),
			poison, this, data->spriteWeapon, 1.0));

		aimline->target = NULL;
		aimline = NULL;
	}

	// slithering
	//if (!(turn_left || turn_right || !thrust))
	{
		//double frac = float(numSegs)/float(specialSegs);
		//double frac2 = numSegs/specialSegs;
		//double frac2 = magnitude(this->vel)/speed_max;
		double frac2 = 1.0;

		Vector2 normal;
		normal = Vector2(-vel.y, vel.x);
		normalize(normal);

		slitherFrame += frame_time * 1E-3;

		double a;
		//a = sin(PI * (frac2)*slitherFrame / 1.0 + (PI/2) ) *( (frac)*slitherAmount);

		// needs some work!
		a = (slitherAmount)*sin(slitherFrame * (frac2*slitherTime)/(PI/2) );

		vel += (accel_rate*frame_time) * a * normal;
		/*

		slitherFrame -= frame_time;

		slitherTick += frame_time * slitherAmount;

		frac2 = sin(slitherTick);

		slitherFrame *= frac;*/

		//if (slitherFrame < 0)
		//{
		//	slitherFrame = slitherTime;
		//	bOtherWay = !bOtherWay;
		//}
		//angle -= (bOtherWay) ? (PI2/64) : -(PI2/64);

		/*accelerate(this,
			//normalize(angle - ((bOtherWay) ? (PI/4) : -(PI/4)), PI2),
			normalize(angle -  (frac2*(PI/4))),
			slitherAccel*frac*frame_time, speed_max);*/
	}
	//	else
	{
		//slitherFrame = slitherTime/2;
		//bOtherWay = turn_right;
	}

	Ship::calculate();

	// ouchification tickdown

	if (hurty_time <= 0)
		vel *= 1 - slitherFriction * frame_time;

	if (hurty_time >= 0)
		hurty_time -= frame_time * 1E-3;
	if (hurty_time < 0)
		hurty_time = 0;

	// regrowth
	if (batt == batt_max && regrowTime > 0) {
		regrow_delay -= frame_time * 1E-3;
		if (regrow_delay <= 0)
			if (Head)
			if (Head->regrow(specialSegs)) {
				batt -= regrowDrain;
			regrow_delay = regrowTime;
		}
	}
}


// ***************************************************************************

void AstromorphBasilisk::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (hurty_time > 0) {
		play_sound(data->sampleSpecial[1]);
		add(new BasiliskAreaHurt(pos, scale_velocity(20), 75, 450, makecol(255,240,140)));
		damage(other, hurt_damage);
		hurty_time = 0;
	}

	Ship::inflict_damage(other);
}


// ***************************************************************************

int AstromorphBasilisk::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (normal+direct == 0)
		return 0;

	if (hurty_time > 0)
		return 0;
	if (Head && Head->Next_Seg) {
		damage(Head,0,direct+normal);
		return 1;
	}

	return Ship::handle_damage(source, normal, direct);
}


// ***************************************************************************

void AstromorphBasilisk::calculate_thrust()
{
	STACKTRACE;
	if (thrust)
		accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);
	else
		accelerate_gravwhip(this, angle, accel_rate * frame_time, cruise_max);
	return;
}


// ***************************************************************************

BasiliskAreaHurt::BasiliskAreaHurt(Vector2 opos, double ov, int onum, int olife, int ocolor) :
Presence(), num(onum), lifetime(olife), life_counter(0), color(ocolor)
{
	STACKTRACE;
	if (onum <= 0) {
		state = 0;
		return;
	}
	set_depth(DEPTH_EXPLOSIONS);
	xp = new Vector2[num];
	xv = new Vector2[num];

	int i;
	for (i=0; i<num; i++) {
		xp[i] = opos;
		xv[i] = ov * (0.5+sqrt(sqrt((random()%1000000001)/1000000000.0))) * unit_vector(PI2 * (random()%1000000)/1000000.0);
	}
}


// ***************************************************************************

void BasiliskAreaHurt::calculate()
{
	STACKTRACE;
	life_counter += frame_time;

	if (life_counter >= lifetime) {
		state = 0;
		return;
	}

	int i;
	for (i=0; i<num; i++)
		xp[i] += xv[i] * frame_time;
}


// ***************************************************************************

void BasiliskAreaHurt::animate(Frame *space)
{
	STACKTRACE;
	if (state == 0)
		return;
	int i, j;
	double t = 1 - life_counter/(double)lifetime;
	double  x0, y0, dx, dy;
	int xi, yi;
	Vector2 p0;
	drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	for (i=0; i<num; i++) {
		p0 = corner(xp[i]);
		x0 = p0.x;
		y0 = p0.y;
		p0 = unit_vector(xv[i]) * 3 * space_zoom;
		dx = p0.x;
		dy = p0.y;
		for (j=3; j>=0; j--) {
			if (space_zoom <= 1)
				set_trans_blender(0, 0, 0, iround(space_zoom * 255 * t * (4-j) / 4.0));
			else
				set_trans_blender(0, 0, 0, iround(1 * 255 * t * (4-j) / 4.0));
			xi = iround(x0 - dx * j);
			yi = iround(y0 - dy * j);
			putpixel(space->surface, xi, yi, color);
			space->add_pixel(xi, yi);
		}
	}
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


// ***************************************************************************

BasiliskAreaHurt::~BasiliskAreaHurt()
{
	if (num > 0) {
		delete xp;
		delete xv;
	}
}


// ***************************************************************************

BasiliskGas::BasiliskGas(double ox, double oy,double oangle,double ov,int odamage, int oduration,
double orange,int oarmour, float opoison, AstromorphBasilisk *oship, SpaceSprite *osprite, double relativity) :
AnimatedShot(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship,
osprite, 50, 21, relativity)

{
	STACKTRACE;
	duration = oduration;
	hitShip = FALSE;
	poison = opoison;
	collide_flag_anyone = bit(LAYER_SHIPS) | bit(LAYER_CBODIES);
}


// ***************************************************************************

void BasiliskGas::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	play_sound2(data->sampleExtra[3]);

	if (other->isShip()) {
		add(new BasiliskPoison(this, duration, poison, (Ship*)other, data->spriteWeapon));
		add(new FixedAnimation(this, other, data->spriteWeaponExplosion, 0, 8, 150, LAYER_EXPLOSIONS));
	} else {
		add(new Animation(this, pos, data->spriteWeaponExplosion, 0, 8, 150, LAYER_EXPLOSIONS));
	}

	AnimatedShot::inflict_damage(other);
	return;
}


// ***************************************************************************

void BasiliskGas::animateExplosion()
{
	STACKTRACE;
	return;
}


// ***************************************************************************

void BasiliskGas::soundExplosion()
{
	STACKTRACE;
	if (!hitShip) {
		play_sound2(data->sampleExtra[3]);
	}
	return;
}


// ***************************************************************************

BasiliskPoison::BasiliskPoison(BasiliskGas *gas, int nduration, float poison, Ship *nship, SpaceSprite *osprite) :
SpaceObject (gas, nship->normal_pos(), 0.0, osprite),
poison(poison),
duration(nduration),
oship(nship)
{
	STACKTRACE;
	target = oship;
	id |= BASILISK_POISON_ID;
	layer = LAYER_HOTSPOTS;
	start = TRUE;
	collide_flag_anyone = 0;
	Query q;
	for (q.begin(oship, bit(LAYER_HOTSPOTS), 10); q.current; q.next()) {
		if ((q.current->getID() == getID()) && (((BasiliskPoison*)q.current)->oship == oship)) {
			((BasiliskPoison*)q.current)->duration = duration;
			state = 0;
		}
	}
	q.end();
}


// ***************************************************************************

void BasiliskPoison::calculate()
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
				//				draw_sprite(oship->spritePanel->get_bitmap(0), data->spriteExtra->get_bitmap(0),16,18);
				oship->update_panel = TRUE;
			}
		}

		chance = random(1000);
		if (chance < frame_time * poison) {
			play_sound(data->sampleExtra[2]);
			damage(oship, 0, 1);
		}
	}
	return;
}


// ***************************************************************************

void BasiliskPoison::animate(Frame *space)
{
	STACKTRACE;
	return;
}


REGISTER_SHIP(AstromorphBasilisk)
