/***************************************************************************
 *            breederdialog.h
 *
 *  So Mai 02 16:22:51 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * breederdialog.h
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
#ifndef __BREEDERDIALOG_H__
#define __BREEDERDIALOG_H__

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/liststore.h>

#include <list>

#include "database.h"

class BreederDialogColumns:
	public Gtk::TreeModelColumnRecord
{
	 public:
		 Gtk::TreeModelColumn<uint64_t> column_id;
		 Gtk::TreeModelColumn<Glib::ustring> column_name;
		 Gtk::TreeModelColumn<Glib::ustring> column_info;
		 Gtk::TreeModelColumn<Glib::ustring> column_description;
		 Gtk::TreeModelColumn<std::string> column_homepage;
		 Gtk::TreeModelColumn<std::string> column_seedfinder;
		 Gtk::TreeModelColumn<bool>	column_changed;
			 
	 public:
	 	BreederDialogColumns();
	 	virtual ~BreederDialogColumns();
};

class BreederDialog:
	public Gtk::Dialog
{
	public:
		using Columns=BreederDialogColumns;
		
	 private:
		 static const char TITLE[];

	 private:
		 Glib::RefPtr<Database> m_database_;
		 Glib::RefPtr<Breeder> m_breeder_;

		 Columns m_columns_;
		
		 Gtk::Entry m_name_entry_;
		 Gtk::Entry m_homepage_entry_;
		 Gtk::ToolButton m_add_button_;
		 Gtk::ToolButton m_edit_button_;
		 Gtk::ToolButton m_delete_button_;
		 Gtk::Toolbar m_toolbar_;
		 Gtk::TreeView m_treeview_;

		 std::list<uint64_t> m_deleted_strains_;
		 
	 public:
		 BreederDialog(const Glib::RefPtr<Database> &database,
		               const Glib::RefPtr<Breeder> &breeder = Glib::RefPtr<Breeder>());
		 BreederDialog(Gtk::Window &parent,
		               const Glib::RefPtr<Database> &database,
		               const Glib::RefPtr<Breeder> &breeder = Glib::RefPtr<Breeder>());
		 virtual ~BreederDialog();

	private:
		 void _add_buttons();
		 void _add_widgets();

		void on_add_clicked();
		void on_edit_clicked();
		void on_delete_clicked();

	protected:
		virtual void on_response(int response_id) override;
};

#endif

