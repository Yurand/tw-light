/* $Id: shpulzin.cpp,v 1.11 2005/08/28 20:34:08 geomannl Exp $ */
#include "../ship.h"
#include "../melee/mview.h"

REGISTER_FILE

class UlzrakMissile : public Missile
{
	public:
		UlzrakMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double orelativity);
		Ship* creator;
		double frictionFactor;

		void calculate(void);
		virtual void inflict_damage(SpaceObject *other);
};

class UlzrakInterceptor : public Ship
{
	public:

		double       shipMass;
		double       weaponRange;
		double       weaponVelocity;
		double       weaponDamage;
		double       weaponArmour;
		double       weaponRelativity;
		double       weaponFrictionEffect;

		int          specialActivationTime;
		int          specialZoomTime;
		double       specialZoomSpeedAddition;
		int          specialZoomSpeedIsAdditive;
		int          specialZeroBaseVelocityOnCollision;
		int          specialZeroBaseVelocityAfterZoom;
		int          specialReverseZoomDirectionOnCollision;
		int          specialReverseFacingOnCollision;
		double       specialSkipForwardDistanceOnCollision;
		int          specialStopZoomOnCollision;
		double       specialCollisionDamage;
		double       specialZoomMass;
		int          specialZoomVelocityBecomesBaseVelocityDirection;

		int          specialCantTurnWhileActivating;
		int          specialCantTurnWhileZooming;
		int          specialCantFireWhileActivating;
		int          specialCantFireWhileZooming;
		int          specialCantThrustWhileActivating;
		int          specialCantThrustWhileZooming;

		Vector2 normalVel;
		Vector2 zoomVel;

		int zoomCounter;
		bool zoomSequenceInitiated;
		bool zoomActive;
		bool zoomReversed;
		int spriteShift;
		int sprite_index_override;
		int inflictDamageCounter;

