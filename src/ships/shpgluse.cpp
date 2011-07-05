
#include "ship.h"
REGISTER_FILE

static int MAX_TARGETS=3;
static int  specialMode;		 //Beacon behavior(0=old, 1=new);
static int  specialMulti;

/** Copy of Strivanar Scrutinizer
 */

//Target structure, includes Ship * and Time_Count.
struct GlutaSensorMarked
{
	Ship *Tagged;				 //Pointer to the object it inhabits.
	int Time_Count;				 //Keeps track of it's age.
};

class GlutaSensor : public Ship
{
	public:
		double  weaponRange;
		double  weaponVelocity;
		int weaponDamage;
		int weaponArmour;
		double  weaponTurnRate;

		double  specialRange;	 //Range of marker beacons
		double  specialFan;		 //How wide a spread the beacons are fired in
		int specialLifespan;	 //How long an attached beacon lives
		int specialGlutaSensorMarkernum;
		int specialArmor;		 //How much damage a loose marker can take
		int specialSoundRate;	 //Inteval between beeps
		double  specialVelocity; //Velocity of marker beacons

								 //Struct that holds data on a tagged ship
		GlutaSensorMarked   Tag[16];
		int Next_Tag;

	public:
		GlutaSensor(Vector2 opos, double shipAngle,
			ShipData *shipData, unsigned int code);

	protected:
		void calculate();
		virtual int activate_weapon();
		virtual int activate_special();

};

/* This section contains ALL my code on the homing missile. */
class GlutaSensorMissile : public HomingMissile
{
	public:
	public:
		GlutaSensorMissile(Vector2 opos, double oangle, double ov,
			int odamage, double orange, int oarmour, double otrate, Ship *oship,
			SpaceSprite *osprite, Ship *Target);

		virtual void calculate();
};

//I have overloaded the missile tracking function with one that can track
//invisible ships.  Note that these missiles are not fired unless a GlutaSensorMarker
//has attached to something.
void GlutaSensorMissile::calculate()
{
	STACKTRACE;

	//This used to be if (target&&!invisible()), or something like that.
	if (target) {
		double d_a = normalize(trajectory_angle(target) - (angle + turn_step), PI2);
		if (d_a > PI) d_a -= PI2;
		if (fabs(d_a) < turn_rate *frame_time) turn_step += d_a;
		else if (d_a > 0) turn_step += turn_rate *frame_time;
		else turn_step -= turn_rate *frame_time;
		while(fabs(turn_step) > (PI2/64)/2) {
			if (turn_step < 0.0) {
				angle -= (PI2/64);
				turn_step += (PI2/64);
			}
			else if (turn_step > 0.0) {
				angle += (PI2/64);
				turn_step -= (PI2/64);
			}
		}
		angle = normalize(angle, PI2);
	}

	//sprite_index = get_index(angle);
	Missile::calculate();

	//	vx = v * cos(angle );
	//	vy = v * sin(angle );
	vel = v * unit_vector(angle);
	return;
}


//This is the overloaded missile function.  It lets it home in on a beacons
//instead of targeting something automatically.
GlutaSensorMissile::GlutaSensorMissile(Vector2 opos, double oangle,
double ov, int odamage, double orange, int oarmour, double otrate,
Ship *oship, SpaceSprite *osprite,Ship *Target) :
HomingMissile(oship, opos, oangle, ov, odamage, orange, oarmour, otrate,
oship, osprite, Target)
{
	STACKTRACE;
	//Uncomment this \/ code to allow missiles to collide w/each other.
	//	collide_flag_sameship = bit(LAYER_SHIPS) | bit(LAYER_SHOTS);

	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = explosionSprite->frames();
	explosionFrameSize  = 50;
}


/* Missile section ends here.	*/

/*This section contains ALL my code for the red flashing hilight on ships. */

//These are the spots of red flashing light that appear on 'GlutaSensorMarked' ships.
//Modified from the Owa disable special.
class GlutaSensorHilight : public SpaceObject
{
	public:
		int     frame_min;
		int frame_max;
		int frame_count;
		int MaxTime;
		GlutaSensorMarked   *mk; //Pointer to target structure

	public:

		GlutaSensorHilight(GlutaSensorMarked *otarget, Ship *ocreator, SpaceSprite *osprite, int Delay,
			int FrameMin, int FrameMax);

		virtual void calculate();

};

