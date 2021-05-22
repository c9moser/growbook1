//           database.cc
//  Fr April 23 19:19:49 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// database.cc
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

#include "database.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cassert>

#include "database-sqlite3.h"

#ifdef HAVE_LIBPQ
# include "database-postgresql.h"
#endif

#ifdef HAVE_MARIADB
# include "database-mariadb.h"
#endif

/*******************************************************************************
 * DatabaseModule functions
 ******************************************************************************/

static std::list<Glib::RefPtr<DatabaseModule> > _db_modules;

void db_init() {
	if (_db_modules.empty()) {
		db_add_module(DatabaseModuleSqlite3::create());
#ifdef HAVE_LIBPQ
		db_add_module(DatabaseModulePostgresql::create());
#endif /* HAVE_LIBPQ */
#ifdef HAVE_MARIADB
		db_add_module(DatabaseModuleMariaDB::create());
#endif
	}
}

std::list<Glib::RefPtr<DatabaseModule> > db_get_modules()
{
	return _db_modules;
}

Glib::RefPtr<DatabaseModule> db_get_module(const Glib::ustring &engine)
{
	Glib::RefPtr<DatabaseModule> ret;
	for (auto iter = _db_modules.begin(); iter != _db_modules.end(); ++iter) {
		if ((*iter)->get_engine() == engine) {
			ret = (*iter);
			break;
		}
	}
	return ret;
}

void db_add_module(const Glib::RefPtr<DatabaseModule> &module) {
	for (auto iter = _db_modules.begin(); iter != _db_modules.end(); ++iter) {
		if ((*iter)->get_engine() == module->get_engine()) {
			(*iter) = module;
			return;
		}
	}
	_db_modules.push_back(module);
}

/*******************************************************************************
 * Database
 ******************************************************************************/

Database::Database(const Glib::RefPtr<DatabaseSettings> &settings) noexcept:
	RefClass{},
	m_settings_{settings}
{
}

Database::~Database()
{
}

Glib::RefPtr<DatabaseSettings>
Database::get_settings()
{
	return m_settings_;
}

Glib::RefPtr<const DatabaseSettings>
Database::get_settings() const
{
	return Glib::RefPtr<const DatabaseSettings>::cast_const(m_settings_);
}

bool
Database::is_connected() const
{
	return this->is_connected_vfunc();
}

bool
Database::test_connection()
{
	return this->test_connection_vfunc();
}

void
Database::create_database()
{
	this->create_database_vfunc();
}

void
Database::connect()
{
	this->connect_vfunc();
}

void
Database::close()
{
	this->close_vfunc();
}

std::list<Glib::RefPtr<Breeder > >
Database::get_breeders() const
{
	return this->get_breeders_vfunc();
}

Glib::RefPtr<Breeder>
Database::get_breeder(uint64_t id) const
{
	return this->get_breeder_vfunc (id);
}

Glib::RefPtr<Breeder>
Database::get_breeder(const Glib::ustring &name) const
{
	return this->get_breeder_vfunc(name);
}

void
Database::add_breeder(const Glib::RefPtr<Breeder> &breeder)
{
	this->add_breeder_vfunc(breeder);
}

void
Database::remove_breeder(uint64_t id)
{
	this->remove_breeder_vfunc (id);
}

void
Database::remove_breeder(const Glib::RefPtr<Breeder> &breeder)
{
	this->remove_breeder_vfunc(breeder->get_id());
}


std::list<Glib::RefPtr<Strain> >
Database::get_strains_for_breeder(uint64_t breeder_id) const
{
	return this->get_strains_for_breeder_vfunc(breeder_id);
}

std::list<Glib::RefPtr<Strain> >
Database::get_strains_for_breeder(const Glib::RefPtr<Breeder> &breeder) const
{
	return this->get_strains_for_breeder_vfunc(breeder->get_id());
}

std::list<Glib::RefPtr<Strain> >
Database::get_strains_for_growlog(uint64_t growlog_id) const
{
	return this->get_strains_for_growlog_vfunc(growlog_id);
}


std::list<Glib::RefPtr<Strain> >
Database::get_strains_for_growlog(const Glib::RefPtr<Growlog> &growlog) const
{
	return this->get_strains_for_growlog_vfunc(growlog->get_id());
}


Glib::RefPtr<Strain>
Database::get_strain(uint64_t id) const
{
	return this->get_strain_vfunc(id);
}

Glib::RefPtr<Strain>
Database::get_strain(const Glib::ustring &breeder_name,
                     const Glib::ustring &strain_name) const
{
	return this->get_strain_vfunc(breeder_name,strain_name);	                              
}

void
Database::add_strain(const Glib::RefPtr<Strain> &strain)
{
	this->add_strain_vfunc(strain);
}

void
Database::remove_strain(uint64_t id)
{
	this->remove_strain_vfunc(id);
}

