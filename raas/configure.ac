AC_INIT([raas], [0.0])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([../boost.m4/build-aux])
AM_INIT_AUTOMAKE([1.10 foreign color-tests subdir-objects parallel-tests -Wall])
AC_PROG_CC                      
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_CONFIG_HEADERS([config.h]) 
AC_CONFIG_FILES([Makefile])

AC_MSG_CHECKING(for WIN32)
AC_TRY_COMPILE(,
	[
#if defined(_WIN32) || defined(WIN32)
#else
#error Probably not Windows.
#endif
	],
	BUILD_WIN32=true; AC_MSG_RESULT(yes),
	BUILD_WIN32=false; AC_MSG_RESULT(no),
)
AM_CONDITIONAL(BUILD_WIN32, test x$BUILD_WIN32 = xtrue)

# the PKG_CONFIG_PATH export seems to lead to pkg.m4 not being able
# to actually find pkg-config so we explicitly set a path for it
# this has the unfortuante side effect of overriding the external
# env value :(
PKG_CONFIG=$(which pkg-config)
if test x$BUILD_WIN32 = xtrue; then
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH;./pkgconfig-mingw;../build-env/lib/pkgconfig"
PKG_CHECK_MODULES([WIN32_DEPS], [])
else
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:./pkgconfig-default:../build-env/lib/pkgconfig"
PKG_CHECK_MODULES([DEFAULT_DEPS], [libgpg-error])
fi

PKG_CHECK_MODULES([DEPS], [openssl = 0.9.8r, QtCore >= 4, libminiupnpc,
                           libevent, libevent_pthreads,
						   libretroshare, libbitdht, libgpgme,
                           jansson, gio-2.0])

BOOST_REQUIRE
BOOST_SYSTEM
BOOST_FILESYSTEM
BOOST_THREADS
BOOST_REGEX
BOOST_BIND
BOOST_STRING_ALGO

AC_OUTPUT
