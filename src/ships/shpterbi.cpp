/* $Id: shpterbi.cpp,v 1.7 2004/03/24 23:51:42 yurand Exp $ */
#include "../ship.h"
REGISTER_FILE
#include <stdio.h>
#include "../melee/mview.h"
#include "../ais.h"

//#define TERON_COLLISION_FORWARDING
#define TERON_TURRET_CONTROLLABLE
#define TERON_SHIPS_TARGETABLE

#define TERON_BUILDER_SPRITE         spriteShip
#define TERON_BUILDER_INDEX          0
#define TERON_DRONE_SPRITE           spriteWeapon
#define TERON_DRONE_INDEX            0
#define TERON_DRONE_BUILD_SPRITE     spriteSpecial
#define TERON_DRONE_BUILD_INDEX      64
#define TERON_FIGHTER_SPRITE         spriteSpecial
#define TERON_FIGHTER_INDEX          0
#define TERON_FIGHTER_BUILD_SPRITE   spriteSpecial
#define TERON_FIGHTER_BUILD_INDEX    64
#define TERON_FIGHTER_SHOT_SPRITE    spriteExtra
#define TERON_FIGHTER_SHOT_INDEX     0
#define TERON_TURRET_BUILD_SPRITE    spriteWeaponExplosion
#define TERON_TURRET_BUILD_INDEX     64
#define TERON_TURRET_SPRITE          spriteWeaponExplosion
#define TERON_TURRET_INDEX           0
#define TERON_TURRET_BASE_SPRITE     spriteWeaponExplosion
#define TERON_TURRET_BASE_INDEX      80
#define TERON_TURRET_SHOT_SPRITE     spriteSpecialExplosion
#define TERON_TURRET_SHOT_INDEX      0
#define TERON_SELECTION_SPRITE       spriteExtraExplosion
#define TERON_SELECTION_INDEX        0

#define TERON_SELECTION_ANIM_RATE    time_ratio
#define TERON_TURRET_DOCK_COUNTER    time_ratio
#define TERON_DRONE_BEAM_COLOR       12
#define TERON_DRONE_SIGHT_RANGE      map_size.x
#define TERON_COLLECT_BEAM_COLOR     15
#define TERON_COLLECT_BEAM_DAMAGE    1
#define TERON_BUILDER_COLLECT_RATE   800
#define TERON_BUILDER_DOCKS          4
#define TERON_BUILDER_OPTIONS        6

typedef struct
{
	double dist;
	double dir;
	double angle;
	bool   avail;
} teron_dock;

typedef struct
{
	char phrase[40];
} teron_option;

class TeronDrone;

class TeronBuilder : public Ship
{
	SpaceObject* collecting;
	int          collect_step;
	double       collectRange;
	int          asteroid_value;

	teron_option option[TERON_BUILDER_OPTIONS];
	enum {
		option_build_drone,
		option_build_turret,
		option_build_fighter,
		option_order_assist_building,
		option_order_collect_resources,
		option_order_return_to_dock
	};
	int          current_option;
	TeronDrone*  current_drone;
	bool         select_all;
	int          selection_sprite_index;
	int          selection_sprite_step;
	double       selectRange;

	public:
		teron_dock   dock[TERON_BUILDER_DOCKS];

		int    drone_cost;
		int    drone_asteroid_value;
		int    drone_crew_max;
		int    drone_batt_max;
		double drone_turn_rate;
		double drone_speed_max;
		double drone_accel_rate;
		int    drone_recharge_amount;
		int    drone_recharge_rate;
		int    drone_weapon_drain;
		int    drone_weapon_rate;
		int    drone_special_drain;
		int    drone_special_rate;
		int    drone_hotspot_rate;
		int    drone_mass;

		double droneWeaponRange;

		int    fighter_cost;
		int    fighter_crew_max;
		int    fighter_batt_max;
		double fighter_turn_rate;
		double fighter_speed_max;
		double fighter_accel_rate;
		int    fighter_recharge_amount;
		int    fighter_recharge_rate;
		int    fighter_weapon_drain;
		int    fighter_weapon_rate;
		int    fighter_special_drain;
		int    fighter_special_rate;
		int    fighter_hotspot_rate;
		int    fighter_mass;

		double fighterWeaponRange;
		double fighterWeaponVelocity;
		int    fighterWeaponDamage;
		int    fighterWeaponArmour;

		int    turret_cost;
		int    turret_crew_max;
		int    turret_batt_max;
		double turret_turn_rate;
		int    turret_recharge_amount;
		int    turret_recharge_rate;
		int    turret_weapon_drain;
		int    turret_weapon_rate;
		int    turret_special_drain;
		int    turret_special_rate;
		int    turret_mass;

		double turretWeaponRange;
		double turretWeaponVelocity;
		int    turretWeaponDamage;
		int    turretWeaponArmour;

		double attached_turret_turn_rate;
		int    attached_turret_recharge_amount;
		int    attached_turret_recharge_rate;
		int    attached_turret_weapon_drain;
		int    attached_turret_weapon_rate;
		int    attached_turret_special_drain;
		int    attached_turret_special_rate;

		double attachedTurretWeaponRange;
		double attachedTurretWeaponVelocity;
		int    attachedTurretWeaponDamage;
		int    attachedTurretWeaponArmour;

		TeronBuilder( Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code );

		virtual int  activate_weapon();
		virtual int  activate_special();
		virtual void animate( Frame* space );
		virtual void calculate();
		virtual void calculate_thrust();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_fire_weapon();
		virtual void calculate_fire_special();
		virtual void materialize();

		void request_clearance( TeronDrone* drone );
};

class TeronBuildPlatform : public SpaceObject
{
	int index_zero;
	Ship* new_ship;

	public:
		TeronBuildPlatform( Ship *creator, Vector2 opos, SpaceSprite *osprite, int osprite_index );

		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		//  virtual void handle_damage( SpaceLocation* other );
		virtual void death();
};

class TeronDrone : public Ship
{
	#ifdef TERON_COLLISION_FORWARDING
	double cvx, cvy, cx, cy;	 // for forwarding collisions
	#endif
	double weaponRange;

