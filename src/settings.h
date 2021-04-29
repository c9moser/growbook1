/***************************************************************************
 *            settings.h
 *
 *  Di April 20 16:22:43 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * settings.h
 *
 * Copyright (C) 2021 - Christian Moser
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "refclass.h"

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <string>
#include <cinttypes>
#include <sqlite3.h> 
#include <sigc++/sigc++.h>

/*******************************************************************************
 * DatabaseSettingsFlags
 ******************************************************************************/

enum DatabaseSettingsFlags {
	 DB_FLAGS_NONE = 0,
	 DB_NAME_IS_FILENAME = (1<<0),
	 DB_HAS_HOST = (1<<1),
	 DB_HAS_PORT = (1<<2),
	 DB_HAS_USER = (1<<3),
	 DB_HAS_PASSWORD = (1<<4)
};

inline DatabaseSettingsFlags operator | (DatabaseSettingsFlags lhs, DatabaseSettingsFlags rhs)
{return static_cast<DatabaseSettingsFlags>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));}

inline DatabaseSettingsFlags operator & (DatabaseSettingsFlags lhs, DatabaseSettingsFlags rhs)
{return static_cast<DatabaseSettingsFlags>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs));}

inline DatabaseSettingsFlags operator ^ (DatabaseSettingsFlags lhs, DatabaseSettingsFlags rhs)
{return static_cast<DatabaseSettingsFlags>(static_cast<unsigned>(lhs) ^ static_cast<unsigned>(rhs));}

inline DatabaseSettingsFlags operator ~ (DatabaseSettingsFlags flags)
{return static_cast<DatabaseSettingsFlags>(~static_cast<unsigned>(flags));}

inline DatabaseSettingsFlags& operator |= (DatabaseSettingsFlags& lhs, DatabaseSettingsFlags rhs) 
{return (lhs = static_cast<DatabaseSettingsFlags>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)));}
	
inline DatabaseSettingsFlags& operator &= (DatabaseSettingsFlags& lhs, DatabaseSettingsFlags rhs) 
{return (lhs = static_cast<DatabaseSettingsFlags>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs)));}

inline DatabaseSettingsFlags& operator ^= (DatabaseSettingsFlags& lhs, DatabaseSettingsFlags rhs) 
{return (lhs = static_cast<DatabaseSettingsFlags>(static_cast<unsigned>(lhs) ^ static_cast<unsigned>(rhs)));}

/*******************************************************************************
 * DatabaseSettings
 ******************************************************************************/

class DatabaseSettings:
	public RefClass
{
	private:
		Glib::ustring m_engine_;
		DatabaseSettingsFlags m_flags_;
		std::string m_dbname_;
		std::string m_host_;
		uint16_t m_port_;
		std::string m_user_;
		std::string m_password_;
		bool m_ask_password_;
		
	private:
		 DatabaseSettings(const DatabaseSettings &src) = delete;
		 DatabaseSettings& operator = (const DatabaseSettings &src) = delete;

	protected:
		 DatabaseSettings() noexcept;
		 DatabaseSettings(const Glib::ustring &engine,
		                  const std::string &dbname,
		                  DatabaseSettingsFlags flags = DB_NAME_IS_FILENAME) noexcept;
		DatabaseSettings (const Glib::ustring &engine,
		                  const std::string &dbname,
		                  const std::string &host,
		                  uint16_t port,
		                  const std::string &user,
		                  const std::string &password,
		                  bool ask_password,
		                  DatabaseSettingsFlags flags = (DB_HAS_HOST
		                                                 | DB_HAS_PORT
		                                                 | DB_HAS_USER
		                                                 | DB_HAS_PASSWORD)) noexcept;
		
	public:
		 virtual ~DatabaseSettings();

	public:
		static Glib::RefPtr<DatabaseSettings> create();
		static Glib::RefPtr<DatabaseSettings> create(const Glib::ustring &engine,
		                                             const std::string &dbname,
		                                             DatabaseSettingsFlags flags = DB_NAME_IS_FILENAME);
		static Glib::RefPtr<DatabaseSettings> create(const Glib::ustring &engine,
		                                             const std::string &dbname,
		                                             const std::string &host,
		                                             uint16_t port,
		                                             const std::string &user,
		                                             const std::string &password,
		                                             bool ask_password,
		                                             DatabaseSettingsFlags = (DB_HAS_HOST 
		                                                                      | DB_HAS_PORT
		                                                                      | DB_HAS_USER
		                                                                      | DB_HAS_PASSWORD));
	public:
		DatabaseSettingsFlags get_flags() const;
		Glib::ustring get_engine() const;
		
		bool get_dbname_is_filename() const;
		bool get_has_host() const;
		bool get_has_port() const;
		bool get_has_user() const;
		bool get_has_password() const;
		
		bool get_ask_password() const;
		void set_ask_password(bool b);

		std::string get_dbname() const;
		void set_dbname(const std::string &dbname);

		std::string get_host() const;
		void set_host(const std::string &host);

		uint16_t get_port() const;
		void set_port(uint16_t port);

		std::string get_user() const;
		void set_user(const std::string &user);

		std::string get_password() const;
		void set_password(const std::string &password);
};