void
Database::remove_strain(const Glib::RefPtr<Strain> &strain)
{
	this->remove_strain_vfunc(strain->get_id());
}

std::list<Glib::RefPtr<Growlog> > 
Database::get_growlogs() const
{
	return this->get_growlogs_vfunc();
}

std::list<Glib::RefPtr<Growlog> > 
Database::get_ongoing_growlogs() const
{
	return this->get_ongoing_growlogs_vfunc();
}

std::list<Glib::RefPtr<Growlog> > 
Database::get_finished_growlogs() const
{
	return this->get_finished_growlogs_vfunc();
}

std::list<Glib::RefPtr<Growlog> >
Database::get_growlogs_for_strain(uint64_t strain_id) const
{
	return this->get_growlogs_for_strain_vfunc (strain_id);
}

std::list<Glib::RefPtr<Growlog> >
Database::get_growlogs_for_strain(const Glib::RefPtr<Strain> &strain) const
{
	return this->get_growlogs_for_strain_vfunc (strain->get_id());
}

Glib::RefPtr<Growlog> 
Database::get_growlog(uint64_t id) const
{
	return this->get_growlog_vfunc(id);
}

Glib::RefPtr<Growlog> 
Database::get_growlog(const Glib::ustring &title) const
{
	return this->get_growlog_vfunc(title);
}

void 
Database::add_growlog(const Glib::RefPtr<Growlog> &growlog)
{
	return this->add_growlog_vfunc(growlog);
}

void 
Database::remove_growlog(uint64_t id)
{
	return this->remove_growlog_vfunc(id);
}

void 
Database::remove_growlog(const Glib::RefPtr<Growlog> &growlog)
{
	return this->remove_growlog_vfunc(growlog->get_id());
}

std::list<Glib::RefPtr<GrowlogEntry> > 
Database::get_growlog_entries(uint64_t growlog_id) const
{
	return this->get_growlog_entries_vfunc(growlog_id);
}

std::list<Glib::RefPtr<GrowlogEntry> > 
Database::get_growlog_entries(const Glib::RefPtr<Growlog> &growlog) const
{
	return this->get_growlog_entries_vfunc(growlog->get_id());
}

Glib::RefPtr<GrowlogEntry> 
Database::get_growlog_entry(uint64_t id) const
{
	return this->get_growlog_entry_vfunc(id);
}

void 
Database::add_growlog_entry(const Glib::RefPtr<GrowlogEntry> &entry)
{
	return this->add_growlog_entry_vfunc(entry);
}

void 
Database::remove_growlog_entry(uint64_t id)
{
	return this->remove_growlog_entry_vfunc(id);
}

void 
Database::remove_growlog_entry(const Glib::RefPtr<GrowlogEntry> &entry)
{
	return this->remove_growlog_entry_vfunc(entry->get_id());
}

void 
Database::add_strain_for_growlog(uint64_t growlog_id,uint64_t strain_id)
{
	return this->add_strain_for_growlog_vfunc(growlog_id,strain_id);
}

void 
Database::add_strain_for_growlog(const Glib::RefPtr<Growlog> &growlog,
                                 const Glib::RefPtr<Strain> &strain)
{
	return this->add_strain_for_growlog_vfunc(growlog->get_id(),strain->get_id());
}

void 
Database::remove_strain_for_growlog(uint64_t growlog_id,
                                    uint64_t strain_id)
{
	return this->remove_strain_for_growlog_vfunc(growlog_id,strain_id);
}

void 
Database::remove_strain_for_growlog(const Glib::RefPtr<Growlog> &growlog,
                                    const Glib::RefPtr<Strain> &strain)
{
	return this->remove_strain_for_growlog_vfunc(growlog->get_id(),strain->get_id());
}

void 
Database::remove_strain_for_growlog(uint64_t growlog_strain_id)
{
	return this->remove_strain_for_growlog_vfunc(growlog_strain_id);
}

/*******************************************************************************
 * DatabaseModule
 ******************************************************************************/

DatabaseModule::DatabaseModule(const Glib::RefPtr<DatabaseSettings> &defaults) noexcept:
	RefClass{},
	m_defaults_{defaults}
{
	assert(m_defaults_);
}

DatabaseModule::~DatabaseModule()
{
}

Glib::RefPtr<DatabaseSettings>
DatabaseModule::get_defaults()
{
	return m_defaults_;
}

Glib::ustring
DatabaseModule::get_engine() const
{
	return m_defaults_->get_engine();
}

Glib::RefPtr<const DatabaseSettings>
DatabaseModule::get_defaults() const
{
	return Glib::RefPtr<const DatabaseSettings>::cast_const(m_defaults_);
}

Glib::RefPtr<Database>
DatabaseModule::create_database(const Glib::RefPtr<DatabaseSettings> &settings)
{
	return this->create_database_vfunc(settings);
}
