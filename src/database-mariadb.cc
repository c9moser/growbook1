//           database-mariadb.cc
//  Fr Mai 21 16:08:45 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// database-mariadb.cc
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

#ifdef HAVE_MARIADB
#include "database-mariadb.h"

#include <glibmm.h>
#include <glibmm/i18n.h>

#include <cassert>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <memory>

#include "error.h"
#include "application.h"

#ifdef NATIVE_WINDOWS
# include "strptime.h"
#endif

/*******************************************************************************
 * DatabaseModuleMariaDB
 ******************************************************************************/

DatabaseModuleMariaDB::DatabaseModuleMariaDB():
	DatabaseModule{DatabaseSettings::create("MariaDB","","localhost",3306,Glib::get_user_name(),"",true,
	                                        (DB_HAS_HOST|DB_HAS_PORT|DB_HAS_USER|DB_HAS_PASSWORD))}
{
}

DatabaseModuleMariaDB::~DatabaseModuleMariaDB()
{
}

Glib::RefPtr<DatabaseModuleMariaDB>
DatabaseModuleMariaDB::create()
{
	return Glib::RefPtr<DatabaseModuleMariaDB>(new DatabaseModuleMariaDB);
}

Glib::RefPtr<Database>
DatabaseModuleMariaDB::create_database_vfunc(const Glib::RefPtr<DatabaseSettings> &settings)
{
	return DatabaseMariaDB::create(settings);
}

/*******************************************************************************
 * DatabaseMariaDB
 ******************************************************************************/
const char DatabaseMariaDB::RESULT_ERROR[] = N_("Getting result for query failed!");

DatabaseMariaDB::DatabaseMariaDB(const Glib::RefPtr<DatabaseSettings> &settings):
	Database{settings},
	m_db_{nullptr}
{
	assert(settings->get_engine() == "MariaDB");
}

DatabaseMariaDB::~DatabaseMariaDB()
{
	if (m_db_)
		mysql_close(m_db_);
}

Glib::RefPtr<DatabaseMariaDB>
DatabaseMariaDB::create(const Glib::RefPtr<DatabaseSettings> &settings)
{
	return Glib::RefPtr<DatabaseMariaDB>(new DatabaseMariaDB(settings));
}

void
DatabaseMariaDB::database_error(const Glib::ustring &message,
                                bool rollback) const
{
	if (!m_db_) {
		throw DatabaseError(message);
	}
	
	int err = mysql_errno(m_db_);
	Glib::ustring msg = message;
	msg += "\n(";
	msg += mysql_error(m_db_);
	msg += ")";
	if (rollback)
		mysql_rollback(m_db_);
	throw DatabaseError(err,msg);
}

/**** database methods ********************************************************/
void
DatabaseMariaDB::begin_transaction()
{
	assert(m_db_);

	const char *sql = "BEGIN;";
	if (mysql_query(m_db_,sql)) {
		int err = mysql_errno(m_db_);
		Glib::ustring msg = _("Unable to start a transaction!");
		msg += "\n(";
		msg += mysql_error(m_db_);
		msg += ")";
		throw DatabaseError(err,msg);
	}
}

void
DatabaseMariaDB::rollback()
{
	if (mysql_rollback(m_db_)) {
		Glib::ustring msg = _("Unable to rollback transaction!");
		msg += "\n(";
		msg += mysql_error(m_db_);
		msg += ")";
		throw DatabaseError(mysql_errno(m_db_),msg);
	}
}

void
DatabaseMariaDB::commit()
{
	if (mysql_commit(m_db_)) {
		Glib::ustring msg = _("Unable to commit transaction!");
		msg += "\n(";
		msg += mysql_error(m_db_);
		msg += ")";
		throw DatabaseError(mysql_errno(m_db_),msg);
	}
}

bool
DatabaseMariaDB::is_connected_vfunc() const
{
	return m_db_;
}

bool
DatabaseMariaDB::test_connection_vfunc()
{
	if (m_db_) {
		int result = mysql_ping(m_db_);
		if (result)
			return false;
		return true;
	}

	MYSQL *db = mysql_init(nullptr);
	if (db == NULL) return false;

	std::string dbname = get_settings()->get_dbname();
	std::string host = get_settings()->get_host();
	std::string user = get_settings()->get_user();
	std::string password = get_settings()->get_password();
	if (mysql_real_connect(db,host.c_str(),user.c_str(),password.c_str(),dbname.c_str(),get_settings()->get_port(),nullptr,0) == NULL) {
		mysql_close(db);
		return false;
	}
	mysql_close(db);
	return true;
}

