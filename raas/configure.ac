AC_INIT([raas], [0.0])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([../boost.m4/build-aux])
AM_INIT_AUTOMAKE([1.10 foreign color-tests subdir-objects parallel-tests -Wall])
AC_CONFIG_HEADERS([config.h])    
AC_PROG_CC                      
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_CONFIG_FILES([Makefile])

BOOST_REQUIRE
BOOST_SYSTEM
BOOST_FILESYSTEM
BOOST_THREADS
BOOST_REGEX
BOOST_STRING_ALGO

PKG_CHECK_MODULES([DEPS], [openssl, QtCore >= 4, libupnp,
                           libevent, libevent_pthreads, gnome-keyring-1,
                           jansson, gio-2.0])


AC_ARG_WITH([libretroshare-include-path],
            [AS_HELP_STRING([--with-libretroshare-include-path],
                            ['location of libretroshare source'])],
            [RS_CFLAGS="-I$withval/src"],
            [RS_CFLAGS='-I../../retroshare/trunk/libretroshare/src'])
AC_SUBST([RS_CFLAGS])

AC_ARG_WITH([libretroshare-lib-path],
            [AS_HELP_STRING([--with-libretroshare-lib-path],
                            ['location of libretroshare build dir'])],
            [RS_LIBS="$withval"],
            [RS_LIBS='../../retroshare/trunk/libretroshare/src/lib/libretroshare.a'])
AC_SUBST([RS_LIBS])

AC_ARG_WITH([libbitdht-lib-path],
            [AS_HELP_STRING([--with-libbitdht-lib-path],
                            ['location of libbitdht build dir'])],
            [DHT_LIBS="$withval"],
            [DHT_LIBS='../../retroshare/trunk/libbitdht/src/lib/libbitdht.a'])
AC_SUBST([DHT_LIBS])



AC_OUTPUT