	double dock_dir;
	double dock_dist;
	double dock_angle;
	int    dock_number;
	double orig_mass;

	bool   just_docked;

	public:
		int    asteroid_value;
		int    resource;

		TeronBuilder* docked;

		enum {
			build,
			dock,
			collect
		}       goal;

		TeronDrone( TeronBuilder *creator, Vector2 opos,
			double shipAngle, SpaceSprite *osprite );

		virtual int  activate_weapon();
		virtual void calculate();
		virtual int  canCollide( SpaceLocation* other );
		virtual void collide( SpaceObject* other );
		virtual void inflict_damage( SpaceObject* other );

		virtual TeronBuildPlatform* get_build_platform();

		void assist_building();
		void collect_resources();
		void return_to_dock();
		void dock_clearance_to( int dock_number );

	private:
		void roger();
		void leave_dock();
};

class TeronDroneLaser : public Laser
{
	SpaceObject* collecting;
	TeronDrone*  drone;

	public:
		TeronDroneLaser( TeronDrone *creator, double langle, int lcolor, double lrange, int ldamage, int lfcount,
			SpaceLocation *opos, Vector2 rel_pos, bool osinc_angle=false );

		virtual void calculate();
		virtual void inflict_damage( SpaceObject *other );
};

class TeronShipController : public ControlWussie
{
	public:
		TeronShipController( const char *name, int channel );

		virtual void animate( Frame* space );
};

class TeronDroneController : public TeronShipController
{
	public:
		TeronDroneController( const char *name, int channel );

		virtual void target_stuff();
};

class TeronFighter : public Ship
{
	double weaponRange;
	double weaponVelocity;
	int    weaponDamage;
	int    weaponArmour;

	public:
		TeronFighter( TeronBuilder *creator, Vector2 opos,
			double shipAngle, SpaceSprite *osprite );

		virtual int  activate_weapon();
		virtual void calculate_thrust();
		virtual void calculate_hotspots();

		virtual TeronBuildPlatform* get_build_platform();
};

class TeronTurret : public Ship
{
	#ifdef TERON_TURRET_CONTROLLABLE
	TeronBuilder* docked;
	double dock_dir;
	double dock_dist;
	double dock_counter;
	Control* orig_control;
	#endif

	SpaceObject* base;
	double weaponRange;
	double weaponVelocity;
	int    weaponDamage;
	int    weaponArmour;

	public:
		TeronTurret( TeronBuilder *creator, Vector2 opos,
			double shipAngle, SpaceSprite *osprite );

		virtual int  activate_weapon();
		virtual int  activate_special();
		virtual void calculate();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_thrust();
		virtual void calculate_hotspots();
		virtual void inflict_damage( SpaceObject* other );
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		//  virtual void handle_damage( SpaceLocation* other );
		virtual void materialize();

		virtual TeronBuildPlatform* get_build_platform();
};

class TeronTurretBase : public SpaceObject
{
	#ifdef TERON_COLLISION_FORWARDING
	double cvx, cvy, cx, cy;	 // for forwarding collisions
	#endif

	public:
		TeronTurretBase( TeronTurret *creator, Vector2 opos,
			double oangle, SpaceSprite *osprite );

		virtual void calculate();
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual int handle_fuel_sap(SpaceLocation *source, double normal);
		virtual double handle_speed_loss(SpaceLocation *source, double normal);
		//  virtual void handle_damage( SpaceLocation* other );
		virtual int  canCollide( SpaceLocation* other );
		virtual void collide( SpaceObject* other );
};

