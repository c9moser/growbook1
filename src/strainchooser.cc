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
#include <glibmm/i18n.h>

#include <cassert>

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
	columns{}
{
	assert(m_database_);

	set_model(_create_model ());
	append_column(_("Name"),columns.column_name);
	set_headers_visible(false);
}

StrainChooserTreeView::~StrainChooserTreeView()
{
}

Glib::RefPtr<Gtk::TreeStore>
StrainChooserTreeView::_create_model()
{
	Glib::RefPtr<Gtk::TreeStore> model = Gtk::TreeStore::create(columns);

	std::list<Glib::RefPtr<Breeder> > breeders{m_database_->get_breeders()};
	for (auto breeder_iter = breeders.begin(); breeder_iter != breeders.end(); ++breeder_iter) {
		Glib::RefPtr<Breeder> breeder = *breeder_iter;
		Gtk::TreeModel::iterator iter0 = model->append();
		Gtk::TreeModel::Row row0 = *iter0;

		row0[columns.column_id] = 0;
		row0[columns.column_breeder_id] = breeder->get_id();
		row0[columns.column_name] = breeder->get_name();

		std::list<Glib::RefPtr<Strain> > strains{m_database_->get_strains_for_breeder(breeder)};
		for (auto strain_iter = strains.begin(); strain_iter != strains.end(); ++strain_iter) {
			Glib::RefPtr<Strain> strain = *strain_iter;
			Gtk::TreeModel::iterator iter1 = model->append(row0.children());
			Gtk::TreeModel::Row row1 = *iter1;

			row1[columns.column_id] = strain->get_id();
			row1[columns.column_breeder_id] = strain->get_breeder_id();
			row1[columns.column_name] = strain->get_name();
		}
	}
	return model;
}

Glib::RefPtr<Strain>
StrainChooserTreeView::get_selected_strain()
{
	Glib::RefPtr<Strain> strain;
	Gtk::TreeModel::iterator iter = get_selection()->get_selected();
	if (iter && (*iter)[columns.column_id]) {
		Gtk::TreeModel::Row row = *iter;
		uint64_t id = row[columns.column_id];
		strain = m_database_->get_strain(id);
	}
	return strain;
}

Glib::RefPtr<Database>
StrainChooserTreeView::get_database()
{
	return m_database_;
}

Glib::RefPtr<const Database>
StrainChooserTreeView::get_database() const
{
	return Glib::RefPtr<const Database>::cast_const(m_database_);
}

/*******************************************************************************
 * StrainChooser
 ******************************************************************************/

StrainChooser::StrainChooser(const Glib::RefPtr<Database> &db):
	Gtk::ScrolledWindow{},
	m_treeview_{db}
{
	add(m_treeview_);
	show_all();
}

StrainChooser::~StrainChooser()
{
}

Glib::RefPtr<Strain>
StrainChooser::get_selected_strain()
{
	return m_treeview_.get_selected_strain();
}

StrainChooser::TreeView*
StrainChooser::get_treeview()
{
	return &m_treeview_;
}

const StrainChooser::TreeView*
StrainChooser::get_treeview() const
{
	return &m_treeview_;
}

Glib::RefPtr<Database>
StrainChooser::get_database()
{
	return m_treeview_.get_database();
}

Glib::RefPtr<const Database>
StrainChooser::get_database() const
{
	return m_treeview_.get_database();
}

/*******************************************************************************
 * StrainChooserDialog
 ******************************************************************************/

const char StrainChooserDialog::TITLE[] = N_("GrowBook: Strain Chooser");

StrainChooserDialog::StrainChooserDialog(const Glib::RefPtr<Database> &db):
	Gtk::Dialog{_(TITLE)},
	m_strain_chooser_{db}
{
	_add_buttons();
	_add_widgets();

	set_default_size(300,300);
	show_all();
}

StrainChooserDialog::StrainChooserDialog(Gtk::Window &parent,
                                         const Glib::RefPtr<Database> &db):
	Gtk::Dialog{_(TITLE),parent},
	m_strain_chooser_{db}
{
	_add_buttons();
	_add_widgets();

	set_default_size(300,300);
	show_all();
}

void
StrainChooserDialog::_add_buttons()
{
	add_button(_("Apply"),Gtk::RESPONSE_APPLY);
	add_button(_("Cancel"),Gtk::RESPONSE_CANCEL);
}

void
StrainChooserDialog::_add_widgets()
{
	Gtk::Box *box = get_content_area();
	StrainChooser::TreeView *tv = m_strain_chooser_.get_treeview();
	tv->get_selection()->signal_changed().connect(sigc::mem_fun(*this,&StrainChooserDialog::on_chooser_selection_changed));
	on_chooser_selection_changed ();
	box->pack_start(m_strain_chooser_,true,true,0);
}

void
StrainChooserDialog::on_chooser_selection_changed()
{
	StrainChooser::TreeView *tv = m_strain_chooser_.get_treeview();
	Gtk::TreeModel::iterator iter = tv->get_selection()->get_selected();
	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		uint64_t id = row[tv->columns.column_id];
		if (id) {
			set_response_sensitive (Gtk::RESPONSE_APPLY,true);
			return;
		}
	}
	set_response_sensitive(Gtk::RESPONSE_APPLY,false);
}

Glib::RefPtr<Strain>
StrainChooserDialog::get_selected_strain()
{
	return m_strain_chooser_.get_selected_strain();
}

Glib::RefPtr<Database>
StrainChooserDialog::get_database()
{
	return m_strain_chooser_.get_database();
}

Glib::RefPtr<const Database>
StrainChooserDialog::get_database() const
{
	return m_strain_chooser_.get_database();
}
