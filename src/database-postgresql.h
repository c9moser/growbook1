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

	public:
		void commit();
		void rollback();
		
	protected:
		bool is_connected_vfunc() const override;
		bool test_connection_vfunc() override;
		void connect_vfunc() override;
		void close_vfunc() override;
		void create_database_vfunc() override;

		virtual std::list<Glib::RefPtr<Breeder> > get_breeders_vfunc() const override;
		virtual Glib::RefPtr<Breeder> get_breeder_vfunc(uint64_t id) const override;
		virtual Glib::RefPtr<Breeder> get_breeder_vfunc(const Glib::ustring &name) const override;
		virtual void add_breeder_vfunc(const Glib::RefPtr<Breeder> &breeder) override;
		virtual void remove_breeder_vfunc(uint64_t id);

		virtual std::list<Glib::RefPtr<Strain> > get_strains_for_breeder_vfunc(uint64_t breeder_id) const override;
		virtual std::list<Glib::RefPtr<Strain> > get_strains_for_growlog_vfunc(uint64_t growlog_id) const override;
		virtual Glib::RefPtr<Strain> get_strain_vfunc(uint64_t id) const override;
		virtual Glib::RefPtr<Strain> get_strain_vfunc(const Glib::ustring &breeder_name,
		                                              const Glib::ustring &strain_name) const override;
		virtual void add_strain_vfunc(const Glib::RefPtr<Strain> &strain) override;
		virtual void remove_strain_vfunc(uint64_t id) override;
		
};

#endif /* __DATABASE_POSTGRESQL_H__ */