TeronBuilder::TeronBuilder( Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code ):
Ship( opos, shipAngle, shipData, code )
{
	STACKTRACE;
	collect_step = 0;
	collecting = NULL;
	collectRange = scale_range( get_config_float( "Extra", "CollectRange", 0 ));
	asteroid_value = get_config_int("Extra", "AsteroidValue", 0);

	drone_cost     = get_config_int("Drone", "Cost", 0);
	drone_asteroid_value = get_config_int("Drone", "AsteroidValue", 0);
	drone_crew_max = get_config_int("Drone", "CrewMax", 0);
	drone_batt_max = get_config_int("Drone", "BattMax", 0);
	drone_recharge_amount  = get_config_int("Drone", "RechargeAmount", 0);
	drone_recharge_rate    = scale_frames(get_config_float("Drone", "RechargeRate", 0));
	drone_weapon_drain     = get_config_int("Drone", "WeaponDrain", 0);
	drone_weapon_rate      = scale_frames(get_config_float("Drone", "WeaponRate", 0));
	drone_special_drain    = get_config_int("Drone", "SpecialDrain", 0);
	drone_special_rate     = scale_frames(get_config_float("Drone", "SpecialRate", 0));
	double drone_raw_hotspot_rate = get_config_float("Drone", "HotspotRate", 0);
	drone_hotspot_rate  = scale_frames(drone_raw_hotspot_rate);
	drone_turn_rate     = scale_turning(get_config_float("Drone", "TurnRate", 0));
	drone_speed_max     = scale_velocity(get_config_float("Drone", "SpeedMax", 0));
	drone_accel_rate    = scale_acceleration(get_config_float("Drone", "AccelRate", 0), drone_raw_hotspot_rate);
	drone_mass          = get_config_int("Drone", "Mass", 0);

	droneWeaponRange = scale_range( get_config_float( "Drone", "WeaponRange", 0 ));

	fighter_cost     = get_config_int("Fighter", "Cost", 0);
	fighter_crew_max = get_config_int("Fighter", "CrewMax", 0);
	fighter_batt_max = get_config_int("Fighter", "BattMax", 0);
	fighter_recharge_amount  = get_config_int("Fighter", "RechargeAmount", 0);
	fighter_recharge_rate    = scale_frames(get_config_float("Fighter", "RechargeRate", 0));
	fighter_weapon_drain     = get_config_int("Fighter", "WeaponDrain", 0);
	fighter_weapon_rate      = scale_frames(get_config_float("Fighter", "WeaponRate", 0));
	fighter_special_drain    = get_config_int("Fighter", "SpecialDrain", 0);
	fighter_special_rate     = scale_frames(get_config_float("Fighter", "SpecialRate", 0));
	double fighter_raw_hotspot_rate = get_config_float("Fighter", "HotspotRate", 0);
	fighter_hotspot_rate  = scale_frames(fighter_raw_hotspot_rate);
	fighter_turn_rate     = scale_turning(get_config_float("Fighter", "TurnRate", 0));
	fighter_speed_max     = scale_velocity(get_config_float("Fighter", "SpeedMax", 0));
	fighter_accel_rate    = scale_acceleration(get_config_float("Fighter", "AccelRate", 0), fighter_raw_hotspot_rate);
	fighter_mass          = get_config_int("Fighter", "Mass", 0);

	fighterWeaponRange = scale_range( get_config_float( "Fighter", "WeaponRange", 0 ));
	fighterWeaponVelocity = scale_velocity( get_config_float( "Fighter", "WeaponVelocity", 0 ));
	fighterWeaponDamage = get_config_int( "Fighter", "WeaponDamage", 0 );
	fighterWeaponArmour = get_config_int( "Fighter", "WeaponArmour", 0 );

	turret_cost     = get_config_int("Turret", "Cost", 0);
	turret_crew_max = get_config_int("Turret", "CrewMax", 0);
	turret_batt_max = get_config_int("Turret", "BattMax", 0);
	turret_recharge_amount  = get_config_int("Turret", "RechargeAmount", 0);
	turret_recharge_rate    = scale_frames(get_config_float("Turret", "RechargeRate", 0));
	turret_weapon_drain     = get_config_int("Turret", "WeaponDrain", 0);
	turret_weapon_rate      = scale_frames(get_config_float("Turret", "WeaponRate", 0));
	turret_special_drain    = get_config_int("Turret", "SpecialDrain", 0);
	turret_special_rate     = scale_frames(get_config_float("Turret", "SpecialRate", 0));
	turret_turn_rate        = scale_turning(get_config_float("Turret", "TurnRate", 0));
	turret_mass             = get_config_int("Turret", "Mass", 0);

	turretWeaponRange = scale_range( get_config_float( "Turret", "WeaponRange", 0 ));
	turretWeaponVelocity = scale_velocity( get_config_float( "Turret", "WeaponVelocity", 0 ));
	turretWeaponDamage = get_config_int( "Turret", "WeaponDamage", 0 );
	turretWeaponArmour = get_config_int( "Turret", "WeaponArmour", 0 );

	attached_turret_recharge_amount  = get_config_int("TurretAttached", "RechargeAmount", 0);
	attached_turret_recharge_rate    = scale_frames(get_config_float("TurretAttached", "RechargeRate", 0));
	attached_turret_weapon_drain     = get_config_int("TurretAttached", "WeaponDrain", 0);
	attached_turret_weapon_rate      = scale_frames(get_config_float("TurretAttached", "WeaponRate", 0));
	attached_turret_special_drain    = get_config_int("TurretAttached", "SpecialDrain", 0);
	attached_turret_special_rate     = scale_frames(get_config_float("TurretAttached", "SpecialRate", 0));
	attached_turret_turn_rate        = scale_turning(get_config_float("TurretAttached", "TurnRate", 0));

	attachedTurretWeaponRange = scale_range( get_config_float( "TurretAttached", "WeaponRange", 0 ));
	attachedTurretWeaponVelocity = scale_velocity( get_config_float( "TurretAttached", "WeaponVelocity", 0 ));
	attachedTurretWeaponDamage = get_config_int( "TurretAttached", "WeaponDamage", 0 );
	attachedTurretWeaponArmour = get_config_int( "TurretAttached", "WeaponArmour", 0 );

	dock[0].dist  = 25;
	dock[0].dir   = 75 * ANGLE_RATIO;
	dock[0].angle = 80 * ANGLE_RATIO;
	dock[0].avail = true;
	dock[1].dist  = 25;
	dock[1].dir   = 115 * ANGLE_RATIO;
	dock[1].angle = 90 * ANGLE_RATIO;
	dock[1].avail = true;
	dock[2].dist  = 25;
	dock[2].dir   = 245 * ANGLE_RATIO;
	dock[2].angle = 270 * ANGLE_RATIO;
	dock[2].avail = true;
	dock[3].dist  = 25;
	dock[3].dir   = 295 * ANGLE_RATIO;
	dock[3].angle = 270 * ANGLE_RATIO;
	dock[3].avail = true;

	int i;
	for ( i = 0; i < 4; ++i )
		dock[i].dist *= 2;

	sprintf( option[option_build_drone].phrase, "Initiate Drone Construction" );
	sprintf( option[option_build_turret].phrase, "Initiate Turret Construction" );
	sprintf( option[option_build_fighter].phrase, "Initiate Fighter Construction" );
	sprintf( option[option_order_assist_building].phrase, "Order Drone to Assist Building" );
	sprintf( option[option_order_collect_resources].phrase, "Order Drone to Collect Resources" );
	sprintf( option[option_order_return_to_dock].phrase, "Order Drone to Return to Dock" );
	current_option = option_order_return_to_dock;
	current_drone  = NULL;
	selection_sprite_index = 0;
	selection_sprite_step = 0;
	select_all = false;

	selectRange = scale_range( get_config_float( "Extra", "Range", 0 ));

	select_all = true;
	current_option = option_order_collect_resources;
}


int TeronBuilder::activate_weapon()
{
	STACKTRACE;
	return false;
}


