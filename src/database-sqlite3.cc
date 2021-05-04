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
#include <fstream>

#include "error.h"
#include "application.h"

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

/**** Database methods ********************************************************/

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
	std::string sql_file = Glib::build_filename(app->get_settings()->get_sql_dir(),
	                                            "growbook.sqlite3.sql");
	std::fstream file;
	
	file.open(sql_file,std::ios::in);
	std::string text,sql;

	char *errmsg;
	int err;
	
	while (std::getline(file,text)) {
		if (text.empty())
			continue;
		if (!sql.empty())
			sql+="\n";
		sql+=text;

		if (sql[sql.size() - 1] == ';') {
			err = sqlite3_exec(m_db_,sql.c_str(),0,0,&errmsg);
			if (err != SQLITE_OK) {
				Glib::ustring msg = _("Unable to create GrowBook sqlite3-database!");
				msg += "\n(";
				msg += errmsg;
				msg += ")";
				sqlite3_free(errmsg);
				throw DatabaseError(err,msg);
			}
			sql = "";
		}
	}
}

void
DatabaseSqlite3::begin_transaction()
{
	const char *sql = "BEGIN TRANSACTION;";
	char *errmsg;
	int err = sqlite3_exec(m_db_,sql,0,0,&errmsg);

	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to start a transaction!");
		msg += "\n(";
		msg += errmsg;
		msg += ")";
		sqlite3_free(errmsg);
		throw DatabaseError(err,msg);
	}
}

void
DatabaseSqlite3::rollback()
{
	const char *sql = "ROLLBACK;";
	char *errmsg;
	int err = sqlite3_exec(m_db_,sql,0,0,&errmsg);

	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to rollback changes!");
		msg += "\n(";
		msg += errmsg;
		msg += ")";
		sqlite3_free(errmsg);
		throw DatabaseError(err,msg);
	}
}

void
DatabaseSqlite3::commit()
{
	const char *sql = "COMMIT;";
	char *errmsg;
	int err = sqlite3_exec(m_db_,sql,0,0,&errmsg);

	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to commit changes!");
		msg += "\n(";
		msg += errmsg;
		msg += ")";
		sqlite3_free(errmsg);
		throw DatabaseError(err,msg);
	}
}

/**** Breeder methods *********************************************************/

std::list<Glib::RefPtr<Breeder> >
DatabaseSqlite3::get_breeders_vfunc() const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,name,homepage FROM breeder ORDER BY name;";
	sqlite3_stmt *stmt = nullptr;
	std::list<Glib::RefPtr<Breeder> > breeders;
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to get breeders from database.");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		uint64_t id = sqlite3_column_int64(stmt,0);
		Glib::ustring name = (const char*) sqlite3_column_text(stmt,1);
		std::string homepage = (const char*) sqlite3_column_text(stmt,2);

		Glib::RefPtr<Breeder> breeder = Breeder::create(id,name,homepage);
		breeders.push_back(breeder);
	}
	sqlite3_finalize(stmt);
	return breeders;
}

Glib::RefPtr<Breeder>
DatabaseSqlite3::get_breeder_vfunc(uint64_t id) const
{
	assert(m_db_);

	const char *sql = "SELECT name,homepage FROM breeder WHERE id=?;";
	Glib::RefPtr<Breeder> breeder{};
	sqlite3_stmt *stmt = nullptr;

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to lookup breeder from sqlite3-database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(id));

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		Glib::ustring name = (const char*) sqlite3_column_text(stmt,0);
		std::string homepage = (const char*) sqlite3_column_text(stmt,1);

		breeder = Breeder::create(id,name,homepage);
	}
	sqlite3_finalize(stmt);
	return breeder;
}

Glib::RefPtr<Breeder>
DatabaseSqlite3::get_breeder_vfunc(const Glib::ustring &name) const
{
	assert(m_db_);

	const char *sql = "SELECT id,homepage FROM breeder WHERE name=?;";
	Glib::RefPtr<Breeder> breeder{};
	sqlite3_stmt *stmt = nullptr;

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to lookup breeder from sqlite3-database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_text(stmt,1,name.c_str(),-1,0);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		uint64_t id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		std::string homepage = (const char*) sqlite3_column_text(stmt,1);

		breeder = Breeder::create(id,name,homepage);
	}
	sqlite3_finalize(stmt);
	return breeder;
}