void
DatabaseMariaDB::connect_vfunc()
{
	if (m_db_) {
		int result = mysql_ping(m_db_);
		if (!result)
			return;
		if (mariadb_reconnect(m_db_)) {
			Glib::ustring msg = _("Reconnecting to database failed!");
			msg += "\n(";
			msg += mysql_error(m_db_);
			msg += ")";
			throw DatabaseError(mysql_errno(m_db_),msg);
		}
		return;
	}

	m_db_ = mysql_init(nullptr);
	if (!m_db_) {
		Glib::ustring msg = _("Unable to create database-handle!");
		msg += "\n(";
		msg += mysql_error(m_db_);
		msg += ")";
		throw DatabaseError(msg);
	}
	my_bool reconnect = 1;
	mysql_options(m_db_,MYSQL_OPT_RECONNECT, &reconnect);
	
	Glib::RefPtr<DatabaseSettings> settings = get_settings();
	std::string host = settings->get_host();
	std::string user = settings->get_user();
	std::string password = settings->get_password();
	std::string dbname = settings->get_dbname();
	
	if (mysql_real_connect(m_db_,host.c_str(),user.c_str(),password.c_str(),dbname.c_str(),settings->get_port(),nullptr,0) == NULL) {
		Glib::ustring msg = _("Connecting to database failed!");
		msg += "\n(";
		msg += mysql_error(m_db_);
		msg += ")";
		mysql_close(m_db_);
		m_db_ = nullptr;
		throw DatabaseError(mysql_errno(m_db_),msg);
	}
}

void
DatabaseMariaDB::close_vfunc()
{
	if (m_db_) {
		mysql_close(m_db_);
		m_db_ = nullptr;
	}
}

void
DatabaseMariaDB::create_database_vfunc()
{
	assert(m_db_);
	
	std::string sql_file = Glib::build_filename (app->get_settings()->get_sql_dir(),
	                                             "growbook.mariadb.sql");
	std::fstream file;
	file.open(sql_file,std::ios::in);

	std::string text, sql;
	
	while (std::getline(file,text)) {
		if (text.empty())
			continue;
		if (!sql.empty())
			sql+="\n";
		sql += text;
		if (sql[sql.size() - 1] == ';') {
		if (mysql_query(m_db_,sql.c_str())) {
				database_error(_("Creating database failed!"));
			}
			sql = "";
		}
	}
}

/**** Breeder methods *********************************************************/

std::list<Glib::RefPtr<Breeder> > 
DatabaseMariaDB::get_breeders_vfunc() const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,name,homepage FROM breeder ORDER BY name;";
	std::list<Glib::RefPtr<Breeder> > ret;

	if (mysql_query(m_db_,sql)) {
		database_error(_("Unable to fetch breeders from database!")); 
	}
	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result) {
		database_error(_(RESULT_ERROR));
	}

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		uint64_t id = std::stoull(row[0]);
		Glib::ustring name = row[1];
		std::string homepage = row[2] ? row[2] : "";

		Glib::RefPtr<Breeder> breeder = Breeder::create(id,name,homepage);
		if (breeder)
			ret.push_back(breeder);
	}
	mysql_free_result(result);
	return ret;
}

Glib::RefPtr<Breeder> 
DatabaseMariaDB::get_breeder_vfunc(uint64_t id) const
{
	assert(m_db_);

	const char *sql = "SELECT name,homepage FROM breeder WHERE id=%s;";
	Glib::RefPtr<Breeder> breeder;
	std::string id_str = std::to_string(id);

	size_t len = strlen(sql) + id_str.size() + 1;
	std::unique_ptr<char[]> buffer(new char[len]);
	
	snprintf(buffer.get(),len,sql,id_str.c_str());

	if (mysql_query(m_db_,buffer.get())) {
		database_error(_("Unable to get breeder from database!"));
	}
	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result) {
		database_error(_(RESULT_ERROR));
	}

	MYSQL_ROW row =mysql_fetch_row(result);
	if (row) {
		breeder = Breeder::create(id,row[0],row[1] ? row[1] : "");
	}
	mysql_free_result(result);
	return breeder;
}

Glib::RefPtr<Breeder> 
DatabaseMariaDB::get_breeder_vfunc(const Glib::ustring &name) const
{
	const char *sql = "SELECT id,homepage FROM breeder WHERE name='%s';";
	Glib::RefPtr<Breeder> breeder;
	
	size_t name_len = name.bytes() * 2 + 1;
	
	std::unique_ptr<char[]> name_buffer(new char[name_len]);
	mysql_real_escape_string(m_db_,name_buffer.get(),name.c_str(),name.bytes());

	size_t len = strlen(sql) + name_len;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,name_buffer.get());

	if (mysql_query(m_db_,buffer.get())) {
		database_error(_("Unable to fetch breeder from Database!"));
	}

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result) 
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row) {
		uint64_t id = std::stoull(row[0]);
		std::string homepage = row[1] ? row[1] : "";

		breeder = Breeder::create(id,name,homepage);
	}
	mysql_free_result(result);
	return breeder;
}

