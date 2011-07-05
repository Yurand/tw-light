/* $Id: shpnisha.cpp,v 1.8 2004/03/24 23:51:42 yurand Exp $ */

#include "../ship.h"
#include "../melee/mview.h"

REGISTER_FILE

//#include "../sc1ships.h"

class NisskHarasser : public Ship
{
	public:
		double       shipSpeedMax;
		double       shipTurnRate;
		int          shipRechargeRate;

		double       weaponRange;
		double       weaponVelocity;
		int          weaponDamage;
		int          weaponArmour;
		double       weaponAngle;
		double       weaponRelativity;

		double       specialSpeedMax;
		double       specialTurnRate;
		int          specialRechargeRate;
		int          specialToggleMode;
		int          specialIsInertialess;
		int          specialIsAutothrust;

		int          driveMode;	 //0 is standard, 1 is inertialess
		int          oldDriveMode;
								 //1 is tandem fire.  0 is alternate fire.
		int          weaponTandemFire;
		int          gunToFire;	 //1 is left gun, 2 is right gun
		int          debounce;
		int          lastSpecial;
		int          currentSpecial;

		NisskHarasser(Vector2 opos, double angle, ShipData *data, unsigned int
			code);

		virtual int activate_weapon();
		virtual int activate_special();
		//virtual void inflict_damage(SpaceObject *other);
		virtual void calculate();
		//virtual void calculate_gravity();
		virtual int accelerate(SpaceLocation *source, double angle, double velocity, double max_speed);
		virtual int accelerate_gravwhip(SpaceLocation *source, double angle, double velocity, double max_speed);
		virtual double handle_speed_loss(SpaceLocation* source, double normal);
};

NisskHarasser::NisskHarasser(Vector2 opos, double angle, ShipData *data,
unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	shipSpeedMax = scale_velocity(get_config_float("Ship", "SpeedMax", 0));
	shipTurnRate = scale_turning(get_config_float("Ship", "TurnRate", 0));
	shipRechargeRate = scale_frames(get_config_float("Ship", "RechargeRate", 0));

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponAngle = get_config_float("Weapon", "Angle", 0) * ANGLE_RATIO;
	weaponTandemFire = get_config_int("Weapon", "TandemFire", 0);
	weaponRelativity = get_config_float("Weapon", "Relativity", 0);

	specialSpeedMax = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));
	specialRechargeRate = scale_frames(get_config_float("Special", "RechargeRate", 0));

	specialToggleMode = get_config_int("Special", "ToggleMode", 0);
	specialIsAutothrust = get_config_int("Special", "IsAutothrust", 0);
	driveMode = 0;
	oldDriveMode = 0;
	specialIsInertialess = get_config_int("Special", "IsInertialess", 0);
	gunToFire = 1;
	debounce = 0;
	lastSpecial = 0;
	currentSpecial = 0;
}


int NisskHarasser::activate_weapon()
{
	STACKTRACE;
	if (gunToFire==1||weaponTandemFire)
		add(new Missile(this, Vector2(size.y*(-0.35), size.y *(0.4)),
			angle-weaponAngle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon, weaponRelativity));
	if (gunToFire==2||weaponTandemFire)
		add(new Missile(this, Vector2(size.y*(0.35), size.y *(0.4)),
			angle+weaponAngle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon, weaponRelativity));
	if (gunToFire==1) gunToFire=2; else gunToFire=1;
	return(TRUE);
}


int NisskHarasser::activate_special()
{
	STACKTRACE;
	if (specialToggleMode) {
		if (lastSpecial==currentSpecial) return(FALSE);
		//if (debounce>0) return(FALSE);
		if (driveMode==0) {
			driveMode=1;
			this->recharge_rate = specialRechargeRate;
		} else {
			driveMode=0;
			this->recharge_rate = shipRechargeRate;
		}
		debounce = 100;
		return(TRUE);
	}
	return(FALSE);
}


void NisskHarasser::calculate()
{
	STACKTRACE;
	lastSpecial = currentSpecial;
	currentSpecial = this->fire_special;
	oldDriveMode = driveMode;
	Ship::calculate();
	if (debounce>0) {
		debounce -= frame_time;
	}
	if (!specialToggleMode) {
		if (specialIsInertialess) {
			//tw_error("special is inertialess!");
			if (fire_special) {
				this->recharge_rate = specialRechargeRate;
				driveMode = 1;
			} else {
				this->recharge_rate = shipRechargeRate;
				driveMode = 0;
			}
		} else {
			if (fire_special) {
				this->recharge_rate = shipRechargeRate;
				driveMode = 0;
			} else {
				this->recharge_rate = specialRechargeRate;
				driveMode = 1 ;
			}
		}
	}
	if (driveMode==0) {
		this->recharge_rate = shipRechargeRate;
		//message.print(1500,9,"recharge_rate = %i", recharge_rate);
		turn_rate = shipTurnRate;
		speed_max = shipSpeedMax;
		sprite_index = get_index(angle) + (0 * 64);
		return;
	} else {
		this->recharge_rate = specialRechargeRate;
		//message.print(1500,9,"recharge_rate = %i", recharge_rate);
		turn_rate = specialTurnRate;
		speed_max = specialSpeedMax;
		sprite_index = get_index(angle) + (1 * 64);

	}
	if (thrust||specialIsAutothrust) {
		vel = specialSpeedMax * unit_vector(angle);
	}
	else
		vel *= exp(- frame_time * 0.02 );

}


double NisskHarasser::handle_speed_loss(SpaceLocation *source, double normal)
{
	STACKTRACE;
	//vux limpets affect both normal speed and inertialess,
	double speed_loss = normal;
	if (speed_loss > 0.0) {
		double sl = (30/(mass+30)) * speed_loss;
		if (sl > 1) tw_error ("Speed loss too large\n(%f)", sl);
		//shipAccelRate *= 1 - sl * shipAccelRate / (shipAccelRate + scale_acceleration(2,4));
		shipSpeedMax *= 1 - sl * shipSpeedMax / (shipSpeedMax + scale_velocity(10));
		shipTurnRate *=  1 - sl * shipTurnRate / (shipTurnRate + scale_turning(4));
		//specialAccelRate *= 1 - sl * specialAccelRate / (specialAccelRate + scale_acceleration(2,4));
		specialSpeedMax *= 1 - sl * specialSpeedMax / (specialSpeedMax + scale_velocity(10));
		specialTurnRate *=  1 - sl * specialTurnRate / (specialTurnRate + scale_turning(4));
		speed_loss = 0;
	}
	return Ship::handle_speed_loss(source, normal);
}


/*void NisskHarasser::calculate_gravity() {
	STACKTRACE;
  if (driveMode==1)return;
  else
	Ship::calculate_gravity();
}*/

int NisskHarasser::accelerate(SpaceLocation *source, double angle, double velocity,
double max_speed)
{
	STACKTRACE;
	if (driveMode==0)
		return(Ship::accelerate(source, angle, velocity, max_speed));

	if (source == this)
		return Ship::accelerate(source, angle, velocity, max_speed);
	return false;
}


int NisskHarasser::accelerate_gravwhip(SpaceLocation *source, double angle, double velocity,
double max_speed)
{
	STACKTRACE;
	if (driveMode==0)
		return(Ship::accelerate_gravwhip(source, angle, velocity, max_speed));

	if (source == this)
		return Ship::accelerate(source, angle, velocity, max_speed);
	return false;
}


REGISTER_SHIP(NisskHarasser)
