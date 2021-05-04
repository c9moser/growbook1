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
#include <gtkmm/messagedialog.h>
#include <cassert>

#include "straindialog.h"
#include "error.h"

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
	m_treeview_.get_selection()->signal_changed().connect(sigc::mem_fun(*this,&BreederDialog::on_selection_changed));
	scrolled->add(m_treeview_);
	box->pack_start(*scrolled,true,true,0);
	
}

Glib::RefPtr<Breeder>
BreederDialog::get_breeder()
{
	return m_breeder_;
}

Glib::RefPtr<const Breeder>
BreederDialog::get_breeder() const
{
	return Glib::RefPtr<Breeder>::cast_const(m_breeder_);
}

void 
BreederDialog::on_response(int response)
{
	typedef Gtk::TreeModel::Children children_t;
	
	if (response == Gtk::RESPONSE_APPLY) {

		if (! m_breeder_) {
			m_breeder_ = Breeder::create(m_name_entry_.get_text(),
			                             m_homepage_entry_.get_text().c_str());
		} else {	
			m_breeder_->set_name(m_name_entry_.get_text());
			m_breeder_->set_homepage(m_homepage_entry_.get_text().c_str());
		}
		

		try {
			m_database_->add_breeder(m_breeder_);
		} catch (DatabaseError ex) {
			Gtk::MessageDialog dialog{*this,ex.what(),false,Gtk::MESSAGE_ERROR,Gtk::BUTTONS_OK,true};
			dialog.run();
			dialog.hide();
			return;
		}

		if (!m_breeder_->get_id())
			m_breeder_ = m_database_->get_breeder(m_breeder_->get_name());

		// delete strains
		for (auto iter = m_deleted_strains_.begin(); iter != m_deleted_strains_.end(); ++iter) {
			try {
				m_database_->remove_strain(*iter);
			} catch (DatabaseError ex) {
				Gtk::MessageDialog dialog{*this,ex.what(),false,Gtk::MESSAGE_ERROR,Gtk::BUTTONS_OK,true};
				dialog.run();
				dialog.hide();
			}
		}
		// add strains
		Glib::RefPtr<Gtk::ListStore> model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_treeview_.get_model());

		assert(model);

		children_t children = model->children();

		for (children_t::iterator iter = children.begin(); iter != children.end(); ++iter) {
			Gtk::TreeModel::Row row = *iter;
			uint64_t strain_id = row[m_columns_.column_id];
			if (!strain_id || row[m_columns_.column_changed]) {
				Glib::RefPtr<Strain> strain = Strain::create(strain_id,
				                                             m_breeder_->get_id(),
				                                             m_breeder_->get_name(),
				                                             row[m_columns_.column_name],
				                                             row[m_columns_.column_info],
				                                             row[m_columns_.column_description],
				                                             row[m_columns_.column_homepage],
				                                             row[m_columns_.column_seedfinder]);
				try {
					m_database_->add_strain(strain);
				} catch (DatabaseError ex) {
					Gtk::MessageDialog dialog{*this,ex.what(),false,Gtk::MESSAGE_ERROR,Gtk::BUTTONS_OK,true};
					dialog.run();
					dialog.hide();
				}
			}
		}
	}
	Gtk::Dialog::on_response(response);
}

void
BreederDialog::on_selection_changed()
{
	Gtk::TreeModel::iterator iter = m_treeview_.get_selection()->get_selected();
	if (iter) {
		m_edit_button_.set_sensitive(true);
		m_delete_button_.set_sensitive(true);
	} else {
		m_edit_button_.set_sensitive (false);
		m_delete_button_.set_sensitive(false);
	}
}

