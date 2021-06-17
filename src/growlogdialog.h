/***************************************************************************
 *            growlogdialog.h
 *
 *  Mi Mai 12 20:23:37 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * growlogdialog.h
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

#ifndef __GROWLOGDIALOG_H__
#define __GROWLOGDIALOG_H__

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/notebook.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>

#include "database.h"

class GrowlogDialogStrainColumns:
	public Gtk::TreeModelColumnRecord
{
	 public:
		 Gtk::TreeModelColumn<uint64_t> column_id;
		 Gtk::TreeModelColumn<Glib::ustring> column_breeder;
		 Gtk::TreeModelColumn<Glib::ustring> column_name;
		 Gtk::TreeModelColumn<bool> column_changed;

	public:
		 GrowlogDialogStrainColumns();
		 virtual ~GrowlogDialogStrainColumns();
};

class GrowlogDialog:
	public Gtk::Dialog
{
	 public:
		using StrainColumns = GrowlogDialogStrainColumns;
		
	 private:
		 static const char TITLE[];

	 private:
		 Glib::RefPtr<Database> m_database_;
		 Glib::RefPtr<Growlog> m_growlog_;

		 bool m_update_database_;

		 std::list<uint64_t> m_deleted_strains_;
		
		 StrainColumns m_strain_columns_;
		 Gtk::Entry m_title_entry_;
		 Gtk::TextView m_description_view_;
		 Gtk::TreeView m_strain_view_;
		 Gtk::ToolButton m_add_button_;
		 Gtk::ToolButton m_remove_button_;
		 Gtk::Toolbar m_toolbar_;
		 Gtk::Notebook m_notebook_;
		 
	 public:
		 GrowlogDialog(const Glib::RefPtr<Database> &database,
		               const Glib::RefPtr<Growlog> &growlog = Glib::RefPtr<Growlog>());
		 GrowlogDialog(Gtk::Window &parent,
		               const Glib::RefPtr<Database> &database,
		               const Glib::RefPtr<Growlog> &growlog = Glib::RefPtr<Growlog>());
		 virtual ~GrowlogDialog();

	private:
		 void _add_buttons();
		 void _add_widgets();

		 void on_add_clicked();
		 void on_remove_clicked();

		 void on_strain_selection_changed();
		
	protected:
		 virtual void on_response(int response_id) override;

	public:
		Glib::RefPtr<Growlog> get_growlog();
		Glib::RefPtr<const Growlog> get_growlog() const;

		bool get_update_database() const;
		void set_update_database(bool set_update = true);
};

#endif /* __GROWLOGDIALOG_H__ */
