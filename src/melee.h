/*
This file is part of "TW-Light"
					http://tw-light.appspot.com/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef __MELEE_H__
#define __MELEE_H__

#include <stdlib.h>
#include <math.h>

#include <list>

#include <allegro.h>

#include "util/port.h"

#ifndef PI
#   define PI 3.14159265358979323846
#endif
#ifdef PI2
#   undef PI2
#endif
#define PI2 (PI*2)

#ifndef MAXINT
#define MAXINT 0x7FFFFFFF
#endif

#define PLATFORM_IS_ALLEGRO

#ifdef PLATFORM_IS_ALLEGRO
struct FONT;
struct BITMAP;
//struct RGB;

//typedef RGB     Color;

#else
#error unknown platform (allegro?)
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4800 )//forcing value to bool (performance warning)
#endif

#include "util/base.h"
#include "util/endian.h"
#include "util/get_time.h"
#include "util/random.h"
#define random  tw_random
#include "util/round.h"
#include "util/vector2.h"
#include "util/sounds.h"
//#include "util/aastr.h"

/*
	error displays an error message, and prompts the user to
	"Abort", "Retry", or "Debug"
	on "Abort", it throws 0, which will be silently caught at the closest catch (int)
	on "Retry" it returns
	on "Debug" it throws -1, which will be rethrown all the way out. or something roughly equivalent
*/
#include "util/errors.h"

//some random space for stings to use for GUIs & whatnot that need short term storage
extern char dialog_string[20][128];

// added GEO
// moved to Game::checksyn() as an overloadable subroutine.
//used to debug desynchronization errors
//does nothing if LOTS_OF_CHECKSUMS isn't defined in libs.h)
#ifdef _DEBUG
#   define checksync() __checksync(__FILE__,__LINE__)
								 //defined in mgame.cpp
void __checksync( const char *fname, int line);
#else
#   define checksync()
#endif
//*/

class Logger;

#define MELEE_ASTEROID          0
#define MELEE_ASTEROIDEXPLOSION 64
#define MELEE_BOOM              84
#define MELEE_BOOMSHIP          88
#define MELEE_HOTSPOT           89
#define MELEE_KABOOM            101
#define MELEE_MUSIC             121
#define MELEE_PANEL             122
#define MELEE_PLANET            125
#define MELEE_SPARK             128
#define MELEE_STAR              134
#define MELEE_XPL1              137

#define SCPPAL_PALLETE 0
#define SCPPAL_FONT    4

#define SPACE_LAYERS 7

#define LAYER_HOTSPOTS   0
#define LAYER_CBODIES    1
#define LAYER_LINES      2
#define LAYER_SHOTS      3
#define LAYER_SHIPS      4
#define LAYER_SPECIAL    5
#define LAYER_EXPLOSIONS 6
#define LAYER_LOCATIONS  7

#define DEPTH_STARS      0.0
#define DEPTH_HOTSPOTS   1.0
#define DEPTH_ASTEROIDS  2.0
#define DEPTH_PLANETS    2.0
#define DEPTH_LINES      3.0
#define DEPTH_SHOTS      4.0
#define DEPTH_SHIPS      5.0
#define DEPTH_SPECIAL    6.0
#define DEPTH_EXPLOSIONS 7.0
#define DEPTH_PRESENCE   10.0

#define ALL_LAYERS ( (1<<SPACE_LAYERS) - 1)
#define OBJECT_LAYERS ( (1<<SPACE_LAYERS) - 1 - (1<<LAYER_LINES) - (1<<LAYER_LOCATIONS) )
#define LINE_LAYERS ( (1<<LAYER_LINES) )

#define MAX_SPACE_ITEMS 1024

#define MAX_FLEET_SIZE 250
#define MAX_SHIP_ID    80

#include "input.h"

#define CAPTAIN_WIDTH   55
#define CAPTAIN_HEIGHT  30

#define bit(a) (1 << (a))

#define BOOM_SAMPLES 4