int TeronBuilder::activate_special()
{
	STACKTRACE;

	if ( current_option == option_build_drone ) {
		if ( batt < drone_cost ) {
			special_low = true;
			return false;
		} else
		batt -= drone_cost;

		TeronDrone* drone = new TeronDrone( this,
		//x+100*cos(angle*ANGLE_RATIO), y+100*sin(angle*ANGLE_RATIO),
			pos + 100 * unit_vector(angle),
			angle, data->TERON_DRONE_SPRITE );
		TeronBuildPlatform* platform = drone->get_build_platform();
		game->add( platform );
		#ifdef TERON_SHIPS_TARGETABLE
		target->add( platform );
		#endif

	}
	else if ( current_option == option_build_turret ) {
		if ( batt < turret_cost ) {
			special_low = true;
			return false;
		}else batt -= turret_cost;
		TeronTurret* turret = new TeronTurret( this,
		//x+100*cos(angle*ANGLE_RATIO), y+100*sin(angle*ANGLE_RATIO),
			pos + 100 * unit_vector(angle),
			angle, data->TERON_TURRET_SPRITE );
		TeronBuildPlatform* platform = turret->get_build_platform();
		game->add( platform );
		#ifdef TERON_SHIPS_TARGETABLE
		target->add( platform );
		#endif

	}
	else if ( current_option == option_build_fighter ) {
		if ( batt < fighter_cost ) {
			special_low = true;
			return false;
		}else batt -= fighter_cost;
		TeronFighter* fighter = new TeronFighter( this,
		//x+100*cos(angle*ANGLE_RATIO), y+100*sin(angle*ANGLE_RATIO),
			pos + 100 * unit_vector(angle),
			angle, data->TERON_FIGHTER_SPRITE );
		TeronBuildPlatform* platform = fighter->get_build_platform();
		game->add( platform );
		#ifdef TERON_SHIPS_TARGETABLE
		target->add( platform );
		#endif

	}
	else if ( !select_all && current_option == option_order_assist_building ) {
		if ( current_drone ) current_drone->assist_building();

	}
	else if ( !select_all && current_option == option_order_collect_resources ) {
		if ( current_drone ) current_drone->collect_resources();

	}
	else if ( !select_all && current_option == option_order_return_to_dock ) {
		if ( current_drone ) current_drone->return_to_dock();

	} else {
		Query q;
		for( q.begin( this, bit(LAYER_SHIPS), selectRange ); q.currento; q.next() ) {
			if ( sameShip( q.currento ) && q.currento->get_sprite() == data->TERON_DRONE_SPRITE ) {
				if ( current_option == option_order_assist_building ) {
					((TeronDrone*)q.currento)->assist_building();
				}
				else if ( current_option == option_order_collect_resources ) {
					((TeronDrone*)q.currento)->collect_resources();
				}
				else if ( current_option == option_order_return_to_dock ) {
					((TeronDrone*)q.currento)->return_to_dock();
				}
			}
		}
		q.end();
	}
	return true;
}


void TeronBuilder::animate( Frame* space )
{
	STACKTRACE;
	Ship::animate( space );
	if ( select_all ) {
		Query q;
		for( q.begin( this, bit(LAYER_SHIPS), selectRange ); q.currento; q.next() ) {
			if ( sameShip( q.currento ) && q.currento->get_sprite() == data->TERON_DRONE_SPRITE ) {
				data->TERON_SELECTION_SPRITE->animate(
					q.currento->normal_pos(),
					selection_sprite_index, space );
			}
		}
		q.end();
	}else if ( current_drone ) data->TERON_SELECTION_SPRITE->animate(
				current_drone->normal_pos(),
				selection_sprite_index, space );
}


void TeronBuilder::calculate()
{
	STACKTRACE;
	if ( selection_sprite_step < 0 ) {
		selection_sprite_step += TERON_SELECTION_ANIM_RATE;
		selection_sprite_index++;
		if ( selection_sprite_index == data->TERON_SELECTION_SPRITE->frames() )
			selection_sprite_index = 0;
	}else selection_sprite_step -= frame_time;
	if ( collecting ) {
		if ( !collecting->exists() ) {
			//fuel_sap -= asteroid_value;
			//handle_damage( this );
			handle_fuel_sap(this, -asteroid_value);
		}
		collecting = NULL;
	} else {
		if ( collect_step <= 0 && collectRange ) {
			Query q;
			for( q.begin( this, bit(LAYER_CBODIES), collectRange ); q.currento; q.next() ) {
				if ( q.currento->isAsteroid() ) {
					collecting = q.currento;
					game->add( new Laser( this, trajectory_angle( collecting ),
						pallete_color[TERON_COLLECT_BEAM_COLOR], distance( collecting ),
						TERON_COLLECT_BEAM_DAMAGE, time_ratio, this, 0, false ));
					collect_step = TERON_BUILDER_COLLECT_RATE;
					break;
				}
			}
			q.end();
		} else {
			collect_step -= frame_time;
		}
	}
	if ( !(current_drone && current_drone->exists()) )
		current_drone = NULL;
	Ship::calculate();
}


void TeronBuilder::calculate_thrust()
{
	STACKTRACE;
	if ( !fire_weapon ) Ship::calculate_thrust();
}


void TeronBuilder::calculate_turn_left()
{
	STACKTRACE;
	if ( !fire_weapon && !fire_special ) Ship::calculate_turn_left();
}


void TeronBuilder::calculate_turn_right()
{
	STACKTRACE;
	if ( !fire_weapon && !fire_special ) Ship::calculate_turn_right();
}


void TeronBuilder::calculate_fire_weapon()
{
	STACKTRACE;
	if ( fire_weapon && !fire_special && (thrust || turn_left || turn_right) ) {
		if ( weapon_recharge > 0 )
			return;

		if ( thrust ) {
			if ( select_all ) {
				select_all = false;
				current_drone = NULL;
				message.out( "All drones deselected" );
			} else {
				select_all = true;
				message.out( "All drones selected" );
			}

		}
		else if ( turn_right ) {
			select_all = false;
			TeronDrone* first_drone = NULL;
			TeronDrone* next_drone = NULL;
			Query q;
			for( q.begin( this, bit(LAYER_SHIPS), selectRange ); q.currento; q.next() ) {
				if ( !next_drone && sameShip( q.currento ) && q.currento->get_sprite() == data->TERON_DRONE_SPRITE ) {
					next_drone = (TeronDrone*)q.currento;
					if ( !first_drone ) first_drone = (TeronDrone*)q.currento;
				}
				if ( q.currento == current_drone ) {
					next_drone = NULL;
				}
			}
			q.end();
			if ( next_drone ) current_drone = next_drone;
			else current_drone = first_drone;
			message.out( "Next drone selected" );
		}
		else if ( turn_left ) {
			select_all = false;
			TeronDrone* prev_drone = NULL;
			Query q;
			for( q.begin( this, bit(LAYER_SHIPS), selectRange ); q.currento; q.next() ) {
				if ( q.currento == current_drone ) {
					if ( prev_drone ) break;
				}
				if ( sameShip( q.currento ) && q.currento->get_sprite() == data->TERON_DRONE_SPRITE ) {
					prev_drone = (TeronDrone*)q.currento;
				}
			}
			q.end();
			current_drone = prev_drone;
			message.out( "Prev drone selected" );
		}
		weapon_recharge += weapon_rate;

		play_sound2(data->sampleWeapon[weapon_sample]);
	}
}


