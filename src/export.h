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
	 public:
		 Glib::RefPtr<Database> m_database_;
		 std::string m_filename_;

		 std::map<uint64_t,Glib::RefPtr<Breeder> > m_breeder_map_;
		 std::map<uint64_t,Glib::RefPtr<Strain> > m_strain_map_;
		 std::map<uint64_t,Glib::RefPtr<Growlog> > m_growlog_map_;
		 

	private:
		 Exporter(const Exporter &src) = delete;
		 Exporter& operator = (const Exporter &src) = delete;
		 
	protected:
	 	Exporter(const Glib::RefPtr<Database> &db,
	    	      const std::string &filename);
	public:
	 	virtual ~Exporter();

	public:
		 static Glib::RefPtr<Exporter> create(const Glib::RefPtr<Database> &database,
		                                      const std::string &filename);
	public:
		bool file_exists() const;

		std::string get_filename() const;
		void set_filename(const std::string &filename);
		 
	 	void export_db();
		void export_db(Gtk::Window &parent);

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