#define ASTEROID_FRAMES          64
#define ASTEROIDEXPLOSION_FRAMES 20
#define HOTSPOT_FRAMES           12
#define KABOOM_FRAMES            20
#define SPARK_FRAMES             6
#define PLANET_FRAMES            3
#define PANEL_FRAMES             3
#define XPL1_FRAMES              40

#define HOT_COLORS 12

#define PHASE_MAX   12
#define PHASE_DELAY 50

extern int hot_color[HOT_COLORS];

//melee/mframe.h
class Physics;
class SpaceSprite;
class SpaceLocation;
class SpaceObject;
class SpaceLine;

//melee/mcontrol.h
class Control;
class ControlWrapper;

//melee/mship.h, melee/mshpdata, melee/mshppan.h
class Ship;
class ShipData;
class ShipPanel;

//melee/mlog.h  //depends on nothing
class Histograph;
class Log;

//melee/mview.h
class View;

//frame.h
struct DirtyItem;
class Frame;

//util/pmask.h
struct PMASK;

//melee/mshot.h
class Shot;
class AnimatedShot;
class Missile;
class HomingMissile;
class Laser;

//melee/mcbodies.h
class Animation;
class Asteroid;
class Planet;

//melee/mgame.h
class Game;

//melee/moptions.h
void options_menu (Game *game) ;
void video_menu (Game *game) ;
void audio_menu (Game *game) ;
void view_menu (Game *game) ;
void config_menu (Game *game) ;
void physics_menu (Game *game) ;

								 //radians per degree
#define ANGLE_RATIO     (PI / 180.0)
#define RANGE_RATIO     40.0	 //???

extern double distance_ratio;
extern int time_ratio;

//blah
class NormalGame;
class GobGame;
extern class Physics *physics;
extern class Game *&game;
extern class NormalGame *&normalgame;
extern class GobGame *&gobgame;
extern Vector2 map_size;

extern class MeleeData meleedata;

//melee/mhelpers.cpp

/// \brief Config event
class ConfigEvent : public Event
{
	public:
		const char *name;
		union
		{
			/// \brief value is read as a const char * on a SET event
			/// value is modified by a GET event to a malloced string
			/// BaseClass *source;
			char *value;
		};
		enum {
			GET,
			SET,
			FIND
		};
};

class VideoEvent : public Event
{
	public:
		class VideoWindow *window;
		enum {
			/// \brief happens before surface is invalidated
			/// before a resolution change, whatever
			INVALID,
			/// \brief the opposite of an invalid event
			/// after an alt-tab back, after a resolution change, whatever
			VALID,
			/// \brief happens when size is changed
			/// after a resolution change, or a window resize
			/// ?? after alt-tabbing back in ??
			RESIZE,
			/// \brief happens when the color format changes
			CHANGE_BPP,
			/// \brief happens when contents are changed
			/// ?? after alt-tabbing back in ??
			/// ?? after window translated ??
			/// ? after any resize event ?
			REDRAW
		};
		virtual int _get_size() const {return sizeof(*this);}
};
extern volatile int debug_value;

//#define make_rgb(r,g,b)
//
//inline RGB makeRGB (int r, int g, int b)
//{
//	RGB rgb =
//	{
//		r, g, b, 0
//	};
//	return rgb;
//}

int tw_color (Color col);
int tw_color (int r, int g, int b);

int scale_frames(double value);
double scale_range(double range);
double scale_turning (double turn_rate) ;
double scale_velocity (double velocity) ;
double scale_acceleration (double acceleration, double hotspot_rate = 0) ;

void show_file(const char *file) ;
void show_text(const char *text) ;

class VideoWindow : public BaseClass
{
	float const_x, const_y, const_w, const_h;
	float propr_x, propr_y, propr_w, propr_h;

	char lock_level;
	//struct VW_lock_data *lock_data;

	public:void update_pos();

	VideoWindow *parent;
	std::list<BaseClass*> callback_list;
	public:
		void add_callback ( BaseClass * );
		void remove_callback ( BaseClass * );
		virtual void _event ( Event *e );