GlutaSensorHilight::GlutaSensorHilight(GlutaSensorMarked *otarget, Ship *ocreator, SpaceSprite *osprite,
int Delay, int FrameMin,int FrameMax) :
SpaceObject(ocreator, otarget->Tagged->normal_pos(), 0.0, osprite),
frame_min(FrameMin),
frame_max(FrameMax),
frame_count(0),
MaxTime(Delay)
{
	STACKTRACE;
	mk=otarget;

	id = 0x36;
	sprite_index = frame_count;
	collide_flag_anyone = 0;
	layer = LAYER_SPECIAL;
}


void GlutaSensorHilight::calculate()
{
	STACKTRACE;
	frame_count+=1;
	if (frame_count>=frame_max)  frame_count=frame_min;
	sprite_index=frame_count;

	//If the mothership doesn't exist, turn the hilight off.
	if (!(ship && ship->exists())) {
		state=0;
		return;
	}

	//If the target is NULL, turn the hilight off.
	if (mk->Tagged==NULL) {
		mk->Tagged=NULL;
		mk->Time_Count=0;

		state=0;

		return;
	}

	//If the target has been blown up, turn the hilight off.
	if (!mk->Tagged->exists()) {

		mk->Tagged=NULL;
		mk->Time_Count=0;

		state=0;

		return;
	}

	//Move GlutaSensorHilight to current position of target
	//	x = mk->Tagged->normal_x()+1;
	//	y = mk->Tagged->normal_y()+1;
	pos = mk->Tagged->normal_pos() + Vector2(1,1);

	//If time has run out, turn off hilight.
	if (mk->Time_Count>=MaxTime) {
		mk->Tagged=NULL;
		mk->Time_Count=1;

		state = 0;
	}

	SpaceObject::calculate();
	return;
}


/*GlutaSensorHilight secton ends here.	*/

/*This contains ALL my code for the loose marker beacons. */
//These are modified from Vux limpets.
class GlutaSensorMarker : public AnimatedShot
{
	public:
		double local_angle;
		int D_Time;
		GlutaSensorMarked *To_Target;
	public:
		GlutaSensorMarker(Vector2 opos, double ov, double s_angle, GlutaSensorMarked *Tag, int Duration,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize, Vector2 ovel);

		virtual void calculate();
		virtual void inflict_damage(SpaceObject *other);
};

GlutaSensorMarker::GlutaSensorMarker(Vector2 opos, double ov, double s_angle, GlutaSensorMarked *Tag, int Duration,
double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
int ofsize, int ofcount,Vector2 ovel) :
AnimatedShot(oship, opos, 0.0, ov, 0, orange, oarmour, oship, osprite,
ofcount, ofsize), local_angle(s_angle)
{
	STACKTRACE;
	To_Target=Tag;
	D_Time=Duration;

	angle = local_angle;
	//	vx    = ovx+v * cos(angle );
	//	vy    = ovy+v * sin(angle );
	vel = ovel + v * unit_vector(angle);

	damage_factor=-1;			 //Actually causes no damage, but gives the
	//APPEARANCE of being able to do so.  This
	//lets it collide, and therefore attach, to
	//things like Chmmr Zapsats. >:D

	//If the mothership is dead, so is this loose beacon.
	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
	}
}


void GlutaSensorMarker::calculate()
{
	STACKTRACE;
	state=1;

	//If the mothership is destroyed, any loose beacons also die.
	if (!(ship && ship->exists())) {
		ship = 0;
		state = 0;
		return;
	}

	AnimatedShot::calculate();
}


void GlutaSensorMarker::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	//If the target isn't the right type, don't latch on.

	int IsType=0;
	switch(specialMode) {		 //specialMode is in strsc.ini
		case 1:					 //New mode 1:  Works for mines, planets, asteroids,
			//etc.
			IsType=other->isObject();
			break;
		default:				 //Old mode 0:  Only works for ships
			IsType=other->isShip();
			break;
	}

	if (!IsType) {
		if (other->damage_factor || other->mass) state = 0;
		return;
	}

								 //Play the 'hull clamp' sound
	sound.play(data->sampleExtra[0]);

	int slot_index=0;			 //Where to put the new marker

	int num;

	//Test to see if you've targeted the same ship
	if (specialMulti==0)		 //from INI file
	for(num=0; num<MAX_TARGETS; num++) {
		if (To_Target[num].Tagged==(Ship *)other) {
			To_Target[num].Time_Count=0;
			state=0;
			return;
		}
	}

	//If it's a new target, find either an empty or the oldest slot.
	for(num=0; num<MAX_TARGETS; num++) {
		if (To_Target[slot_index].Tagged==NULL)  continue;

		if (To_Target[num].Tagged==NULL) {
			slot_index=num;
			continue;
		}

		if (To_Target[num].Time_Count>=To_Target[slot_index].Time_Count)
			slot_index=num;
	}

								 //Record pointer
	To_Target[slot_index].Tagged=(Ship *)other;
	To_Target[slot_index].Time_Count=0;

	add(new GlutaSensorHilight(&To_Target[slot_index],ship,data->spriteExtraExplosion,
		D_Time, 1,63));

	state = 0;					 //Remove loose marker from melee
}


