/* $Id$ */ 
#include "../ship.h"
#include "../scp.h"
REGISTER_FILE

class HeraldExterminator : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  int cloak;
  int cloak_frame;

  public:
	static int cloak_color[3];
  HeraldExterminator(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual double isInvisible() const;
  virtual int activate_weapon();
  virtual void calculate_fire_special();
  virtual void calculate_hotspots();
  virtual void calculate();
  virtual void animate(Frame *space);
};
int HeraldExterminator::cloak_color[3] = { 15, 11, 9 };

HeraldExterminator::HeraldExterminator(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  cloak = FALSE;
  cloak_frame = 0;
}

double HeraldExterminator::isInvisible() const {
	if(cloak_frame >= 300)
		return(1);
	return(false);
	}

int HeraldExterminator::activate_weapon()
{
  cloak = FALSE;
  add(new Missile( this,Vector2(0.0, (size.y / 2.0)),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  return(TRUE);
}

void HeraldExterminator::calculate_fire_special()
{
  special_low = FALSE;

  if(fire_special) {
    if((batt < special_drain) && (!cloak)) {
      special_low = TRUE;
      return;
    }

    if(special_recharge != 0)
      return;

    if(cloak) {
      cloak = FALSE;

      play_sound2(data->sampleSpecial[1]);
    } else {
      cloak = TRUE;
      play_sound2(data->sampleSpecial[0]);
      batt -= special_drain;
    }

    special_recharge = special_rate;
  }
}

void HeraldExterminator::calculate_hotspots()
{
  if(!cloak)
    Ship::calculate_hotspots();
}

void HeraldExterminator::calculate()
{
  if((cloak) && (cloak_frame < 300))
    cloak_frame += frame_time;
  if((!cloak) && (cloak_frame > 0))
    cloak_frame -= frame_time;

  Ship::calculate();
}

void HeraldExterminator::animate(Frame *space)
{
	if((cloak_frame > 0) && (cloak_frame < 300))
	{
		sprite->animate_character( pos, sprite_index, 
				pallete_color[cloak_color[cloak_frame / 100]], space);
	} else if ((cloak_frame >= 300))
	{
		//if (!show_red_cloaker || (control && game->is_bot(control->channel)) || (control && !game->is_local(control->channel)) || (!game_networked && num_network>1))	// bots and remote players are "hidden"
		if ((control && game->is_bot(control->channel)) || (control && !game->is_local(control->channel)))	// bots and remote players are "hidden"
			sprite->animate_character( pos, sprite_index, pallete_color[0], space);
		else
			sprite->animate_character( pos, sprite_index, pallete_color[4], space);
	}
  else Ship::animate(space);
	return;
	}



REGISTER_SHIP(HeraldExterminator)
