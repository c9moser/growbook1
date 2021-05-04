//           strainchooser.cc
//  Sa Mai 01 14:49:52 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// strainchooser.cc
//
// Copyright (C) 2021 - Christian Moser
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "strainchooser.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/*******************************************************************************
 * StrainChooserColumns
 ******************************************************************************/

StrainChooserColumns::StrainChooserColumns():
	Gtk::TreeModelColumnRecord{}
{
	add(column_id);
	add(column_breeder_id);
	add(column_name);
}

StrainChooserColumns::~StrainChooserColumns()
{
}

/*******************************************************************************
 * StrainChooserTreeView
 ******************************************************************************/

StrainChooserTreeView::StrainChooserTreeView(const Glib::RefPtr<Database> &db):
	Gtk::TreeView{},
	m_database_{db},
	m_columns_{}
{
}

StrainChooserTreeView::~StrainChooserTreeView()
{
}

Glib::RefPtr<Gtk::TreeStore>
StrainChooserTreeView::_create_model()
{
	Glib::RefPtr<Gtk::TreeStore> model = Gtk::TreeStore::create(m_columns_);

	return model;
}

/*******************************************************************************
 * StrainChooser
 ******************************************************************************/

StrainChooser::StrainChooser(const Glib::RefPtr<Database> &db):
	Gtk::ScrolledWindow{},
	m_treeview_{db}
{
}

StrainChooser::~StrainChooser()
{
}
