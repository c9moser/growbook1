/***************************************************************************
 *            straindialog.h
 *
 *  Mo Mai 03 18:14:42 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * straindialog.h
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
#ifndef __STRAINDIALOG_H__
#define __STRAINDIALOG_H__

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/notebook.h>
#include <gtkmm/textview.h>

#include "database.h"

class StrainDialog:
	public Gtk::Dialog
{
	 private:
		 static const char TITLE[];
	 private:
		 bool m_update_database_;
		 
		 Glib::RefPtr<Database> m_database_;
		 Glib::RefPtr<Strain> m_strain_;

		 Gtk::Entry m_name_entry_;
		 Gtk::Entry m_homepage_entry_;
		 Gtk::Entry m_seedfinder_entry_;

		 Gtk::TextView m_info_textview_;
		 Gtk::TextView m_description_textview_;
		 Gtk::Notebook m_notebook_;

	public:
		 StrainDialog(const Glib::RefPtr<Database> &database,
		              const Glib::RefPtr<Strain> &strain);
		 StrainDialog(Gtk::Window &parent,
		              const Glib::RefPtr<Database> &database,
		              const Glib::RefPtr<Strain> &strain);
		 virtual ~StrainDialog();

	private:
		 void _add_buttons();
		 void _add_widgets();

	public:
		 Glib::RefPtr<Strain> get_strain();
		 Glib::RefPtr<const Strain> get_strain() const;

		 bool get_update_database() const;
		 void set_update_database(bool set=true);

	protected:
		 void on_response(int response_id);
};

#endif /* __STRAINDIALOG_H__ */