void
DatabaseSqlite3::add_breeder_vfunc(const Glib::RefPtr<Breeder> &breeder)
{
	assert(m_db_);
	assert(breeder);

	begin_transaction();
	
	sqlite3_stmt *stmt = nullptr;
	std::string sql;
	if (!breeder->get_id()) {
		sql = "INSERT INTO breeder(name,homepage) VALUES (?1,?2)";
	} else {
		sql = "UPDATE breeder SET name=?1,homepage=?2 WHERE id=?3";
	}
	int err = sqlite3_prepare(m_db_,sql.c_str(),-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to add breeder to sqlite3-database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_text(stmt,1,breeder->get_name().c_str(),-1,0);
	sqlite3_bind_text(stmt,2,breeder->get_homepage().c_str(),-1,0);
	if (breeder->get_id())
		sqlite3_bind_int64(stmt,3,static_cast<sqlite3_int64>(breeder->get_id()));
	err = sqlite3_step(stmt);
	if (err != SQLITE_OK && err != SQLITE_DONE) {
		Glib::ustring msg = _("Adding breeder to database failed!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}
	sqlite3_finalize(stmt);
	commit();
}

void
DatabaseSqlite3::remove_breeder_vfunc (uint64_t id)
{
	assert(m_db_);

	begin_transaction();
	
	const char *sql = "DELETE FROM breeder WHERE id=?;";
	sqlite3_stmt *stmt = nullptr;
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to delete breeder from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(id));

	err = sqlite3_step(stmt);
	if (err != SQLITE_OK && err != SQLITE_DONE) {
		Glib::ustring msg = _("Deleting breeder from database failed!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}
	sqlite3_finalize(stmt);
	commit();
}

/**** Strain methods **********************************************************/

std::list<Glib::RefPtr<Strain> >
DatabaseSqlite3::get_strains_for_breeder_vfunc(uint64_t breeder_id) const
{
	assert(m_db_);

	const char *sql = "SELECT id,breeder_name,name,info,description,homepage,seedfinder FROM strain_view WHERE breeder_id=? ORDER BY name;";
	sqlite3_stmt *stmt = nullptr;
	std::list<Glib::RefPtr<Strain> > ret;

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to lookup strains from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(breeder_id));
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		uint64_t id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		Glib::ustring breeder_name = (const char*) sqlite3_column_text(stmt,1);
		Glib::ustring name = (const char*) sqlite3_column_text(stmt,2);
		Glib::ustring info = (const char*) sqlite3_column_text(stmt,3);
		Glib::ustring desc = (const char*) sqlite3_column_text(stmt,4);
		std::string homepage = (const char*) sqlite3_column_text(stmt,5);
		std::string seedfinder = (const char*) sqlite3_column_text(stmt,6);

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
	sqlite3_finalize(stmt);
	return ret;
}

std::list<Glib::RefPtr<Strain> >
DatabaseSqlite3::get_strains_for_growlog_vfunc(uint64_t growlog_id) const
{
	assert(m_db_);
	
	const char *sql = "SELECT strain FROM growlog_strain WHERE growlog=?;";
	sqlite3_stmt *stmt = nullptr;
	std::list<Glib::RefPtr<Strain> > ret;
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to lookup strains for growlog!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(growlog_id));

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		uint64_t strain_id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		Glib::RefPtr<Strain> strain = this->get_strain(strain_id);
		if (strain)
			ret.push_back(strain);
	}
	sqlite3_finalize(stmt);
	return ret;
}

Glib::RefPtr<Strain>
DatabaseSqlite3::get_strain_vfunc(uint64_t id) const
{
	assert(m_db_);

	const char *sql = "SELECT breeder_id,breeder_name,name,info,description,homepage,seedfinder FROM strain_view WHERE id=?;";
	Glib::RefPtr<Strain> strain;
	sqlite3_stmt *stmt = nullptr;

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to lookup strain in database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(id));
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		uint64_t breeder_id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		Glib::ustring breeder_name = (const char*) sqlite3_column_text(stmt,1);
		Glib::ustring name = (const char*) sqlite3_column_text(stmt,2);
		Glib::ustring info = (const char*) sqlite3_column_text(stmt,3);
		Glib::ustring desc = (const char*) sqlite3_column_text(stmt,4);
		std::string homepage = (const char*) sqlite3_column_text(stmt,5);
		std::string seedfinder = (const char*) sqlite3_column_text(stmt,6);

		strain = Strain::create(id,
		                        breeder_id,
		                        breeder_name,
		                        name,
		                        info,
		                        desc,
		                        homepage,
		                        seedfinder);
	}
	sqlite3_finalize(stmt);
	return strain;
}

