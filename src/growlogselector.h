/***************************************************************************
 *            growlogselector.h
 *
 *  Mo Mai 10 17:01:26 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * growlogselector.h
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
#ifndef __GROWLOGSELECTOR_H__
#define __GROWLOGSELECTOR_H__

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>

#include "database.h"

class GrowlogSelectorColumns:
	public Gtk::TreeModelColumnRecord
{
	 public:
		 Gtk::TreeModelColumn<uint64_t> column_id;
		 Gtk::TreeModelColumn<Glib::ustring> column_title;

	public:
		 GrowlogSelectorColumns();
		 virtual ~GrowlogSelectorColumns();
};

class GrowlogSelectorTreeView:
	public Gtk::TreeView
{
	public:
		using Columns = GrowlogSelectorColumns;
	public:
		Columns columns;
	private:
		Glib::RefPtr<Database> m_database_;

		Gtk::MenuItem m_refresh_menuitem_;
		Gtk::MenuItem m_open_menuitem_;
		Gtk::MenuItem m_new_menuitem_;
		Gtk::MenuItem m_edit_menuitem_;
		Gtk::MenuItem m_delete_menuitem_;
		Gtk::Menu m_popup_menu_;
	public:
		GrowlogSelectorTreeView(const Glib::RefPtr<Database> &database);
		virtual ~GrowlogSelectorTreeView();

	private:
		Glib::RefPtr<Gtk::TreeStore> _create_model();

		void on_open();
		void on_new();
		void on_edit();
		void on_delete();
	public:
		void refresh();

		Glib::RefPtr<Database> get_database();
		Glib::RefPtr<const Database> get_database() const;

	protected:
		virtual bool on_button_press_event(GdkEventButton *button_event) override;
		virtual void on_row_activated (const Gtk::TreeModel::Path &path,
		                               Gtk::TreeViewColumn *column) override;
};

class GrowlogSelector:
	public Gtk::ScrolledWindow
{
	public:
		using Columns = GrowlogSelectorColumns;
		using TreeView = GrowlogSelectorTreeView;
		
	private:
		TreeView m_treeview_;

	public:
		GrowlogSelector(const Glib::RefPtr<Database> &database);
		virtual ~GrowlogSelector();

	public:
		TreeView* get_treeview();
		const TreeView* get_treeview() const;

		Glib::RefPtr<Database> get_database();
		Glib::RefPtr<const Database> get_database() const;

		void refresh();
};



#endif /* __GROWLOGSELECTOR_H__ */
