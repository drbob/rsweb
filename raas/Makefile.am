ACLOCAL_AMFLAGS = -I ../boost.m4/build-aux
bin_PROGRAMS = raas
raas_SOURCES = src/raas.cc src/notifytxt.cc
AM_CFLAGS = $(RS_CFLAGS) $(DEPS_CFLAGS) -std=c++0x
AM_CPPFLAGS = $(AM_CFLAGS) $(BOOST_CPPFLAGS) -I../threadpool
AM_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_THREAD_LDFLAGS)
LIBS += $(RS_LIBS) $(DHT_LIBS) $(DEPS_LIBS) $(BOOST_LIBS) $(BOOST_THREAD_LIBS) -lgpgme
