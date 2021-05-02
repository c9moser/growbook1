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

#include <glibmm/i18n.h>
#include <glibmm.h>
#include <string>
#include <fstream>
#include <cassert>

#include "error.h"
#include "application.h"

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
	std::string sql_file = Glib::build_filename(app->get_settings()->get_sql_dir (),
	                                            "growbook.postgresql.sql");
	std::string sql,text;
	std::fstream file;
	
	file.open(sql_file,std::ios::in);
	while (getline(file,text)) {
		if (text.empty())
			continue;
		if (!sql.empty())
			sql+="\n";

		sql+=text;
		if (sql[sql.size() - 1] == ';') {
			PGresult *result = PQexec(m_db_,sql.c_str());
			if (PQresultStatus(result) != PGRES_COMMAND_OK) {
				rollback();
				Glib::ustring msg = _("Unable to create GrowBook database!");
				msg += "\n(";
				msg += PQerrorMessage(m_db_);
				msg += ")";
				PQclear(result);
				throw DatabaseError(msg);
			}
			PQclear(result);
			sql = "";
		}
	}
}

void
DatabasePostgresql::commit()
{
	assert(m_db_);

	PGresult *result = PQexec(m_db_,"COMMIT;");
	if (PQresultStatus(result) != PGRES_COMMAND_OK) {
		Glib::ustring msg = _("Unable to commit changes!");
		msg += "\n(";
		msg += PQerrorMessage(m_db_);
		msg += ")";
		PQclear(result);
		rollback();
		throw DatabaseError(msg);
	}
	PQclear(result);
}

void
DatabasePostgresql::rollback()
{
	assert(m_db_);

	PGresult *result = PQexec(m_db_,"ROLLBACK;");
	if (PQresultStatus(result) != PGRES_COMMAND_OK) {
		Glib::ustring msg = _("Unable to rollback changes!");
		msg += "\n(";
		msg += PQerrorMessage(m_db_);
		msg += ")";
		PQclear(result);
		throw DatabaseError(msg);
	}
	PQclear(result);
}

std::list<Glib::RefPtr<Breeder> >
DatabasePostgresql::get_breeders_vfunc() const
{
	assert(m_db_);
	
	const char *sql="SELECT id,name,homepage FROM breeder ORDER BY name";
	std::list<Glib::RefPtr<Breeder> > breeders;

	PGresult *result = PQexec(m_db_,sql);
	if (PQresultStatus(result) == PGRES_TUPLES_OK) {
		int rows = PQntuples(result);
		for (int i = 0; i < rows; ++i) {
			uint64_t id = std::stoull(PQgetvalue(result,i,0));
			Glib::ustring name = PQgetvalue(result,i,1);
			std::string homepage = PQgetvalue(result,i,2);

			Glib::RefPtr<Breeder> breeder = Breeder::create(id,name,homepage);
			breeders.push_back(breeder);
		}
	}
	PQclear(result);
	return breeders;
}

Glib::RefPtr<Breeder>
DatabasePostgresql::get_breeder_vfunc(uint64_t id) const
{
	assert(m_db_);

	const char *sql = "SELECT name,homepage FROM breeder WHERE id=$1;";
	Glib::RefPtr<Breeder> breeder;
	const char *values[1];
	std::string str = std::to_string(id);
	values[0] = str.c_str();

	PGresult *result = PQexecParams(m_db_,sql,1,NULL,values,NULL,NULL,0);

	if (PQresultStatus(result) == PGRES_TUPLES_OK) {
		Glib::ustring name = PQgetvalue(result,0,0);
		std::string homepage = PQgetvalue(result,0,1);

		breeder = Breeder::create(id,name,homepage);
	}
	PQclear(result);
	
	return breeder;
}

Glib::RefPtr<Breeder>
DatabasePostgresql::get_breeder_vfunc(const Glib::ustring &name) const
{
	assert(m_db_);

	const char *sql = "SELECT id,homepage FROM breeder WHERE name=$1;";
	Glib::RefPtr<Breeder> breeder;
	const char *values[1];
	values[0] = name.c_str();

	PGresult *result = PQexecParams(m_db_,sql,1,NULL,values,NULL,NULL,0);
	if (PQresultStatus(result) == PGRES_TUPLES_OK) {
		uint64_t id = std::stoull(PQgetvalue(result,0,0));
		std::string homepage = PQgetvalue(result,0,1);

		breeder = Breeder::create(id,name,homepage);
	}
	PQclear(result);
	
	return breeder;
}

