dnl Process this file with autoconf to produce a configure script.

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(tw-light, 0.4)

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CXX

# check for C++ compiler explicitly and fail if none is found, do this check
# after AC_PROG_CXX has set the CXX environment variable
AC_CHECK_PROG([have_cxx], [$CXX], [yes], [no])
if test "x$have_cxx" = "xno"; then
	AC_MSG_ERROR([No C++ compiler not found. Please install a C++ compiler.])
fi

AC_PROG_INSTALL

#AC_DISABLE_STATIC
AM_PROG_LIBTOOL
LIBTOOL="$LIBTOOL --silent"

PKG_CHECK_MODULES(allegro, [allegro >= 4.0])
AC_SUBST(allegro_CFLAGS)
AC_SUBST(allegro_LIBS)

# intltool hack to define install_sh on Debian/Ubuntu systems
if test "x$install_sh" = "x"; then
	install_sh="`pwd`/install-sh"
	AC_SUBST(install_sh)
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