	public:
		UlzrakInterceptor(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
		virtual int activate_weapon();
		virtual int activate_special();
		virtual void calculate();
		virtual void calculate_turn_left();
		virtual void calculate_turn_right();
		virtual void calculate_thrust();
		virtual double handle_speed_loss(SpaceLocation* source, double normal);
		virtual void inflict_damage(SpaceObject *other);
		void DrawZoomLines(void);
		void UnZoom(void);
};

UlzrakInterceptor::UlzrakInterceptor(Vector2 opos, double angle,
ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	STACKTRACE;
	shipMass = tw_get_config_float("Ship", "Mass", 0);

	weaponRange = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_float("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_float("Weapon", "Armour", 0);
	weaponRelativity = tw_get_config_float("Weapon", "Relativity", 0);
	weaponFrictionEffect = tw_get_config_float("Weapon", "FrictionEffect", 0);

	specialActivationTime = tw_get_config_int("Special", "ActivationTime", 0);
	specialZoomTime = tw_get_config_int("Special", "ZoomTime", 0);
	specialZoomSpeedAddition  = scale_velocity(tw_get_config_float("Special", "ZoomSpeedAddition", 0));
	specialZoomSpeedIsAdditive = tw_get_config_int("Special", "ZoomSpeedIsAdditive", 0);
	specialZeroBaseVelocityOnCollision = tw_get_config_int("Special", "ZeroBaseVelocityOnCollision", 0);
	specialZeroBaseVelocityAfterZoom = tw_get_config_int("Special", "ZeroBaseVelocityAfterZoom", 0);
	specialZoomVelocityBecomesBaseVelocityDirection = tw_get_config_int("Special", "ZoomVelocityBecomesBaseVelocityDirection", 0);
	//message.print(2000,2,"ZVBBVD = %d", this->specialZoomVelocityBecomesBaseVelocityDirection);
	specialReverseZoomDirectionOnCollision = tw_get_config_int("Special", "ReverseZoomDirectionOnCollision", 0);
	specialReverseFacingOnCollision = tw_get_config_int("Special", "ReverseFacingOnCollision", 0);

	specialSkipForwardDistanceOnCollision   = scale_range(tw_get_config_float("Special", "SkipForwardDistanceOnCollision", 0));
	specialStopZoomOnCollision  = tw_get_config_int("Special", "StopZoomOnCollision", 0);
	specialCollisionDamage = tw_get_config_float("Special", "CollisionDamage", 0);

	specialCantTurnWhileActivating= tw_get_config_int("Special", "CantTurnWhileActivating", 0);
	specialCantTurnWhileZooming= tw_get_config_int("Special", "CantTurnWhileZooming", 0);
	specialCantFireWhileActivating= tw_get_config_int("Special", "CantFireWhileActivating", 0);
	specialCantFireWhileZooming = tw_get_config_int("Special", "CantFireWhileZooming", 0);
	specialCantThrustWhileActivating= tw_get_config_int("Special", "CantThrustWhileActivating", 0);
	specialCantThrustWhileZooming = tw_get_config_int("Special", "CantThrustWhileZooming", 0);
	specialZoomMass = tw_get_config_float("Special", "ZoomMass", 0);

	zoomCounter = 0;
	zoomSequenceInitiated = false;
	zoomActive = false;
	spriteShift = 0;
	sprite_index_override = 0;
	normalVel = Vector2(0,0);
	zoomVel=  Vector2(0,0);
	inflictDamageCounter = 0;
	zoomReversed = false;
}


int UlzrakInterceptor::activate_special()
{
	STACKTRACE;
	if (this->zoomSequenceInitiated || this->zoomActive)
		return(FALSE);
	//message.print(1000,10,"special");
	this->zoomSequenceInitiated = true;
	return(TRUE);
}


int UlzrakInterceptor::activate_weapon()
{
	STACKTRACE;
	UlzrakMissile* UM;
	if (this->zoomActive && this->specialCantFireWhileZooming)
		return(FALSE);
	if (this->zoomSequenceInitiated && this->specialCantFireWhileActivating)
		return(FALSE);
	UM = new UlzrakMissile(Vector2(0,0), this->angle, this->weaponVelocity,
		iround(this->weaponDamage), this->weaponRange, iround(this->weaponArmour), this, this->data->spriteWeapon,
		this->weaponRelativity);
	UM->frictionFactor = this->weaponFrictionEffect;
	game->add(UM);
	return(TRUE);
}


void UlzrakInterceptor::calculate()
{
	STACKTRACE;
	double fracDone;
	if ((!zoomActive) && (!zoomSequenceInitiated))
		spriteShift = 0;
	if (zoomActive) {
		mass = this->specialZoomMass;
		if (zoomReversed)
			this->zoomVel = (-1) * unit_vector(this->angle) * this->specialZoomSpeedAddition;
		else
			this->zoomVel = unit_vector(this->angle) * this->specialZoomSpeedAddition;
		if (this->specialZoomSpeedIsAdditive)
			set_vel ( this->normalVel + this->zoomVel );
		else
			set_vel ( this->zoomVel );
		this->damage_factor = this->specialCollisionDamage;
		this->zoomCounter += frame_time;
		this->DrawZoomLines();
		spriteShift = 3;
		if (zoomCounter>=this->specialZoomTime) {
			UnZoom();
		}
	}
	else if (zoomCounter >= this->specialActivationTime) {

		play_sound2((this->data->sampleSpecial[1]));
		this->zoomActive = true;
		this->zoomReversed = false;
		this->zoomSequenceInitiated = false;
		this->zoomCounter = 0;
								 //storing away the normal Velocity
		this->normalVel = this->vel;
		this->zoomVel = unit_vector(this->angle) * this->specialZoomSpeedAddition;
		this->inflictDamageCounter = 0;
	}
	else if (zoomSequenceInitiated) {
		zoomCounter += frame_time;
		fracDone = (double)zoomCounter / (double)this->specialActivationTime;
		spriteShift = (int)((fracDone * 3.0) + 0.5);
	}
	sprite_index_override = get_index(this->angle) + 64 * spriteShift;
	sprite_index = sprite_index_override;
	Ship::calculate();
	sprite_index = sprite_index_override;
}


double UlzrakInterceptor::handle_speed_loss(SpaceLocation *source, double normal)
{
	STACKTRACE;
	return Ship::handle_speed_loss(source, normal);
}


void UlzrakInterceptor::calculate_turn_left()
{
	STACKTRACE;
	if (this->zoomSequenceInitiated) {
		if (this->specialCantTurnWhileActivating)
			return;
		else
			Ship::calculate_turn_left();
	}
	else if (this->zoomActive) {
		if (this->specialCantTurnWhileZooming)
			return;
		else
			Ship::calculate_turn_left();
	} else {
		Ship::calculate_turn_left();
	}
}


void UlzrakInterceptor::calculate_turn_right()
{
	STACKTRACE;
	if (this->zoomSequenceInitiated) {
		if (this->specialCantTurnWhileActivating)
			return;
		else
			Ship::calculate_turn_right();
	}
	else if (this->zoomActive) {
		if (this->specialCantTurnWhileZooming)
			return;
		else
			Ship::calculate_turn_right();
	} else {
		Ship::calculate_turn_right();
	}
}


void UlzrakInterceptor::calculate_thrust()
{
	STACKTRACE;
	if (this->zoomSequenceInitiated) {
		if (this->specialCantThrustWhileActivating)
			return;
		else
			Ship::calculate_thrust();
	}
	else if (this->zoomActive) {
		if (this->specialCantThrustWhileZooming)
			return;
		else
			Ship::calculate_thrust();
	} else {
		Ship::calculate_thrust();
	}
}


void UlzrakInterceptor::inflict_damage(SpaceObject* other)
{
	STACKTRACE;
	if (this->damage_factor<0.0001) return;
	if (other->isShot() || other->isLine()) {
		return;
	}
	if (other->isPlanet()) {
		UnZoom();
	}
	if (other->mass > 0.0001 && this->specialStopZoomOnCollision) {
		UnZoom();
	}
	Ship::inflict_damage(other);
	if (other->isShip() || other->isAsteroid() || other->isPlanet())
		this->damage_factor = 0.0;
	if (TRUE) {
		if (this->specialZeroBaseVelocityOnCollision) {
			this->normalVel = Vector2(0,0);
		}
		if (this->specialReverseZoomDirectionOnCollision) {
			zoomReversed = !zoomReversed;
		}
		if (this->specialReverseFacingOnCollision) {
			this->angle += PI;
		}
		this->translate(unit_vector(this->angle) * this->specialSkipForwardDistanceOnCollision);
		if (other->mass <= 0) return;
		if (other->isShip())
			game->add(new FixedAnimation(this, other,
				data->spriteSpecialExplosion, 0, 10,
				100, DEPTH_EXPLOSIONS)
				);
		else
			game->add(new Animation(this, pos,
				data->spriteSpecialExplosion, 0, 10,
				100, DEPTH_EXPLOSIONS)
				);
	}
}


void UlzrakInterceptor::DrawZoomLines(void)
{
	STACKTRACE;
	Laser* L;
	double angleMod;
	if (zoomReversed)
		angleMod = 0;
	else
		angleMod = PI;
	L = new Laser(this, this->angle + angleMod, pallete_color[7], scale_range(2), 0, 5,
		this, Vector2(0,0), true);
	game->add(L);
	L = new Laser(this, this->angle + angleMod, pallete_color[7], scale_range(1.5), 0, 5,
		this, Vector2(size.x * 0.125, 0), true);
	game->add(L);
	L = new Laser(this, this->angle + angleMod, pallete_color[7], scale_range(1.5), 0, 5,
		this, Vector2(-size.x * 0.125, 0), true);
	game->add(L);
	L = new Laser(this, this->angle + angleMod, pallete_color[7], scale_range(1), 0, 5,
		this, Vector2(size.x * 0.275, 0), true);
	game->add(L);
	L = new Laser(this, this->angle + angleMod, pallete_color[7], scale_range(1), 0, 5,
		this, Vector2(-size.x * 0.275, 0), true);
	game->add(L);

}


void UlzrakInterceptor::UnZoom(void)
{
	STACKTRACE;
	//message.print(1000,1,"unzoom");
	this->zoomActive = false;
	this->zoomSequenceInitiated = false;
	if (this->specialZeroBaseVelocityAfterZoom!=0) {
		this->normalVel = Vector2(0,0);
		set_vel ( Vector2(0,0) );
	}
	if (this->specialZoomVelocityBecomesBaseVelocityDirection!=0) {
		this->normalVel = unit_vector(this->zoomVel) * this->speed_max;
		if (this->zoomReversed) this->normalVel = -this->normalVel;
		//message.print(1000,10, "normalVel override");
	}
	set_vel ( this->normalVel );
	this->spriteShift = 0;
	this->zoomCounter = 0;
	this->zoomReversed = false;
	this->mass = this->shipMass;
	this->damage_factor = 0;
}


UlzrakMissile::UlzrakMissile(Vector2 opos, double oangle, double ov,
int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
double orelativity)
:
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite, orelativity)
{
	STACKTRACE;
	explosionSprite     = data->spriteWeaponExplosion;
	frictionFactor = 0.0;

}


void UlzrakMissile::calculate(void)
{
	STACKTRACE;
	Missile::calculate();
}


void UlzrakMissile::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	Missile::inflict_damage(other);
	other->vel *= (1 - this->frictionFactor);
}


REGISTER_SHIP(UlzrakInterceptor)
