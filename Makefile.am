SUBDIRS = src data

ACLOCAL_AMFLAGS = -I mk/autoconf
AUTOMAKE_OPTIONS = 1.6 dist-bzip2

desktopdir = $(datadir)/applications
desktop_DATA = supertux-milestone1.desktop

icondir = $(datadir)/pixmaps
icon_DATA = supertux-milestone1.png

svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = supertux-milestone1.svg

metainfodir = $(datadir)/metainfo
metainfo_DATA = supertux-milestone1.appdata.xml

EXTRA_DIST = AUTHORS README COPYING INSTALL NEWS LEVELDESIGN ChangeLog \
	     supertux-milestone1.desktop supertux-milestone1.png autogen.sh \
	     $(wildcard mk/autoconf/*.m4)