		int x, y, w, h;
		Surface *surface;
		void event(int subtype);
		void redraw() {event(VideoEvent::REDRAW);}
		void preinit ();
		void init ( VideoWindow *parent_window);
		void locate ( double c_x, double p_x, double c_y, double p_y, double c_w, double p_w, double c_h, double p_h );
		void lock();
		void unlock();
		void hide();
		void match ( VideoWindow *old );
		void deinit();
		virtual ~VideoWindow();
		virtual int _get_size() const {return sizeof(*this);}
};

class VideoSystem : public BaseClass
{
	public:
		int width, height, bpp, gamma;
		int fullscreen;
		TW_DATAFILE *font_data;	 //fonts
		FONT *basic_font;		 //font to use if no other is available
		Color *palette;
		volatile bool screen_corrupted;
		int last_poll;
		Surface *surface;
		VideoWindow window;

		FONT *get_font(int size);

		void preinit() ;
		int poll_redraw();
								 //returns 0 on failure
		int set_resolution (int width, int height, int bpp, int fullscreen) ;
		void set_palette(Color *pal);
		void (*color_effects)(Color *color);
		void update_colors();
		void redraw();
} extern videosystem;

int get_gamma();
void set_gamma(int gamma);
void gamma_color_effects (Color *color) ;

struct registered_file_type
{
	const char *fname;
	const char *fdate;
	const char *ftime;
};
extern registered_file_type *registered_files;
extern int num_registered_files;
void _register_file (const char *fname, const char *fdate, const char *ftime);

#ifdef _DEBUG
#   define REGISTER_FILE static const char *__registered_filename = __FILE__; static void __ignore_me_rf() {_register_file(__FILE__, __DATE__, __TIME__);} CALL_BEFORE_MAIN(__ignore_me_rf);
#else
#   define REGISTER_FILE static void __ignore_me_rf() {_register_file(__FILE__, __DATE__, __TIME__);} CALL_BEFORE_MAIN(__ignore_me_rf);
#endif

//mmath.h
Vector2 normalize(Vector2 n, Vector2 max = map_size) ;
Vector2 normalize2(Vector2 n, Vector2 max) ;

double atan3(double y, double x) ;
inline double atan ( double y, double x ) {return atan3(y,x);}
inline double atan3 ( Vector2 a ) {return atan(a);}
double trajectory_angle(Vector2 p1, Vector2 p2) ;
double intercept_angle(Vector2 from_pos, Vector2 from_vel, double v, Vector2 to, Vector2 to_vel) ;
double intercept_angle2(Vector2 from_pos, Vector2 from_vel, double v, Vector2 to, Vector2 to_vel) ;
double normalize (double value, double max = PI2);
double normalize2(double value, double max);
Vector2 corner ( Vector2 pos, Vector2 size );
Vector2 corner ( Vector2 pos );

double min_delta(double from, double to, double max=PI2);
Vector2 min_delta ( Vector2 from, Vector2 to, Vector2 max = map_size) ;
double nearest_coord(double from, double to, double max);
inline Vector2 nearest_coord ( Vector2 from, Vector2 to, Vector2 max = map_size)
{return Vector2 ( nearest_coord(from.x,to.x,max.x),nearest_coord(from.y,to.y,max.y));}
double distance_from(Vector2 from, Vector2 to);

int sign(double a);
inline int get_index(double angle, double o=PI/2, int n=64) {int i = iround((o + angle) * n / PI2); while (i < 0) i += n; while (i >= n) i -= n; return i;}

//graphics stuff, melee/msprite.cpp

void line ( Surface *dest, Vector2 p1, Vector2 p2, int color ) ;
void line ( Frame *dest, Vector2 p1, Vector2 p2, int color ) ;

void convert_bitmap(BITMAP *src, BITMAP *dest, int masked) ;
void color_correct_bitmap(BITMAP *bmp, int masked) ;

void set_tw_aa_mode( int a );
int get_tw_aa_mode();