void
DatabasePostgresql::add_breeder_vfunc(const Glib::RefPtr<Breeder> &breeder)
{
	assert(m_db_);
	assert(breeder);

	if (breeder->get_id()) {
		const char *sql = "UPDATE breeder SET name=$1,homepage=$2 WHERE id=$3;";
		const char *values[3];
		std::string id_str=std::to_string(breeder->get_id());
		Glib::ustring name = breeder->get_name();
		std::string homepage = breeder->get_homepage(); 
		values[0] = name.c_str();
		values[1] = homepage.c_str();
		values[2] = id_str.c_str();

		PGresult *result = PQexecParams(m_db_,sql,3,NULL,values,NULL,NULL,0);
		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			Glib::ustring msg = _("Unable to update breeder!");
			msg += "\n(";
			msg += PQerrorMessage(m_db_);
			msg += ")";
			PQclear(result);
			rollback();
			throw DatabaseError(msg);
		}
		PQclear(result);
	} else {
		const char *sql = "INSERT INTO breeder (name,homepage) VALUES ($1,$2);";
		const char *values[2];
		Glib::ustring name = breeder->get_name();
		std::string homepage = breeder->get_homepage();
		values[0] = name.c_str();
		values[1] = homepage.c_str();

		PGresult *result = PQexecParams(m_db_,sql,2,NULL,values,NULL,NULL,0);
		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			Glib::ustring msg = _("Unable to insert breeder!");
			msg += "\n(";
			msg += PQerrorMessage(m_db_);
			msg += ")";
			PQclear(result);
			rollback();
			throw DatabaseError(msg);
		}
		PQclear(result);
	}
	commit();
}

std::list<Glib::RefPtr<Strain> >
DatabasePostgresql::get_strains_for_breeder_vfunc(uint64_t breeder_id) const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,breeder_name,name,info,decription,homepage,seedfinder FROM strain_view WHERE breeder_id=$1 ORDER BY name;";
	const char *values[1];
	std::list<Glib::RefPtr<Strain> > ret;
	std::string breeder_id_str = std::to_string(breeder_id);
	values[0] = breeder_id_str.c_str();

	PGresult *result = PQexecParams(m_db_,sql,1,NULL,values,NULL,NULL,0);
	if (PQresultStatus(result) == PGRES_TUPLES_OK) {
		int rows = PQntuples(result);
		for (int i=0; i<rows; ++i) {
			uint64_t id = std::stoull(PQgetvalue(result,i,0));
			Glib::ustring breeder_name = PQgetvalue(result,i,1);
			Glib::ustring name = PQgetvalue(result,i,2);
			Glib::ustring info = PQgetvalue(result,i,3);
			Glib::ustring desc = PQgetvalue(result,i,4);
			std::string homepage = PQgetvalue(result,i,5);
			std::string seedfinder = PQgetvalue(result,i,6);

			Glib::RefPtr<Strain> strain = Strain::create(id,
			                                             breeder_id,
			                                             breeder_name,
			                                             name,
			                                             info,
			                                             desc,
			                                             homepage,
			                                             seedfinder);
			ret.push_back(strain);
		}
	}
	PQclear(result);
	return ret;
}

std::list<Glib::RefPtr<Strain> >
DatabasePostgresql::get_strains_for_growlog_vfunc(uint64_t growlog_id) const
{
	assert(m_db_);

	const char *sql = "SELECT strain FROM growlog_strain WHERE growlog=$1;";
	std::list<Glib::RefPtr<Strain> > ret;
	const char* values[1];
	std::string growlog_id_str = std::to_string(growlog_id);
	values[0] = growlog_id_str.c_str();

	PGresult *result = PQexecParams(m_db_,sql,1,NULL,values,NULL,NULL,0);
	if (PQresultStatus(result) == PGRES_TUPLES_OK) {
		int rows = PQntuples(result);
		for (int i=0; i<rows; ++i) {
			uint64_t strain_id = std::stoull(PQgetvalue(result,0,0));
			Glib::RefPtr<Strain> strain = this->get_strain(strain_id);
			if (strain)
				ret.push_back(strain);
		}
	}
	PQclear(result);
	return ret;
}

