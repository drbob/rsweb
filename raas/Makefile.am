ACLOCAL_AMFLAGS = -I ../boost.m4/build-aux

DEPS_CFLAGS += $(WIN32_DEPS_CFLAGS)
DEPS_LIBS += $(WIN32_DEPS_LIBS)

AM_CFLAGS = $(WIN32_CFLAGS) $(DEPS_CFLAGS) -g \
            -std=c++0x -fdata-sections -ffunction-sections \
			-pedantic -Wall -Wpointer-arith -Wcast-qual \
			-Wextra -fno-common -Wno-unused-function

BOOST_LDFLAGS_ALL = $(BOOST_THREAD_LDFLAGS) \
					$(BOOST_FILESYSTEM_LDFLAGS) \
					$(BOOST_BIND_LDFLAGS) \
					$(BOOST_SYSTEM_LDFLAGS) $(BOOST_REGEX_LDFLAGS) \
					$(BOOST_STRING_ALGO_LDFLAGS)

AM_CPPFLAGS = $(AM_CFLAGS) $(BOOST_CPPFLAGS) 
AM_LDFLAGS = $(BOOST_LDFLAGS_ALL) -Wl,--gc-sections

if BUILD_WIN32
WIN32_CFLAGS = -DWIN32 -DWINDOWS_SYS -mwindows -mconsole
WIN32_LIBS = -lws2_32
endif

bin_PROGRAMS = raas
raas_SOURCES = src/raas.cc \
               src/raas_openssl_win32_applink.c \
			   src/notifytxt.cc \
			   src/http_errors.cc \
			   src/entrypoints/friends.cc \
			   src/entrypoints/chat.cc \
			   src/entrypoints/file_share.cc \
			   src/entrypoints/static.cc \
			   src/entrypoints/profile.cc \
			   src/entrypoints/forums.cc \
			   src/middleware/check_profile.cc

LIBS += $(DHT_LIBS) \
		$(DEPS_LIBS) \
		$(BOOST_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_FILESYSTEM_LIBS) \
		$(BOOST_SYSTEM_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_BIND_LIBS) \
		$(BOOST_STRING_ALGO_LIBS) \
		$(WIN32_LIBS) \
        -lstdc++
