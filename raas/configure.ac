AC_INIT([raas], [0.0])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])    
AC_PROG_CC                      
AC_PROG_CXX 
AC_CONFIG_FILES([Makefile])

PKG_CHECK_MODULES([DEPS], [openssl-xpgp >= 0.9.7, QtCore >= 4.7.4, QtGui, libupnp])

AC_ARG_WITH([libretroshare-include-path],
            [AS_HELP_STRING([--with-libretroshare-include-path],
                            ['location of libretroshare source'])],
            [RS_CFLAGS="-I$withval/src"],
            [RS_CFLAGS='-I../../retroshare/libretroshare/src'])
AC_SUBST([RS_CFLAGS])

AC_ARG_WITH([libretroshare-lib-path],
            [AS_HELP_STRING([--with-libretroshare-lib-path],
                            ['location of libretroshare build dir'])],
            [RS_LIBS="-L$withval/src/lib -lretroshare"],
            [RS_LIBS='-L../../retroshare/libretroshare/src/lib -lretroshare'])
AC_SUBST([RS_LIBS])




AC_OUTPUT
