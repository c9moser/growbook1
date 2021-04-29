//           database.cc
//  Fr April 23 19:19:49 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// database.cc
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

#include "database.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cassert>

#include "database-sqlite3.h"

#ifdef HAVE_LIBPQ
# include "database-postgresql.h"
#endif

/*******************************************************************************
 * DatabaseModule functions
 ******************************************************************************/

static std::list<Glib::RefPtr<DatabaseModule> > _db_modules;

void db_init() {
	if (_db_modules.empty()) {
		db_add_module(DatabaseModuleSqlite3::create());
#ifdef HAVE_LIBPQ
		db_add_module(DatabaseModulePostgresql::create());
#endif /* HAVE_LIBPQ */
	}
}

std::list<Glib::RefPtr<DatabaseModule> > db_get_modules()
{
	return _db_modules;
}

Glib::RefPtr<DatabaseModule> db_get_module(const Glib::ustring &engine)
{
	Glib::RefPtr<DatabaseModule> ret;
	for (auto iter = _db_modules.begin(); iter != _db_modules.end(); ++iter) {
		if ((*iter)->get_engine() == engine) {
			ret = (*iter);
			break;
		}
	}
	return ret;
}

void db_add_module(const Glib::RefPtr<DatabaseModule> &module) {
	for (auto iter = _db_modules.begin(); iter != _db_modules.end(); ++iter) {
		if ((*iter)->get_engine() == module->get_engine()) {
			(*iter) = module;
			return;
		}
	}
	_db_modules.push_back(module);
}

/*******************************************************************************
 * Database
 ******************************************************************************/

Database::Database(const Glib::RefPtr<DatabaseSettings> &settings) noexcept:
	RefClass{},
	m_settings_{settings}
{
}

Database::~Database()
{
}

Glib::RefPtr<DatabaseSettings>
Database::get_settings()
{
	return m_settings_;
}

Glib::RefPtr<const DatabaseSettings>
Database::get_settings() const
{
	return Glib::RefPtr<const DatabaseSettings>::cast_const(m_settings_);
}

bool
Database::is_connected() const
{
	return this->is_connected_vfunc();
}

bool
Database::test_connection()
{
	return this->test_connection_vfunc();
}

void
Database::create_database()
{
	this->create_database_vfunc();
}

void
Database::connect()
{
	this->connect_vfunc();
}

void
Database::close()
{
	this->close_vfunc();
}

/*******************************************************************************
 * DatabaseModule
 ******************************************************************************/

DatabaseModule::DatabaseModule(const Glib::RefPtr<DatabaseSettings> &defaults) noexcept:
	RefClass{},
	m_defaults_{defaults}
{
	assert(m_defaults_);
}

DatabaseModule::~DatabaseModule()
{
}

Glib::RefPtr<DatabaseSettings>
DatabaseModule::get_defaults()
{
	return m_defaults_;
}

Glib::ustring
DatabaseModule::get_engine() const
{
	return m_defaults_->get_engine();
}

Glib::RefPtr<const DatabaseSettings>
DatabaseModule::get_defaults() const
{
	return Glib::RefPtr<const DatabaseSettings>::cast_const(m_defaults_);
}

Glib::RefPtr<Database>
DatabaseModule::create_database(const Glib::RefPtr<DatabaseSettings> &settings)
{
	return this->create_database_vfunc(settings);
}
