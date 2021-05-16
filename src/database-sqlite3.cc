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
#include <time.h>

#include "error.h"
#include "application.h"

#ifdef NATIVE_WINDOWS
# include "strptime.h"
#endif

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
		const char *c_homepage = (const char*) sqlite3_column_text(stmt,1);
		std::string homepage;
		if (c_homepage)
			homepage = c_homepage;

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
	int err = SQLITE_OK;
	
	if (breeder->get_id()) {
		const char *sql = "UPDATE breeder SET name=?,homepage=? WHERE id=?;";
		err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
		if (err != SQLITE_OK) {
			Glib::ustring msg = _("Unable to update breeder!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			if (stmt)
				sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
		Glib::ustring name = breeder->get_name();
		std::string homepage = breeder->get_homepage();
		sqlite3_bind_text(stmt,1,name.c_str(),-1,0);
		sqlite3_bind_text(stmt,2,homepage.c_str(),-1,0);
		sqlite3_bind_int64(stmt,3,static_cast<sqlite3_int64>(breeder->get_id()));

		err = sqlite3_step(stmt);
		if (err != SQLITE_OK && err != SQLITE_DONE) {
			Glib::ustring msg = _("Updating breeder failed!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
	} else {
		const char *sql = "INSERT INTO breeder (name,homepage) VALUES (?,?);";
		err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
		if (err != SQLITE_OK) {
			Glib::ustring msg = _("Unable to insert breeder into database!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			if (stmt)
				sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
		Glib::ustring name = breeder->get_name();
		std::string homepage = breeder->get_homepage();
		
		sqlite3_bind_text(stmt,1,name.c_str(),-1,0);
		sqlite3_bind_text(stmt,2,homepage.c_str(),-1,0);

		err = sqlite3_step(stmt);
		if (err != SQLITE_OK && err != SQLITE_DONE) {
			Glib::ustring msg = _("Inserting breeder into database failed!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
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

	begin_transaction();
	
	sqlite3_stmt *stmt = nullptr;
	int err = 0;
	
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

/**** Growlog methods *********************************************************/

std::list<Glib::RefPtr<Growlog> > 
DatabaseSqlite3::get_growlogs_vfunc() const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,title,description,created_on,flower_on,finished_on FROM growlog ORDER BY title;";
	std::list<Glib::RefPtr<Growlog> > ret;
	sqlite3_stmt *stmt = nullptr;

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to fetch growlogs from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		tm datetime;
		uint64_t id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		Glib::ustring title = (const char*) sqlite3_column_text(stmt,1);
		Glib::ustring description = (const char*) sqlite3_column_text(stmt,2);
		Glib::ustring created_on_str = (const char*) sqlite3_column_text(stmt,3);
		strptime(created_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);
		time_t flower_on = 0;
		time_t finished_on = 0;
		
		if (sqlite3_column_type(stmt,4) != SQLITE_NULL) {
			Glib::ustring flower_on_str = (const char*) sqlite3_column_text(stmt,4);
			if (!flower_on_str.empty()) {
				strptime(flower_on_str.c_str(),DATE_ISO_FORMAT,&datetime);
				flower_on = mktime(&datetime);
			}
		}

		if (sqlite3_column_type(stmt,5) != SQLITE_NULL) {
			Glib::ustring finished_on_str = (const char*) sqlite3_column_text(stmt,5);
			if (!finished_on_str.empty()) {
				strptime(finished_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
				finished_on = mktime(&datetime);
			}
		}
		Glib::RefPtr<Growlog> growlog = Growlog::create(id,title,description,created_on,flower_on,finished_on);
		if (growlog)
			ret.push_back(growlog);
	}
	sqlite3_finalize(stmt);
	return ret;
}

std::list<Glib::RefPtr<Growlog> >
DatabaseSqlite3::get_ongoing_growlogs_vfunc() const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,title,description,created_on,flower_on,finished_on FROM growlog WHERE finished_on IS NULL ORDER BY title;";
	std::list<Glib::RefPtr<Growlog> > ret;
	sqlite3_stmt *stmt = nullptr;

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to fetch growlogs from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		tm datetime;
		uint64_t id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		Glib::ustring title = (const char*) sqlite3_column_text(stmt,1);
		Glib::ustring description = (const char*) sqlite3_column_text(stmt,2);
		Glib::ustring created_on_str = (const char*) sqlite3_column_text(stmt,3);
		strptime(created_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);
		time_t flower_on = 0;
		time_t finished_on = 0;
		
		if (sqlite3_column_type(stmt,4) != SQLITE_NULL) {
			Glib::ustring flower_on_str = (const char*) sqlite3_column_text(stmt,4);
			if (!flower_on_str.empty()) {
				strptime(flower_on_str.c_str(),DATE_ISO_FORMAT,&datetime);
				flower_on = mktime(&datetime);
			}
		}

		if (sqlite3_column_type(stmt,5) != SQLITE_NULL) {
			Glib::ustring finished_on_str = (const char*) sqlite3_column_text(stmt,5);
			if (!finished_on_str.empty()) {
				strptime(finished_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
				finished_on = mktime(&datetime);
			}
		}
		Glib::RefPtr<Growlog> growlog = Growlog::create(id,title,description,created_on,flower_on,finished_on);
		if (growlog)
			ret.push_back(growlog);
	}
	sqlite3_finalize(stmt);
	return ret;
}

std::list<Glib::RefPtr<Growlog> >
DatabaseSqlite3::get_finished_growlogs_vfunc() const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,title,description,created_on,flower_on,finished_on FROM growlog WHERE finished_on IS NOT NULL ORDER BY title;";
	std::list<Glib::RefPtr<Growlog> > ret;
	sqlite3_stmt *stmt = nullptr;

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to fetch growlogs from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		tm datetime;
		uint64_t id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		Glib::ustring title = (const char*) sqlite3_column_text(stmt,1);
		Glib::ustring description = (const char*) sqlite3_column_text(stmt,2);
		Glib::ustring created_on_str = (const char*) sqlite3_column_text(stmt,3);
		strptime(created_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);
		time_t flower_on = 0;
		time_t finished_on = 0;
		
		if (sqlite3_column_type(stmt,4) != SQLITE_NULL) {
			Glib::ustring flower_on_str = (const char*) sqlite3_column_text(stmt,4);
			if (!flower_on_str.empty()) {
				strptime(flower_on_str.c_str(),DATE_ISO_FORMAT,&datetime);
				flower_on = mktime(&datetime);
			}
		}

		if (sqlite3_column_type(stmt,5) != SQLITE_NULL) {
			Glib::ustring finished_on_str = (const char*) sqlite3_column_text(stmt,5);
			if (!finished_on_str.empty()) {
				strptime(finished_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
				finished_on = mktime(&datetime);
			}
		}
		Glib::RefPtr<Growlog> growlog = Growlog::create(id,title,description,created_on,flower_on,finished_on);
		if (growlog)
			ret.push_back(growlog);
	}
	sqlite3_finalize(stmt);
	return ret;
}

std::list<Glib::RefPtr<Growlog> > 
DatabaseSqlite3::get_growlogs_for_strain_vfunc(uint64_t strain_id) const
{
	assert(m_db_);
	
	const char *sql0 = "SELECT growlog FROM growlog_strain WHERE strain=?;";
	const char *sql1 = "SELECT title,description,created_on,flower_on,finished_on FROM growlog WHERE id=?;";

	sqlite3_stmt *stmt0 = nullptr;
	sqlite3_stmt *stmt1 = nullptr;

	std::list<Glib::RefPtr<Growlog> > ret;

	int err = sqlite3_prepare(m_db_,sql0,-1,&stmt0,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to fetch growlog from 'growlog_strain'!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt0)
			sqlite3_finalize(stmt0);
	}
	sqlite3_bind_int64(stmt0,1,static_cast<sqlite3_int64>(strain_id));
	while (sqlite3_step(stmt0) == SQLITE_ROW) {
		sqlite3_int64 growlog_id = sqlite3_column_int64(stmt0,0);
		err = sqlite3_prepare(m_db_,sql1,-1,&stmt1,0);
		if (err != SQLITE_OK) {
			Glib::ustring msg = _("Unable to fetch growlog from database!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			sqlite3_finalize(stmt0);
			if (stmt1)
				sqlite3_finalize(stmt1);
			throw DatabaseError(err,msg);
		}
		sqlite3_bind_int64(stmt1,1,growlog_id);
		if (sqlite3_step(stmt1) == SQLITE_ROW) {
			Glib::ustring title = (const char*) sqlite3_column_text(stmt1,0);
			Glib::ustring desc = (const char*) sqlite3_column_text(stmt1,1);
			Glib::ustring created_on_str = (const char*) sqlite3_column_text(stmt1,2);
			tm datetime;
			strptime(created_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
			time_t created_on = mktime(&datetime);
			time_t flower_on = 0;
			time_t finished_on = 0;

			if (sqlite3_column_type(stmt1,3) != SQLITE_NULL) {
				Glib::ustring flower_on_str = (const char*) sqlite3_column_text(stmt1,3);
				if (!flower_on_str.empty()) {
					strptime(flower_on_str.c_str(),DATE_ISO_FORMAT,&datetime);
					flower_on = mktime(&datetime);
				}
			}

			if (sqlite3_column_type(stmt1,4) != SQLITE_NULL) {
				Glib::ustring finished_on_str = (const char*) sqlite3_column_text(stmt1,4);
				if (!finished_on_str.empty()) {
					strptime(finished_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
					finished_on = mktime(&datetime);
				}
			}
			Glib::RefPtr<Growlog> growlog = Growlog::create(static_cast<uint64_t>(growlog_id),
			                                                title,
			                                                desc,
			                                                created_on,
			                                                flower_on,
			                                                finished_on);
			ret.push_back(growlog);
		}
		sqlite3_finalize(stmt1);
	}
	sqlite3_finalize(stmt0);
	return ret;
}

Glib::RefPtr<Growlog>
DatabaseSqlite3::get_growlog_vfunc(uint64_t id) const
{
	assert(m_db_);
	
	const char *sql = "SELECT title,description,created_on,flower_on,finished_on FROM growlog WHERE id=?;";
	sqlite3_stmt *stmt = nullptr;
	Glib::RefPtr<Growlog> ret;

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to fetch growlog from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}

	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(id));

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		tm datetime;
		Glib::ustring title = (const char*) sqlite3_column_text(stmt,0);
		Glib::ustring desc = (const char*) sqlite3_column_text(stmt,1);
		Glib::ustring created_on_str = (const char*) sqlite3_column_text(stmt,2);
		strptime(created_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);
		time_t flower_on = 0;
		time_t finished_on = 0;
		if (sqlite3_column_type(stmt,3) != SQLITE_NULL) {
			Glib::ustring flower_on_str = (const char*) sqlite3_column_text(stmt,3);
			if (!flower_on_str.empty()) {
				strptime(flower_on_str.c_str(),DATE_ISO_FORMAT,&datetime);
				flower_on = mktime(&datetime);
			}
		}
		if (sqlite3_column_type(stmt,4) != SQLITE_NULL) {
			Glib::ustring finished_on_str = (const char*) sqlite3_column_text(stmt,4);
			if (!finished_on_str.empty()) {
				strptime(finished_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
				finished_on = mktime(&datetime);
			}
		}
		ret = Growlog::create(id,title,desc,created_on,flower_on,finished_on);
	}
	sqlite3_finalize(stmt);
	return ret;
}

Glib::RefPtr<Growlog>
DatabaseSqlite3::get_growlog_vfunc(const Glib::ustring &title) const
{
	assert(m_db_);

	const char *sql = "SELECT id,description,created_on,flower_on,finished_on FROM growlog WHERE title=?;";
	sqlite3_stmt *stmt = nullptr;
	Glib::RefPtr<Growlog> ret;
	
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to fetch growlog from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}

	sqlite3_bind_text(stmt,1,title.c_str(),-1,0);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		tm datetime;
		uint64_t id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		Glib::ustring desc = (const char*) sqlite3_column_text(stmt,1);
		Glib::ustring created_on_str = (const char*) sqlite3_column_text(stmt,2);
		strptime(created_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);
		time_t flower_on = 0;
		time_t finished_on = 0;
		if (sqlite3_column_type(stmt,3) != SQLITE_NULL) {
			Glib::ustring flower_on_str = (const char*) sqlite3_column_text(stmt,3);
			if (!flower_on_str.empty()) {
				strptime(flower_on_str.c_str(),DATE_ISO_FORMAT,&datetime);
				flower_on = mktime(&datetime);
			}
		}
		if (sqlite3_column_type(stmt,4) != SQLITE_NULL) {
			Glib::ustring finished_on_str = (const char*) sqlite3_column_text(stmt,4);
			if (!finished_on_str.empty()) {
				strptime(finished_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
				finished_on = mktime(&datetime);
			}
		}
		ret = Growlog::create(id,title,desc,created_on,flower_on,finished_on);
	}
	sqlite3_finalize(stmt);
	return ret;
}

void
DatabaseSqlite3::add_growlog_vfunc(const Glib::RefPtr<Growlog> &growlog)
{
	assert(m_db_);
	assert(growlog);
	
	begin_transaction ();
	
	sqlite3_stmt *stmt = nullptr;
	Glib::ustring title = growlog->get_title();
	Glib::ustring desc = growlog->get_description();
	Glib::ustring created_on_str = growlog->get_created_on_format(DATETIME_ISO_FORMAT);
	Glib::ustring flower_on_str;
	Glib::ustring finished_on_str;
	if (growlog->get_flower_on())
		flower_on_str = growlog->get_flower_on_format(DATE_ISO_FORMAT);
	if (growlog->get_finished_on())
		finished_on_str = growlog->get_finished_on_format(DATETIME_ISO_FORMAT);
	
	if (growlog->get_id()) {
		const char *sql = "UPDATE growlog SET title=?,description=?,created_on=?,flower_on=?,finished_on=? WHERE id=?;";
			
		int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
		if (err != SQLITE_OK) {
			Glib::ustring msg = _("Unable to update growlog!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			if (stmt)
				sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
		
		sqlite3_bind_text(stmt,1,title.c_str(),-1,0);
		sqlite3_bind_text(stmt,2,desc.c_str(),-1,0);
		sqlite3_bind_text(stmt,3,created_on_str.c_str(),-1,0);
		if (flower_on_str.empty()) {
			sqlite3_bind_null(stmt,4);
		} else {
			sqlite3_bind_text(stmt,4,flower_on_str.c_str(),-1,0);
		}
		if (finished_on_str.empty()) {
			sqlite3_bind_null(stmt,5);
		} else {
			sqlite3_bind_text(stmt,5,finished_on_str.c_str(),-1,0);
		}
		sqlite3_bind_int64(stmt,6,static_cast<sqlite3_int64>(growlog->get_id()));

		err = sqlite3_step(stmt);
		if ((err != SQLITE_OK) && (err != SQLITE_DONE)) {
			Glib::ustring msg = _("Updating growlog failed!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
	} else {
		const char *sql = "INSERT INTO growlog (title,description,created_on,flower_on,finished_on) VALUES (?,?,?,?,?);";
		
		int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
		if (err != SQLITE_OK) {
			Glib::ustring msg = _("Unable to insert growlog!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			if (stmt)
				sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
		sqlite3_bind_text(stmt,1,title.c_str(),-1,0);
		sqlite3_bind_text(stmt,2,desc.c_str(),-1,0);
		sqlite3_bind_text(stmt,3,created_on_str.c_str(),-1,0);
		if (flower_on_str.empty()) {
			sqlite3_bind_null(stmt,4);
		} else {
			sqlite3_bind_text(stmt,4,flower_on_str.c_str(),-1,0);
		}
		if (finished_on_str.empty()) {
			sqlite3_bind_null(stmt,5);
		} else {
			sqlite3_bind_text(stmt,5,finished_on_str.c_str(),-1,0);
		}
		
		err = sqlite3_step(stmt);
		if ((err != SQLITE_OK) && (err != SQLITE_DONE)) {
			Glib::ustring msg = _("Inserting growlog into database failed!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
	}
	sqlite3_finalize(stmt);
	commit();
}

void
DatabaseSqlite3::remove_growlog_vfunc(uint64_t id)
{
	assert(m_db_);

	begin_transaction ();
	
	const char *sql = "DELETE FROM growlog WHERE id=?;";
	sqlite3_stmt *stmt = nullptr;
	
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to delete growlog from database!");
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
	if ((err != SQLITE_OK) && (err != SQLITE_DONE)) {
		Glib::ustring msg = _("Deleting growlog failed!");
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

/**** GrowlogEntry methods ****************************************************/

std::list<Glib::RefPtr<GrowlogEntry> >
DatabaseSqlite3::get_growlog_entries_vfunc(uint64_t growlog_id) const
{
	assert(m_db_);

	const char *sql = "SELECT id,entry,created_on FROM growlog_entry WHERE growlog=? ORDER BY created_on;";
	sqlite3_stmt *stmt = nullptr;
	std::list<Glib::RefPtr<GrowlogEntry> > ret;
	
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to fetch growlog-entries from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(growlog_id));

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		uint64_t id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		Glib::ustring text = (const char*) sqlite3_column_text(stmt,1);
		Glib::ustring created_on_str = (const char*) sqlite3_column_text(stmt,2);
		tm datetime;
		strptime(created_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);

		Glib::RefPtr<GrowlogEntry> entry = GrowlogEntry::create(id,growlog_id,text,created_on);
		if (entry)
			ret.push_back(entry);
	}
	sqlite3_finalize(stmt);
	return ret;
}

Glib::RefPtr<GrowlogEntry>
DatabaseSqlite3::get_growlog_entry_vfunc(uint64_t id) const
{
	assert(m_db_);

	const char *sql = "SELECT growlog,entry,created_on FROM growlog_entry WHERE id=?;";
	sqlite3_stmt *stmt = nullptr;
	Glib::RefPtr<GrowlogEntry> ret;

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to fetch growlog-entry from database!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(id));
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		uint64_t growlog_id = static_cast<uint64_t>(sqlite3_column_int64(stmt,0));
		Glib::ustring text = (const char*) sqlite3_column_text(stmt,1);
		Glib::ustring created_on_str = (const char*) sqlite3_column_text(stmt,2);
		tm datetime;
		strptime(created_on_str.c_str(),DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);
		ret = GrowlogEntry::create(id,growlog_id,text,created_on);
	}
	sqlite3_finalize(stmt);
	return ret;
}

void
DatabaseSqlite3::add_growlog_entry_vfunc(const Glib::RefPtr<GrowlogEntry> &entry)
{
	assert(m_db_);
	assert(entry);
	
	begin_transaction();
	
	int err = SQLITE_OK;
	sqlite3_stmt *stmt = nullptr;
	Glib::ustring text = entry->get_text();
	Glib::ustring created_on_str = entry->get_created_on_format(DATETIME_ISO_FORMAT);
	
	if (entry->get_id()) {
		const char *sql = "UPDATE growlog_entry SET entry=? WHERE id=?;";
		err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
		if (err != SQLITE_OK) {
			Glib::ustring msg = _("Unable to update growlog-entry!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			if (stmt)
				sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
		sqlite3_bind_text(stmt,1,text.c_str(),-1,0);
		sqlite3_bind_int64(stmt,2,static_cast<sqlite3_int64>(entry->get_id()));

		err = sqlite3_step(stmt);

		if (err != SQLITE_OK && err != SQLITE_DONE) {
			Glib::ustring msg = _("Updating growlog-entry failed!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
	} else {
		const char *sql = "INSERT INTO growlog_entry (growlog,entry,created_on) VALUES (?,?,?);";
		err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
		if (err != SQLITE_OK) {
			Glib::ustring msg = _("Unable to insert growlog-entry into database!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			if (stmt)
				sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
		sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(entry->get_growlog_id()));
		sqlite3_bind_text(stmt,2,text.c_str(),-1,0);
		sqlite3_bind_text(stmt,3,created_on_str.c_str(),-1,0);

		err = sqlite3_step(stmt);
		if ((err != SQLITE_OK) && (err != SQLITE_DONE)) {
			Glib::ustring msg = _("Inserting growlog-entry failed!");
			msg += "\n(";
			msg += sqlite3_errmsg(m_db_);
			msg += ")";
			sqlite3_finalize(stmt);
			rollback();
			throw DatabaseError(err,msg);
		}
	}
	sqlite3_finalize(stmt);
	commit();
}

void
DatabaseSqlite3::remove_growlog_entry_vfunc(uint64_t id)
{
	assert(m_db_);

	const char *sql = "DELETE FROM growlog_entry WHERE id=?;";
	sqlite3_stmt *stmt = nullptr;

	begin_transaction();

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to delete growlog_entry!");
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
		Glib::ustring msg = _("Deleting growlog-entry failed!");
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

/**** growlog_strain methods **************************************************/

void
DatabaseSqlite3::add_strain_for_growlog_vfunc(uint64_t growlog_id,
                                              uint64_t strain_id)
{
	assert(m_db_);

	const char *sql = "INSERT INTO growlog_strain (growlog,strain) VALUES (?,?);";
	sqlite3_stmt *stmt = nullptr;
	
	begin_transaction();

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to insert into growlog_strain!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(growlog_id));
	sqlite3_bind_int64(stmt,2,static_cast<sqlite3_int64>(strain_id));

	err = sqlite3_step(stmt);
	if (err != SQLITE_OK && err != SQLITE_DONE) {
		Glib::ustring msg = _("Inserting into growlog_strain failed!");
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
DatabaseSqlite3::remove_strain_for_growlog_vfunc(uint64_t growlog_id,
                                                 uint64_t strain_id)
{
	assert(m_db_);

	const char *sql = "DELETE FROM growlog_strain WHERE growlog=? AND strain=?;";
	sqlite3_stmt *stmt = nullptr;

	begin_transaction();

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to delete from growlog_strain!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}

	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(growlog_id));
	sqlite3_bind_int64(stmt,2,static_cast<sqlite3_int64>(strain_id));

	err = sqlite3_step(stmt);
	if (err != SQLITE_OK && err != SQLITE_DONE) {
		Glib::ustring msg = _("Deleting from growlog_strain failed!");
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
DatabaseSqlite3::remove_strain_for_growlog_vfunc(uint64_t growlog_strain_id)
{
	assert(m_db_);

	const char *sql = "DELETE FROM growlog_strain WHERE id=?;";
	sqlite3_stmt *stmt = nullptr;

	begin_transaction();

	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to delete strain from growlog_strain!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		if (stmt)
			sqlite3_finalize(stmt);
		rollback();
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_int64(stmt,1,static_cast<sqlite3_int64>(growlog_strain_id));

	err = sqlite3_step(stmt);
	if (err != SQLITE_OK && err != SQLITE_DONE) {
		Glib::ustring msg = _("Deleting from growlog_strain failed!");
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