// added equivalent routines as a space-sprites', which treat a bmp directly (used by tau mc)
void animate_bmp(BITMAP *bmp, Vector2 p, Vector2 s, Frame *space);
void animate_bmp(BITMAP *bmp, Vector2 pos, Frame *space);

class SpaceSprite
{
	public:
		static int mip_min, mip_max, mip_bias;
	protected:
		enum { MAX_MIP_LEVELS = 8 };
		int         count;
		char bpp;
		char highest_mip;
		int originaltype;
		int         w;
		int         h;
		struct PMASK **m;
		Surface **b[MAX_MIP_LEVELS];

		//char *type;
		int references;
		char *attributes;
		enum { DEALLOCATE_IMAGE = 0x01, DEALLOCATE_MASK = 0x02 };
		unsigned int general_attributes;
	public:
		enum {
			MATCH_SCREEN_FORMAT = 0x001,
			IRREGULAR           = 0x002,
			MIPMAPED            = 0x004,

			MASKED              = 0x100,
			ALPHA               = 0x200,
			DITHER              = 0x400,
			NO_AA               = 0x800,

			NONE = 0
		};
		//sprite_count * rotation new images based upon sprite_count RLE_SPRITES, with gamma correction
		SpaceSprite(const TW_DATAFILE *sprites, int sprite_count, int attributes = -1, int rotations = 1);
		SpaceSprite(BITMAP *image, int _attributes = -1);
		SpaceSprite(SpaceSprite &old);
		SpaceSprite(BITMAP **bmplist, int sprite_count, int rotations, int _attributes);

		virtual ~SpaceSprite();

		//methods for direct access:
		Surface     *get_bitmap(int index, int miplevel = 0);
		Surface     *get_bitmap_readonly(int index);
		const struct PMASK *get_pmask(int index) {return m[index];}
		void lock();			 //make surface writable
		void unlock();

		void overlay ( int index1, int index2, Surface *dest);

		void draw(Vector2 pos, Vector2 size, int index, Frame *frame) ;
		void draw(Vector2 pos, Vector2 size, int index, Surface *bmp) ;
		//void draw(double x, double y, int index, Frame *frame) ;
		//void draw(double x, double y, int index, Surface *bmp) ;
		//void draw(int x, int y, int w, int h, int index, Frame *frame) ;
		//void draw(int x, int y, int w, int h, int index, Surface *bmp) ;
		//void draw(int x, int y, int index, Frame *frame) ;
		void draw(int x, int y, int index, Surface *bmp) ;

		// added GEO
		virtual void animate(Vector2 pos, int index, Frame *space, double scale = 1);
		void animate_character(Vector2 pos, int index, int color, Frame *space, double scale = 1);

		void draw_character(int x, int y, int index, int color, Surface *bmp);
		void draw_character(int x, int y, int index, int color, Frame *space);
		void draw_character(int x, int y, int w, int h, int index, int color, Surface *bmp);
		void draw_character(int x, int y, int w, int h, int index, int color, Frame *space);

		void generate_mipmaps();
		void regenerate_mipmaps();

		void permanent_phase_shift ( int index );

		virtual void change_color_depth(int dest);

		int collide(int x, int y, int i, int ox, int oy, int oi,
			SpaceSprite *other);
		int collide_ray(int lx1, int ly1, int *lx2, int *ly2, int sx, int sy,
			int sindex);

		INLINE int frames() const {return count;}
		// changed Rob.
								 //   const {return Vector2(b[0][i]->w, b[0][i]->h);}
		Vector2  size(int i = 0) const;
		int      width()  const {return w;}
		int      height() const {return h;}
};

int string_to_sprite_attributes ( const char *s, int recommended = SpaceSprite::MASKED | SpaceSprite::MATCH_SCREEN_FORMAT | SpaceSprite::MIPMAPED) ;

//melee/mship

struct ShipClass
{
	int link_order;
	const char *name;
	const char *source;
	Ship *get_ship(Vector2 pos, double angle, ShipData *data, unsigned int code);
	Ship *(*_get_ship)(Vector2 pos, double angle, ShipData *data, unsigned int code);
};
extern ShipClass *shipclasses;
extern int num_shipclasses;
ShipClass *shipclass(const char *name);