Glib::RefPtr<Strain>
DatabasePostgresql::get_strain_vfunc(uint64_t id) const
{
	assert(m_db_);

	const char *sql = "SELECT breeder_id,breeder_name,name,info,description,homepage,seedfinder FROM strain_view WHERE id=$1;";
	Glib::RefPtr<Strain> ret;
	const char *values[1];
	std::string id_str = std::to_string(id);
	values[0] = id_str.c_str();

	PGresult *result = PQexecParams(m_db_,sql,1,NULL,values,NULL,NULL,0);
	if (PQresultStatus(result) == PGRES_TUPLES_OK) {
		uint64_t breeder_id = std::stoull(PQgetvalue(result,0,0));
		Glib::ustring breeder_name = PQgetvalue(result,0,1);
		Glib::ustring name = PQgetvalue(result,0,2);
		Glib::ustring info = PQgetvalue(result,0,3);
		Glib::ustring desc = PQgetvalue(result,0,4);
		std::string homepage = PQgetvalue(result,0,5);
		std::string seedfinder = PQgetvalue(result,0,6);

		ret = Strain::create(id,breeder_id,breeder_name,name,info,desc,homepage,seedfinder);
	}
	PQclear(result);
	return ret;
}

Glib::RefPtr<Strain>
DatabasePostgresql::get_strain_vfunc(const Glib::ustring &breeder_name,
                               	     const Glib::ustring &strain_name) const
{
	assert(m_db_);

	const char *sql = "SELECT id,breeder_id,info,description,homepage,seedfinder FROM strain_view WHERE breeder_name=$1 AND name=$2;";
	Glib::RefPtr<Strain> ret;
	const char *values[2];
	values[0] = breeder_name.c_str();
	values[1] = strain_name.c_str();

	PGresult *result = PQexecParams(m_db_,sql,2,NULL,values,NULL,NULL,0);
	if (PQresultStatus(result) == PGRES_TUPLES_OK) {
		uint64_t id = std::stoull(PQgetvalue(result,0,0));
		uint64_t breeder_id = std::stoull(PQgetvalue(result,0,1));
		Glib::ustring info = PQgetvalue(result,0,2);
		Glib::ustring desc = PQgetvalue(result,0,3);
		std::string homepage = PQgetvalue(result,0,4);
		std::string seedfinder = PQgetvalue(result,0,5);

		ret = Strain::create(id,
		                     breeder_id,
		                     breeder_name,
		                     strain_name,
		                     info,
		                     desc,
		                     homepage,
		                     seedfinder);
	}
	PQclear(result);
	return ret;
}

void
DatabasePostgresql::add_strain_vfunc(const Glib::RefPtr<Strain> &strain)
{
	assert(m_db_);
	assert(strain);

	if (strain->get_id()) {
		const char *sql = "UPDATE strain SET name=$1,info=$2,description=$3,homepage=$4,seedfinder=$5 WHERE id=$6;";
		Glib::ustring name = strain->get_name();
		Glib::ustring info = strain->get_info();
		Glib::ustring desc = strain->get_description();
		std::string homepage = strain->get_homepage();
		std::string seedfinder = strain->get_seedfinder();
		std::string id_str = std::to_string(strain->get_id());
		
		const char *values[6];
		values[0] = name.c_str();
		values[1] = info.c_str();
		values[2] = desc.c_str();
		values[3] = homepage.c_str();
		values[4] = seedfinder.c_str();
		values[5] = id_str.c_str();

		PGresult *result = PQexecParams(m_db_,sql,6,NULL,values,NULL,NULL,0);
		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			Glib::ustring msg = _("Unable to update strain!");
			msg += "\n(";
			msg += PQerrorMessage(m_db_);
			msg += ")";
			PQclear(result);
			rollback();
			throw DatabaseError(msg);
		}
		PQclear(result);
	} else {
		const char *sql = "INSERT INTO strain (breeder,name,info,description,homepage,seedfinder) VALUES ($1,$2,$3,$4,$5,$6);";
		std::string breeder_id_str = std::to_string(strain->get_breeder_id());
		Glib::ustring name = strain->get_name();
		Glib::ustring info = strain->get_info();
		Glib::ustring desc = strain->get_description();
		std::string homepage = strain->get_homepage();
		std::string seedfinder = strain->get_seedfinder();

		const char *values[6];
		values[0] = breeder_id_str.c_str();
		values[1] = name.c_str();
		values[2] = info.c_str();
		values[3] = desc.c_str();
		values[4] = homepage.c_str();
		values[5] = seedfinder.c_str();

		PGresult *result = PQexecParams(m_db_,sql,6,NULL,values,NULL,NULL,0);
		if (PQresultStatus(result) != PGRES_COMMAND_OK) {
			Glib::ustring msg = _("Unable to insert strain into database!");
			msg += "\n(";
			msg += PQerrorMessage(m_db_);
			msg += ")";
			PQclear(result);
			rollback();
			throw DatabaseError(msg);
		}
		PQclear(result);
	}
	commit();
}

#endif /* HAVE_LIBPQ */
