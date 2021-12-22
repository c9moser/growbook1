dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT([growbook],[0.1.4])

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CXX


dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
#IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=growbook
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT



dnl ***************************************************************************
dnl Check for Windows
dnl ***************************************************************************
AC_CANONICAL_HOST

case $host_os in
  *mingw* | *MINGW* | *MinGW*)
    platform_win32=yes
    native_win32=yes
    AC_DEFINE(NATIVE_WIN32,1,[Define 1 if we are compiling for Windows])
    AC_DEFINE(NATIVE_WINDOWS,1,[Define 1 if we are compiling for Windows])
    ;;
  pw32* | *cygwin*)
    platform_win32=yes
    native_win32=no
    ;;
  *)
    platform_win32=no
    native_win32=no
    ;;
esac

AM_CONDITIONAL(PLATFORM_WIN32, test x"$platform_win32" = "xyes")
AM_CONDITIONAL(NATIVE_WIN32, test x"$native_win32" = "xyes")

LT_INIT([win32-dll])

dnl ***************************************************************************
dnl Check for Packages
dnl ***************************************************************************

PKG_CHECK_MODULES(GROWBOOK, [gtkmm-3.0 >= 3.24])
AC_CHECK_HEADER(sqlite3.h, 
				[AC_DEFINE(HAVE_SQLITE3_H,1,[define 1 if we have sqlite3])],
				AC_MSG_ERROR([No sqlite3 header found!],1))
AC_CHECK_LIB([sqlite3],[sqlite3_open],[],
			 AC_MSG_ERROR([sqlite3 not installed!]))			 

AC_CHECK_LIB(sqlite3,sqlite3_open)

PSQL_CFLAGS=""
PSQL_LIBS=""
PKG_CHECK_MODULES(PGSQL, [libpq],
				  AC_DEFINE(HAVE_LIBPQ,1,[define 1 if we have libpq (PostgreSQL) installed.]),
				  [])

MARIADB_CFLAGS=""
MARIADB_LIBS=""
PKG_CHECK_MODULES(MARIADB, [libmariadb],
				  AC_DEFINE(HAVE_MARIADB,1,[define 1 if we have MariaDB client installed.]),
				  [])

AC_CONFIG_FILES([
Makefile
src/Makefile
po/Makefile.in
])
AC_OUTPUT
