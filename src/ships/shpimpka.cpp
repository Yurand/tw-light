/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE
#include "../scp.h"

#define IMPERIAL_MIN_DEBRIS_LIFE 2000
#define IMPERIAL_MAX_DEBRIS_LIFE 6000
#define IMPERIAL_DEBRIS_SPEED    0.03

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class ImperialKatana : public Ship {
public:
// the ship

  double weaponRange;        // length of the blade
  int    weaponDamage;
  int    weaponRadHeat;      // heat above which radioactivity starts to build up
  double weaponRadBuildup;   // radioactivity buildup per second above weaponRadHeat

  double specialRange;
  double specialVelocity;
  double specialDamageRate;
  int    specialDamage;

  double rad;
  void show_panel( int r );  // shows proper radioactivity sign on the captain panel

  public:
  double residualDamage;

  ImperialKatana(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code );

  virtual int activate_weapon();         // creates the blade
  virtual int activate_special();        // ejects radioactivity

  virtual void calculate();              // these two functions handle the
  virtual void calculate_fire_weapon();  // reversed battery recharge

//  virtual void handle_damage( SpaceLocation* other );  // eject radioactivity when destroyed and radioactive

  virtual void animate( Frame *space );  // adds the glow to the handle
};

class ImperialBlade : public Laser {
public:
  // blade. a laser that handles non-integer damage and splits victim in two after kill

  double res;
  ImperialKatana* katana;
  
  public:
  ImperialBlade( ImperialKatana* creator, double langle, int lcolor, double lrange,
    double ldamage, int lfcount, SpaceLocation *opos, double rel_x, double rel_y,
    bool osinc_angle = false );

  virtual void collide( SpaceObject* other );
  virtual void inflict_damage( SpaceObject* other );
};

class ImperialRadioactivity : public SpaceObject {
public:
  // radioactivity -- either latched or floating free

  Ship* latched;
  double v;
  double range;
  double d;
  double damage_rate;
  double damage_step;
  int    damage_amount;

  int    sprite_step;
  int    sprite_dir;

  public:
  ImperialRadioactivity( SpaceLocation* creator, Vector2 opos,
    double ov, double oangle, SpaceSprite* osprite, double orange,
    double orate, int odamage );

  virtual void calculate();                           // damage object latched on
  virtual void animate( Frame* space );               // hide if victim is invisible
  virtual void inflict_damage( SpaceObject* other );  // infect and spread on contact
};

class ImperialHalfObject : public SpaceObject {
public:
// debris -- part of an object that was cut in two

  int life;

  public:
  ImperialHalfObject( SpaceLocation* creator, Vector2 opos,
    double ovx, double ovy, double oangle, int omass, SpaceSprite* osprite, int olife );

  virtual void calculate();  // explodes after its life time expired
};

ImperialKatana::ImperialKatana(Vector2 opos, double shipAngle,
  ShipData* shipData, unsigned int code ):
  Ship( opos, shipAngle, shipData, code ){
  
  weaponRange       = scale_range(get_config_float("Weapon", "Range", 0));
  weaponDamage      = get_config_int("Weapon", "Damage", 0);
  weaponRadHeat     = get_config_int("Weapon", "RadHeat", 0);
  weaponRadBuildup  = get_config_float("Weapon", "RadBuildup", 1);

  specialRange      = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity   = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialDamageRate = get_config_float("Special", "DamageSeconds", 0) * 1000;
  specialDamage     = get_config_int("Special", "Damage", 0);

  residualDamage = 0;
  rad = 0;
}

int ImperialKatana::activate_weapon(){
	STACKTRACE
  double rate = (double)batt/(double)batt_max;
  double damage = weaponDamage * rate * rate;

  if( (int)(HOT_COLORS * (1.0 - rate)) == 0 ){
    /* if we are close to blowing up, let the laser be white */
    game->add( new ImperialBlade(this, angle, palette_color[15],
      weaponRange, damage, weapon_rate, this, 0.0, 25.0, true ));
  }else{
    /* otherwise let it be a shade of red or yellow, as defined in the hot_color array */
    game->add( new ImperialBlade(this, angle,
		palette_color[hot_color[(int)(HOT_COLORS*(1.0 - rate)) - 1]],
      weaponRange, damage, weapon_rate, this, 0.0, 25.0, true ));
  }
  return TRUE;
}

