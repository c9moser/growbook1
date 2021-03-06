/***************************************************************************
 *            database-mariadb.h
 *
 *  Fr Mai 21 16:08:45 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * database-mariadb.h
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

#ifndef __DATABASE_MARIADB_H__
#define __DATABASE_MARIADB_H__

#ifdef HAVE_MARIADB

#include "database.h"

#include <mysql.h>

class DatabaseModuleMariaDB:
	public DatabaseModule
{
	 private:
		 DatabaseModuleMariaDB(const DatabaseModuleMariaDB &src) = delete;
		 DatabaseModuleMariaDB& operator = (const DatabaseModuleMariaDB &src) = delete;
		 
	 protected:
		 DatabaseModuleMariaDB();

	public:
		 virtual ~DatabaseModuleMariaDB();

	public:
		 static Glib::RefPtr<DatabaseModuleMariaDB> create();

	protected:
		Glib::RefPtr<Database> create_database_vfunc(const Glib::RefPtr<DatabaseSettings> &settings) override;
}; // DatabaseModuleMariaDB class

class DatabaseMariaDB:
	public Database
{
	public:
		static const char ENGINE[];
	private:
		static const char RESULT_ERROR[];
		
	private:
		MYSQL *m_db_;
	
	protected:
		DatabaseMariaDB(const Glib::RefPtr<DatabaseSettings> &settings);
		
	public:
		virtual ~DatabaseMariaDB();

	public:
		static Glib::RefPtr<DatabaseMariaDB> create(const Glib::RefPtr<DatabaseSettings> &settings);

	public:
		void begin_transaction ();
		void rollback();
		void commit();

	protected:
		void database_error(const Glib::ustring &message, bool rollback = false) const;
		
	protected:
		virtual bool is_connected_vfunc() const override;
		virtual bool test_connection_vfunc() override;
		virtual void connect_vfunc() override;
		virtual void close_vfunc() override;
		virtual void create_database_vfunc() override;

		virtual std::list<Glib::RefPtr<Breeder> > get_breeders_vfunc() const override;
		virtual Glib::RefPtr<Breeder> get_breeder_vfunc(uint64_t id) const override;
		virtual Glib::RefPtr<Breeder> get_breeder_vfunc(const Glib::ustring &name) const override;
		virtual void add_breeder_vfunc(const Glib::RefPtr<Breeder> &breeder) override;
		virtual void remove_breeder_vfunc(uint64_t breeder_id) override;

		virtual std::list<Glib::RefPtr<Strain> > get_strains_for_breeder_vfunc(uint64_t breeder_id) const override;
		virtual std::list<Glib::RefPtr<Strain> > get_strains_for_growlog_vfunc(uint64_t growlog_id) const override;
		virtual Glib::RefPtr<Strain> get_strain_vfunc(uint64_t id) const override;
		virtual Glib::RefPtr<Strain> get_strain_vfunc(const Glib::ustring &breeder_name,
		                                               const Glib::ustring &strain_name) const override;
		virtual void add_strain_vfunc(const Glib::RefPtr<Strain> &strain) override;
		virtual void remove_strain_vfunc(uint64_t strain_id) override;

		virtual std::list<Glib::RefPtr<Growlog> > get_growlogs_vfunc() const override;
		virtual std::list<Glib::RefPtr<Growlog> > get_ongoing_growlogs_vfunc() const override;
		virtual std::list<Glib::RefPtr<Growlog> > get_finished_growlogs_vfunc() const override;
		virtual std::list<Glib::RefPtr<Growlog> > get_growlogs_for_strain_vfunc(uint64_t strain_id) const override;
		virtual Glib::RefPtr<Growlog> get_growlog_vfunc(uint64_t id) const override;
		virtual Glib::RefPtr<Growlog> get_growlog_vfunc(const Glib::ustring &title) const override;
		virtual void add_growlog_vfunc(const Glib::RefPtr<Growlog> &growlog) override;
		virtual void remove_growlog_vfunc(uint64_t id) override;

		virtual std::list<Glib::RefPtr<GrowlogEntry> > get_growlog_entries_vfunc(uint64_t growlog_id) const override;
		virtual Glib::RefPtr<GrowlogEntry> get_growlog_entry_vfunc(uint64_t id) const override;
		virtual void add_growlog_entry_vfunc(const Glib::RefPtr<GrowlogEntry> &entry) override;
		virtual void remove_growlog_entry_vfunc(uint64_t id) override;

		virtual void add_strain_for_growlog_vfunc(uint64_t growlog_id,uint64_t strain_id) override;
		virtual void remove_strain_for_growlog_vfunc(uint64_t growlog_id,uint64_t strain_id) override;
		virtual void remove_strain_for_growlog_vfunc(uint64_t growlog_strain_id) override;		
}; // DatabaseMariaDB class



#endif /* HAVE_MARIADB */

#endif /* __DATABASE_MARIADB_H__ */