void 
DatabaseMariaDB::add_breeder_vfunc(const Glib::RefPtr<Breeder> &breeder)
{
	assert(m_db_);

	begin_transaction();
	
	std::string sql_command;
	if (breeder->get_id()) {
		const char *sql = "UPDATE breeder SET name='%s',homepage='%s' WHERE id=%s;";
		std::string id_str = std::to_string(breeder->get_id());

		Glib::ustring name = breeder->get_name();
		size_t name_len = name.bytes() * 2 + 1; 
		std::unique_ptr<char[]> name_buffer(new char[name_len]);
		mysql_real_escape_string(m_db_,name_buffer.get(),name.c_str(),name.bytes());

		std::string homepage = breeder->get_homepage();
		size_t homepage_len = homepage.size() * 2 + 1;
		std::unique_ptr<char[]> homepage_buffer(new char[homepage_len]);
		mysql_real_escape_string(m_db_,homepage_buffer.get(),homepage.c_str(),homepage.size());

		size_t len = (strlen(sql) + id_str.size() + name_len + homepage_len); 
		std::unique_ptr<char[]> buffer(new char[len]);
		snprintf(buffer.get(),len,sql,name_buffer.get(),homepage_buffer.get(),id_str.c_str());

		sql_command = (const char*) buffer.get();
	} else {
		const char *sql = "INSERT INTO breeder (name,homepage) VALUES ('%s','%s');";
		
		Glib::ustring name = breeder->get_name();
		size_t name_len = name.bytes() * 2 + 1;
		std::unique_ptr<char[]> name_buffer(new char[name_len]);
		mysql_real_escape_string(m_db_,name_buffer.get(),name.c_str(),name.bytes());

		std::string homepage = breeder->get_homepage();
		size_t homepage_len = homepage.size() * 2 + 1;
		std::unique_ptr<char[]> homepage_buffer(new char[homepage_len]);
		mysql_real_escape_string(m_db_,homepage_buffer.get(),homepage.c_str(),homepage.size());

		size_t len = (strlen(sql) + name_len + homepage_len);
		std::unique_ptr<char[]> buffer(new char[len]);
		snprintf(buffer.get(),len,sql,name_buffer.get(),homepage_buffer.get());

		sql_command = (const char*) buffer.get();
	}

	if (mysql_query(m_db_,sql_command.c_str())) {
		database_error(_("Unable to add breeder to database!"),true);
	}
	commit();
}

void 
DatabaseMariaDB::remove_breeder_vfunc(uint64_t breeder_id)
{
	assert(m_db_);

	begin_transaction();
	
	const char *sql = "DELETE FROM breeder WHERE id=%s;";
	std::string id_str = std::to_string(breeder_id);
	size_t len = strlen(sql) + id_str.size() + 1;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,id_str.c_str());

	if (mysql_query(m_db_,buffer.get()))
		database_error(_("Unable to delete breeder from database!"),true);
	
	commit();
}

/**** Strain methods **********************************************************/

std::list<Glib::RefPtr<Strain> > 
DatabaseMariaDB::get_strains_for_breeder_vfunc(uint64_t breeder_id) const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,breeder_name,name,info,description,homepage,seedfinder FROM strain_view WHERE breeder_id=%s;";
	std::list<Glib::RefPtr<Strain> > ret;

	std::string breeder_id_str = std::to_string(breeder_id);
	size_t len = strlen(sql) + breeder_id_str.size() + 1;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,breeder_id_str.c_str());

	if (mysql_query(m_db_,buffer.get()))
		database_error(_("Unable to fetch strains for breeder from database!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		uint64_t id = std::stoull(row[0]);
		Glib::ustring breeder_name(row[1]);
		Glib::ustring name(row[2]);
		Glib::ustring info(row[3] ? row[3] : "");
		Glib::ustring desc(row[4] ? row[4] : "");
		std::string homepage(row[5] ? row[5] : "");
		std::string seedfinder(row[6] ? row[6] : "");

		Glib::RefPtr<Strain> strain = Strain::create(id,
		                                             breeder_id,
		                                             breeder_name,
		                                             name,
		                                             info,
		                                             desc,
		                                             homepage,
		                                             seedfinder);
		if (strain)
			ret.push_back(strain);
	}
	mysql_free_result(result);
	return ret;
}