/*******************************************************************************
 * Settings
 ******************************************************************************/

class Settings:
	public RefClass
{
	private:
		sqlite3 *m_db_;
		Glib::RefPtr<DatabaseSettings> m_db_settings_;
		bool m_first_run_;
		std::string m_prefix_;
		bool m_open_ongoing_growlogs_;
		Glib::ustring m_date_format_;
		Glib::ustring m_datetime_format_;
		sigc::signal0<void> m_signal_load_;
		sigc::signal0<void> m_signal_save_;
		
	private:
		Settings(const Settings &src) = delete;
		Settings& operator = (const DatabaseSettings &src) = delete;

	protected:
		Settings(int argc, char **argv) noexcept;

	public:
		virtual ~Settings();
		
	public:
		static Glib::RefPtr<Settings> create(int agrc, char **argv);

	protected:
		void begin_transaction();
		void commit();
		void rollback();
		void create_database();
	public:
		void load();
		void save();
		
	protected:
		virtual void on_load();
		virtual void on_save();

	public:
		sigc::signal0<void> signal_load();
		sigc::signal0<void> signal_save();
		
		bool has_key(const Glib::ustring &key) const;
		void set(const Glib::ustring &key, const Glib::ustring &value);
		Glib::ustring get(const Glib::ustring &key) const;
		
		void set_bool(const Glib::ustring &key, bool value);
		bool get_bool(const Glib::ustring &key) const;

		void set_int8(const Glib::ustring &key, int8_t value);
		int8_t get_int8(const Glib::ustring &key) const;

		void set_uint8(const Glib::ustring &key, uint8_t value);
		uint8_t get_uint8(const Glib::ustring &key) const;

		void set_int16(const Glib::ustring &key, int16_t value);
		int16_t get_int16(const Glib::ustring &key) const;

		void set_uint16(const Glib::ustring &key, uint16_t value);
		uint16_t get_uint16(const Glib::ustring &key) const;

		void set_int32(const Glib::ustring &key, int32_t value);
		int32_t get_int32(const Glib::ustring &key) const;

		void set_uint32(const Glib::ustring &key, uint32_t value);
		uint32_t get_uint32(const Glib::ustring &key) const;

		void set_int64(const Glib::ustring &key, int64_t value);
		int64_t get_int64(const Glib::ustring &key) const;

		void set_uint64(const Glib::ustring &key, uint64_t value);
		uint64_t get_uint64(const Glib::ustring &key) const;

		void set_float(const Glib::ustring &key, float value);
		float get_float(const Glib::ustring &key);

		void set_double(const Glib::ustring &key,double value);
		double get_double(const Glib::ustring &key) const;

		void get_long_double(const Glib::ustring &key, long double value);
		long double get_long_double(const Glib::ustring &key);

	// get/set config values
	public:
		Glib::RefPtr<DatabaseSettings> get_database_settings();
		Glib::RefPtr<const DatabaseSettings> get_database_settings() const;
		void set_database_settings(const Glib::RefPtr<DatabaseSettings> &db_settings);

		std::string get_prefix() const;
		std::string get_package_data_dir() const;
		std::string get_locale_dir() const;
		std::string get_sql_dir() const;
		std::string get_config_sql_file() const;
		
		bool get_first_run() const;

		bool get_open_ongoing_growlogs() const;
		void set_open_ongoing_growlogs(bool b);
		
		Glib::ustring get_date_format() const;
		void set_date_format(const Glib::ustring &format);

		Glib::ustring get_datetime_format() const;
		void set_datetime_format(const Glib::ustring &format);
};

#endif /* __SETTINGS_H__ */
