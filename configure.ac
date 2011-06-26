dnl Process this file with autoconf to produce a configure script.

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(tw-light, 0.4)

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_LIBTOOL

# check for C++ compiler explicitly and fail if none is found, do this check
# after AC_PROG_CXX has set the CXX environment variable
AC_CHECK_PROG([have_cxx], [$CXX], [yes], [no])
if test "x$have_cxx" = "xno"; then
	AC_MSG_ERROR([No C++ compiler not found. Please install a C++ compiler.])
fi

PKG_CHECK_MODULES(allegro, [allegro >= 4.0])
AC_SUBST(allegro_CFLAGS)
AC_SUBST(allegro_LIBS)

AC_ARG_ENABLE(debug-sound,
	AC_HELP_STRING([--enable-debug-sound],[Enable sound system debug logs]),
		[
			case "${enableval}" in
				yes) ENABLE_DEBUGSOUND=yes ;;
				no) ENABLE_DEBUGSOUND=no ;;
				*) AC_MSG_ERROR(bad value ${enableval} for --enable-debugsound) ;;
			esac
		], [ENABLE_DEBUGSOUND=no])
if test "${ENABLE_DEBUGSOUND}" = "yes"
then
AC_DEFINE([TW_DEBUG_SOUND], [], [Enable sound system debug logs])
fi

AC_OUTPUT([
Makefile
src/libraries/Makefile
src/ships/Makefile
src/Makefile
])

#src/utils/Makefile
#src/Makefile

echo ""
echo "Configuration is done OK."
echo ""
echo "Debug sound			${ENABLE_DEBUGSOUND}"
