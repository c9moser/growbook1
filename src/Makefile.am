## Process this file with automake to produce Makefile.in

## Created by Anjuta


sqldir = $(pkgdatadir)/sql
dist_sql_DATA = config.sql \
	growbook.sqlite3.sql \
	growbook.postgresql.sql \
	growbook.mariadb.sql

iconsdir = $(datadir)/icons/default/scalable

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GROWBOOK_CFLAGS) 

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = growbook

growbook_SOURCES = \
	main.cc \
	application.cc \
	application.h \
	appwindow.cc \
	appwindow.h \
	settings.cc \
	settings.h \
	refclass.h \
	error.cc \
	error.h \
	database.cc \
	database.h \
	database-sqlite3.cc \
	database-sqlite3.h \
	database-postgresql.cc \
	database-postgresql.h \
	databasesettingsdialog.cc \
	databasesettingsdialog.h \
	settingsdialog.cc \
	settingsdialog.h \
	refclass.cc \
	aboutdialog.cc \
	aboutdialog.h \
	datatypes.cc \
	datatypes.h \
	strainchooser.cc \
	strainchooser.h \
	strainselector.cc \
	strainselector.h \
	browserpage.cc \
	browserpage.h \
	strainview.cc \
	strainview.h \
	breederdialog.cc \
	breederdialog.h \
	straindialog.cc \
	straindialog.h \
	strptime.h \
	strptime.cc \
	growlogselector.cc \
	growlogselector.h \
	growlogview.cc \
	growlogview.h \
	growlogdialog.cc \
	growlogdialog.h \
	growlogentrydialog.cc \
	growlogentrydialog.h \
	database-mariadb.cc \
	database-mariadb.h \
	export.cc \
	export.h \
	import.cc \
	import.h \
	xml_importer.cc \
	xml_importer.h \
	debug.h 

growbook_LDFLAGS = 

growbook_LDADD = $(GROWBOOK_LIBS)


if NATIVE_WIN32
growbook_LDFLAGS += -mwindows
endif


icons_DATA = flower-icon.svg

EXTRA_DIST = $(sql_DATA) $(icons_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(sqldir)
	-rm -r $(pkgdatadir)



