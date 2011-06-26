class Upgrade;
extern Upgrade **upgrade_list;

class UpgradeIndex
{
	public:
		enum {
			crewpod,
			battery,
			thrusters,
			controljets,
			dynamo,
			supoxrange,
			supoxdamage,
			supoxblade,
			orzmissile,
			orzmarinespeed,
			orzabsorption,
			kohrahbladedamage,
			kohrahbladespeed,
			kohrahfirerange,
			kohrahfiredamage,
			utwigrange,
			utwigdamage,
			utwigrof,
			utwigmask1,
			utwigmask2,
			divinefavor,
			unholyaura,
			defender,
			planetlocater,
			hyperdynamo,
			//gobradar,
			//roswelldevice,
			NULL_UPGRADE
		};
};

class GobDefender : public SpaceObject
{
	public:
		GobDefender ( Ship *ship);
		double base_phase;
		virtual void calculate();
		int next_shoot_time;
};

/*class RoswellDevice : public Presence {
	public:
	RoswellDevice ( Ship *ship, double angle );
	virtual void calculate();
	int next_shoot_time;
	double phase;
};*/

class UnholyAura : public Presence
{
	public:
		SpaceLocation *focus;
		double angle;
		virtual void calculate ();
		virtual void animate ( Frame * frame);
		UnholyAura ( SpaceLocation *ship );
};

/*
class GobRadar : public Presence {
public:
	TeamCode team;
	double gx, gy, gw, gh;
	VideoWindow *window;
	//virtual void calculate ();
	virtual void animate ( Frame * frame );
	virtual void animate_item ( SpaceLocation *item);
	GobRadar();
};*/