void
BreederDialog::on_add_clicked()
{
	typedef Gtk::TreeModel::Children children_t;
	
	uint64_t id = 0;
	if (m_breeder_)
		id = m_breeder_->get_id();
	Glib::ustring name = m_name_entry_.get_text();
	
	Glib::RefPtr<Strain> strain = Strain::create(id,name,"","","","","");
	StrainDialog dialog{*this,m_database_,strain};
	dialog.set_update_database(false);
	
	int response = dialog.run();
	dialog.hide();
	if (response == Gtk::RESPONSE_APPLY) {
		strain = dialog.get_strain();
		Glib::RefPtr<Gtk::ListStore> model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_treeview_.get_model());

		assert(model);
		
		children_t children = model->children();
		for (children_t::iterator iter = children.begin(); iter != children.end(); ++iter) {
			Gtk::TreeModel::Row row = *iter;
			Glib::ustring name = row[m_columns_.column_name];
			if (strain->get_name() == name) {
				Gtk::MessageDialog dialog{*this,
				                          _("Strain already exists. Do you want to overwrite existing strain?"),
				                          false,
				                          Gtk::MESSAGE_WARNING,
				                          Gtk::BUTTONS_YES_NO,
				                          true};
				int response1 = dialog.run();
				dialog.hide();
				if (response1 == Gtk::RESPONSE_YES) {
					row[m_columns_.column_info] = strain->get_info();
					row[m_columns_.column_description] = strain->get_description();
					row[m_columns_.column_homepage] = strain->get_homepage();
					row[m_columns_.column_seedfinder] = strain->get_seedfinder();
					row[m_columns_.column_changed] = true;
				}
				return;
			}
		}	
		
		Gtk::TreeModel::iterator iter = model->append();
		Gtk::TreeModel::Row row = *iter;

		row[m_columns_.column_id] = 0;
		row[m_columns_.column_name] = strain->get_name();
		row[m_columns_.column_info] = strain->get_info();
		row[m_columns_.column_description] = strain->get_description();
		row[m_columns_.column_homepage] = strain->get_homepage();
		row[m_columns_.column_seedfinder] = strain->get_seedfinder();
		row[m_columns_.column_changed] = true;
	}
}

void
BreederDialog::on_edit_clicked()
{
	Gtk::TreeModel::iterator iter = m_treeview_.get_selection()->get_selected();
	if (!iter)
		return;

	uint64_t breeder_id = 0;
	if (m_breeder_)
		breeder_id = m_breeder_->get_id();
	
	Gtk::TreeModel::Row row = *iter;
	Glib::RefPtr<Strain> strain = Strain::create(row[m_columns_.column_id],
	                                             breeder_id,
	                                             m_name_entry_.get_text(),
	                                             row[m_columns_.column_name],
	                                             row[m_columns_.column_info],
	                                             row[m_columns_.column_description],
	                                             row[m_columns_.column_homepage],
	                                             row[m_columns_.column_seedfinder]);
	StrainDialog dialog{*this,m_database_,strain};
	dialog.set_update_database(false);
	int response = dialog.run();
	if (response == Gtk::RESPONSE_APPLY) {
		strain = dialog.get_strain();
		row[m_columns_.column_name] = strain->get_name();
		row[m_columns_.column_info] = strain->get_info();
		row[m_columns_.column_description] = strain->get_description();
		row[m_columns_.column_homepage] = strain->get_homepage();
		row[m_columns_.column_seedfinder] = strain->get_seedfinder();
		row[m_columns_.column_changed] = true;
	}
}

void
BreederDialog::on_delete_clicked()
{
	Gtk::TreeModel::iterator iter = m_treeview_.get_selection()->get_selected();
	if (!iter)
		return;
	
	Gtk::MessageDialog dialog{*this,
	                          _("Do you really want to delete the selected strain?"),
	                          false,
	                          Gtk::MESSAGE_QUESTION,
	                          Gtk::BUTTONS_YES_NO,
	                          true};
	int response = dialog.run();
	if (response == Gtk::RESPONSE_YES) {
		Gtk::TreeModel::Row row = *iter;
		uint64_t id = row[m_columns_.column_id];
		if (id)
			m_deleted_strains_.push_back(id);
		Glib::RefPtr<Gtk::ListStore> model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_treeview_.get_model());
		
		assert(model);

		model->erase(iter);
	}
}

