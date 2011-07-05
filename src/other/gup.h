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
			sensor,
			repairsystem,
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

			androsynthbubblerate,
			androsynthcomet,
			ariloulaserdamage,
			ariloulaserrange,
			chenjesumainweapon,
			chenjesuspecial,
			earthlingwarhead,
			earthlingmissile,
			earthlinghoming,
			earthlingdefense,
			myconplasmashield,
			urquanfusion,
			spathigun,
			syreengun,

			//SC1 ships not done yet:
			// Ilwrath
			// Mmrnmhrm
			// Umgah
			// VUX
			// Yehat
			//SC2 ships not done yet:
			// Chmmr
			// Druuge
			// Melnorme
			// Pkunk
			// Slylandro
			// Thraddash
			// ZFP

			divinefavor,
			unholyaura,
			defender,
			defender2,
			planetlocater,
			hyperdynamo,
			//gobradar,
			roswelldevice,

			NULL_UPGRADE
		};
};

class GobDefender : public SpaceObject
{
	public:
		GobDefender ( Ship *ship );
		double base_phase;
		virtual void calculate();
		virtual void animate(Frame *space);
		int next_shoot_time;
		int advanced;
};

class RepairSystem : public SpaceLocation
{
	public:
		double efficiency;
		double rate;

		RepairSystem ( Ship *ship );
		virtual void reset ( );
		virtual void calculate();

		double old_crew;
		double accum;
		double accum2;
};

class UnholyAura : public Presence
{
	public:
		int level;
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
