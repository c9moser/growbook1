//           settings.cc
//  Di April 20 16:22:43 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// settings.cc
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

#include "settings.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glibmm/i18n.h>
#include <glibmm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <fstream>

#include "error.h"

#include "database.h"
#include "database-sqlite3.h"

/*******************************************************************************
 * DatabaseSettings
 ******************************************************************************/

DatabaseSettings::DatabaseSettings() noexcept:
	RefClass{},
	m_engine_{"sqlite3"},
	m_flags_{DB_NAME_IS_FILENAME},
	m_dbname_{Glib::build_filename(Glib::get_user_special_dir (Glib::USER_DIRECTORY_DOCUMENTS),"growbook.db")},
	m_host_{""},
	m_port_{0},
	m_user_{""},
	m_password_{""},
	m_ask_password_{false}
{}

DatabaseSettings::DatabaseSettings(const Glib::ustring &engine,
                                   const std::string &dbname,
                                   DatabaseSettingsFlags flags) noexcept:
	RefClass{},
	m_engine_{engine},
	m_flags_{flags},
	m_dbname_{dbname},
	m_host_{""},
	m_port_{0},
	m_user_{""},
	m_password_{""},
	m_ask_password_{false}
{}

DatabaseSettings::DatabaseSettings(const Glib::ustring &engine,
                                   const std::string &dbname,
                                   const std::string &host,
                                   uint16_t port,
                                   const std::string &user,
                                   const std::string &password,
                                   bool ask_password,
                                   DatabaseSettingsFlags flags) noexcept:
	RefClass{},
	m_engine_{engine},
	m_flags_{flags},
	m_dbname_{dbname},
	m_host_{host},
	m_port_{port},
	m_user_{user},
	m_password_{password},
	m_ask_password_{ask_password}
{}

                                   
DatabaseSettings::~DatabaseSettings()
{}

Glib::RefPtr<DatabaseSettings>
DatabaseSettings::create()
{
	return Glib::RefPtr<DatabaseSettings>(new DatabaseSettings());
}

Glib::RefPtr<DatabaseSettings>
DatabaseSettings::create(const Glib::ustring &engine,
                         const std::string &dbname,
                         DatabaseSettingsFlags flags)
{
	return Glib::RefPtr<DatabaseSettings>(new DatabaseSettings(engine,dbname,flags));
}

Glib::RefPtr<DatabaseSettings>
DatabaseSettings::create(const Glib::ustring &engine,
                         const std::string &dbname,
                         const std::string &host,
                         uint16_t port,
                         const std::string &user,
                         const std::string &password,
                         bool ask_password,
                         DatabaseSettingsFlags flags)
{
	return Glib::RefPtr<DatabaseSettings>(new DatabaseSettings(engine,
	                                                           dbname,
	                                                           host,
	                                                           port,
	                                                           user,
	                                                           password,
	                                                           ask_password,
	                                                           flags));
}

DatabaseSettingsFlags
DatabaseSettings::get_flags() const
{
	return m_flags_;
}

Glib::ustring
DatabaseSettings::get_engine() const
{
	return m_engine_;
}

bool
DatabaseSettings::get_dbname_is_filename() const
{
	return (m_flags_ & DB_NAME_IS_FILENAME);
}

bool
DatabaseSettings::get_has_host() const
{
	return (m_flags_ & DB_HAS_HOST);
}

bool
DatabaseSettings::get_has_port() const
{
	return (m_flags_ & DB_HAS_PORT);
}

bool
DatabaseSettings::get_has_user() const
{
	return (m_flags_ & DB_HAS_USER);
}

bool
DatabaseSettings::get_has_password() const
{
	return (m_flags_ & DB_HAS_PASSWORD);
}

std::string
DatabaseSettings::get_dbname() const
{
	return m_dbname_;
}

void
DatabaseSettings::set_dbname(const std::string &name)
{
	m_dbname_ = name;
}

std::string
DatabaseSettings::get_host() const
{
	return m_host_;
}

void
DatabaseSettings::set_host(const std::string &host)
{
	m_host_ = host;
}

uint16_t
DatabaseSettings::get_port() const
{
	return m_port_;
}

