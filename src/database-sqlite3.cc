//           database-sqlite3.cc
//  Sa April 24 13:24:21 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// database-sqlite3.cc
//
// Copyright (C) 2021 - Christian Moser
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "database-sqlite3.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sqlite3.h>
#include <cassert>
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <unistd.h>

#include "error.h"

/*******************************************************************************
 * DatabaseModuleSqlite3
 ******************************************************************************/

DatabaseModuleSqlite3::DatabaseModuleSqlite3():
	DatabaseModule{DatabaseSettings::create("sqlite3",
	                                        Glib::build_filename(Glib::get_user_special_dir(Glib::USER_DIRECTORY_DOCUMENTS),
	                                                             "growbook.db").c_str(),
	                                        DB_NAME_IS_FILENAME)}
{}

DatabaseModuleSqlite3::~DatabaseModuleSqlite3()
{}

Glib::RefPtr<DatabaseModuleSqlite3>
DatabaseModuleSqlite3::create()
{
	return Glib::RefPtr<DatabaseModuleSqlite3>(new DatabaseModuleSqlite3());
}

Glib::RefPtr<Database>
DatabaseModuleSqlite3::create_database(const Glib::RefPtr<DatabaseSettings> &settings)
{
	assert(settings->get_engine() == "sqlite3");
	return Glib::RefPtr<Database>::cast_dynamic(DatabaseSqlite3::create(settings));

}

Glib::RefPtr<Database>
DatabaseModuleSqlite3::create_database_vfunc(const Glib::RefPtr<DatabaseSettings> &settings)
{
	assert(settings->get_engine() == "sqlite3");
	return Glib::RefPtr<Database>::cast_dynamic(DatabaseSqlite3::create(settings));

}

/*******************************************************************************
 * DatabaseSqlite3
 ******************************************************************************/

DatabaseSqlite3::DatabaseSqlite3(const Glib::RefPtr<DatabaseSettings> &settings):
	Database{settings},
	m_db_{nullptr}
{
	assert(settings->get_engine() == "sqlite3");
}

DatabaseSqlite3::~DatabaseSqlite3()
{
	if (m_db_)
		sqlite3_close(m_db_);
}

Glib::RefPtr<DatabaseSqlite3>
DatabaseSqlite3::create(const Glib::RefPtr<DatabaseSettings> &settings)
{
	assert(settings->get_engine() == "sqlite3");
	return Glib::RefPtr<DatabaseSqlite3>(new DatabaseSqlite3(settings));
}

bool
DatabaseSqlite3::is_connected_vfunc() const
{
	return (m_db_ != nullptr); 
}

bool
DatabaseSqlite3::test_connection_vfunc()
{
	if (m_db_)
		return true;
	
	bool file_exists = Glib::file_test(get_settings()->get_dbname(),Glib::FILE_TEST_EXISTS);
	sqlite3 *db = nullptr;
	int err = sqlite3_open(get_settings()->get_dbname().c_str(),&db);
	if (db)
		sqlite3_close(db);
	if (!file_exists && Glib::file_test(get_settings()->get_dbname(), Glib::FILE_TEST_EXISTS)) {
		unlink(get_settings()->get_dbname().c_str());
	}
	return (err == SQLITE_OK);
}

void
DatabaseSqlite3::connect_vfunc()
{
	if (m_db_)
		return;

	int err = sqlite3_open(get_settings()->get_dbname().c_str(),&m_db_);
	if (err != SQLITE_OK) {
		if (m_db_) {
			sqlite3_close(m_db_);
			m_db_ = nullptr;
		}
		Glib::ustring msg=_("Unable to connect to database!");
		throw DatabaseError(err,msg);
	}
}

void
DatabaseSqlite3::close_vfunc()
{
	if (m_db_) {
		sqlite3_close(m_db_);
		m_db_ = nullptr;
	}
}

void
DatabaseSqlite3::create_database_vfunc()
{
}