void TeronBuilder::calculate_fire_special()
{
	STACKTRACE;
	special_low = FALSE;

	if ( fire_special && (fire_weapon || turn_left || turn_right) ) {
		if ( special_recharge > 0 )
			return;

		if ( fire_weapon ) {
			if ( !activate_special() )
				return;
			message.out( "> Activated Command:" );
		}
		else if ( turn_right ) {
			if ( current_option < TERON_BUILDER_OPTIONS-1 )
				current_option++;
			else
				current_option = 0;
		}
		else if ( turn_left ) {
			if ( current_option > 0 )
				current_option--;
			else
				current_option = TERON_BUILDER_OPTIONS-1;
		}
		message.out( option[current_option].phrase );

		special_recharge += special_rate;

		play_sound2(data->sampleSpecial[special_sample]);
	}
}


void TeronBuilder::materialize()
{
	STACKTRACE;
	TeronDrone* drone;
	for( int i=0; i < TERON_BUILDER_DOCKS; i++ ) {
		drone = new TeronDrone( this, pos, angle, data->TERON_DRONE_SPRITE );
		game->add( drone );
		#ifdef TERON_SHIPS_TARGETABLE
		target->add( drone );
		#endif
		drone->materialize();
		drone->dock_clearance_to( i );
	}
}


void TeronBuilder::request_clearance( TeronDrone* drone )
{
	STACKTRACE;
	for( int i = 0; i < TERON_BUILDER_DOCKS; i++ ) {
		if ( dock[i].avail ) {
			drone->dock_clearance_to( i );
			return;
		}
	}
}


TeronDrone::TeronDrone( TeronBuilder *creator, Vector2 opos,
double shipAngle, SpaceSprite *osprite ):
Ship( creator, opos, shipAngle, osprite ),
just_docked( false ), resource( 0 ), docked( NULL ), goal( dock )
{
	STACKTRACE;
	set_depth( (double)LAYER_SHIPS + 0.01 );
	layer = LAYER_SHIPS;

	asteroid_value   = creator->drone_asteroid_value;
	crew             = creator->drone_crew_max;
	crew_max         = creator->drone_crew_max;
	batt             = creator->drone_batt_max;
	batt_max         = creator->drone_batt_max;
	recharge_amount  = creator->drone_recharge_amount;
	recharge_rate    = creator->drone_recharge_rate;
	recharge_step    = recharge_rate;
	weapon_drain     = creator->drone_weapon_drain;
	weapon_rate      = creator->drone_weapon_rate;
	weapon_sample    = 0;
	weapon_recharge  = 0;
	weapon_low       = FALSE;
	special_drain    = creator->drone_special_drain;
	special_rate     = creator->drone_special_rate;
	special_sample   = 0;
	special_recharge = 0;
	special_low      = FALSE;

	hotspot_rate     = creator->drone_hotspot_rate;
	hotspot_frame    = 0;
	turn_rate        = creator->drone_turn_rate;
	turn_step        = 0.0;
	speed_max        = creator->drone_speed_max;
	accel_rate       = creator->drone_accel_rate;
	mass             = creator->drone_mass;
	orig_mass        = mass;

	weaponRange      = creator->droneWeaponRange;

	control = new TeronDroneController( "Teron Drone", Game::channel_none );
	control->load( "scp.ini", "Config0" );
	game->add( control );
	control->temporary = true;
	control->select_ship( this, "terbi" );
	((ControlWussie*)control)->option_velocity[0][0] = scale_velocity( 999 );
	((ControlWussie*)control)->option_range[0][0] = weaponRange;
}


TeronBuildPlatform* TeronDrone::get_build_platform()
{
	STACKTRACE;
	return new TeronBuildPlatform( this, pos, data->TERON_DRONE_BUILD_SPRITE, TERON_DRONE_BUILD_INDEX );
}


int TeronDrone::canCollide( SpaceLocation* other )
{
	STACKTRACE;
	if ( !docked ) return Ship::canCollide( other );
	if ( other == docked ) return false;
	if ( other->isObject() ) if ( ((SpaceObject*)other)->get_sprite() == sprite )
		if ( ((TeronDrone*)other)->docked ) return false;
	return Ship::canCollide( other );
}


void TeronDrone::inflict_damage( SpaceObject* other )
{
	STACKTRACE;
	Ship::inflict_damage( other );
	if ( goal == dock && other == ship ) {
		((TeronBuilder*)other)->request_clearance( this );
	}
}


void TeronDrone::assist_building()
{
	STACKTRACE;
	Query q;
	for( q.begin( this, bit(LAYER_SPECIAL), TERON_DRONE_SIGHT_RANGE ); q.currento; q.next() ) {
		if ( sameShip( q.currento )) {
			TeronBuildPlatform* platform = ((TeronBuildPlatform*)q.currento);
			control->target = platform;
			target = platform;

			goal = build;
			if ( docked ) leave_dock();
			roger();
			break;
		}
	}
	q.end();
}


