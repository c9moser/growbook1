/***************************************************************************
 *            import.h
 *
 *  Sa Mai 29 11:37:55 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * import.h
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

#ifndef __IMPORT_H__
#define __IMPORT_H__


#include <gtkmm/filechooserdialog.h>
#include <map>
#include "refclass.h"
#include "database.h"

enum ImportResponseID {
	 RESPONSE_NONE = 0,
	 RESPONSE_UPDATE,
	 RESPONSE_UPDATE_ALL,
	 RESPONSE_MERGE,
	 RESPONSE_MERGE_ALL,
	 RESPONSE_EDIT,
	 RESPONSE_IGNORE,
	 RESPONSE_IGNORE_ALL		 
};

class Importer:
	public RefClass
{
	private:
		Glib::RefPtr<Database> m_database_;
		std::string m_filename_;

	private:
		Importer(const Importer &src) = delete;
		Importer& operator=(const Importer &src) = delete;
		
	protected:
		Importer(const Glib::RefPtr<Database> &database,
		         const std::string &filename = std::string());

	public:
		virtual ~Importer();

	public:
		Glib::RefPtr<Database> get_database();
		Glib::RefPtr<const Database> get_database() const;
		
		std::string get_filename() const;
		void set_filename(const std::string &filename);

		void import_db();
		void import_db(Gtk::Window &parent);

		bool file_exists() const;
		
	protected:
		virtual void import_vfunc(Gtk::Window &parent) = 0; 
};

/******************************************************************************/

enum XmlNode {
	XML_NONE,
	XML_GROWBOOK,
	XML_BREEDER,
	XML_STRAIN,
	XML_GROWLOG,
	XML_GROWLOG_ENTRY		
};

/******************************************************************************/

class DB_Importer:
	public Importer
{
	 private:
		std::map<uint64_t,Glib::RefPtr<Breeder> > m_breeder_map_;
		std::map<uint64_t,Glib::RefPtr<Strain> > m_strain_map_;
		std::map<uint64_t,Glib::RefPtr<Growlog> > m_growlog_map_;
		

	private:
		 DB_Importer(const DB_Importer &src) = delete;
		 DB_Importer& operator = (const DB_Importer &src) = delete;
		 
	protected:
		 DB_Importer(const Glib::RefPtr<Database> &database,
		             const std::string &filename);

	public:
		 virtual ~DB_Importer();

	public:
		 static Glib::RefPtr<DB_Importer> create(const Glib::RefPtr<Database> & database,
		                                         const std::string &filename);

	protected:
		 virtual void import_vfunc(Gtk::Window &parent);
		
	private:
		 void _import_strains(Gtk::Window &parent,
		                      const Glib::RefPtr<Database> &import_from);
		 void _import_growlogs(Gtk::Window &parent,
		                       const Glib::RefPtr<Database> &import_from);
		
};

/******************************************************************************/

class ImportDialog:
	public Gtk::FileChooserDialog
{
	private:
		static const char TITLE[];

	private:
		Glib::RefPtr<Database> m_database_;
		
	public:
		ImportDialog(const Glib::RefPtr<Database> &database);
		ImportDialog(Gtk::Window &parent,
		             const Glib::RefPtr<Database> &database);

	private:
		void _add_buttons();
		void _configure();
		
	 public:
		Glib::RefPtr<Importer> get_importer();
};

#endif /* __IMPORT_H__ */