Glib::RefPtr<Strain>
DatabaseSqlite3::get_strain_vfunc(const Glib::ustring &breeder_name,
                                  const Glib::ustring &strain_name) const
{
	assert(m_db_);

	const char *sql = "SELECT id,breeder_id,info,description,homepage,seedfinder FROM strain_view WHERE breeder_name=? AND name=?;";
	Glib::RefPtr<Strain> strain;
	sqlite3_stmt *stmt = nullptr;
	
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to lookup strain in database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_text(stmt,1,breeder_name.c_str(),-1,0);
	sqlite3_bind_text(stmt,1,strain_name.c_str(),-1,0);
	
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		uint64_t id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		uint64_t breeder_id = static_cast<uint64_t>(sqlite3_column_int64(stmt,1));
		Glib::ustring info = (const char*) sqlite3_column_text(stmt,2);
		Glib::ustring desc = (const char*) sqlite3_column_text(stmt,3);
		std::string homepage = (const char*) sqlite3_column_text(stmt,4);
		std::string seedfinder = (const char*) sqlite3_column_text(stmt,5);

		strain = Strain::create(id,
		                        breeder_id,
		                        breeder_name,
		                        strain_name,
		                        info,
		                        desc,
		                        homepage,
		                        seedfinder);
	}
	sqlite3_finalize(stmt);
	return strain;
}

void
DatabaseSqlite3::add_strain_vfunc(const Glib::RefPtr<Strain> &strain)
{
	assert(m_db_);
	assert(strain);

	sqlite3_stmt *stmt = nullptr;
	int err = 0;

	begin_transaction();
	
	Glib::ustring name = strain->get_name();
	Glib::ustring info = strain->get_info();
	Glib::ustring desc = strain->get_description();
	std::string homepage = strain->get_homepage();
	std::string seedfinder = strain->get_seedfinder();
	
	if (strain->get_id()) {
		const char *sql = "UPDATE strain SET name=?,info=?,description=?,homepage=?,seedfinder=? WHERE id=?;";
		err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
		if (err != SQLITE_OK) {
			Glib::ustring msg = _("Unable to update strain!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			if (stmt)
				sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
		sqlite3_bind_text(stmt,1,name.c_str(),-1,0);
		sqlite3_bind_text(stmt,2,info.c_str(),-1,0);
		sqlite3_bind_text(stmt,3,desc.c_str(),-1,0);
		sqlite3_bind_text(stmt,4,homepage.c_str(),-1,0);
		sqlite3_bind_text(stmt,5,seedfinder.c_str(),-1,0);
		sqlite3_bind_int64(stmt,6,static_cast<sqlite3_int64>(strain->get_id()));
	} else {
		const char *sql = "INSERT INTO strain (breeder,name,info,description,homepage,seedfinder) VALUES (?1,?2,?3,?4,?5,?6);";
		err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
		if (err != SQLITE_OK) {
			Glib::ustring msg = _("Unable to insert strain into database!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			if (stmt)
				sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
		sqlite3_bind_int64(stmt,1,strain->get_breeder_id());
		sqlite3_bind_text(stmt,2,name.c_str(),-1,0);
		sqlite3_bind_text(stmt,3,info.c_str(),-1,0);
		sqlite3_bind_text(stmt,4,desc.c_str(),-1,0);
		sqlite3_bind_text(stmt,5,homepage.c_str(),-1,0);
		sqlite3_bind_text(stmt,6,seedfinder.c_str(),-1,0);
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_OK && err != SQLITE_DONE) {
		Glib::ustring msg = _("Could not add strain to database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}
	sqlite3_finalize(stmt);
	commit();
}

void
DatabaseSqlite3::remove_strain_vfunc (uint64_t id)
{
	assert(m_db_);

	const char *sql = "DELETE FROM strain WHERE id=?;";
	sqlite3_stmt *stmt = nullptr;

	begin_transaction ();
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);

	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to delete strain from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}

	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(id));

	err = sqlite3_step(stmt);
	if (err != SQLITE_OK && err != SQLITE_DONE) {
		Glib::ustring msg = _("Deleting strain from database failed!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}
	sqlite3_finalize(stmt);
	commit();
}
