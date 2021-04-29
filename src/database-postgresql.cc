//           database-psql.cc
//  Sa April 24 18:38:09 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// database-psql.cc
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_LIBPQ
# include "database-postgresql.h"

#include "error.h"
#include <glibmm/i18n.h>

/*******************************************************************************
 * DatabaseModulePostgresql
 ******************************************************************************/

DatabaseModulePostgresql::DatabaseModulePostgresql():
	DatabaseModule{DatabaseSettings::create("postgresql",
	                                        "postgres",
	                                        "localhost",
	                                        5432,
	                                        "postgres",
	                                        "",
	                                        true,
	                                        (DB_HAS_HOST|DB_HAS_PORT|DB_HAS_USER|DB_HAS_PASSWORD))}
	                                        
{}

DatabaseModulePostgresql::~DatabaseModulePostgresql()
{
}

Glib::RefPtr<DatabaseModulePostgresql>
DatabaseModulePostgresql::create()
{
	return Glib::RefPtr<DatabaseModulePostgresql>(new DatabaseModulePostgresql());
}

Glib::RefPtr<Database>
DatabaseModulePostgresql::create_database_vfunc(const Glib::RefPtr<DatabaseSettings> &settings)
{
	return DatabasePostgresql::create(settings);
}

/*******************************************************************************
 * DatabasePostgresql
 ******************************************************************************/

DatabasePostgresql::DatabasePostgresql(const Glib::RefPtr<DatabaseSettings> &settings):
	Database{settings},
	m_db_{nullptr}
{
}

DatabasePostgresql::~DatabasePostgresql()
{
	if (m_db_)
		PQfinish(m_db_);
}

Glib::RefPtr<DatabasePostgresql>
DatabasePostgresql::create(const Glib::RefPtr<DatabaseSettings> &settings)
{
	return Glib::RefPtr<DatabasePostgresql>(new DatabasePostgresql(settings));
}

bool
DatabasePostgresql::is_connected_vfunc() const
{
	return (m_db_ != nullptr);
}

bool
DatabasePostgresql::test_connection_vfunc()
{
	if (m_db_)
		return true;

	Glib::RefPtr<DatabaseSettings> dbs = get_settings();
	
	PGconn *db  = PQsetdbLogin(dbs->get_host().c_str(),
	                           std::to_string(dbs->get_port()).c_str(),
	                           "",
	                           "",
	                           dbs->get_dbname().c_str(),
	                           dbs->get_user().c_str(),
	                           dbs->get_password().c_str());
	if (PQstatus(db) == CONNECTION_OK) {
		PQfinish(db);
		return true;
	}
	
	PQfinish(db);
	return false;	
}

void
DatabasePostgresql::connect_vfunc()
{
	if (m_db_)
		return;

	Glib::RefPtr<DatabaseSettings> dbs = get_settings();
	
	m_db_ = PQsetdbLogin(dbs->get_host().c_str(),
	                     std::to_string(dbs->get_port()).c_str(),
	                     "",
	                     "",
	                     dbs->get_dbname().c_str(),
	                     dbs->get_user().c_str(),
	                     dbs->get_password().c_str());
	if (!m_db_)
		throw DatabaseError(_("Unable to connect to database!"));
	if (PQstatus(m_db_) == CONNECTION_BAD) {
		Glib::ustring msg = _("Unable to connect to database");
		msg += "\n(";
		msg += PQerrorMessage(m_db_);
		msg += ")";
		PQfinish(m_db_);
		m_db_ = nullptr;
		throw DatabaseError(msg);
	}
}

void
DatabasePostgresql::close_vfunc()
{
	if (m_db_) {
		PQfinish(m_db_);
		m_db_ = nullptr;
	}
}

void
DatabasePostgresql::create_database_vfunc()
{
}

#endif /* HAVE_LIBPQ */