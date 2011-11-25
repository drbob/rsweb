ACLOCAL_AMFLAGS = -I ../boost.m4/build-aux
bin_PROGRAMS = raas
raas_SOURCES = src/raas.cc \
			   src/notifytxt.cc \
			   src/entrypoints/friends.cc \
			   src/entrypoints/chat.cc \
			   src/entrypoints/file_share.cc \
			   src/entrypoints/static.cc \ 
			   src/entrypoints/forums.cc 
AM_CFLAGS = $(RS_CFLAGS) $(DEPS_CFLAGS) -std=c++0x -fdata-sections -ffunction-sections
AM_CPPFLAGS = $(AM_CFLAGS) $(BOOST_CPPFLAGS) -I../threadpool 
AM_LDFLAGS = $(BOOST_LDFLAGS) \
			 $(BOOST_THREAD_LDFLAGS) \
			 $(BOOST_FILESYSTEM_LDFLAGS) \
			 $(BOOST_SYSTEM_LDFLAGS) \
			 $(BOOST_REGEX_LDFLAGS) \
			 $(BOOST_STRING_ALGO_LDFLAGS) \
			 -Wl,--gc-sections

LIBS += $(RS_LIBS) \
		$(DHT_LIBS) \
		$(DEPS_LIBS) \
		$(BOOST_LIBS) \
		$(BOOST_THREAD_LIBS) \
		$(BOOST_FILESYSTEM_LIBS) \
		$(BOOST_SYSTEM_LIBS) \
		$(BOOST_REGEX_LIBS) \
		$(BOOST_STRING_ALGO_LIBS) \
        -lgpgme -lstdc++ -lmagic
