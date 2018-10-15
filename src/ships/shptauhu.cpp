/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

class TauHunterPPIPortal;

class TauHunterPPI  :  public Ship
{
public:
	double		weaponRange, weaponVelocity, weaponRelativity, weaponSpread;
	double		weaponLength, weaponDamage;
	int			weaponNumber;
	bool		weaponAlternating;
	
	//double		specialRange, specialVelocity;
	//double		specialDamage, specialArmour;

	double		extraVelocity, extraExitVelocityFactor;
	int			extraFrameTime;

	//double		residual_drain;
	int			weapon_side, weapon_angle;
	//int			ship_recharge_amount;
	//int			exit_countdown;
	//Vector2		ee, vv;
	double		s_a;

	//int			batt_delay, weapon_delay, special_delay, extra_delay;

	//double		default_recharge_rate;

	bool		prevent_more_special;

	int			jumptime, max_jumptime;
	bool		SlidingEntry;
	Vector2		jumpvector;

public:

	//double		extraCriticalRange;
	//double		extraCriticalAngle;
	//bool		holding_spec;
	bool		in_jump;
	bool		just_exited;

	TauHunterPPI    (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);


	virtual  int activate_weapon();
	virtual void calculate_fire_special();
	virtual void calculate();
	virtual int  handle_damage(SpaceLocation* source, double normal, double direct);
	virtual int	 handle_fuel_sap(SpaceLocation* source, double normal);
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


class TauHunterPPILaser : public SpaceLine
{
public:
	double        range, d, v;

public:

	TauHunterPPILaser   (double ox, double oy, double oangle, double ov, double orange, double olength, SpaceLocation *creator, double relativity, double odamage);
	virtual void  calculate();
	virtual void  inflict_damage(SpaceObject *other);
};





TauHunterPPI::TauHunterPPI (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
		Ship(opos, shipAngle, shipData, code)
{
	//weapon_delay			= scale_frames(get_config_float("Weapon", "RechargeDelay", 0));
	//special_delay			= scale_frames(get_config_float("Special", "RechargeDelay", 0));
    //extra_delay				= scale_frames(get_config_float("Extra", "RechargeDelay", 0));

    weaponRange				= scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity			= scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponLength			= get_config_int("Weapon", "Length", 0);
	weaponSpread			= get_config_float("Weapon", "Spread", 0)*PI/180;
	weaponAlternating		= (get_config_int("Weapon", "Alternating", 0) != 0);
	weaponNumber			= get_config_int("Weapon", "Number", 1);
	weaponDamage			= get_config_float("Weapon", "Damage", 0);
	weaponRelativity		= get_config_float("Weapon", "Relativity", 0.0);

	//specialRange			= scale_range(get_config_float("Special", "Range", 0));
	//specialVelocity			= scale_velocity(get_config_float("Special", "Velocity", 0));
	//specialDamage			= get_config_int("Special", "Damage", 0);
	//specialArmour			= get_config_int("Special", "Armour", 0);

	extraVelocity			= scale_velocity(get_config_float("Extra", "Velocity", 0));
//	extraDrain				= get_config_float("Extra", "Drain", 0) / 1000.0;
	extraExitVelocityFactor	= get_config_float("Extra", "ExitVelocityFactor", 1.0);
//	extraCriticalAngle		= get_config_float("Extra", "CriticalAngle", 60)*PI/180;
//	extraCriticalRange		= get_config_float("Extra", "CriticalRange", 0);
	extraFrameTime			= get_config_int("Extra", "FrameTime", 50);

	weapon_side				= tw_random(3);
	weapon_angle			= tw_random(6);
	in_jump					= false;
	//ship_recharge_amount	= recharge_amount;
	//exit_countdown			= 0;
//	holding_spec			= false;
	just_exited				= false;

	prevent_more_special = true;

	jumptime = 0;
	max_jumptime            = get_config_int("Extra", "JumpTime", 1000);
	SlidingEntry            = get_config_int("Extra", "SlidingEntry", 0);
	jumpvector = 0;
}

int TauHunterPPI::activate_weapon()
{
	 
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
			a = (1 - (random(2.0))/100.0);
			b = (1 - 0.1*(random(2.0))/100.0);
			c = (1 - 0.1*(random(2.0))/100.0);
			SpaceLocation* s =	new TauHunterPPILaser(wx, wy, angle + weaponSpread * a,
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
	

	return true;
}


void TauHunterPPI::calculate_fire_special()
{
	 


	special_low = false;
	
	if (!fire_special)
	{
		prevent_more_special = false;
		return;
	}

	// check timer
	if(special_recharge > 0)
		return;

	if (prevent_more_special)
		return;

	if (in_jump)
		return;

	if ( batt < special_drain )
	{
		special_low = true;
		return;
	}
	
	prevent_more_special = true;
	
	batt -= special_drain;
    special_recharge += special_rate;	// reset timer
	
	//special_recharge += special_rate;
//	SpaceLocation *s = new	TauHunterPPIShot(0, 16, angle, specialVelocity, specialDamage, specialRange, specialArmour, this,
//								data->spriteWeapon, data->spriteWeaponExplosion, data->spriteSpecial, extraFrameTime);
//	add(s);

	add(new Animation(this, pos, data->spriteSpecial,
		0, data->spriteSpecial->frames(), extraFrameTime,
		DEPTH_SPECIAL));
	
	
	play_sound2(data->sampleSpecial[0]);

	jumptime = max_jumptime;

	if (SlidingEntry && vel != 0)
		jumpvector = unit_vector(vel);
	else
		jumpvector = unit_vector(angle);

	in_jump = true;


}

void TauHunterPPI::calculate()
{
	 
	just_exited = false;
                        
	Ship::calculate();
        
	if ( in_jump ) {

		jumptime -= frame_time;


		// exiting the jump
		if (!fire_special || jumptime < 0)
		{
			jumptime = 0;

			in_jump = false;
			just_exited = true;
			prevent_more_special = false;
			//recharge_amount = 0;
			
			targets->add(this);
			//pos = ee;

			// reset the battery recharge to immediate recharge.
			recharge_step = 0;

			// reset the special-timer to a full period of waiting.
			special_recharge = special_rate;
			
			//pos = normalize(pos + vv * frame_time, map_size);
			
			// movement on exit of jumpspace...
			vel = speed_max * extraExitVelocityFactor * jumpvector;//unit_vector(angle);
		} else {
			// prevent recharge timing.
			recharge_step = 1000;

			// movement while in jump-space.
			Vector2 vv = extraVelocity * jumpvector;
			pos = normalize(pos + vv * frame_time, map_size);
			vel = 0;
		}
	} else if (prevent_more_special)
	{

		// prevent recharge timing if you're maintaining a shot.
		recharge_step = 1000;
	}
}

int TauHunterPPI::handle_damage(SpaceLocation *source, double normal, double direct)
{
	 
	if ( in_jump )
		return 0;
	if ( just_exited && source->isPlanet() )
		normal = crew+1;
	return Ship::handle_damage(source, normal, direct);
}

int TauHunterPPI::handle_fuel_sap(SpaceLocation *source, double normal) {
	 
	if ( in_jump )
		return 0;
	return Ship::handle_fuel_sap(source, normal);
}

double TauHunterPPI::handle_speed_loss(SpaceLocation *source, double normal) {
	 
	if ( in_jump )
		return 0;
	return Ship::handle_speed_loss(source, normal);
}

int TauHunterPPI::canCollide(SpaceLocation *other)
{
	 
	if ( in_jump )
		return false;
	else
		return Ship::canCollide(other);
}

int TauHunterPPI::translate(Vector2 rel_pos)
{
	 
	if ( !in_jump ) return Ship::translate(rel_pos);
	return false;
}

int TauHunterPPI::accelerate(SpaceLocation *source, double oangle, double vel, double omax_speed)
{
	 
	if ( !in_jump ) return Ship::accelerate(source, oangle, vel, omax_speed);
	return false;
}

void TauHunterPPI::animate(Frame* space)
{
	 
	if ( !in_jump ) Ship::animate(space);
}

double TauHunterPPI::isProtected() const
{
	 
	if ( in_jump )
		return 1.0;
	else   
		return Ship::isProtected();
}

double TauHunterPPI::isInvisible() const
{
	 
	if ( in_jump )
		return 1.0;
	else    
		return Ship::isInvisible();
}

void TauHunterPPI::calculate_hotspots()
{
	 
	if ( !in_jump ) Ship::calculate_hotspots();
}

void TauHunterPPI::calculate_turn_left()
{
	 
	//if ( exit_countdown <= 0 )
	if ( !in_jump ) Ship::calculate_turn_left();
}

void TauHunterPPI::calculate_turn_right()
{
	 
	//if ( exit_countdown <= 0 )
	if ( !in_jump ) Ship::calculate_turn_right();
}

Color TauHunterPPI::battPanelColor()
{
	Color c = {50,50,170};
	if ( in_jump ) return c;
	return Ship::battPanelColor();
}


TauHunterPPILaser::TauHunterPPILaser (double ox, double oy, double oangle, double ov, double orange, double olength, SpaceLocation *creator, double relativity, double odamage) :
		SpaceLine(creator, creator->normal_pos(), oangle, olength, makecol(255,255,255)),
		range(orange), d(0), v(ov)
{

	pos = normalize(pos + rotate(Vector2(-ox, oy), -PI/2+creator->get_angle()));

	damage_factor = odamage;
	vel = unit_vector(angle) + creator->get_vel() * relativity;
}

void TauHunterPPILaser::calculate()
{
	 
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

void TauHunterPPILaser::inflict_damage(SpaceObject *other)
{
	 
	if (d >= range) return;
	damage_factor *= pow((1-d/range), 0.2);
	SpaceLine::inflict_damage(other);
	state = 0;
}

REGISTER_SHIP(TauHunterPPI)
