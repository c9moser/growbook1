/***************************************************************************
 *            database.h
 *
 *  Fr April 23 19:19:49 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * database.h
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
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "refclass.h"
#include "settings.h"

#include <glibmm/ustring.h>
#include <string>
#include <list>

/*******************************************************************************
 * Database
 ******************************************************************************/

class Database:
	public RefClass
{
	 private:
		 Glib::RefPtr<DatabaseSettings> m_settings_;
		 
	 private:
		 Database(const Database &src) = delete;
		 Database& operator = (const Database &src) = delete;
		 
	 protected:
		 Database(const Glib::RefPtr<DatabaseSettings> &settings) noexcept;
		 
	 public:
		 virtual ~Database();

	public:
		 Glib::RefPtr<DatabaseSettings> get_settings();
		 Glib::RefPtr<const DatabaseSettings> get_settings() const;

	public:
		 /*! test if we are connected to a database.
		  * @return true on success.
		  */
		 bool is_connected() const;
		 
		 /*! Test connection to database.
		  * @return true on success
		  */
		 bool test_connection();
		 /*! Creates a new empty database.
		  */
		 void create_database();

		 /*! Connect to database.
		  */
		 void connect();

		 /*! Close database connection.
		  */
		 void close();
		 
	protected:
		 virtual bool is_connected_vfunc() const = 0;
		 virtual bool test_connection_vfunc() = 0;
		 virtual void create_database_vfunc() = 0;
		 virtual void connect_vfunc() = 0;
		 virtual void close_vfunc() = 0;
		 
			 
}; // Database class

/*******************************************************************************
 * DatabaseModule
 ******************************************************************************/

class DatabaseModule:
	public RefClass
{
	private:
		Glib::RefPtr<DatabaseSettings> m_defaults_;

	private:
		DatabaseModule(const DatabaseModule &src) = delete;
		DatabaseModule& operator = (const DatabaseModule &src) = delete;

	protected:
		DatabaseModule(const Glib::RefPtr<DatabaseSettings> &default_settings) noexcept;

	public:
		virtual ~DatabaseModule();

	public:
		Glib::RefPtr<DatabaseSettings> get_defaults();
		Glib::RefPtr<const DatabaseSettings> get_defaults() const;

		virtual Glib::RefPtr<Database> create_database(const Glib::RefPtr<DatabaseSettings> &settings);

		Glib::ustring get_engine() const;
		
	protected:
		virtual Glib::RefPtr<Database> create_database_vfunc(const Glib::RefPtr<DatabaseSettings> &settings) = 0;
}; // DatabaseModule class

/*******************************************************************************
 * database functions
 ******************************************************************************/

void db_init();
std::list<Glib::RefPtr<DatabaseModule> > db_get_modules();
Glib::RefPtr<DatabaseModule> db_get_module(const Glib::ustring &engine);
void db_add_module(const Glib::RefPtr<DatabaseModule> &module);

#endif
