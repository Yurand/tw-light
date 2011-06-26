## Process this file with automake to produce Makefile.in
# $Id: Makefile.am 2378 2008-03-21 14:28:26Z eht16 $

SUBDIRS = libraries ships

bin_PROGRAMS = tw-light

SRCS = \
games/gdefender.cpp \
games/gflmelee.cpp \
games/gflmelee.h \
games/ggob.cpp \
games/ggob.h \
doxygen.cpp \
ais.h \
input.h \
ais/c_input.cpp \
ais/c_other.cpp \
ais/c_wussie.cpp \
twgui/twmenuexamples.h \
twgui/twmenuexamples.cpp \
twgui/twwindow.h \
twgui/gamebuttonevent.h \
twgui/twbuttontypes.h \
twgui/twbutton.h \
twgui/twhelpers.h \
twgui/twgui.cpp \
twgui/twpopup.h \
twgui/twwindow.cpp \
twgui/twhelpers.cpp \
twgui/twgui.h \
twgui/utils.cpp \
twgui/utils.h \
twgui/twbuttontypes.cpp \
twgui/twpopup.cpp \
twgui/twbutton.cpp \
scp.cpp \
gui.h \
frame.h \
input.cpp  \
util/history.cpp \
util/sounds.cpp \
util/types.h \
util/aastr.h \
util/history.h \
util/vector2.cpp \
util/get_time.h \
util/types.cpp \
util/errors.cpp \
util/sounds.h \
util/vector2.h \
util/helper.c \
util/round.h \
util/helper.h \
util/errors_c.c \
util/pmask.c \
util/errors.h \
util/aarot.c \
util/get_time.c \
util/base.cpp \
util/pmask.h \
util/port.h \
util/port_allegro.c \
util/round.c \
util/random.cpp \
util/aautil.c \
util/endian.h \
util/random.h \
util/base.h \
util/endian.cpp \
util/aastr.c \
util/aautil.h \
melee.h \
id.h \
ship.h \
melee/mframe.h \
melee/mtarget.cpp \
melee/moptions.cpp \
melee/mshpdata.cpp \
melee/mcbodies.cpp \
melee/mship.h \
melee/mshppan.cpp \
melee/mfleet.h \
melee/mitems.h \
melee/mlog.cpp \
melee/manim.h \
melee/mnet1.cpp \
melee/mframe.cpp \
melee/mship.cpp \
melee/moptions.h \
melee/mlog.h \
melee/manim.cpp \
melee/mfleet.cpp \
melee/mshppan.h \
melee/mcbodies.h \
melee/mhelpers.cpp \
melee/msprite.cpp \
melee/mgame.cpp \
melee/mcontrol.cpp \
melee/mnet1.h \
melee/mcontrol.h \
melee/mmain.cpp \
melee/mmain.h \
melee/mitems.cpp \
melee/mview.h \
melee/mshot.cpp \
melee/mgame.h \
melee/mmath.cpp \
melee/mtarget.h \
melee/mview.cpp \
melee/mshot.h \
scp.h \
other/gameconf.cpp \
other/gameconf.h \
other/gup.cpp \
other/gup.h \
other/shippart.cpp \
other/twconfig.cpp \
other/radar.h \
other/twconfig.h \
other/orbit.h \
other/vbodies.h \
other/objanim.h \
other/orbit.cpp \
other/nullphas.h \
other/radar.cpp \
other/objanim.cpp \
other/configrw.cpp \
other/nullphas.cpp \
other/vtarget.h \
other/planet3d.cpp \
other/vbodies.cpp \
other/vtarget.cpp \
other/shippart.h \
other/planet3d.h \
other/fontmorph.cpp \
other/fontmorph.h \
other/configrw.h \
other/dialogs.h \
other/dialogs.cpp \
frame.cpp \
gui.cpp

INCLUDES = -I$(srcdir) $(allegro_CFLAGS)

# build Geany for all other platforms

tw_light_SOURCES = $(SRCS)

tw_light_LDADD = libraries/libinternal.a ships/libships.a $(allegro_LIBS)

AM_CFLAGS = -DTWLIGHT_DATADIR=\""$(datadir)/tw-light"\" \
	-DTWLIGHT_DOCDIR=\""$(docdir)"\" \
	-DTWLIGHT_LIBDIR=\""$(libdir)"\" \
	-DTWLIGHT_LOCALEDIR=\""$(localedir)"\" \
	-DTWLIGHT_PREFIX=\""$(prefix)"\"

AM_CXXFLAGS = $(AM_CFLAGS)
