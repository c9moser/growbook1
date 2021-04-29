/***************************************************************************
 *            database-sqlite3.h
 *
 *  Sa April 24 13:24:21 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * database-sqlite3.h
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

#ifndef __DATABASE_SQLITE3_H__
#define __DATABASE_SQLITE3_H__

#include "database.h"
#include <sqlite3.h>
/*******************************************************************************
 * DatabaseModuleSqlite3
 ******************************************************************************/

class DatabaseModuleSqlite3:
	public DatabaseModule
{
	 private:
		 DatabaseModuleSqlite3(const DatabaseModuleSqlite3 &src) = delete;
		 DatabaseModuleSqlite3& operator = (const DatabaseModuleSqlite3 &src) = delete;
		 
	 protected:
		 DatabaseModuleSqlite3();

	 public:
		 virtual ~DatabaseModuleSqlite3();
		 
	 public:
		 static Glib::RefPtr<DatabaseModuleSqlite3> create();

	 public:
		 virtual Glib::RefPtr<Database> create_database(const Glib::RefPtr<DatabaseSettings> &settings) override;
	 protected:
		 virtual Glib::RefPtr<Database> create_database_vfunc(const Glib::RefPtr<DatabaseSettings> &settings) override;		 
};

/*******************************************************************************
 * DatabaseSqlite3
 ******************************************************************************/

class DatabaseSqlite3:
	public Database
{
	private:
		sqlite3 *m_db_;
		
	private:
		DatabaseSqlite3(const DatabaseSqlite3 &src) = delete;
		DatabaseSqlite3& operator = (const DatabaseSqlite3 &src) = delete;

	protected:
		DatabaseSqlite3(const Glib::RefPtr<DatabaseSettings> &settings);
		
	public:
		virtual ~DatabaseSqlite3();

	public:
		static Glib::RefPtr<DatabaseSqlite3> create(const Glib::RefPtr<DatabaseSettings> &settings);
		
	protected:
		void begin_transaction();
		void commit();
		void rollback();
		
	protected:
		bool is_connected_vfunc() const override;
		bool test_connection_vfunc() override;
		void create_database_vfunc() override;
		void connect_vfunc() override;
		void close_vfunc() override;

		virtual std::list<Glib::RefPtr<Breeder> > get_breeders_vfunc() const override;
		virtual Glib::RefPtr<Breeder> get_breeder_vfunc(uint64_t id) const override;
		virtual Glib::RefPtr<Breeder> get_breeder_vfunc(const Glib::ustring &name) const override;
		virtual void add_breeder_vfunc(const Glib::RefPtr<Breeder> &breeder) override;
};

#endif /* __DATABASE_SQLITE3_H__ */