void
DatabaseSettings::set_port(uint16_t port)
{
	m_port_ = port;
}

std::string
DatabaseSettings::get_user() const
{
	return m_user_;
}

void
DatabaseSettings::set_user(const std::string &user)
{
	m_user_ = user;
}

std::string
DatabaseSettings::get_password() const
{
	return m_password_;
}

void
DatabaseSettings::set_password(const std::string &passwd)
{
	m_password_ = passwd;
}

bool
DatabaseSettings::get_ask_password() const
{
	return m_ask_password_;
}

void
DatabaseSettings::set_ask_password(bool b)
{
	m_ask_password_ = b;
}

/*******************************************************************************
 * Settings
 ******************************************************************************/

/*** Ctor/Dtor ****************************************************************/

Settings::Settings(int argc, char **argv) noexcept:
	m_db_{nullptr},
	m_db_settings_{DatabaseSettings::create()},
	m_first_run_{false},
	m_open_ongoing_growlogs_{true},
	m_date_format_{"%m-%d-%Y"},
	m_datetime_format_{"%m-%d-%Y %H:%M:%S"},
	m_signal_load_{},
	m_signal_save_{}
{
	// set default dirs and first run
	std::string cfg_dir = Glib::build_filename(Glib::get_user_config_dir(),"growbook");
	std::string cfg_db = Glib::build_filename(cfg_dir,"config.db");
	if (!Glib::file_test(Glib::get_user_config_dir(),Glib::FILE_TEST_EXISTS)) {
#ifdef NATIVE_WINDOWS
		mkdir(Glib::get_user_config_dir().c_str());
#else
		mkdir(Glib::get_user_config_dir().c_str(),S_IRWXU|S_IRWXG);
#endif /* NATIVE_WINDOWS */
	}
	if (!Glib::file_test(cfg_dir,Glib::FILE_TEST_EXISTS)) {
#ifdef NATIVE_WINDOWS
		mkdir(cfg_dir.c_str());
#else
		mkdir(cfg_dir.c_str(),S_IRWXU | S_IRWXG);
#endif /* NATIVE_WINDOWS */
	}
	if (!Glib::file_test(cfg_db,Glib::FILE_TEST_EXISTS)) {
		m_first_run_ = true;
	}
#ifdef NAITVE_WINDOWS
	m_prefix_ = Glib::path_get_dirname(Glib::path_get_dirname(argv[0]));
#else
	m_prefix_ = Glib::path_get_dirname(Glib::path_get_dirname(PACKAGE_DATA_DIR));
#endif

	
	m_signal_load_.connect(sigc::mem_fun(*this,&Settings::on_load));
	m_signal_save_.connect(sigc::mem_fun(*this,&Settings::on_save));
	
	if (sqlite3_open(cfg_db.c_str(), &m_db_) != SQLITE_OK) {
		fprintf(stderr,_("Unable to open config-database!\nYour settings wont be saved!\n"));
		exit(EXIT_FAILURE);
	}
	
	if (m_first_run_) {
		try {
			create_database();
		} catch (DatabaseError ex) {
			fprintf(stderr,"%s\n",ex.what());
		}
		try {
			this->save();
		} catch(DatabaseError ex) {
			fprintf(stderr,"%s\n",ex.what());
		}
	} else {
		try {
			this->load();
		} catch (DatabaseError ex) {
			fprintf(stderr,"%s\n",ex.what());
		}
	}
}

Settings::~Settings()
{
	if (m_db_) {
		sqlite3_close(m_db_);
		m_db_=nullptr;
	}
}

/*** create methods ***********************************************************/

Glib::RefPtr<Settings>
Settings::create(int argc, char **argv)
{
	return Glib::RefPtr<Settings>(new Settings(argc,argv));
}

/*** signals ******************************************************************/

sigc::signal0<void>
Settings::signal_load()
{
	return m_signal_load_;
}

sigc::signal0<void>
Settings::signal_save()
{
	return m_signal_save_;
}

/*** Database methods *********************************************************/