std::list<Glib::RefPtr<Strain> > 
DatabaseMariaDB::get_strains_for_growlog_vfunc(uint64_t growlog_id) const
{
	assert(m_db_);
	
	const char *sql = "SELECT strain FROM growlog_strain WHERE id=%s;";
	std::list<Glib::RefPtr<Strain> > ret;

	std::string growlog_id_str = std::to_string(growlog_id);
	size_t len = strlen(sql) + growlog_id_str.size() + 1;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,growlog_id_str.c_str());

	if (mysql_query(m_db_,sql)) 
		database_error(_("Unable to fetch strains for grwolog!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		uint64_t strain_id = std::stoull(row[0]);
		try {
			Glib::RefPtr<Strain> strain = get_strain(strain_id);
			if (strain)
				ret.push_back(strain);
		} catch (DatabaseError ex) {
			mysql_free_result(result);
			throw ex;
		}
	}
	mysql_free_result(result);
	return ret;
}

Glib::RefPtr<Strain> 
DatabaseMariaDB::get_strain_vfunc(uint64_t id) const
{
	assert(m_db_);
	
	const char *sql = "SELECT breeder_id,breeder_name,name,info,description,homepage,seedfinder FROM strain_view WHERE id=%s;";
	Glib::RefPtr<Strain> strain;
	
	std::string id_str = std::to_string(id);
	size_t len = strlen(sql) + id_str.size() + 1;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,id_str.c_str());

	if (mysql_query(m_db_,buffer.get()))
		database_error(_("Unable to lookup strain!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row) {
		strain = Strain::create(id,
		                        std::stoull(row[0]),
		                        row[1],
		                        row[2],
		                        row[3] ? row[3] : "",
		                        row[4] ? row[4] : "",
		                        row[5] ? row[5] : "",
		                        row[6] ? row[6] : "");
	}
	mysql_free_result(result);
	return strain;
}


Glib::RefPtr<Strain> 
DatabaseMariaDB::get_strain_vfunc(const Glib::ustring &breeder_name,
                                  const Glib::ustring &strain_name) const
{
	assert(m_db_);

	const char *sql = "SELECT id,breeder_id,info,description,homepage,seedfinder FROM strain_view WHERE breeder_name='%s' AND name='%s';";
	Glib::RefPtr<Strain> strain;

	size_t breeder_len = breeder_name.bytes() * 2 + 1;
	std::unique_ptr<char[]> breeder_buffer(new char[breeder_len]);
	mysql_real_escape_string(m_db_,breeder_buffer.get(),breeder_name.c_str(),breeder_name.bytes());

	size_t strain_len = strain_name.bytes() * 2 + 1;
	std::unique_ptr<char[]> strain_buffer(new char[strain_len]);
	mysql_real_escape_string(m_db_,strain_buffer.get(),strain_name.c_str(),strain_name.bytes());

	size_t len = strlen(sql) + breeder_len + strain_len;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,breeder_name.c_str(),strain_name.c_str());

	if (mysql_query(m_db_,buffer.get()))
		database_error(_("Unable to fetch strain from database!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row) {
		strain = Strain::create(std::stoull(row[0]),
		                        std::stoull(row[1]),
		                        breeder_name,
		                        strain_name,
		                        row[2] ? row[2] : "",
		                        row[3] ? row[3] : "",
		                        row[4] ? row[4] : "",
		                        row[5] ? row[5] : "");
	}
	mysql_free_result(result);
	return strain;
}

