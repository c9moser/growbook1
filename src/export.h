/***************************************************************************
 *            export.h
 *
 *  Mo Mai 24 13:53:48 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * export.h
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

#ifndef __EXPORT_H__
#define __EXPORT_H__

#include <gtkmm/filechooserdialog.h>
#include <string>
#include <cstdint>

#include "refclass.h"
#include "database.h"

class Exporter:
	public RefClass
{
	 private:
		 Glib::RefPtr<Database> m_database_;
		 std::string m_filename_;

	private:
		 Exporter(const Exporter &src) = delete;
		 Exporter& operator=(const Exporter &src) = delete;

	protected:
		 Exporter(const Glib::RefPtr<Database> &database,
		          const std::string &filename=std::string());

	public:
		 virtual ~Exporter();

	public:
		 std::string get_filename() const;
		 void set_filename(const std::string &filename);
		 bool file_exists() const;

		 Glib::RefPtr<Database> get_database();
		 Glib::RefPtr<const Database> get_database() const;

		 void export_db();
		 void export_db(Gtk::Window &parent);

	protected:
		 virtual void export_vfunc(Gtk::Window &parent) = 0;
		 
};

/******************************************************************************/

class XML_Exporter:
	public Exporter
{
	 private:
		 Glib::RefPtr<Database> m_database_;
		 std::string m_filename_;

	private:
		 XML_Exporter(const XML_Exporter &src) = delete;
		 XML_Exporter& operator = (const XML_Exporter &src) = delete;

	protected:
		 XML_Exporter(const Glib::RefPtr<Database> &db,
		              const std::string &filename);

	public:
		 virtual ~XML_Exporter();

		 static Glib::RefPtr<XML_Exporter> create(const Glib::RefPtr<Database> &db,
		                                          const std::string &filename);

	protected:
		virtual void export_vfunc(Gtk::Window &parent);

	private:
		Glib::ustring escape_text(const Glib::ustring &text) const;
};

/******************************************************************************/
class DB_Exporter:
	public Exporter
{
	 public:
		 std::map<uint64_t,Glib::RefPtr<Breeder> > m_breeder_map_;
		 std::map<uint64_t,Glib::RefPtr<Strain> > m_strain_map_;
		 std::map<uint64_t,Glib::RefPtr<Growlog> > m_growlog_map_;
		 

	private:
		 DB_Exporter(const Exporter &src) = delete;
		 DB_Exporter& operator = (const Exporter &src) = delete;
		 
	protected:
	 	DB_Exporter(const Glib::RefPtr<Database> &db,
	    	        const std::string &filename);
	public:
	 	virtual ~DB_Exporter();

	public:
		 static Glib::RefPtr<DB_Exporter> create(const Glib::RefPtr<Database> &database,
		                        	             const std::string &filename);
	
	protected:
		virtual void export_vfunc(Gtk::Window &parent);
		
	private:
		 void _export_strains(Gtk::Window &parent,
		                      const Glib::RefPtr<Database> &export_database);
		 void _export_growlogs(Gtk::Window &parent,
		                       const Glib::RefPtr<Database> &export_database);
};
	

class ExportDialog:
	public Gtk::FileChooserDialog
{
	 private:
		 static const char TITLE[];

	 private:
			Glib::RefPtr<Database> m_database_;
		
	 public:
		 ExportDialog(const Glib::RefPtr<Database> &database);
		 ExportDialog(Gtk::Window &parent,
		              const Glib::RefPtr<Database> &database);
		 virtual ~ExportDialog();

	 private:
		 void _add_buttons();
		 void _configure();

	public:
		Glib::RefPtr<Exporter> get_exporter();
};

#endif /* __EXPORT_H__ */