void
Settings::begin_transaction()
{
	if (!m_db_)
		return;

	char *errmsg;
	int err = sqlite3_exec(m_db_, "BEGIN TRANSACTION;",0,0,&errmsg);
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
Settings::commit()
{
	if (!m_db_)
		return;

	char *errmsg;
	int err = sqlite3_exec(m_db_, "COMMIT;",0,0,&errmsg);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to commit changes!");
		msg += "\n(";
		msg += errmsg;
		msg += ")";
		sqlite3_free(errmsg);
		throw DatabaseError(err,msg);
	}
}

void
Settings::rollback()
{
	if (!m_db_)
		return;

	char *errmsg;
	int err = sqlite3_exec(m_db_, "ROLLBACK;",0,0,&errmsg);
	if (err != SQLITE_OK) {
		Glib::ustring msg = _("Unable to rollback transaction!");
		msg += "\n(";
		msg += errmsg;
		msg += ")";
		sqlite3_free(errmsg);
		throw DatabaseError(err,msg);
	}
}

void
Settings::create_database()
{
	std::string sql,text;
	std::fstream file;
	char *errmsg;
	int err;
	
	file.open(get_config_sql_file(),std::ios::in);
	while (std::getline(file,text)) {
		if (text.empty())
			continue;
		if (!sql.empty())
			sql+="\n";
		sql+=text;
		
		if (sql[sql.size() -1] == ';') {
			err=sqlite3_exec(m_db_,sql.c_str(),0,0,&errmsg);
			if (err != SQLITE_OK) {
				fprintf(stderr, "Unable to execute sql (%s)!\n%s",errmsg,sql.c_str());
				sqlite3_free(errmsg);
				exit(EXIT_FAILURE);
			}
			sql="";
		}
	}
}

void 
Settings::load()
{
	m_signal_load_.emit();
}

void
Settings::on_load()
{
	if (has_key("open-ongoing-growlogs"))
		m_open_ongoing_growlogs_ = get_bool("open-ongoing-growlogs");
	if (has_key("date-format"))
		m_date_format_ = get("date-format");
	if (has_key("datetime-format"))
		m_datetime_format_ = get("datetime-format");

	if (has_key("db-engine")) {
		Glib::ustring db_engine = get("db-engine");
		Glib::RefPtr<DatabaseModule> db_module=db_get_module(db_engine);
		if (db_module) {			
			Glib::RefPtr<DatabaseSettings> defaults = db_module->get_defaults();
			DatabaseSettingsFlags flags = defaults->get_flags();

			Glib::ustring db_name = defaults->get_dbname();
			if (has_key("db-name"))
				db_name = get("db-name");
			
			Glib::ustring db_host = defaults->get_host();
			if (has_key("db-host"))
				db_host = get("db-host");

			uint16_t db_port = defaults->get_port();
			if (has_key("db-port"))
				db_port = get_uint16("db-port");

			Glib::ustring db_user = defaults->get_user();
			if (has_key("db-user"))
				db_user = get("db-user");

			Glib::ustring db_password = defaults->get_password();
			if (has_key("db-password"))
				db_password = get("db-password");

			bool db_ask_passwd = defaults->get_ask_password();
			if (has_key("db-ask-password"))
				db_ask_passwd = get_bool("db-ask-password");

			m_db_settings_ = DatabaseSettings::create(db_engine,
			                                          db_name,
			                                          db_host,
			                                          db_port,
			                                          db_user,
			                                          db_password,
			                                          db_ask_passwd,
			                                          flags);
		}
	}
}

void
Settings::save()
{
	m_signal_save_.emit();
}

void
Settings::on_save()
{
	set_bool("open-ongoing-growlogs",m_open_ongoing_growlogs_);
	set("date-format",m_date_format_);
	set("datetime_format_",m_datetime_format_);

	/* Database Settings */
	set("db-name",m_db_settings_->get_dbname());
	set("db-host",m_db_settings_->get_host());
	set_uint16("db-port",m_db_settings_->get_port());
	set("db-user", m_db_settings_->get_user());
	set_bool("db-ask-password",m_db_settings_->get_ask_password());

	if (!m_db_settings_->get_ask_password()) {
		set("db-password",m_db_settings_->get_password());
	} else {
		set("db-password","");
	}
}