void
DatabaseMariaDB::add_strain_vfunc(const Glib::RefPtr<Strain> &strain)
{
	assert(m_db_);

	std::string sql_command;

	begin_transaction ();

	if (strain->get_id()) {
		const char *sql="UPDATE strain SET name='%s',info='%s',description='%s',homepage='%s',seedfinder='%s' WHERE id=%s;";

		std::string id_str = std::to_string(strain->get_id());

		Glib::ustring name = strain->get_name();
		Glib::ustring info = strain->get_info();
		Glib::ustring desc = strain->get_description();
		std::string homepage = strain->get_homepage();
		std::string seedfinder = strain->get_seedfinder();

		size_t name_len = name.bytes() * 2 + 1;
		std::unique_ptr<char[]> name_buffer(new char[name_len]);
		mysql_real_escape_string(m_db_,name_buffer.get(),name.c_str(),name.bytes());

		size_t info_len = info.bytes() * 2 + 1;
		std::unique_ptr<char[]> info_buffer(new char[info_len]);
		mysql_real_escape_string(m_db_,info_buffer.get(),info.c_str(),info.bytes());

		size_t desc_len = desc.bytes() * 2 + 1;
		std::unique_ptr<char[]> desc_buffer(new char[desc_len]);
		mysql_real_escape_string(m_db_,desc_buffer.get(),desc.c_str(),desc.bytes());

		size_t homepage_len = homepage.size() * 2 + 1;
		std::unique_ptr<char[]> homepage_buffer(new char[homepage_len]);
		mysql_real_escape_string(m_db_,homepage_buffer.get(),homepage.c_str(),homepage.size());

		size_t seedfinder_len = seedfinder.size() * 2 + 1;
		std::unique_ptr<char[]> seedfinder_buffer(new char[seedfinder_len]);
		mysql_real_escape_string(m_db_,seedfinder_buffer.get(),seedfinder.c_str(),seedfinder.size());

		size_t len = strlen(sql) + id_str.size() + name_len + info_len + desc_len + homepage_len + seedfinder_len;
		std::unique_ptr<char[]> buffer(new char[len]);
		snprintf(buffer.get(), len, sql,
		         name_buffer.get(),
		         info_buffer.get(),
		         desc_buffer.get(),
		         homepage_buffer.get(),
		         seedfinder_buffer.get(),
		         id_str.c_str());
		sql_command = (const char*) buffer.get();
	} else {
		const char *sql = "INSERT INTO strain (breeder,name,info,description,homepage,seedfinder) VALUES (%s,'%s','%s','%s','%s','%s');";

		std::string breeder_id_str = std::to_string(strain->get_breeder_id());

		Glib::ustring name = strain->get_name();
		size_t name_len = name.bytes() * 2 + 1;
		std::unique_ptr<char[]> name_buffer(new char[name_len]);
		mysql_real_escape_string(m_db_,name_buffer.get(),name.c_str(),name.bytes());

		Glib::ustring info = strain->get_info();
		size_t info_len = info.bytes() * 2 + 1;
		std::unique_ptr<char[]> info_buffer(new char[info_len]);
		mysql_real_escape_string(m_db_,info_buffer.get(),info.c_str(),info.bytes());

		Glib::ustring desc = strain->get_description();
		size_t desc_len = desc.bytes() * 2 + 1;
		std::unique_ptr<char[]> desc_buffer(new char[desc_len]);
		mysql_real_escape_string(m_db_,desc_buffer.get(),desc.c_str(),desc.bytes());

		std::string homepage = strain->get_homepage();
		size_t homepage_len = homepage.size() * 2 + 1;
		std::unique_ptr<char[]> homepage_buffer(new char[homepage_len]);
		mysql_real_escape_string(m_db_,homepage_buffer.get(),homepage.c_str(),homepage.size());

		std::string seedfinder = strain->get_seedfinder();
		size_t seedfinder_len = seedfinder.size() * 2 + 1;
		std::unique_ptr<char[]> seedfinder_buffer(new char[seedfinder_len]);
		mysql_real_escape_string(m_db_,seedfinder_buffer.get(),seedfinder.c_str(),seedfinder.size());

		size_t len = (breeder_id_str.size() + name_len + info_len + desc_len + homepage_len + seedfinder_len);
		std::unique_ptr<char[]> buffer(new char[len]);
		snprintf(buffer.get(), len, sql,
		         breeder_id_str.c_str(),
		         name_buffer.get(),
		         info_buffer.get(),
		         desc_buffer.get(),
		         homepage_buffer.get(),
		         seedfinder_buffer.get());
		sql_command = (const char*) buffer.get();
	}

	if (mysql_query(m_db_,sql_command.c_str()))
		database_error(_("Unable to add strain to database!"),true);

	commit();
}

void 
DatabaseMariaDB::remove_strain_vfunc(uint64_t strain_id)
{
	assert(m_db_);

	const char *sql = "DELETE FROM strain WHERE id=%s;";

	begin_transaction();
	
	std::string id_str = std::to_string(strain_id);

	size_t len = strlen(sql) + id_str.size() + 1;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,id_str.c_str());

	if (mysql_query(m_db_,buffer.get()))
		database_error(_("Unable to delete strain!"),true);

	commit();
}

/**** Growlog methods *********************************************************/

