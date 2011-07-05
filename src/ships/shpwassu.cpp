/* $Id: shpwassu.cpp,v 1.17 2004/03/24 23:51:43 yurand Exp $ */
/* sorry for the tons of tests... dunno where to insert checks/validations */

#include "../ship.h"
REGISTER_FILE
#include <string.h>
#include "../melee/mview.h"

class WasxClone;

class WasxSuperposition : public Ship
{
	double weaponRange;
	double weaponVelocity;
	int    weaponDamage;
	double weaponArmour;

	int    specialArmour;
	int    num_Clone;
	int    max_Clone;

	//int	   FireWeapon[3];
	int      FireWeapon[4];

	int    SpawnFresh;
	int    SpawnLifeCost;
	int    SpawnFormation;

	WasxClone **Clone;

	public:
		WasxSuperposition(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

		friend class WasxClone;

		virtual double  isInvisible() const;
		virtual int  activate_weapon();
		virtual int  activate_special();
		// virtual int  canCollide(SpaceLocation *source);

};

class WasxShot : public Shot
{
	public:
		WasxShot(Vector2 opos, double angle, double velocity, int damage,
			double range, int armour, Ship *ship);
		virtual void inflict_damage(SpaceObject *other) ;
};

class WasxClone : public Ship
{
	WasxSuperposition *MotherShip;
	Control  *cntrl;
	ShipData *shipData;

	double weaponRange;
	double weaponVelocity;
	int    weaponDamage;
	double weaponArmour;

	int    specialArmour;
	int    initialVelocity;
	int    Forming;

	public:
		WasxClone(Vector2 opos, double shipAngle,
			ShipData *shipData, Control *cntrl, WasxSuperposition *MotherShip);
		int     CloneIndex;

		virtual int  activate_weapon();
		// virtual int  canCollide(SpaceLocation *source);
		virtual int handle_damage(SpaceLocation *source, double normal, double direct);
		virtual void calculate();
		virtual void death();

};

WasxSuperposition::WasxSuperposition(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)

{
	STACKTRACE;

	weaponDamage = get_config_int("Weapon", "Damage", 0);
	weaponArmour = get_config_int("Weapon", "Armour", 0);
	weaponRange  = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity  = scale_velocity(get_config_float("Weapon", "Velocity", 0));

	specialArmour = get_config_int( "Special", "Armour", 0);

	max_Clone = 4;				 // there are hard-coded routines here ie. switch statements
	num_Clone = 0;

	SpawnFresh     = get_config_int("Others", "SpawnFresh", 0);
	SpawnLifeCost  = get_config_int("Others", "SpawnLifeCost", 0);
	SpawnFormation = get_config_int("Others", "SpawnFormation", 0);

	if (SpawnLifeCost == 0 && crew >= 4) crew -= 4;

	collide_flag_anyone   = ALL_LAYERS;
	collide_flag_sameship = ~bit(LAYER_SHOTS) & ~bit(LAYER_SHIPS);
	collide_flag_sameteam = bit(LAYER_SHIPS);

	// collide_flag_sameship    = ~bit(LAYER_SHOTS) & ~bit(LAYER_SHIPS);
	//collide_flag_anyone  = ALL_LAYERS;

	Clone      = new WasxClone*[max_Clone];
	/*for (int i = 0; i < max_Clone; i += 1) {
		Clone[i] = NULL;
		FireWeapon[i] = FALSE;
		}
		*/

	for (int i = 1; i <= max_Clone; i += 1) {
		Clone[i] = NULL;
		FireWeapon[i] = FALSE;
	}
}


WasxShot::WasxShot(Vector2 opos, double angle, double velocity,
int damage, double range, int armour, Ship *ship) :
Shot(ship, opos, angle, velocity, damage, range, armour, ship,
ship->data->spriteWeapon, 0)
{
	STACKTRACE;
}


void WasxShot::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	if (!sameShip(other)) {
		Shot::inflict_damage(other);
	}
	return;
}


