project('growbook', 'cpp',
		version: '0.2.0',
		default_options: ['warning_level=3','cpp_std=c++14'])

subdir('po')

VERSION='"0.2.0"'
PREFIX=get_option('prefix')
DATADIR=get_option('datadir')
PACKAGE_DATADIR=PREFIX + '/' + DATADIR + '/growbook'
PACKAGE_SQLDIR=PACKAGE_DATADIR + '/sql'
PACKAGE_DOCDIR=PREFIX + '/' + DATADIR + '/doc/growbook'

conf=configuration_data()
conf.set('VERSION', VERSION)
conf.set('PACKAGE_VERSION', VERSION)
conf.set('GETTEXT_PACKAGE','"growbook"')
conf.set('PACKAGE_DATA_DIR', '"' + PACKAGE_DATADIR + '"')

if host_machine.system() == 'windows'
	conf.set('NATIVE_WINDOWS',1)
	conf.set('NATIVE_WIN32',1)
endif

cxx=meson.get_compiler('cpp')
sqlite3_dep=cxx.find_library('sqlite3',
				 required: true, 
				 has_headers: ['sqlite3.h'])


gtkmm_dep=dependency('gtkmm-3.0', version: '>=3.24',
					 required: true)
deps=[sqlite3_dep, gtkmm_dep]

libpq_dep=dependency('libpq', required: false)
if libpq_dep.found()
	conf.set('HAVE_LIBPQ',1)
	deps+=[libpq_dep]
endif
mariadb_dep=dependency('libmariadb', required: false)
if mariadb_dep.found()
	conf.set('HAVE_MARIADB',1)
	deps+=[mariadb_dep]
endif

cpp_sources=[
	'src/aboutdialog.cc',
	'src/application.cc',
	'src/appwindow.cc',
	'src/breederdialog.cc',
	'src/browserpage.cc',
	'src/database-mariadb.cc',
	'src/database-postgresql.cc',
	'src/database-sqlite3.cc',
	'src/database.cc',
	'src/databasesettingsdialog.cc',
	'src/datatypes.cc',
	'src/error.cc',
	'src/export.cc',
	'src/growlogdialog.cc',
	'src/growlogentrydialog.cc',
	'src/growlogselector.cc',
	'src/growlogview.cc',
	'src/import.cc',
	'src/main.cc',
	'src/refclass.cc',
	'src/settings.cc',
	'src/settingsdialog.cc',
	'src/strainchooser.cc',
	'src/straindialog.cc',
	'src/strainselector.cc',
	'src/strainview.cc',
	'src/strptime.cc',
	'src/xml_importer.cc']

cpp_headers=[
	'src/aboutdialog.h',
	'src/application.h',
	'src/appwindow.h',
	'src/breederdialog.h',
	'src/browserpage.h',
	'src/database-mariadb.h',
	'src/database-postgresql.h',
	'src/database-sqlite3.h',
	'src/database.h',
	'src/databasesettingsdialog.h',
	'src/datatypes.h',
	'src/debug.h',
	'src/error.h',
	'src/export.h',
	'src/growlogdialog.h',
	'src/growlogentrydialog.h',
	'src/growlogselector.h',
	'src/growlogview.h',
	'src/import.h',
	'src/refclass.h',
	'src/settings.h',
	'src/settingsdialog.h',
	'src/strainchooser.cc',
	'src/strainchooser.h',
	'src/straindialog.h',
	'src/strainselector.h',
	'src/strainview.h',
	'src/strptime.h',
	'src/xml_importer.h']

growbook_cpp_files=cpp_sources
growbook_cpp_files+=cpp_headers

doc_files=[
	'README',
	'COPYING',
	'AUTHORS',
	'ChangeLog',
	'INSTALL',
	'NEWS']
install_data(doc_files,
		install_dir: PACKAGE_DOCDIR)

sql_files=[
	'src/config.sql',
	'src/growbook.sqlite3.sql',
	'src/growbook.postgresql.sql',
	'src/growbook.mariadb.sql']
install_data(sql_files,
		install_dir: PACKAGE_SQLDIR)

includedir=include_directories('src')

executable('growbook', growbook_cpp_files,
		cpp_args: '-DHAVE_CONFIG_H=1',
		install: true,
		dependencies: deps,
		include_directories: [includedir])

configure_file(output: 'config.h',
			   configuration: conf)
