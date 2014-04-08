AC_INIT([zSILENCERlinux], [1.0], [bug@zsilencer.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_PROG_CC
AC_CONFIG_FILES([
 Makefile
])
CFLAGS="$CFLAGS `sdl2-config --cflags` `pkg-config SDL2_mixer --cflags`"
CXXFLAGS="$CXXFLAGS `sdl2-config --cflags` `pkg-config SDL2_mixer --cflags`"
LIBS="$LIBS `sdl2-config --libs` `pkg-config SDL2_mixer --libs`"
AC_OUTPUT