/*** get/set methods **********************************************************/

bool
Settings::has_key(const Glib::ustring &key) const
{
	if (!m_db_)
		return false;
	
	static const char *sql="SELECT key FROM config WHERE key=?";
	sqlite3_stmt *stmt = nullptr;
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		if (stmt)
			sqlite3_finalize(stmt);
		Glib::ustring msg=_("Unable to lookup keys in config.db!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_text(stmt,1,key.c_str(),-1,0);

	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return false;
	}
	Glib::ustring result = (const char*) sqlite3_column_text(stmt,0);
	sqlite3_finalize(stmt);
	return (key == result);
}


Glib::ustring
Settings::get(const Glib::ustring &key) const
{
	static const char *sql = "SELECT value FROM config WHERE key=?;";
	Glib::ustring value;
	
	if (!m_db_)
		return value;

	sqlite3_stmt *stmt = nullptr;
	
	int err = sqlite3_prepare(m_db_,sql,-1,&stmt,0);
	if (err != SQLITE_OK) {
		if (stmt)
			sqlite3_finalize(stmt);
		Glib::ustring msg = _("Unable to get values from config.db ");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		throw DatabaseError(err,msg);
	}
	sqlite3_bind_text(stmt,1,key.c_str(),-1,0);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		value = (const char *) sqlite3_column_text(stmt,0);
	}
	sqlite3_finalize(stmt);
	return value;
}


