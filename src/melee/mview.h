/*
This file is part of "TW-Light"
					http://timewarp.sourceforge.net/
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

#ifndef __MVIEW_H__
#define __MVIEW_H__

#include "melee.h"
#include "mframe.h"

//global flags: read/write
extern int camera_hides_cloakers;
extern int FULL_REDRAW;

//View stuff: READ ONLY!
// units
extern View  *space_view;

extern Vector2 space_view_size;	 // pixels (should be an integer value)

extern Vector2 space_size;		 // game-pixels

extern double  space_zoom;		 // 1
extern int     space_mip_i;		 // ?
extern double  space_mip;		 // ?

extern Vector2 space_corner;	 // game-pixels : uppper left corner
								 // game-pixels : center, non-wrapped
extern Vector2 space_center_nowrap;
extern Vector2 space_vel;		 // game-pixels / millisecond : center
extern Vector2 space_center;	 // game-pixels : center

/// \brief Camera position information
struct CameraPosition
{
	/// \brief game-pixels : unwrapped position of camera
	Vector2 pos;
	/// \brief game-pixels / ms : velocity of camera
	Vector2 vel;
	/// \brief game-pixels : zoom of camera position
	double z;
};

class View;

/*

What we need:

  list of view types
  get view of default type
  set the default type
  get view of specific type

*/

extern int num_views;
extern char **view_name;

struct ViewType
{
	View *create( View *old = NULL );
	const char *name;
	View *(*_create)();			 //treate as private
};
extern ViewType *viewtypelist;

void __register_viewtype( ViewType n );
#define REGISTER_VIEW(a, b) class a;static View *__create_view ## a (){return new a;} static void __register_view_ ## a () { view_name=(char**)realloc(view_name,sizeof(char*)*(num_views+2));view_name[num_views+1]=NULL;view_name[num_views] = b; viewtypelist=(ViewType*)realloc(viewtypelist,sizeof(ViewType)*(num_views+1));viewtypelist[num_views].name = b; viewtypelist[num_views]._create = __create_view ## a; num_views += 1;} CALL_BEFORE_MAIN( __register_view_ ## a );

void set_view ( View *new_default ) ;
View *get_view( const char *name, View *match ) ;
int get_view_num ( const char * name );

/// Visible area
class View : public BaseClass
{
	public:
		ViewType *type;
		Frame *frame;
		VideoWindow *window;

		CameraPosition camera;
		Vector2 view_size;		 //should contain integer values

		virtual void preinit();
		virtual void init( View *old );
		virtual ~View();
		virtual void config();

		void replace ( View * v );
	protected:

		int focus (CameraPosition *pos, SpaceLocation *a, SpaceLocation *b = NULL);
		//	void see_also (SpaceLocation *other);
		virtual void track ( const CameraPosition &target, CameraPosition *origin = NULL ) ;
		virtual void track ( const CameraPosition &target, double smooth_time, CameraPosition *origin = NULL) ;
	public:

		int key_zoom_in;
		int key_zoom_out;
		int key_alter1;
		int key_alter2;

		virtual void _event( Event *e);

		//	virtual void animate_target(Frame *frame, SpaceLocation *t, int dx, int dy, int r, int c) ;
		//	virtual void set_background (int color);
		//translates screen coordinates to game coordinates
		virtual bool screen2game(Vector2 *pos);
								 //only valid during animate()
		virtual double in_view(Vector2 pos, Vector2 size);
		//called more or less every game tic, with the amount of time passed
		virtual void calculate(Game *game);

		virtual void prepare(Frame *frame, int time = 0);
		virtual void animate ( Game *game );
		void refresh () ;
};

class message_type
{
	enum { max_messages = 16 };
	struct entry_type
	{
		char *string;
		int end_time;
		int color;
	} messages[max_messages];
	int ox, oy;
	int num_messages;
	void clean();
	public:
		message_type() {num_messages = ox = oy = 0;}
		void animate(Frame *frame);
		void flush();
		void out(char *string, int dur = 2000, int c = 15);
		void print(int dur, int c, const char *format, ...);
} extern message;
#endif							 // __MVIEW_H__