void register_ships();

void register_shipclass (const char *name, const char *source_name, Ship *(*getShip)(Vector2 pos, double a, ShipData *data, unsigned int code));
#define REGISTER_SHIP_EX(ship,func) static void __register_ship_ex ## ship ## _ ## func () { register_ship( ship, __FILE__, func);} CALL_BEFORE_MAIN(__register_shipclass_ex ## ship ## _ ## func);
//#define REGISTER_SHIP(ship) static Ship *get_shipclass_ ## ship (Vector2 pos, double a, ShipData *d, unsigned int c){return(new ship(pos,a,d,c));} static void __register_shipclass_ ## ship () {register_shipclass(#ship, __FILE__, &get_shipclass_ ## ship);} CALL_BEFORE_MAIN(__register_shipclass_ ## ship);

#define REGISTER_SHIP(ship) \
	static Ship *get_shipclass_ ## ship (Vector2 pos, double a, ShipData *d, unsigned int c) \
	{ \
		return(new ship(pos,a,d,c)); \
	} \
	void __register_shipclass_ ## ship () \
	{ \
		register_shipclass(#ship, __FILE__, &get_shipclass_ ## ship); \
	}

//extern ShipData **shipdatas;
//extern int num_shipdatas;
//ShipData *shipdata( const char *file );

struct ShipType
{
	const char *id;
	const char *file;
	const char *name;
	const char *text;
	int cost;
	ShipData  *data;
	ShipClass *code;
	Ship *get_ship(Vector2 pos, double angle, unsigned int team);
	int origin;
};
extern ShipType *shiptypes;
extern int num_shiptypes;
ShipType *shiptype(const char *name);

//melee/mshpdata
extern int auto_unload;

ShipData *shipdata ( const char *file ) ;
void unload_all_ship_data() ;
void unload_unused_ship_data() ;

class ShipData
{
	enum {
		LOADED_NONE,
		LOADED_FULL,
		LOADED_PARTIAL,
		LOADED_MINIMAL
	};
	int references;
	public:
		int status;
		char *file;
		void lock();
		void unlock();
		void load();
		void unload();
		TW_DATAFILE    *data;
		//	int num_panel_bitmaps;
		//	Surface     **bitmapPanel;
		SpaceSprite *spritePanel;
		SpaceSprite *spriteShip;
		SpaceSprite *spriteWeapon;
		SpaceSprite *spriteWeaponExplosion;
		SpaceSprite *spriteSpecial;
		SpaceSprite *spriteSpecialExplosion;
		SpaceSprite *spriteExtra;
		SpaceSprite *spriteExtraExplosion;

		int num_more_sprites;
		SpaceSprite **more_sprites;

		int num_weapon_samples;
		Sound     **sampleWeapon;
		int num_special_samples;
		Sound     **sampleSpecial;
		int num_extra_samples;
		Sound     **sampleExtra;

		Music       *moduleVictory;

		ShipData(const char *filename);
		~ShipData();
};

//gui.h
class TW_Dialog_Player : public BaseClass
{
	public:
		struct DIALOG_PLAYER *player;
		struct DIALOG *dialog;
		struct BITMAP *subscreen;
		int *old_sizes;
		TW_Dialog_Player *prev_level;
		int length, level, ifocus;
		VideoWindow *window;
		void init ( VideoWindow *window, DIALOG *dialog, int index = 0);
		int update ();
		void redraw ();
		void deinit ();
		virtual void _event( Event * e);
};
struct DIALOG;
int tw_do_dialog ( VideoWindow *window, DIALOG *d, int index );
int tw_popup_dialog ( VideoWindow *window, DIALOG *d, int index );
int d_tw_bitmap_proc(int msg, DIALOG *d, int c);
int d_tw_yield_proc(int msg, DIALOG *d, int c);
#endif							 // __MELEE_H__