void
Settings::set(const Glib::ustring &key, const Glib::ustring &value)
{
	if (!m_db_)
		return;
	std::string sql;
	if (this->has_key(key)) {
		sql = "UPDATE config SET value=?2 WHERE key=?1;";
	} else {
		sql = "INSERT INTO config (key,value) VALUES (?,?);";
	}
	sqlite3_stmt *stmt = nullptr;
	
	begin_transaction();

	int err = sqlite3_prepare(m_db_,sql.c_str(),-1,&stmt,0);
	if (err != SQLITE_OK) {
		if (stmt)
			sqlite3_finalize(stmt);
		Glib::ustring msg = _("Unable to set values in config.db!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		throw DatabaseError(err,msg);
	}
	
	sqlite3_bind_text(stmt,1,key.c_str(),-1,0);
	sqlite3_bind_text(stmt,2,value.c_str(),-1,0);

	err = sqlite3_step(stmt);
	if (err != SQLITE_OK && err != SQLITE_DONE) {
		rollback();
		Glib::ustring msg = _("Updating config.db failed!");
		msg += "\n(";
		msg += sqlite3_errmsg(m_db_);
		msg += ")";
		throw DatabaseError(err,msg);
	}
	
	commit();
}

bool
Settings::get_bool(const Glib::ustring &key) const
{
	Glib::ustring value = this->get(key);
	if (value.empty())
		return false;
	
	if (value == "yes" || value == "1" || value == "true" || value == "on")
		return true;
	if (value == "no" || value == "0" || value == "false" || value == "off")
		return false;
	
	return false;
}

void
Settings::set_bool(const Glib::ustring &key, bool b)
{
	Glib::ustring value;
	if (b) {
		value = "yes";
	} else {
		value = "no";
	}
	this->set(key,value);
}

int8_t
Settings::get_int8(const Glib::ustring &key) const
{
	Glib::ustring value=this->get(key);
	if (value.empty())
		return 0;
	return static_cast<int8_t>(std::stoi(value.c_str()));
}

void
Settings::set_int8(const Glib::ustring &key, int8_t value)
{
	Glib::ustring vstring = std::to_string(value);
	this->set(key,vstring);
}

uint8_t
Settings::get_uint8(const Glib::ustring &key) const
{
	Glib::ustring value = this->get(key);
	if (value.empty())
		return 0;
	return static_cast<uint8_t>(std::stoi(value.c_str()));
}

void
Settings::set_uint8(const Glib::ustring &key, uint8_t value)
{
	Glib::ustring vstring = std::to_string(value);
	this->set(key,vstring);
}

int16_t
Settings::get_int16(const Glib::ustring &key) const
{
	Glib::ustring value = this->get(key);
	if (value.empty())
		return 0;
	return static_cast<int16_t>(std::stoi(value.c_str()));
}

void
Settings::set_int16(const Glib::ustring &key, int16_t value)
{
	Glib::ustring vstring = std::to_string(value);
	this->set(key,vstring);
}

uint16_t
Settings::get_uint16(const Glib::ustring &key) const
{
	Glib::ustring value = this->get(key);
	if (value.empty())
		return 0;
	return static_cast<uint16_t>(std::stoi(value.c_str()));
}

void
Settings::set_uint16(const Glib::ustring &key,uint16_t value)
{
	Glib::ustring vstring = std::to_string(value);
	this->set(key,vstring);
}

int32_t
Settings::get_int32(const Glib::ustring &key) const
{
	Glib::ustring value = this->get(key);
	if (value.empty())
		return 0;
	
	return std::stol(value.c_str());
}

void
Settings::set_int32(const Glib::ustring &key, int32_t value)
{
	Glib::ustring vstring = std::to_string(value);
	this->set(key, vstring);
}

uint32_t
Settings::get_uint32(const Glib::ustring &key) const
{
	Glib::ustring value = this->get(key);
	if (value.empty())
		return 0;
	return std::stoul(value.c_str());
}

void
Settings::set_uint32(const Glib::ustring &key, uint32_t value)
{
	Glib::ustring vstring = std::to_string(value);
	this->set(key,vstring);
}

int64_t
Settings::get_int64(const Glib::ustring &key) const
{
	Glib::ustring value = this->get(key);
	if (value.empty())
		return 0;
	return std::stoll(value.c_str());
}

void
Settings::set_int64(const Glib::ustring &key, int64_t value)
{
	Glib::ustring vstring = std::to_string(value);
	this->set(key,vstring);
}

uint64_t
Settings::get_uint64(const Glib::ustring &key) const
{
	Glib::ustring value = this->get(key);
	if (value.empty())
		return 0;
	return std::stoull(value.c_str());
}

void
Settings::set_uint64(const Glib::ustring &key, uint64_t value)
{
	Glib::ustring vstring = std::to_string(value);
	this->set(key,vstring);
}

/*** Get/set configuration values *********************************************/

std::string 
Settings::get_prefix() const
{
	return m_prefix_;
}

std::string 
Settings::get_package_data_dir() const
{
#ifdef NATIVE_WINDOWS
	return Glib::build_filename(m_prefix_,"share","growbook");
#else
	return std::string(PACKAGE_DATA_DIR);
#endif
}

std::string
Settings::get_locale_dir() const
{
//#ifdef NATIVE_WINDOWS
	return Glib::build_filename (m_prefix_,"share","locale");
//#else
//	return std::string(LOCALEDIR);
//#endif
}

std::string 
Settings::get_sql_dir () const
{
	return Glib::build_filename(get_package_data_dir (),"sql");
}

std::string 
Settings::get_config_sql_file() const
{
	return Glib::build_filename (get_sql_dir(),"config.sql");
}

Glib::RefPtr<DatabaseSettings>
Settings::get_database_settings()
{
	return m_db_settings_;
}

Glib::RefPtr<const DatabaseSettings>
Settings::get_database_settings() const
{
	return Glib::RefPtr<const DatabaseSettings>::cast_const(m_db_settings_);
}

void
Settings::set_database_settings(const Glib::RefPtr<DatabaseSettings> &db_settings)
{
	m_db_settings_ = db_settings;
}

bool
Settings::get_first_run() const
{
	return m_first_run_;
}

bool 
Settings::get_open_ongoing_growlogs() const
{
	return m_open_ongoing_growlogs_;
}

void
Settings::set_open_ongoing_growlogs(bool b)
{
	m_open_ongoing_growlogs_ = b;
}

Glib::ustring
Settings::get_date_format() const
{
	return m_date_format_;
}

void
Settings::set_date_format(const Glib::ustring &fmt)
{
	m_date_format_ = fmt;
}

Glib::ustring
Settings::get_datetime_format() const
{
	return m_datetime_format_;
}

void
Settings::set_datetime_format(const Glib::ustring &fmt)
{
	m_datetime_format_ = fmt;
}