double WasxSuperposition::isInvisible() const
{
	if (num_Clone == max_Clone) return(1);
	else return 0;
}


int WasxSuperposition::activate_weapon()
{
	STACKTRACE;

	if (this == NULL
		|| !this->exists()
		|| ship == NULL
		|| !ship->exists()) return(FALSE);

	double AngleWithLead = angle;
	double TargetRange   = 0;
	int    WeaponOffSet;
	int    j;

	/* for (j=0; j < max_Clone; j += 1) {
		FireWeapon[j] = TRUE;
	} */

	for (j=1; j <= max_Clone; j += 1) {
		FireWeapon[j] = TRUE;
	}

	if (target !=NULL && target->exists()) TargetRange = distance(target);

	TargetRange = TargetRange / weaponVelocity;

	for (WeaponOffSet=1; WeaponOffSet < 4; WeaponOffSet += 1 ) {

		double WOS  = (ship->get_size().x + ship->get_size().y) / 2;

		switch (WeaponOffSet) {
			case 1:
				WOS = WOS * 0.15;
				break;
			case 2:
				WOS = WOS * -0.15;
				break;
			default:
				WOS = 0.0;
		}						 // switch

		if (target != NULL && target->exists()) {

			//			double x = min_delta( normal_x(),target->normal_x() -
			//					(get_vx() - target->get_vx()) * TargetRange, X_MAX) ;
			//
			//			double y = min_delta( normal_y(), target->normal_y() -
			//					(get_vy() - target->get_vy()) * TargetRange, Y_MAX) ;

			Vector2 tmppos;
			tmppos = min_delta(normal_pos(),
				target->normal_pos() - TargetRange*(get_vel() - target->get_vel()),
				map_size);

			if (!target->isInvisible()) AngleWithLead = (atan(tmppos) - PI);

		}
		double rVelocity;
		int rv = int (random() & 3);
		switch (rv) {
			case 1:
				rVelocity = weaponVelocity * .75;
				break;
			case 2:
				rVelocity = weaponVelocity * .85;
				break;
			case 3:
				rVelocity = weaponVelocity * .95;
				break;
			default:
				rVelocity = weaponVelocity;
		}

		add(new WasxShot( Vector2(WOS, WOS), AngleWithLead, rVelocity, weaponDamage,
			weaponRange, iround(weaponArmour), this));
	}							 // for

	return(TRUE);

}


int WasxSuperposition::activate_special()
{
	STACKTRACE;

	if (this == NULL
		|| !this->exists()
		|| ship == NULL
		|| !ship->exists()) return(FALSE);

	int i;
	double da;
	Vector2 D, M;
	int SpecialActivated = FALSE;

	da = 0;
	D = 0;
	M = 0;

	for (i=0; (i < 2) && (num_Clone < max_Clone) && (crew > SpawnLifeCost); i +=1) {
		if (num_Clone == max_Clone) {
			num_Clone -= 1;
			Clone[0]->state = 0;
			memcpy(&Clone[0], &Clone[1], sizeof(WasxSuperposition*) * num_Clone);
			Clone[num_Clone] = NULL;
		}

		// these lines are made obsolete/overridden by formation routine???
		if (i==0) {
			//			dx = normal_x() + cos((angle +  30) ) * 100;
			//			dy = normal_y() + sin((angle +  30) ) * 100;
			D = normal_pos() + 100 * unit_vector((angle +  30 * ANGLE_RATIO) );
			da = angle + (PI2/64);
		} else {
			//			 dx = normal_x() + cos((angle + 330) ) * 100;
			//			 dy = normal_y() + sin((angle + 330) ) * 100;
			D = normal_pos() + 100 * unit_vector((angle +  330 * ANGLE_RATIO) );
			da = angle - (PI2/64);
		}
		// upto here
		crew -= int(SpawnLifeCost);
		num_Clone += 1;
		WasxClone *tmp = new WasxClone(D,da,data ,control, this);

		//		mx = normal_x();
		//		my = normal_y();
		M = normal_pos();
		if (turn_left || turn_right) {
			tmp->translate(M - D);
			translate(D - M);
		}

		add(tmp);
		SpecialActivated = TRUE;

		targets->add(tmp);

	}
	if (!SpecialActivated) return(FALSE);
	return(TRUE);
}


