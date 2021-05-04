/***************************************************************************
 *            strainchooser.h
 *
 *  Sa Mai 01 14:49:52 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * strainchooser.h
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
#ifndef __STRAINCHOOSER_H__
#define __STRAINCHOOSER_H__

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>

#include "database.h"

class StrainChooserColumns:
	public Gtk::TreeModelColumnRecord
{
	 public:
		 Gtk::TreeModelColumn<uint64_t> column_id;
		 Gtk::TreeModelColumn<uint64_t> column_breeder_id;
		 Gtk::TreeModelColumn<Glib::ustring> column_name;
		 
	 public:
		 StrainChooserColumns();
		 virtual ~StrainChooserColumns();
};

class StrainChooserTreeView:
	public Gtk::TreeView
{
	public:
		using Columns = StrainChooserColumns; 
	private:
		Glib::RefPtr<Database> m_database_;
		Columns m_columns_;
	
	public:
		StrainChooserTreeView(const Glib::RefPtr<Database> &database);
		virtual ~StrainChooserTreeView();

	private:
		Glib::RefPtr<Gtk::TreeStore> _create_model();
};

class StrainChooser:
	public Gtk::ScrolledWindow
{
	public:
		using TreeView = StrainChooserTreeView;
		using Columns = StrainChooserColumns;
		
	private:
		TreeView m_treeview_;

	public:
		StrainChooser(const Glib::RefPtr<Database> &database);
		virtual ~StrainChooser();
};

	

#endif /* __STRAINCHOOSER_H__ */
