/***************************************************************************
 *            database-psql.h
 *
 *  Sa April 24 18:38:09 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * database-psql.h
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
#ifndef __DATABASE_POSTGRESQL_H__
#define __DATABASE_POSTGRESQL_H__

#include "database.h"
#include <libpq-fe.h>

class DatabaseModulePostgresql:
	public DatabaseModule
{
	 private:
		 DatabaseModulePostgresql(const DatabaseModulePostgresql &src) = delete;
		 DatabaseModulePostgresql& operator = (const DatabaseModulePostgresql &src) = delete;
		 
	 protected:
		 DatabaseModulePostgresql();
		 
	 public:
		 virtual ~DatabaseModulePostgresql();

	public:
		 static Glib::RefPtr<DatabaseModulePostgresql> create();

	protected:
		 virtual Glib::RefPtr<Database> create_database_vfunc(const Glib::RefPtr<DatabaseSettings> &settings) override;
};

class DatabasePostgresql:
	public Database
{
	private:
		PGconn *m_db_;
		
	private:
		DatabasePostgresql(const DatabasePostgresql &src) = delete;
		DatabasePostgresql& operator = (const DatabasePostgresql &src) = delete;

	protected:
		DatabasePostgresql(const Glib::RefPtr<DatabaseSettings> &settings);
		virtual ~DatabasePostgresql();

	public:
		static Glib::RefPtr<DatabasePostgresql> create(const Glib::RefPtr<DatabaseSettings> &settings);

	protected:
		bool is_connected_vfunc() const override;
		bool test_connection_vfunc() override;
		void connect_vfunc() override;
		void close_vfunc() override;
		void create_database_vfunc() override;
};

#endif /* __DATABASE_POSTGRESQL_H__ */