/* int WasxSuperposition::canCollide(SpaceLocation *other) {
	STACKTRACE;
	if (this == NULL
	|| !this->exists()
	|| ship == NULL
	|| !ship->exists()) return(FALSE);

	if (other->isShip() && sameTeam(other)) return(FALSE);
		else return(TRUE); // isAlly doesn't work?

	} */

//WasxClone::WasxClone(Vector2 opos, double shipAngle,
//		ShipData *shipData, Control *cntrl, WasxSuperposition *MotherShip) :
//		Ship(opos, shipAngle, shipData, MotherShip->ally_flag),
//		CloneIndex(0)
//Ship(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
WasxClone::WasxClone(Vector2 opos, double shipAngle,
ShipData *shipData, Control *cntrl, WasxSuperposition *MotherShip) :
Ship(MotherShip, MotherShip->pos, shipAngle, shipData->spriteShip),
CloneIndex(0)
{
	STACKTRACE;

	if (this == NULL
		|| !this->exists()
		|| ship == NULL
		|| !ship->exists()
		|| MotherShip == NULL
		|| !MotherShip->exists()) return;

	// extra stuff

	data = shipData;
	data->lock();

	crew     = MotherShip->crew;
	crew_max = MotherShip->crew_max;
	batt     = MotherShip->batt;
	batt_max = MotherShip->batt_max;

	recharge_amount  = MotherShip->recharge_amount;
	recharge_rate    = MotherShip->recharge_rate;
	recharge_step    = MotherShip->recharge_step;
	weapon_drain     = MotherShip->weapon_drain;
	weapon_rate      = MotherShip->weapon_rate;
	weapon_sample    = MotherShip->weapon_sample;
	weapon_recharge  = MotherShip->weapon_recharge;
	weapon_low       = MotherShip->weapon_low;
	special_drain    = MotherShip->special_drain;
	special_rate     = MotherShip->special_rate;
	special_sample   = MotherShip->special_sample;
	special_recharge = MotherShip->special_recharge;
	special_low      = FALSE;

	hotspot_rate  = MotherShip->hotspot_rate;
	hotspot_frame = MotherShip->hotspot_frame;
	turn_rate     = MotherShip->turn_rate;
	speed_max     = MotherShip->speed_max;
	accel_rate    = MotherShip->accel_rate;
	mass          = MotherShip->mass;

	control = cntrl;
	ship = this;

	// end extra stuff

	layer = LAYER_SHIPS;

	initialVelocity = TRUE;
	Forming         = TRUE;

	// all this assignments really needed??? or is there a better way?
	this->MotherShip            = MotherShip;
	this->shipData              = shipData;
	this->ally_flag             = MotherShip->ally_flag;

	collide_flag_anyone   = ALL_LAYERS;
	collide_flag_sameship = ~bit(LAYER_SHOTS) & ~bit(LAYER_SHIPS);
	collide_flag_sameteam = bit(LAYER_SHIPS);

	//this->collide_flag_sameship		= ~bit(LAYER_SHOTS) & ~bit(LAYER_SHIPS); // how do i make this thing work?
	// this->collide_flag_ally		= 0;
	this->collide_flag_anyone   = ALL_LAYERS;

	this->weaponDamage          = MotherShip->weaponDamage;
	this->weaponArmour          = MotherShip->weaponArmour;
	this->weaponRange           = MotherShip->weaponRange;
	this->weaponVelocity        = MotherShip->weaponVelocity;
	this->accel_rate            = MotherShip->accel_rate;
	this->speed_max             = MotherShip->speed_max ;
	this->mass                  = MotherShip->mass;

	if (MotherShip->SpawnFresh==0)  this->crew  = MotherShip->crew ;
	else this->crew = MotherShip->specialArmour;

	if (MotherShip->target != NULL && MotherShip->target->exists()) this->target = MotherShip->target;

	//this->CloneIndex			= MotherShip->num_Clone-1;
	this->CloneIndex            = MotherShip->num_Clone;

	this->specialArmour         = MotherShip->specialArmour;

	//	vx = ship->get_vx() + .2 * cos(angle);
	//	vy = ship->get_vy() + .2 * sin(angle);
	vel = ship->get_vel() + 0.2 * unit_vector(angle);
}


