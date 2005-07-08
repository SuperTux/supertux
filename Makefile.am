SUBDIRS = src data

ACLOCAL_AMFLAGS = -I mk/autoconf
AUTOMAKE_OPTIONS = 1.6 dist-bzip2

desktopdir = $(datadir)/applications
desktop_DATA = supertux.desktop

icondir = $(datadir)/pixmaps
icon_DATA = supertux.png

EXTRA_DIST = AUTHORS README COPYING INSTALL NEWS LEVELDESIGN ChangeLog \
	     supertux.desktop supertux.png autogen.sh \
	     $(wildcard mk/autoconf/*.m4)