void TeronDrone::collect_resources()
{
	STACKTRACE;
	if ( !ship ) {
		assist_building();
		if ( target ) return;
	}
	SpaceObject* ast = NULL;
	double d = TERON_DRONE_SIGHT_RANGE;
	Query q;
	for( q.begin( this, bit(LAYER_CBODIES), TERON_DRONE_SIGHT_RANGE ); q.currento; q.next() ) {
		if ( q.currento->isAsteroid() ) {
			double i = distance( q.currento );
			if ( i < d ) {
				ast = q.currento;
				d = i;
			}
		}
	}
	q.end();
	if ( ast ) {
		control->target = ast;
		target = ast;

		if ( goal != collect ) roger();
		goal = collect;
		if ( docked ) leave_dock();
	}
}


void TeronDrone::return_to_dock()
{
	STACKTRACE;
	if ( ship ) {
		goal = dock;
		control->target = ship;
		target = ship;
		roger();
	} else {
		collect_resources();
	}
}


void TeronDrone::dock_clearance_to( int number )
{
	STACKTRACE;
	docked      = (TeronBuilder*)ship;
	dock_number = number;
	dock_dir    = docked->dock[dock_number].dir;
	dock_dist   = docked->dock[dock_number].dist;
	dock_angle  = docked->dock[dock_number].angle;
	docked->dock[dock_number].avail = false;
	mass        = 0;
	just_docked = true;
	if ( resource ) {
		int b = iround(docked->getBatt());
		//docked->fuel_sap -= resource;
		docked->handle_fuel_sap(this, -resource);
		resource -= iround(docked->getBatt() - b);
	}
}


void TeronDrone::roger()
{
	STACKTRACE;
	//  game->add( new FixedAnimation( this, this, data->TERON_SELECTION_SPRITE, 0, data->TERON_SELECTION_SPRITE->frames(), time_ratio, LAYER_HOTSPOTS ));
}


void TeronDrone::leave_dock()
{
	STACKTRACE;
	docked->dock[dock_number].avail = true;
	docked = NULL;
	mass = orig_mass;
}


int TeronDrone::activate_weapon()
{
	STACKTRACE;
	if ( goal == build ) {
		game->add( new TeronDroneLaser( this, angle, pallete_color[TERON_DRONE_BEAM_COLOR],
			weaponRange, 0, time_ratio, this, Vector2(size)*Vector2(0,0.5)/*0.0, 0.5 * h*/, true ));
		//vx = -vx;
		//vy = -vy;
		vel = -vel;
		return true;
	}
	else if ( goal == collect ) {
		game->add( new TeronDroneLaser( this, angle,
			pallete_color[TERON_COLLECT_BEAM_COLOR], weaponRange,
			TERON_COLLECT_BEAM_DAMAGE, time_ratio, this, Vector2(size)*Vector2(0,0.5)/*0.0, 0.5 * h*/, true ));
		return true;
	}
	return false;
}


void TeronDrone::calculate()
{
	STACKTRACE;
	nextkeys &= ~keyflag::next;
	nextkeys &= ~keyflag::prev;
	nextkeys &= ~keyflag::closest;
	if ( docked ) {
		SpaceObject::calculate();
		//x = docked->normal_x() + dock_dist*cos(( dock_dir + docked->get_angle() )* ANGLE_RATIO );
		//y = docked->normal_y() + dock_dist*sin(( dock_dir + docked->get_angle() )* ANGLE_RATIO );
		pos = docked->pos + dock_dist*unit_vector(dock_dir + docked->get_angle());
		//vx = docked->get_vx();
		//vy = docked->get_vy();
		vel = docked->get_vel();
		angle = normalize( dock_angle + docked->get_angle(), 360.0 );
								 //(int)(angle / 5.625) + 16;
		sprite_index = get_index(angle);
		sprite_index &= 63;
		just_docked = false;
		mass = docked->mass;
		#ifdef TERON_COLLISION_FORWARDING
		cx = x;
		cy = y;
		cvx = vx;
		cvy = vy;
		#endif
		if ( !ship ) leave_dock();
	} else {
		Ship::calculate();
		if ( !target || goal == collect ) {
			if ( goal == build ) assist_building();
			else if ( goal == collect ) collect_resources();
			if ( !target ) return_to_dock();
		}
	}
}


void TeronDrone::collide( SpaceObject* other )
{
	STACKTRACE;
	Ship::collide( other );
	#ifdef TERON_COLLISION_FORWARDING
	if ( !docked ) return;
	if ( just_docked ) return;
	docked->vx += vx - cvx;
	docked->vy += vy - cvy;
	docked->x += x - cx;
	docked->y += y - cy;
	#endif
}


TeronDroneLaser::TeronDroneLaser( TeronDrone *creator, double langle,
int lcolor, double lrange, int ldamage, int lfcount,
SpaceLocation *opos, Vector2 rel_pos, bool osinc_angle ):
Laser( creator, langle, lcolor, lrange, ldamage, lfcount, opos,
rel_pos, osinc_angle ), collecting( NULL ), drone( creator )
{
	STACKTRACE;
	if ( !damage_factor ) {
		collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = bit(LAYER_SPECIAL);
	} else {
		collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = bit(LAYER_CBODIES);
	}
}


void TeronDroneLaser::calculate()
{
	STACKTRACE;
	if ( collecting ) {
		if ( !collecting->exists() ) {
			if ( ship ) {
				//ship->fuel_sap -= drone->asteroid_value;
				//ship->handle_damage( ship );
				ship->handle_fuel_sap(this, -drone->asteroid_value);
			} else {
				drone->resource += drone->asteroid_value;
			}
		}
		collecting = NULL;
	}
	Laser::calculate();
}


void TeronDroneLaser::inflict_damage( SpaceObject* other )
{
	STACKTRACE;
	Laser::inflict_damage( other );
	if ( !other->isShip() && sameTeam( other )) {
		//other->repair++;
		//other->handle_damage( this );
		other->handle_damage( this, -1 );
	}
	if ( other->isAsteroid() ) collecting = other;
}


TeronShipController::TeronShipController( const char *name, int channel ):
ControlWussie( name, channel )
{
	STACKTRACE;
	return;
}


void TeronShipController::animate( Frame* space )
{
	STACKTRACE;
	return;
}


TeronDroneController::TeronDroneController( const char *name, int channel ):
TeronShipController( name, channel )
{
	STACKTRACE;
	return;
}