int WasxClone::activate_weapon()
{
	STACKTRACE;

	if (this == NULL
		|| !this->exists()
		|| ship == NULL
		|| !ship->exists()
		|| MotherShip == NULL
		|| !MotherShip->exists()) return(FALSE);

	// redundant code starts here : same as WasxSuperposition::activate_weapon
	// use function instead in future version passing ship and target as parameters

	double AngleWithLead = angle;
	double TargetRange   = 0;
	int    WeaponOffSet;

	if (target != NULL && target->exists()) TargetRange = distance(target);

	TargetRange = TargetRange / weaponVelocity;

	for (WeaponOffSet=1; WeaponOffSet < 4; WeaponOffSet += 1 ) {

		//		double WOS  = (ship->width() + ship->height()) / 2;
		double WOS  = (ship->get_size().x + ship->get_size().y) / 2;

		switch (WeaponOffSet) {
			case 1:
				WOS = WOS * 0.15;
				break;
			case 2:
				WOS = WOS * -0.15;
				break;
			default:
				WOS = 0.0;
		}						 // switch

		if (target != NULL && target->exists()) {
			//			double x = min_delta(normal_x(),target->normal_x() -
			//					(get_vx() - target->get_vx()) * TargetRange, X_MAX) ;
			//
			//			double y = min_delta( normal_y(), target->normal_y() -
			//					(get_vy() - target->get_vy()) * TargetRange, Y_MAX) ;

			Vector2 tmppos;
			tmppos = min_delta(normal_pos(),
				target->normal_pos() - TargetRange*(get_vel() - target->get_vel() ),
				map_size);		 // use of getvel is wrong ... should include frame_time to predict distance

			if (!target->isInvisible()) AngleWithLead = (atan(tmppos) - PI);

		}
		double rVelocity;
		int rv = int (random() & 3);
		switch (rv) {
			case 1:
				rVelocity = weaponVelocity * .75;
				break;
			case 2:
				rVelocity = weaponVelocity * .85;
				break;
			case 3:
				rVelocity = weaponVelocity * .95;
				break;
			default:
				rVelocity = weaponVelocity;
		}

		add(new WasxShot( Vector2(WOS, WOS), AngleWithLead, rVelocity, weaponDamage,
			weaponRange, iround(weaponArmour), ship));
	}							 // for

	return(TRUE);

}


/* int WasxClone::canCollide(SpaceLocation *other) {
	STACKTRACE;
	if (this == NULL
	|| !this->exists()
	|| ship == NULL
	|| !ship->exists()
	|| MotherShip == NULL
	|| !MotherShip->exists()) return(FALSE);

   if (other->isShip() && sameTeam(other)) return(FALSE);
		else return(TRUE);

}  */

int WasxClone::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;
	if (this == NULL
		|| !this->exists()
		|| ship == NULL
		|| !ship->exists()
		|| MotherShip == NULL
		|| !MotherShip->exists()) return 0;

	if (normal+direct > 0) {
		crew -= normal + direct;
		if (crew <= 0) {
			MotherShip->num_Clone -= 1;
			state=0;
		}
	}
	return iround(normal + direct);
}