int ImperialKatana::activate_special(){
	STACKTRACE

  bool found = false;
  Query q;
  for( q.begin( this, OBJECT_LAYERS, size.x ); q.currento; q.next() ){
    if( q.currento->get_sprite() == data->spriteSpecialExplosion && q.currento->ship == this ){
      q.currento->state = 0;
      if( found ) continue;
      found = true;
      blit( data->spritePanel->get_bitmap(0), spritePanel->get_bitmap(0),
        16, 18, 16, 18, 32, 30 );
      update_panel = TRUE;
      rad = 0;
      show_panel( (int)rad );

      if( batt ){
		  damage(this, batt / 4);


	int i = iround(batt / 8);
	if(i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
	play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
        game->add(new Animation(this, pos,
          meleedata.kaboomSprite, 0, KABOOM_FRAMES, time_ratio, LAYER_EXPLOSIONS));
      }else{
        for( double alpha = angle + 60; alpha <= angle + 300; alpha += 40 ){
          game->add( new ImperialRadioactivity( this,
			  pos + unit_vector(alpha) * data->spriteWeaponExplosion->size().x / 2,
				specialVelocity, alpha, data->spriteWeaponExplosion, specialRange, specialDamageRate, specialDamage ));
        }
      }
    }
  }
  q.end();

  return found;
}

void ImperialKatana::calculate_fire_weapon() {
	STACKTRACE
	weapon_low = FALSE;

	/* the blade is always visible */
	activate_weapon();
	if (fire_weapon) {
		/* heat is generated only when fire_weapon is pressed */
		if (batt+weapon_drain > batt_max) {
			/*  if above maximum we blow up */
			weapon_low = true;
			damage(this, crew_max);
			return;
			}

		if (weapon_recharge > 0)
				return;

		batt += weapon_drain;
		if (recharge_amount > 1) recharge_step = recharge_rate;
		weapon_recharge += weapon_rate;

		play_sound2(data->sampleWeapon[weapon_sample], 255, iround(batt * 20) );
		}
	return;
	}

void ImperialKatana::calculate()
{
	STACKTRACE
  /* the battery discharges with time */
  if(batt > 0) {
    recharge_step -= frame_time;
    if(recharge_step < 0) {
      batt -= recharge_amount;
			if (batt < 0) batt = 0;
      recharge_step += recharge_rate;
    }
  }
  /* we would like to skip the original code */
  int real_recharge_step = recharge_step;
  recharge_step = frame_time;
  Ship::calculate();
  recharge_step = real_recharge_step;

  if( batt > weaponRadHeat ){
//    double up = (double)(batt - weaponRadHeat) * frame_time / 1000.0;
    double up = weaponRadBuildup * (double)frame_time / 1000.0;
    rad += up;
    if( (rad - up < 1.0 && rad >= 1.0) || (rad - up < 2.0 && rad >= 2.0) ){
      show_panel( (int)rad );
      play_sound2( data->sampleSpecial[1] );
    }
  }
  bool raded = false;
  Query q;
  for( q.begin( this, OBJECT_LAYERS, size.x ); q.currento; q.next() ){
    if( q.currento->get_sprite() == data->spriteSpecialExplosion && q.currento->ship == this ){
      raded = true;
    }
  }

  if( !raded && rad >= 3 ){
    show_panel( (int)rad );
    SpaceObject* r = new ImperialRadioactivity( this, pos, specialVelocity, 0, sprite, specialRange, specialDamageRate, specialDamage );
    r->inflict_damage( this );
    game->add( r );
    play_sound2( data->sampleSpecial[1] );
  }
}

void ImperialKatana::show_panel( int r ){
	STACKTRACE
  if( r > 2 ) r = 2;
  spritePanel->overlay( 1, 7 + r, spritePanel->get_bitmap( 2 ));
  spritePanel->overlay( 1, 7 + r, spritePanel->get_bitmap( 3 ));
  spritePanel->overlay( 1, 7 + r, spritePanel->get_bitmap( 4 ));
  spritePanel->overlay( 1, 7 + r, spritePanel->get_bitmap( 5 ));
  spritePanel->overlay( 1, 7 + r, spritePanel->get_bitmap( 1 ));
    
  update_panel = true;
}

void ImperialKatana::animate(Frame *space){
	STACKTRACE

  Ship::animate(space);

  if( (int)(HOT_COLORS*(1.0-(double)batt/(double)batt_max)) == 0 ){
    /* if we are close to blowing up, let the glow be white */
    data->spriteWeapon->animate_character( pos, sprite_index, palette_color[15], space );
  }else{
    /* otherwise let it be a shade of red or yellow, as defined in the hot_color array */
    data->spriteWeapon->animate_character( pos, sprite_index,
		palette_color[hot_color[(int)(HOT_COLORS*(1.0-(double)batt/(double)batt_max))-1]],
    space );
  }

};

ImperialRadioactivity::ImperialRadioactivity( SpaceLocation* creator, Vector2 opos,
    double ov, double oangle, SpaceSprite* osprite, double orange, double orate, int odamage ):
SpaceObject( creator, opos, oangle, osprite ), latched( NULL ), v( ov ), range( orange ),
  d( 0 ), damage_rate( orate ), damage_step( orate ), damage_amount( odamage ),
  sprite_step( time_ratio * 3 ), sprite_dir( 1 )
{
	layer = LAYER_SPECIAL;

  collide_flag_sameship = 0;
  collide_flag_sameteam = collide_flag_anyone = bit(LAYER_SHIPS);
  vel = v * unit_vector(angle);
	attributes &= ~ATTRIB_STANDARD_INDEX;
}

void ImperialRadioactivity::calculate(){
	STACKTRACE
  SpaceObject::calculate();

  if( !latched ){
    d += v * frame_time;
    if( range <= d ) state = 0;
    else{
      sprite_index = get_index(angle);
      sprite_index += ((int)(14.0*d/range)) * 64;
    }
    return;
  }

  pos = latched->normal_pos();
  vel = latched->get_vel();

  if( !(latched && latched->exists()) ){
    state = 0;

    for( double alpha = latched->get_angle(); alpha < latched->get_angle() + PI2; alpha += 40 ){
      game->add( new ImperialRadioactivity( this,
		  pos + unit_vector(alpha) * data->spriteWeaponExplosion->size().x / 2,
        v, alpha, data->spriteWeaponExplosion, range, damage_rate, damage_amount ));
    }

	latched = 0;
    return;
  }


  damage_step -= frame_time;
  if( damage_step < 0 ){
	damage(latched, damage_amount);
    damage_step += damage_rate;
  }

  sprite_step -= frame_time;
  if( sprite_step < 0 ){
    sprite_step += time_ratio * 3;
    sprite_index += sprite_dir;
    if( sprite_index == 10 ){
      sprite_index = 8;
      sprite_dir = -1;
    }else if( sprite_index == -1 ){
      sprite_index = 1;
      sprite_dir = 1;
    }
  }
}

void ImperialRadioactivity::animate( Frame* space ){
	STACKTRACE
  if( latched ) if( latched->isInvisible() ) return;
  SpaceObject::animate( space );
}

void ImperialRadioactivity::inflict_damage( SpaceObject* other ){
	STACKTRACE
  if( !other->isShip() ) return;

  Query q;
  for( q.begin( other, OBJECT_LAYERS, size.x ); q.currento; q.next() ){
    if( q.currento->get_sprite() == data->spriteSpecialExplosion && q.currento->ship == other ){
      q.end();
      return;
    }
  }
  q.end();

  play_sound2( data->sampleExtra[0] );
  if( !latched ){
    latched = (Ship*)other;
    sprite = data->spriteSpecialExplosion;
    change_owner( latched );
	if (latched->spritePanel)
	{
		data->spriteSpecial->lock();
		draw_sprite( latched->spritePanel->get_bitmap(0), data->spriteSpecial->get_bitmap(0), 16, 18 );
		data->spriteSpecial->unlock();
		latched->update_panel = true;
	}
    sprite_index = 0;
  }else{
    SpaceObject* r = new ImperialRadioactivity( this, pos, v, 0,
      data->spriteWeaponExplosion, range, damage_rate, damage_amount );
    r->inflict_damage( other );
    game->add( r );
  }
}

ImperialBlade::ImperialBlade( ImperialKatana *creator, double langle, int lcolor,
  double lrange, double ldamage, int lfcount, SpaceLocation *opos, double rel_x,
  double rel_y, bool osinc_angle ):
Laser( creator, langle, lcolor, lrange, (int)ldamage, lfcount, opos, Vector2(rel_x,rel_y), osinc_angle ),
  res( ldamage - (int)ldamage ), katana( creator ){
}

void ImperialBlade::collide( SpaceObject* other ){
	STACKTRACE
  double old_length = length;
  SpaceLine::collide( other );
  length = old_length;
}

void ImperialBlade::inflict_damage( SpaceObject *other ){
	STACKTRACE
  if( !other->exists() ) return;

  katana->residualDamage += res;
  if( katana->residualDamage >= 1 ){
    katana->residualDamage -= 1;
    damage_factor++;
    Laser::inflict_damage( other );
    damage_factor--; }

  //if( !other->isShip() || other->exists() ) return;
  if ( other->isShip() && ((Ship*)other)->crew > 0.1 )	// it's not dead
	  return;

  if ( damage_factor <= -1 )
	  return;

  if ( !other->isShip() )
	  return;	// cannot be split.

  // slice code starts here

  int si = other->get_sprite_index();
  BITMAP* bmp = other->get_sprite()->get_bitmap( si );
  BITMAP* tmp;
  int mcol = bitmap_mask_color( bmp );

  // skip second gamma correction
 // int gc = gamma_correction;
 // gamma_correction = 0;


  // determining the position of the cutting points starts here
  double tx = cos( angle );
  double ty = sin( angle );
  double lx = normal_pos().x;
  double ly = normal_pos().y;
  double sx = lx - min_delta( lx, other->normal_pos().x, map_size.x );
  double sy = ly - min_delta( ly, other->normal_pos().y, map_size.y );
  double rw = bmp->w;
  double rh = bmp->h;
  double rx = sx - (rw / 2);
  double ry = sy - (rh / 2);
  double zx = lx - rx;
  double zy = ly - ry;

  int x1 = 0;
  if( ty ) x1 = (int)( zx - tx * zy / ty );
  int y1 = 0;
  if( tx ) y1 = (int)( zy - ty * zx / tx );
  int x2 = 0;
  if( ty ) x2 = (int)( zx - tx * (zy - rw) / ty );
  int y2 = 0;
  if( tx ) y2 = (int)( zy - ty * (zx - rh) / tx );
  if( x1 < 0 || x1 > bmp->w ) x1 = 0;
  if( y1 < 0 || y1 > bmp->h ) y1 = 0;
  if( x2 < 0 || x2 > bmp->w ) x2 = 0;
  if( y2 < 0 || y2 > bmp->h ) y2 = 0;
  if( !( x1 || y1 || x2 || y2 )) return;
  // determining the position of the cutting points ends here


  TW_DATAFILE* image = new TW_DATAFILE;
  image->type = DAT_RLE_SPRITE;


  // creating the left sprite
  tmp = create_bitmap_ex( bitmap_color_depth( bmp ), bmp->w, bmp->h );
  blit( bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h );

  if( x1 && y1 ){
    int v[] = { x1, 0, bmp->w, 0, bmp->w, bmp->h, 0, bmp->h, 0, y1 };
    polygon( tmp, 5, v, mcol );
  }else if( x1 && x2 ){
    int v[] = { x1, 0, bmp->w, 0, bmp->w, bmp->h, x2, bmp->h };
    polygon( tmp, 4, v, mcol );
  }else if( x1 && y2 ){
    int v[] = { x1, 0, bmp->w, 0, bmp->w, y2 };
    polygon( tmp, 3, v, mcol );
  }else if( y1 && x2 ){
    int v[] = { 0, 0, bmp->w, 0, bmp->w, bmp->h, x2, bmp->h, 0, y1 };
    polygon( tmp, 5, v, mcol );
  }else if( y1 && y2 ){
    int v[] = { 0, y1, bmp->w, y2, bmp->w, bmp->h, 0, bmp->h };
    polygon( tmp, 4, v, mcol );
  }else if( x2 && y2 ){
    int v[] = { bmp->w, y2, bmp->w, bmp->h, x2, bmp->h };
    polygon( tmp, 3, v, mcol );
  }

  image->dat = get_rle_sprite( tmp );
  SpaceSprite* spriteLeft = new SpaceSprite( image, 1 );
  destroy_bitmap( tmp );


  // creating the right sprite
  tmp = create_bitmap_ex( bitmap_color_depth( bmp ), bmp->w, bmp->h );
  blit( bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h );

  if( x1 && y1 ){
    int v[] = { 0, 0, x1, 0, 0, y1 };
    polygon( tmp, 3, v, mcol );
  }else if( x1 && x2 ){
    int v[] = { 0, 0, x1, 0, x2, bmp->h, 0, bmp->h };
    polygon( tmp, 4, v, mcol );
  }else if( x1 && y2 ){
    int v[] = { 0, 0, x1, 0, bmp->w, y2, bmp->w, bmp->h, 0, bmp->h };
    polygon( tmp, 5, v, mcol );
  }else if( y1 && x2 ){
    int v[] = { 0, y1, x2, bmp->h, 0, bmp->h };
    polygon( tmp, 3, v, mcol );
  }else if( y1 && y2 ){
    int v[] = { 0, 0, bmp->w, 0, bmp->w, y2, 0, y1 };
    polygon( tmp, 4, v, mcol );
  }else if( x2 && y2 ){
    int v[] = { 0, 0, bmp->w, 0, bmp->w, y2, x2, bmp->h, 0, bmp->h };
    polygon( tmp, 5, v, mcol );
  }

  image->dat = get_rle_sprite( tmp );
  SpaceSprite* spriteRight = new SpaceSprite( image, 1 );
  destroy_bitmap( tmp );

//  gamma_correction = gc;
  delete image;

  if( zy < 0 || (zy == 0 && zx < 0) ){
    ty *= -1;
    tx *= -1;
  }
	

  // adding the objects to the game world
  ImperialHalfObject *left = new ImperialHalfObject( other, other->normal_pos(),
    other->get_vel().x + ty * IMPERIAL_DEBRIS_SPEED,
    other->get_vel().y - tx * IMPERIAL_DEBRIS_SPEED, other->get_angle(), iround(other->mass),
    spriteLeft,
    iround(IMPERIAL_MIN_DEBRIS_LIFE + tw_random(IMPERIAL_MAX_DEBRIS_LIFE -
    IMPERIAL_MIN_DEBRIS_LIFE)) );
  game->add( left );

  ImperialHalfObject *right = new ImperialHalfObject( other, other->normal_pos(),
    other->get_vel().x - ty * IMPERIAL_DEBRIS_SPEED,
    other->get_vel().y + tx * IMPERIAL_DEBRIS_SPEED, other->get_angle(), iround(other->mass),
    spriteRight,
    iround(IMPERIAL_MIN_DEBRIS_LIFE + random(IMPERIAL_MAX_DEBRIS_LIFE -
    IMPERIAL_MIN_DEBRIS_LIFE)) );
  game->add( right );
  // slice code ends here
}

ImperialHalfObject::ImperialHalfObject( SpaceLocation* creator, Vector2 opos,
    double ovx, double ovy, double oangle, int omass, SpaceSprite* osprite, int olife ):
SpaceObject( creator, opos, oangle, osprite ), life( olife )
{
  collide_flag_sameship = collide_flag_sameteam = collide_flag_anyone = ALL_LAYERS;
  vel = Vector2(ovx,ovy);
  mass = omass;
  layer = LAYER_SHIPS;
}

void ImperialHalfObject::calculate(){
	STACKTRACE
  SpaceObject::calculate();
  life -= frame_time;
  if( life <=0 ){
    state = 0;
    play_sound( (SAMPLE *)(melee[MELEE_BOOMSHIP].dat), 100 );
    game->add( new Animation(this, pos,
      meleedata.kaboomSprite, 0, KABOOM_FRAMES, time_ratio, LAYER_EXPLOSIONS ));
  }
}

REGISTER_SHIP(ImperialKatana)