std::list<Glib::RefPtr<Growlog> > 
DatabaseMariaDB::get_growlogs_vfunc() const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,title,description,created_on,flower_on,finished_on FROM growlog ORDER BY title;";
	std::list<Glib::RefPtr<Growlog> > ret;
	
	if (mysql_query(m_db_,sql))
		database_error(_("Unable to fetch growlogs from database!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		uint64_t id = std::stoull(row[0]);

		tm datetime;
		strptime(row[3],DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);

		time_t flower_on = 0;
		if (row[4] && strlen(row[4])) {
			strptime(row[4],DATE_ISO_FORMAT,&datetime);
			flower_on = mktime(&datetime);
		}

		time_t finished_on = 0;
		if (row[5] && strlen(row[5])) {
			strptime(row[5],DATETIME_ISO_FORMAT,&datetime);
			finished_on = mktime(&datetime);
		}
		
		Glib::RefPtr<Growlog> growlog = Growlog::create(id,
		                                                row[1],
		                                                row[2] ? row[2] : "",
		                                                created_on,
		                                                flower_on,
		                                                finished_on);
		
		ret.push_back(growlog);
	}
	mysql_free_result(result);
	return ret;	
}

std::list<Glib::RefPtr<Growlog> > 
DatabaseMariaDB::get_ongoing_growlogs_vfunc() const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,title,description,created_on,flower_on,finished_on FROM growlog WHERE finished_on IS NULL ORDER BY title;";
	std::list<Glib::RefPtr<Growlog> > ret;
	
	if (mysql_query(m_db_,sql))
		database_error(_("Unable to fetch growlogs from database!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		uint64_t id = std::stoull(row[0]);

		tm datetime;
		strptime(row[3],DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);

		time_t flower_on = 0;
		if (row[4] && strlen(row[4])) {
			strptime(row[4],DATE_ISO_FORMAT,&datetime);
			flower_on = mktime(&datetime);
		}

		time_t finished_on = 0;
		if (row[5] && strlen(row[5])) {
			strptime(row[5],DATETIME_ISO_FORMAT,&datetime);
			finished_on = mktime(&datetime);
		}
		
		Glib::RefPtr<Growlog> growlog = Growlog::create(id,
		                                                row[1],
		                                                row[2] ? row[2] : "",
		                                                created_on,
		                                                flower_on,
		                                                finished_on);
		
		ret.push_back(growlog);
	}
	mysql_free_result(result);
	return ret;	
}

std::list<Glib::RefPtr<Growlog> > 
DatabaseMariaDB::get_finished_growlogs_vfunc() const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,title,description,created_on,flower_on,finished_on FROM growlog WHERE finished_on IS NOT NULL ORDER BY title;";
	std::list<Glib::RefPtr<Growlog> > ret;
	
	if (mysql_query(m_db_,sql))
		database_error(_("Unable to fetch growlogs from database!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		uint64_t id = std::stoull(row[0]);

		tm datetime;
		strptime(row[3],DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);

		time_t flower_on = 0;
		if (row[4] && strlen(row[4])) {
			strptime(row[4],DATE_ISO_FORMAT,&datetime);
			flower_on = mktime(&datetime);
		}

		time_t finished_on = 0;
		if (row[5] && strlen(row[5])) {
			strptime(row[5],DATETIME_ISO_FORMAT,&datetime);
			finished_on = mktime(&datetime);
		}
		
		Glib::RefPtr<Growlog> growlog = Growlog::create(id,
		                                                row[1],
		                                                row[2] ? row[2] : "",
		                                                created_on,
		                                                flower_on,
		                                                finished_on);
		
		ret.push_back(growlog);
	}
	mysql_free_result(result);
	return ret;	
}