void WasxClone::calculate()
{
	STACKTRACE;

	// first check for a dead mother; otherwise the control access in the ship::calculate
	// function may freak out (since the control depends on the mother and if the mother
	// is dead then ...

	if (!(MotherShip && MotherShip->exists())) {
		MotherShip = 0;
		state = 0;
		return;
	}

	Ship::calculate();

	if (this == NULL
		|| !this->exists()
		|| ship == NULL
		|| !ship->exists()
		|| MotherShip == NULL
	|| !MotherShip->exists()) {
		MotherShip = 0;
		state = 0;
		return;
	}

	// Angle for left/right
	angle = MotherShip->get_angle();

	// Velocity for initial thrust
	if (initialVelocity) {
		//		vx *= 1 - .0005 * frame_time;
		//		vy *= 1 - .0005 * frame_time;

		vel *= 1 - 0.0005 * frame_time;

		if (magnitude_sqr(vel) < 0.05 * 0.05) {
			//			vx *= 0;
			//			vy *= 0;
			vel *= 0;
			initialVelocity=FALSE;

		}
		if (MotherShip->thrust) initialVelocity = FALSE;
	}							 //else if (MotherShip->thrust) accelerate(this, angle, accel_rate * frame_time , speed_max);

	// Try to retain a certain distance to gain formation from Mothership
	if (MotherShip->SpawnFormation == 1) {
		double RelAngle;

		if (fabs(distance(MotherShip)) > 150 ||
		fabs(distance(MotherShip)) < 50) {
			switch (CloneIndex) {
				case 1:			 // upper right
					RelAngle = 45 * ANGLE_RATIO;
					break;
				case 2:			 // upper left
					RelAngle = 315 * ANGLE_RATIO;
					break;
				case 3:			 // lower left
					RelAngle = 225 * ANGLE_RATIO;
					break;
				case 4:			 // lower right
					RelAngle = 135 * ANGLE_RATIO;
					break;
				default:
					tw_error("This is not supposed to happend");
					RelAngle = -1;
					break;
			}
			RelAngle += trajectory_angle(MotherShip);

			//			vx = (.2 * cos(RelAngle)) ;
			//			vy = (.2 * sin(RelAngle)) ;
			vel = 0.2 * unit_vector(RelAngle);

			Forming=TRUE;
		}

		if (Forming) {
			if (fabs(distance(MotherShip)) >  50 &&
			fabs(distance(MotherShip)) < 150) {
				Forming = FALSE;
			}
		}

	}

	// Hot spots
	if ((MotherShip->thrust) && (MotherShip->hotspot_frame <= 0)) {
		add(new Animation(this,
			normal_pos() - product(unit_vector(angle), get_size()) / 2.5,
		//				normal_x() - (cos(angle ) * w / 2.5),
		//				normal_y() - (sin(angle ) * h / 2.5),
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, LAYER_HOTSPOTS));
	}

	// Re-acquire target (needed once targetting a new opponent)
	if (MotherShip->target != NULL && MotherShip->target->exists()) target = MotherShip->target;

	// Activate Weapon
	if (MotherShip->FireWeapon[CloneIndex])
		this->activate_weapon();

	MotherShip->FireWeapon[CloneIndex] = FALSE;

}


void WasxClone::death()
{
	STACKTRACE;

	if (this == NULL
		|| !this->exists()
		|| ship == NULL
		|| !ship->exists()
		|| MotherShip == NULL
	|| !MotherShip->exists()) {
		MotherShip = 0;
		state = 0;
		return;
	}

	for (int i = 1; i <= MotherShip->num_Clone; i += 1) {
		if (MotherShip->Clone[i] == this) {

			MotherShip->Clone[i] = NULL;
			delete MotherShip->Clone[i];
			MotherShip->num_Clone -= 1;

			memmove(&MotherShip->Clone[i], &MotherShip->Clone[i+1],
				(MotherShip->num_Clone-i) * sizeof(WasxClone*));

			MotherShip->Clone[i]->CloneIndex = i;

			return;
		}
	}

}


REGISTER_SHIP(WasxSuperposition)
