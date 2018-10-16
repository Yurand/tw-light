/* $Id$ */ 
#include "../ship.h"
#include "../scp.h"
REGISTER_FILE

class KorvianSniper : public Ship {
public:
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	int cloak;
	int cloak_frame;
	int beep;

	public:
	static int cloak_color[3];
	KorvianSniper(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual double isInvisible() const;
	virtual int activate_weapon();
	virtual void calculate_fire_special();
	virtual void calculate_hotspots();
	virtual void calculate();
	virtual void animate(Frame *space);
};

class SniperMissile : public Missile {
public:
	public:
	SniperMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite);
};

KorvianSniper::KorvianSniper(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  beep = FALSE;

  cloak = TRUE;
}

double KorvianSniper::isInvisible() const {
		return 1.0;

	}

int KorvianSniper::activate_weapon() {
	// note that target=0 is only set after this routine is called in ship::calculate
	// so we need to check if it exists ...
	/*if (cloak && target && target->exists()) {
		if (distance(target) < weaponRange * 3) {
			angle = 
				intercept_angle2(pos, vel * 1.0, weaponVelocity, 
					target->normal_pos(), target->get_vel() );
			}
		else angle = trajectory_angle(target);
		}*/
	game->add(new SniperMissile( Vector2(0.0, size.y / 2.0),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	play_sound2(data->sampleWeapon[0]);
	/*game->add(new AnimatedShot(this, Vector2(0.0, size.y / 2.0),
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour, 
			this, data->spriteWeapon, 1, 12, 1.0));*/
	return(TRUE);
	}

void KorvianSniper::calculate_hotspots()
{
    //Ship::calculate_hotspots();
}

void KorvianSniper::calculate_fire_special()
{

	if(fire_special) {
	  special_low = FALSE;
	  			beep = TRUE;
	}

  if(fire_special) {
    if((batt < special_drain)) {
      special_low = TRUE;
      return;
    }
	if(abs(get_vel()) > 0.00001) {
		play_sound2(data->sampleSpecial[1]);
		change_vel(-get_vel());
    //if(special_recharge > 0)
      //return;
	
     batt -= special_drain;
	}
    special_recharge = special_rate;
  }
}


void KorvianSniper::calculate()
{
  double angle2;

  
  if(beep == TRUE) {
	  if(target && target->exists() && !target->isInvisible()) {
		angle2 = angle - intercept_angle2(pos, vel * 1.0, weaponVelocity,
					target->normal_pos(), target->get_vel() );
		Vector2 v = 350*Vector2(unit_vector(angle+PI/2));
		double w = v.dot(target->get_vel())/distance(target);
		if(w < 0.0) w = -w;
		if(w > angle2 && angle2 > -w) {
			play_sound2(data->sampleSpecial[0]);
			beep = FALSE;
		}
	  }
	}

  Ship::calculate();
}

void KorvianSniper::animate(Frame *space)
{
/*	if((cloak_frame > 0) && (cloak_frame < 300))
		sprite->animate_character( pos, sprite_index, 
				pallete_color[cloak_color[cloak_frame / 100]], space);
	else if ((cloak_frame >= 300)) {
		sprite->animate_character( pos, sprite_index, pallete_color[255], space);
		}
	else */
	//if (!show_red_cloaker || (control && game->is_bot(control->channel)) || (control && !game->is_local(control->channel)) || (!game_networked && num_network>1))	// bots and remote players are "hidden"
	if ((control && game->is_bot(control->channel)) || (control && !game->is_local(control->channel)))	// bots and remote players are "hidden"
		sprite->animate_character( pos, sprite_index, palette_color[255], space);
	else
		sprite->animate_character( pos, sprite_index, palette_color[4], space);
	//Ship::animate(space);		
	return;
	}

SniperMissile::SniperMissile(Vector2 opos, double oangle, double ov,
	int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite) 
	:
	Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship ,osprite) 
	{
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = explosionSprite->frames();
	explosionFrameSize  = 50;
}


REGISTER_SHIP(KorvianSniper)