std::list<Glib::RefPtr<Growlog> > 
DatabaseMariaDB::get_growlogs_for_strain_vfunc(uint64_t strain_id) const
{
	assert(m_db_);
	
	const char *sql = "SELECT growlog FROM growlog_strain WHERE strain=%s;";
	std::list<Glib::RefPtr<Growlog> > ret;
		
	std::string strain_id_str = std::to_string(strain_id);
	size_t len = strlen(sql) + strain_id_str.size() + 1;
	std::unique_ptr<char[]> buffer(new char[len]);

	snprintf(buffer.get(),len,sql,strain_id_str.c_str());

	if (mysql_query(m_db_,buffer.get()))
		database_error(_("Unable to lookup growlogs for strain!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		uint64_t growlog_id = std::stoull(row[0]);
		try {
			Glib::RefPtr<Growlog> growlog = this->get_growlog(growlog_id);
			if (growlog)
				ret.push_back(growlog);
		} catch (DatabaseError ex) {
			mysql_free_result(result);
			throw ex;
		}			
	}
	mysql_free_result(result);
	return ret;
}

Glib::RefPtr<Growlog> 
DatabaseMariaDB::get_growlog_vfunc(uint64_t id) const
{
	assert(m_db_);
	
	const char *sql = "SELECT title,description,created_on,flower_on,finished_on FROM growlog WHERE id=%s;";
	Glib::RefPtr<Growlog> growlog;

	std::string id_str = std::to_string(id);

	size_t len = strlen(sql) + id_str.size() + 1;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,id_str.c_str());

	if (mysql_query(m_db_,buffer.get()))
		database_error(_("Unable to lookup growlog by id!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

	MYSQL_ROW row = mysql_fetch_row(result);
	if (row) {
		tm datetime;

		strptime(row[2],DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);

		time_t flower_on = 0;
		if (row[3] && strlen(row[3])) {
			strptime(row[3],DATE_ISO_FORMAT,&datetime);
			flower_on = mktime(&datetime);
		}

		time_t finished_on = 0;
		if (row[4] && strlen(row[4])) {
			strptime(row[4],DATETIME_ISO_FORMAT,&datetime);
			finished_on = mktime(&datetime);
		}

		growlog = Growlog::create(id,
		                          row[0],
		                          row[1] ? row[1] : "",
		                          created_on,
		                          flower_on,
		                          finished_on);
	}
	mysql_free_result(result);
	
	return growlog;
}

Glib::RefPtr<Growlog> 
DatabaseMariaDB::get_growlog_vfunc(const Glib::ustring &title) const
{
	assert(m_db_);
	
	const char *sql = "SELECT id,description,created_on,flower_on,finished_on FROM growlog WHERE title='%s';";
	Glib::RefPtr<Growlog> growlog;

	size_t title_len = title.bytes() * 2 + 1;
	std::unique_ptr<char[]> title_buffer(new char[title_len]);
	mysql_real_escape_string(m_db_,title_buffer.get(),title.c_str(),title.bytes());

	size_t len = strlen(sql) + title_len;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,title_buffer.get());

	if (mysql_query(m_db_,buffer.get()))
		database_error(_("Unable to lookup growlog by title!"));

	MYSQL_RES *result = mysql_store_result(m_db_);
	if (!result)
		database_error(_(RESULT_ERROR));

 	MYSQL_ROW row = mysql_fetch_row(result);
	if (row) {
		uint64_t id = std::stoull(row[0]);

		tm datetime;
		strptime(row[2],DATETIME_ISO_FORMAT,&datetime);
		time_t created_on = mktime(&datetime);

		time_t flower_on = 0;
		if (row[3] && strlen(row[3])) {
			strptime(row[3],DATE_ISO_FORMAT,&datetime);
			flower_on = mktime(&datetime);
		}

		time_t finished_on = 0;
		if (row[4] && strlen(row[4])) {
			strptime(row[4],DATETIME_ISO_FORMAT,&datetime);
			finished_on = mktime(&datetime);
		}

		growlog = Growlog::create(id,
		                          title,
		                          row[1] ? row[1] : "",
		                          created_on,
		                          flower_on,
		                          finished_on);
	}
	mysql_free_result(result);
	
	return growlog;
}

void 
DatabaseMariaDB::add_growlog_vfunc(const Glib::RefPtr<Growlog> &growlog)
{
	assert(m_db_);

	std::string sql_command;

	begin_transaction();
	
	if (growlog->get_id()) {
		const char *sql = "UPDATE growlog SET title='%s',description='%s',flower_on='%s',finished_on='%s' WHERE id=%s;";

		std::string id_str = std::to_string(growlog->get_id());

		Glib::ustring title = growlog->get_title();
		size_t title_len = title.bytes() * 2 + 1;
		std::unique_ptr<char[]> title_buffer(new char[title_len]);
		mysql_real_escape_string(m_db_,title_buffer.get(),title.c_str(),title.bytes());

		Glib::ustring desc = growlog->get_description();
		size_t desc_len = desc.bytes() * 2 + 1;
		std::unique_ptr<char[]> desc_buffer(new char[desc_len]);
		mysql_real_escape_string(m_db_,desc_buffer.get(),desc.c_str(),desc.bytes());

		
		Glib::ustring flower_on = growlog->get_flower_on_format(DATE_ISO_FORMAT);
		size_t flower_on_len = flower_on.bytes() * 2 + 1;
		std::unique_ptr<char[]> flower_on_buffer(new char[flower_on_len]);
		mysql_real_escape_string(m_db_,flower_on_buffer.get(),flower_on.c_str(),flower_on.bytes());

		Glib::ustring finished_on = growlog->get_finished_on_format(DATETIME_ISO_FORMAT);
		size_t finished_on_len = finished_on.bytes() * 2 + 1;
		std::unique_ptr<char[]> finished_on_buffer(new char[finished_on_len]);
		mysql_real_escape_string(m_db_,finished_on_buffer.get(),finished_on.c_str(),finished_on.bytes());

		size_t len = strlen(sql) + id_str.size() + title_len + desc_len + flower_on_len + finished_on_len;
		std::unique_ptr<char[]> buffer(new char[len]);
		snprintf(buffer.get(),len,sql,
		         title_buffer.get(),
		         desc_buffer.get(),
		         flower_on_buffer.get(),
		         finished_on_buffer.get(),
		         id_str.c_str());
		sql_command = (const char*) buffer.get();
	} else {
		const char *sql = "INSERT INTO growlog (title,description,created_on,flower_on,finished_on) VALUES ('%s','%s','%s','%s','%s');";

		Glib::ustring title = growlog->get_title();
		size_t title_len = title.bytes() * 2 + 1;
		std::unique_ptr<char[]> title_buffer(new char[title_len]);
		mysql_real_escape_string(m_db_,title_buffer.get(),title.c_str(),title.bytes());

		Glib::ustring desc = growlog->get_description();
		size_t desc_len = desc.bytes() * 2 + 1;
		std::unique_ptr<char[]> desc_buffer(new char[desc_len]);
		mysql_real_escape_string(m_db_,desc_buffer.get(),desc.c_str(),desc.bytes());

		Glib::ustring created_on = growlog->get_created_on_format(DATETIME_ISO_FORMAT);
		size_t created_on_len = created_on.bytes() * 2 + 1;
		std::unique_ptr<char[]> created_on_buffer(new char[created_on_len]);
		mysql_real_escape_string(m_db_,created_on_buffer.get(),created_on.c_str(),created_on.bytes());

		Glib::ustring flower_on = growlog->get_flower_on_format(DATE_ISO_FORMAT);
		size_t flower_on_len = flower_on.bytes() * 2 + 1;
		std::unique_ptr<char[]> flower_on_buffer(new char[flower_on_len]);
		mysql_real_escape_string(m_db_,flower_on_buffer.get(),flower_on.c_str(),flower_on.bytes());

		Glib::ustring finished_on = growlog->get_finished_on_format(DATETIME_ISO_FORMAT);
		size_t finished_on_len = finished_on.bytes() * 2 + 1;
		std::unique_ptr<char[]> finished_on_buffer(new char[finished_on_len]);
		mysql_real_escape_string(m_db_,finished_on_buffer.get(),finished_on.c_str(),finished_on.bytes());

		size_t len = title_len + desc_len + created_on_len + flower_on_len + finished_on_len;
		std::unique_ptr<char[]> buffer(new char[len]);
		snprintf(buffer.get(),len,sql,
		         title_buffer.get(),
		         desc_buffer.get(),
		         created_on_buffer.get(),
		         flower_on_buffer.get(),
		         finished_on_buffer.get());
		sql_command = (const char*) buffer.get();
	}

	if (mysql_query(m_db_,sql_command.c_str()))
		database_error(_("Unable to add growlog!"),true);

	commit();
}

void 
DatabaseMariaDB::remove_growlog_vfunc(uint64_t id)
{
	assert(m_db_);
	
	const char *sql = "DELETE FROM growlog WHERE id=%s";
	std::string id_str = std::to_string(id);

	begin_transaction();
	
	size_t len = strlen(sql) + id_str.size() + 1;
	std::unique_ptr<char[]> buffer(new char[len]);
	snprintf(buffer.get(),len,sql,id_str.c_str());

	if (mysql_query(m_db_,buffer.get()))
		database_error(_("Unable to delete growlog!"),true);

	commit();
}

/**** GrowlogEntry methods ****************************************************/

std::list<Glib::RefPtr<GrowlogEntry> > 
DatabaseMariaDB::get_growlog_entries_vfunc(uint64_t growlog_id) const
{
}

Glib::RefPtr<GrowlogEntry> 
DatabaseMariaDB::get_growlog_entry_vfunc(uint64_t id) const
{
}

void 
DatabaseMariaDB::add_growlog_entry_vfunc(const Glib::RefPtr<GrowlogEntry> &entry)
{
}

void 
DatabaseMariaDB::remove_growlog_entry_vfunc(uint64_t id)
{
}

/**** growlog_strain methods **************************************************/

void 
DatabaseMariaDB::add_strain_for_growlog_vfunc(uint64_t growlog_id,uint64_t strain_id)
{
}

void 
DatabaseMariaDB::remove_strain_for_growlog_vfunc(uint64_t growlog_id,uint64_t strain_id)
{
}

void 
DatabaseMariaDB::remove_strain_for_growlog_vfunc(uint64_t growlog_strain_id)
{
}


#endif /* HAVE_MARIADB */