void TeronDroneController::target_stuff()
{
	STACKTRACE;
	return;
}


TeronFighter::TeronFighter( TeronBuilder *creator, Vector2 opos,
double shipAngle, SpaceSprite *osprite ):
Ship( creator, opos, shipAngle, osprite )
{
	STACKTRACE;
	crew             = creator->fighter_crew_max;
	crew_max         = creator->fighter_crew_max;
	batt             = creator->fighter_batt_max;
	batt_max         = creator->fighter_batt_max;
	recharge_amount  = creator->fighter_recharge_amount;
	recharge_rate    = creator->fighter_recharge_rate;
	recharge_step    = recharge_rate;
	weapon_drain     = creator->fighter_weapon_drain;
	weapon_rate      = creator->fighter_weapon_rate;
	weapon_sample    = 0;
	weapon_recharge  = 0;
	weapon_low       = FALSE;
	special_drain    = creator->fighter_special_drain;
	special_rate     = creator->fighter_special_rate;
	special_sample   = 0;
	special_recharge = 0;
	special_low      = FALSE;

	hotspot_rate     = creator->fighter_hotspot_rate;
	hotspot_frame    = 0;
	turn_rate        = creator->fighter_turn_rate;
	turn_step        = 0.0;
	speed_max        = creator->fighter_speed_max;
	accel_rate       = creator->fighter_accel_rate;
	mass             = creator->fighter_mass;

	weaponRange      = creator->fighterWeaponRange;
	weaponVelocity   = creator->fighterWeaponVelocity;
	weaponDamage     = creator->fighterWeaponDamage;
	weaponArmour     = creator->fighterWeaponArmour;

	control = new TeronShipController( "Teron Fighter", Game::channel_none );
	control->load( "scp.ini", "Config0" );
	game->add( control );
	control->temporary = true;
	control->select_ship( this, "terbi" );
	((ControlWussie*)control)->option_velocity[0][0] = weaponVelocity;
	((ControlWussie*)control)->option_range[0][0] = weaponRange;
}


TeronBuildPlatform* TeronFighter::get_build_platform()
{
	STACKTRACE;
	return new TeronBuildPlatform( this, pos, data->TERON_FIGHTER_BUILD_SPRITE, TERON_FIGHTER_BUILD_INDEX );
}


int TeronFighter::activate_weapon()
{
	STACKTRACE;
								 //0.0, 0.5 * h,
	game->add( new Missile( this, size * Vector2(0, 0.5),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->TERON_FIGHTER_SHOT_SPRITE ));
	return true;
}


void TeronFighter::calculate_thrust()
{
	STACKTRACE;
	thrust = true;
	Ship::calculate_thrust();
}


void TeronFighter::calculate_hotspots()
{
	STACKTRACE;
	thrust = true;
	Ship::calculate_hotspots();
}


TeronTurret::TeronTurret( TeronBuilder *creator, Vector2 opos,
double shipAngle, SpaceSprite *osprite ):
Ship( creator, opos, shipAngle, osprite ),
dock_counter( 0 )
{
	STACKTRACE;
	set_depth( (double)LAYER_SHIPS + 0.01 );

	crew             = creator->turret_crew_max;
	crew_max         = creator->turret_crew_max;
	batt             = creator->turret_batt_max;
	batt_max         = creator->turret_batt_max;
	recharge_amount  = creator->turret_recharge_amount;
	recharge_rate    = creator->turret_recharge_rate;
	recharge_step    = recharge_rate;
	weapon_drain     = creator->turret_weapon_drain;
	weapon_rate      = creator->turret_weapon_rate;
	weapon_sample    = 0;
	weapon_recharge  = 0;
	weapon_low       = FALSE;
	special_drain    = creator->turret_special_drain;
	special_rate     = creator->turret_special_rate;
	special_sample   = 0;
	special_recharge = 0;
	special_low      = FALSE;

	turn_rate        = creator->turret_turn_rate;
	turn_step        = 0.0;
	mass             = creator->turret_mass;
	speed_max        = 0.0;

	weaponRange      = creator->turretWeaponRange;
	weaponVelocity   = creator->turretWeaponVelocity;
	weaponDamage     = creator->turretWeaponDamage;
	weaponArmour     = creator->turretWeaponArmour;

	control = new TeronShipController( "Teron Turret", Game::channel_none );
	control->load( "scp.ini", "Config0" );
	game->add( control );
	control->temporary = true;
	control->select_ship( this, "terbi" );
	((ControlWussie*)control)->option_velocity[0][0] = weaponVelocity;
	((ControlWussie*)control)->option_range[0][0] = weaponRange;

	docked = 0;
}


TeronBuildPlatform* TeronTurret::get_build_platform()
{
	STACKTRACE;
	return new TeronBuildPlatform( this, pos, data->TERON_TURRET_BUILD_SPRITE, TERON_TURRET_BUILD_INDEX );
}


int TeronTurret::activate_weapon()
{
	STACKTRACE;
								 //0.0, 0.5 * h,
	game->add( new Missile( this, size * Vector2(0, 0.5),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->TERON_TURRET_SHOT_SPRITE ));
	return true;
}


int TeronTurret::activate_special()
{
	STACKTRACE;
	#ifdef TERON_TURRET_CONTROLLABLE
	if ( docked ) {
		recharge_amount  = docked->turret_recharge_amount;
		recharge_rate    = docked->turret_recharge_rate;
		weapon_drain     = docked->turret_weapon_drain;
		weapon_rate      = docked->turret_weapon_rate;
		special_drain    = docked->turret_special_drain;
		special_rate     = docked->turret_special_rate;
		turn_rate        = docked->turret_turn_rate;

		weaponRange      = docked->turretWeaponRange;
		weaponVelocity   = docked->turretWeaponVelocity;
		weaponDamage     = docked->turretWeaponDamage;
		weaponArmour     = docked->turretWeaponArmour;

		control->select_ship( docked, "terbi" );
		control = orig_control;
		dock_counter = TERON_TURRET_DOCK_COUNTER;
		collide_flag_sameship = ALL_LAYERS;

		docked = NULL;

		return true;
	}
	#endif
	return false;
}


