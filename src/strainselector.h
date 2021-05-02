/***************************************************************************
 *            strainselector.h
 *
 *  Sa Mai 01 15:09:58 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * strainselector.h
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
#ifndef __STRAINSELECTOR_H__
#define __STRAINSELECTOR_H__

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>

#include "database.h"

class StrainSelectorColumns:
	public Gtk::TreeModelColumnRecord
{
	 public:
		 Gtk::TreeModelColumn<uint64_t> column_id;
		 Gtk::TreeModelColumn<uint64_t> column_breeder_id;
		 Gtk::TreeModelColumn<Glib::ustring> column_name;

	public:
		 StrainSelectorColumns();
		 virtual ~StrainSelectorColumns();
}; // StrainSelectorColumns class

class StrainSelectorTreeView:
	public Gtk::TreeView
{
	public:
		using Columns = StrainSelectorColumns;

	public:
		Columns columns;
	private:
		Glib::RefPtr<Database> m_database_;
		
	public:
		StrainSelectorTreeView(const Glib::RefPtr<Database> &database);
		virtual ~StrainSelectorTreeView();

	private:
		Glib::RefPtr<Gtk::TreeStore> _create_model();
		
	public:
		Glib::RefPtr<Database> get_database();
		Glib::RefPtr<const Database> get_database() const;

	protected:
		void on_row_activated(const Gtk::TreeModel::Path &path, 
		                      Gtk::TreeViewColumn *column) override;
		
}; // StrainSelectorTreeView class

class StrainSelector:
	public Gtk::ScrolledWindow
{
	public:
		using Columns = StrainSelectorColumns;
		using TreeView = StrainSelectorTreeView;
		
	private:
		TreeView m_treeview_;
		
	public:
		StrainSelector(const Glib::RefPtr<Database> &database);
		virtual ~StrainSelector();

	public:
		 TreeView* get_treeview();
		 const TreeView* get_treeview () const;

		Glib::RefPtr<Database> get_database();
		Glib::RefPtr<const Database> get_database() const;
}; // StrainSelector class


#endif /* __STRAINSELECTOR_H__ */