GlutaSensor::GlutaSensor(Vector2 opos, double shipAngle,
ShipData *shipData, unsigned int code) :
Ship(opos, shipAngle, shipData, code)
{
	STACKTRACE;

	weaponRange    = scale_range(tw_get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(tw_get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = tw_get_config_int("Weapon", "Damage", 0);
	weaponArmour   = tw_get_config_int("Weapon", "Armour", 0);
	weaponTurnRate = scale_turning(tw_get_config_float("Weapon", "TurnRate", 0));

	specialRange       = scale_range(tw_get_config_float("Special", "Range", 0));
	specialFan         = tw_get_config_float("Special","Fan",0) * ANGLE_RATIO;
	specialVelocity    = scale_velocity(tw_get_config_float("Special","Velocity",0));
	specialGlutaSensorMarkernum    = tw_get_config_int("Special","Markers",0);
	specialArmor       = tw_get_config_int("Special","Armor",0);
	specialMode        = tw_get_config_int("Special","Mode",0);
	specialSoundRate   = tw_get_config_int("Special","SoundRate",0);
	specialLifespan    = tw_get_config_int("Special","Lifespan",0);
	MAX_TARGETS        = tw_get_config_int("Special","MaxTargets",0);

	if (MAX_TARGETS>16)  MAX_TARGETS=16;

	specialMulti=tw_get_config_int("Special","Multi",0);

	collide_flag_sameship = bit(LAYER_SHOTS);
	for(int num=0; num<MAX_TARGETS; num++) {
		Tag[num].Tagged=NULL;
		Tag[num].Time_Count=0;
	}
}


void GlutaSensor::calculate()
{
	STACKTRACE;
	/*My code*/
	static int prev_time[16]={0};

	//If beacon has latched onto something...
	for(int num=0; num<MAX_TARGETS; num++)
	if (Tag[num].Tagged!=NULL) {
								 //If that 'something' doesn't exist, turn it off.
		if (!Tag[num].Tagged->exists()) {
			Tag[num].Tagged=NULL;
			Tag[num].Time_Count=0;
		}

		if (prev_time[num]>specialSoundRate) {
			sound.play(data->sampleExtra[1]);
			prev_time[num]=0;
		}

		Tag[num].Time_Count+=frame_time;
		prev_time[num]+=frame_time;
	}
	/*End of my code*/

	Ship::calculate();
}


int GlutaSensor::activate_weapon()
{
	STACKTRACE;
	int flag=0;
	for(int Cur_Target=0; Cur_Target<MAX_TARGETS; Cur_Target++) {

		if (Tag[Cur_Target].Tagged==NULL)    continue;

		if (!Tag[Cur_Target].Tagged->exists()) {
			Tag[Cur_Target].Tagged=NULL;
			Tag[Cur_Target].Time_Count=0;

			continue;
		}

		flag=1;

		float xpos=10.*sin(-angle+51.42*ANGLE_RATIO);
		add(new GlutaSensorMissile(Vector2(xpos, get_size().y * .5), angle, weaponVelocity, weaponDamage, weaponRange,
			weaponArmour, weaponTurnRate, this, data->spriteWeapon,Tag[Cur_Target].Tagged));

	}

	// dryfire ability, added by Jad
	if (flag == 0) {
		flag=1;
		float xpos=10.*sin(-angle+51.42*ANGLE_RATIO);
		add(new GlutaSensorMissile(Vector2(xpos, get_size().y * .5), angle, weaponVelocity, weaponDamage, weaponRange,
			weaponArmour, weaponTurnRate, this, data->spriteWeapon,NULL));
	}
	return(flag);
}


int GlutaSensor::activate_special()
{
	STACKTRACE;
	float min=angle-(specialFan/2);
	float max=angle+(specialFan/2);
	float step=(max-min)/specialGlutaSensorMarkernum;

	int index=0;
	for(float num=min; num<max; num+=step) {
		add(new GlutaSensorMarker(Vector2(0, 0) ,specialVelocity, num, Tag,specialLifespan, specialRange, specialArmor, this,data->spriteSpecial, 100, 5, vel));
		index++;
	}

	return(TRUE);
}


/*My code ends here. */

REGISTER_SHIP(GlutaSensor)