void TeronTurret::calculate()
{
	STACKTRACE;
	Ship::calculate();
	//vx = vy = 0;
	vel = 0;
	//base->vx = base->vy = 0;
	base->vel = 0;
	//base->x = x;
	//base->y = y;
	base->pos = pos;
	#ifdef TERON_TURRET_CONTROLLABLE
	if ( dock_counter > 0 ) dock_counter -= frame_time;
	else dock_counter = 0;
	if ( docked ) {
		//docked->x = x + dock_dist * cos( dock_dir * ANGLE_RATIO );
		//docked->y = y + dock_dist * sin( dock_dir * ANGLE_RATIO );
		docked->pos = pos + dock_dist * unit_vector( dock_dir );
		//docked->vx = docked->vy = 0;
		docked->vel = 0;
		docked->nextkeys = 0;
		if ( !ship ) {
			activate_special();
		}
	}
	#endif
}


void TeronTurret::calculate_turn_left()
{
	STACKTRACE;
	Ship::calculate_turn_left();
}


void TeronTurret::calculate_turn_right()
{
	STACKTRACE;
	Ship::calculate_turn_right();
}


void TeronTurret::materialize()
{
	STACKTRACE;
	Ship::materialize();
	base = new TeronTurretBase( this, pos, 0, data->TERON_TURRET_BASE_SPRITE );
	game->add( base );
	base->calculate();			 // or else maybe collision variables won't be set before too late
}


void TeronTurret::inflict_damage( SpaceObject* other )
{
	STACKTRACE;
	SpaceObject::inflict_damage( other );
	#ifdef TERON_TURRET_CONTROLLABLE
	if ( other == ship && !dock_counter ) {
		docked = (TeronBuilder*)other;
		dock_dist  = distance( other );
		dock_dir   = trajectory_angle( other );
		orig_control = control;
		control = docked->control;
		control->select_ship( this, "terbi" );
		collide_flag_sameship = 0;

		recharge_amount  = docked->attached_turret_recharge_amount;
		recharge_rate    = docked->attached_turret_recharge_rate;
		weapon_drain     = docked->attached_turret_weapon_drain;
		weapon_rate      = docked->attached_turret_weapon_rate;
		special_drain    = docked->attached_turret_special_drain;
		special_rate     = docked->attached_turret_special_rate;
		turn_rate        = docked->attached_turret_turn_rate;

		weaponRange      = docked->attachedTurretWeaponRange;
		weaponVelocity   = docked->attachedTurretWeaponVelocity;
		weaponDamage     = docked->attachedTurretWeaponDamage;
		weaponArmour     = docked->attachedTurretWeaponArmour;
	}
	#endif
}


int TeronTurret::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	Ship::handle_damage( source, normal, direct );
	if ( !exists() ) base->die();
	return 0;
}


void TeronTurret::calculate_thrust()
{
	STACKTRACE;
	return;
}


void TeronTurret::calculate_hotspots()
{
	STACKTRACE;
	return;
}


TeronTurretBase::TeronTurretBase( TeronTurret *creator, Vector2 opos,
double oangle, SpaceSprite *osprite ):
SpaceObject( creator, opos, oangle, osprite )
{
	STACKTRACE;
	ship = creator;
	sprite_index = TERON_TURRET_BASE_INDEX;
	#ifdef TERON_TURRET_CONTROLLABLE
	collide_flag_anyone = collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS & ~bit(LAYER_SHIPS);
	#else
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
	#endif
	mass = creator->mass;
	layer = LAYER_SHIPS;
}


void TeronTurretBase::calculate()
{
	STACKTRACE;
	SpaceObject::calculate();
	#ifdef TERON_COLLISION_FORWARDING
	cx = x;
	cy = y;
	cvx = vx;
	cvy = vy;
	#endif
}


int TeronTurretBase::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if ( !ship ) return 0;

	return ship->handle_damage( source, normal, direct );
}


int TeronTurretBase::handle_fuel_sap(SpaceLocation *source, double normal)
{
	STACKTRACE;
	if ( !ship ) return 0;
	return ship->handle_fuel_sap(source, normal);
}


double TeronTurretBase::handle_speed_loss(SpaceLocation *source, double normal)
{
	STACKTRACE;
	if ( !ship ) return 0;
	return ship->handle_speed_loss(source, normal);
}


int TeronTurretBase::canCollide( SpaceLocation* other )
{
	STACKTRACE;
	if ( other == ship ) return false;
	else return SpaceObject::canCollide( other );
}


void TeronTurretBase::collide( SpaceObject* other )
{
	STACKTRACE;
	SpaceObject::collide( other );
	#ifdef TERON_COLLISION_FORWARDING
	vx = vy = 0;
	ship->x += x - cx;
	ship->y += y - cy;
	#endif
}


TeronBuildPlatform::TeronBuildPlatform( Ship* creator, Vector2 opos, SpaceSprite* osprite, int osprite_index ):
SpaceObject( creator, opos, 0, osprite ),
index_zero( osprite_index )
{
	STACKTRACE;
	new_ship = creator;
	sprite_index = osprite_index;
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_SPECIAL;
}


int TeronBuildPlatform::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if ( !new_ship ) return 0;

	double tot;
	tot = normal + direct;

	if ( tot > 0 ) {
		sprite_index--;
		if ( sprite_index < index_zero ) {
			sprite_index = index_zero;
			state = 0;
			delete new_ship;
			new_ship = NULL;
		}
	}

	if ( tot < 0 ) {
		sprite_index++;
		if ( sprite_index == sprite->frames() ) {
			sprite_index--;
			state = 0;
			new_ship->pos = pos;
			new_ship->vel = 0;
			game->add( new_ship );
			new_ship->materialize();
			new_ship = NULL;
		}
	}

	return 0;
}


void TeronBuildPlatform::death()
{
	STACKTRACE;
	if ( new_ship ) {
		sprite_index = index_zero;
		delete new_ship;
		new_ship = NULL;
	}
}


REGISTER_SHIP(TeronBuilder)
