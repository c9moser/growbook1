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

#include "refclass.h"
#include "database.h"

class Importer:
	public RefClass
{
	 private:
		 Glib::RefPtr<Database> m_database_;
		 std::string m_filename_;

	private:
		 Importer(const Importer &src) = delete;
		 Importer& operator = (const Importer &src) = delete;
		 
	protected:
		 Importer(const Glib::RefPtr<Database> &database,
		          const std::string &filename);

	public:
		 virtual ~Importer();

	public:
		 static Glib::RefPtr<Importer> create(const Glib::RefPtr<Database> & database,
		                                      const std::string &filename);
	public:
		 void import_db();
		 void import_db(Gtk::Window &parent);
};

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
