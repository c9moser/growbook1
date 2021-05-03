//           breederdialog.cc
//  So Mai 02 16:22:51 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// breederdialog.cc
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

#include "breederdialog.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glibmm/i18n.h>

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/separator.h>
#include <gtkmm/scrolledwindow.h>
#include <cassert>

/*******************************************************************************
 * BreederDialogColumns
 ******************************************************************************/
BreederDialogColumns::BreederDialogColumns():
	Gtk::TreeModelColumnRecord{},
	column_id{},
	column_name{},
	column_info{},
	column_description{},
	column_homepage{},
	column_seedfinder{},
	column_changed{}
{
	add(column_id);
	add(column_name);
	add(column_info);
	add(column_description);
	add(column_homepage);
	add(column_seedfinder);
	add(column_changed);
}

BreederDialogColumns::~BreederDialogColumns()
{}

/*******************************************************************************
 * BreederDialog
 ******************************************************************************/

const char BreederDialog::TITLE[] = N_("GrowBook: Breeder");

BreederDialog::BreederDialog(const Glib::RefPtr<Database> &database,
                             const Glib::RefPtr<Breeder> &breeder):
	Gtk::Dialog{TITLE},
	m_database_{database},
	m_breeder_{breeder},
	m_columns_{},
	m_name_entry_{},
	m_homepage_entry_{},
	m_add_button_{},
	m_edit_button_{},
	m_delete_button_{},
	m_toolbar_{},
	m_treeview_{}
{
	assert(m_database_);
	
	_add_buttons();
	_add_widgets();

	set_default_size(400,400);
	show_all();
	
}

BreederDialog::BreederDialog(Gtk::Window &parent,
                             const Glib::RefPtr<Database> &database,
                             const Glib::RefPtr<Breeder> &breeder):
	Gtk::Dialog{TITLE,parent},
	m_database_{database},
	m_breeder_{breeder}
{
	assert(m_database_);

	_add_buttons();
	_add_widgets();

	set_default_size(400,400);
	show_all();
}

BreederDialog::~BreederDialog()
{
}

void
BreederDialog::_add_buttons()
{
	add_button(_("Apply"),Gtk::RESPONSE_APPLY);
	add_button(_("Cancel"),Gtk::RESPONSE_CANCEL);
}

void
BreederDialog::_add_widgets()
{
	if (m_breeder_) {
		m_name_entry_.set_text(m_breeder_->get_name());
		m_homepage_entry_.set_text(m_breeder_->get_homepage());
	}

	// Entries /////////////////////////////////////////////////////////////////
	
	Gtk::Box *box = get_content_area();
	Gtk::Grid *grid = Gtk::manage(new Gtk::Grid());

	Gtk::Label *label = Gtk::manage(new Gtk::Label(_("ID:")));
	grid->attach(*label,0,0,1,1);
	if (m_breeder_) {
		label = Gtk::manage(new Gtk::Label(std::to_string(m_breeder_->get_id())));
	} else {
		label = Gtk::manage(new Gtk::Label("0"));
	}
	grid->attach(*label,1,0,1,1);

	label = Gtk::manage(new Gtk::Label(_("Name:")));
	grid->attach(*label,0,1,1,1);
	grid->attach(m_name_entry_,1,1,1,1);

	label = Gtk::manage(new Gtk::Label(_("Homepage:")));
	grid->attach(*label,0,2,1,1);
	grid->attach(m_homepage_entry_,1,2,1,1);
	box->pack_start(*grid,false,false,0);

	box->pack_start(*Gtk::manage(new Gtk::VSeparator()),false,false,3);

	// Toolbar /////////////////////////////////////////////////////////////////

	Gtk::TreeModel::iterator tree_iter = m_treeview_.get_selection()->get_selected();
	if (!tree_iter) {
		m_edit_button_.set_sensitive(false);
		m_delete_button_.set_sensitive(false);
	}
	
	m_add_button_.signal_clicked().connect(sigc::mem_fun(*this,&BreederDialog::on_add_clicked));
	m_add_button_.set_icon_name("list-add");
	m_add_button_.set_tooltip_text (_("Add strain"));
	m_toolbar_.append(m_add_button_);

	m_edit_button_.signal_clicked().connect(sigc::mem_fun(*this,&BreederDialog::on_edit_clicked));
	m_edit_button_.set_icon_name("gtk-edit");
	m_edit_button_.set_tooltip_text(_("Edit strain"));
	m_toolbar_.append(m_edit_button_);

	m_delete_button_.signal_clicked().connect(sigc::mem_fun(*this,&BreederDialog::on_delete_clicked));
	m_delete_button_.set_icon_name("list-remove");
	m_delete_button_.set_tooltip_text(_("Delete strain"));
	m_toolbar_.append(m_delete_button_);

	box->pack_start(m_toolbar_,false,false,0);

	// StrainView //////////////////////////////////////////////////////////////

	Gtk::ScrolledWindow *scrolled = Gtk::manage(new Gtk::ScrolledWindow());
	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(m_columns_);

	if (m_breeder_) {
		std::list<Glib::RefPtr<Strain> > strains = m_database_->get_strains_for_breeder(m_breeder_);
		for (auto strain_iter = strains.begin(); strain_iter != strains.end(); ++strain_iter) {
			Glib::RefPtr<Strain> strain = *strain_iter;
			tree_iter = model->append();
			Gtk::TreeModel::Row row = *tree_iter;
			row[m_columns_.column_id] = strain->get_id();
			row[m_columns_.column_name] = strain->get_name();
			row[m_columns_.column_info] = strain->get_info();
			row[m_columns_.column_description] = strain->get_description();
			row[m_columns_.column_homepage] = strain->get_homepage();
			row[m_columns_.column_seedfinder] = strain->get_seedfinder();
			row[m_columns_.column_changed] = false;
		}
	}
	m_treeview_.set_model(model);
	m_treeview_.append_column(_("Name"),m_columns_.column_name);
	m_treeview_.set_headers_visible(false);

	scrolled->add(m_treeview_);
	box->pack_start(*scrolled,true,true,0);
	
}

void 
BreederDialog::on_response(int response)
{
	Gtk::Dialog::on_response(response);
}

void
BreederDialog::on_add_clicked()
{
}

void
BreederDialog::on_edit_clicked()
{
}

void
BreederDialog::on_delete_clicked()
{
}

