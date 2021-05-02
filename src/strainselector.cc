//           strainselector.cc
//  Sa Mai 01 15:09:58 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// strainselector.cc
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

#include "strainselector.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>

#include "strainview.h"
#include "application.h"

#include <iostream>

/*******************************************************************************
 * StrainSelectorColumns
 ******************************************************************************/
StrainSelectorColumns::StrainSelectorColumns():
	Gtk::TreeModelColumnRecord{},
	column_id{},
	column_breeder_id{},
	column_name{}
{
	add(column_id);
	add(column_breeder_id);
	add(column_name);
}

StrainSelectorColumns::~StrainSelectorColumns()
{}

/*******************************************************************************
 * StrainSelectorTreeView
 ******************************************************************************/
StrainSelectorTreeView::StrainSelectorTreeView(const Glib::RefPtr<Database> &db):
	Gtk::TreeView{},
	columns{},
	m_database_{db}
{
	set_model(_create_model());
	append_column ("Name",columns.column_name);
	set_headers_visible(false);
}

StrainSelectorTreeView::~StrainSelectorTreeView()
{
}

Glib::RefPtr<Gtk::TreeStore>
StrainSelectorTreeView::_create_model()
{
	Glib::RefPtr<Gtk::TreeStore> model = Gtk::TreeStore::create(columns);

	std::list<Glib::RefPtr<Breeder> > breeders{m_database_->get_breeders()};
	for (auto breeder_iter = breeders.begin(); breeder_iter != breeders.end(); ++breeder_iter) {
		Gtk::TreeModel::iterator model_iter = model->append();
		Gtk::TreeModel::Row row = *model_iter;
		row[columns.column_id] = 0;
		row[columns.column_breeder_id] = (*breeder_iter)->get_id();
		row[columns.column_name] = (*breeder_iter)->get_name();
		
		std::list<Glib::RefPtr<Strain> > strains{m_database_->get_strains_for_breeder(*breeder_iter)};
		for (auto strain_iter = strains.begin(); strain_iter != strains.end(); ++strain_iter) {
			Gtk::TreeModel::iterator child_iter = model->append(model_iter->children());
			Gtk::TreeModel::Row row = *child_iter;
			Glib::RefPtr<Strain> strain = *strain_iter;
			row[columns.column_id] = strain->get_id();
			row[columns.column_breeder_id] = strain->get_breeder_id();
			row[columns.column_name] = strain->get_name();
		}
	}
	return model;
}

Glib::RefPtr<Database>
StrainSelectorTreeView::get_database()
{
	return m_database_;
}

Glib::RefPtr<const Database>
StrainSelectorTreeView::get_database() const
{
	return Glib::RefPtr<const Database>::cast_const(m_database_);
}

void
StrainSelectorTreeView::on_row_activated(const Gtk::TreeModel::Path &path,
                                         Gtk::TreeViewColumn *column)
{
	Gtk::TreeView::on_row_activated (path,column);
	
	Gtk::TreeModel::iterator iter = get_model()->get_iter(path);
	Gtk::TreeModel::Row row = *iter;

	uint64_t id = row[columns.column_id];

	if (id) {
		Glib::RefPtr<Strain> strain = m_database_->get_strain(id);
		if (strain) {
			StrainView *strain_view = Gtk::manage(new StrainView(m_database_,strain));
			AppWindow *appwindow = dynamic_cast<AppWindow*>(get_toplevel());
			if (!appwindow)
				appwindow = app->get_appwindow();
			
			int pagenum = appwindow->add_browser_page(*strain_view);
			if (pagenum == -1)
				delete strain_view;
		}
	}
}
/*******************************************************************************
 * StrainSelector
 ******************************************************************************/

StrainSelector::StrainSelector(const Glib::RefPtr<Database> &db):
	Gtk::ScrolledWindow{},
	m_treeview_{db}
{
	set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	add(m_treeview_);
}

StrainSelector::~StrainSelector()
{
}

StrainSelector::TreeView*
StrainSelector::get_treeview()
{
	return &m_treeview_;
}

const StrainSelector::TreeView*
StrainSelector::get_treeview() const
{
	return &m_treeview_;
}
