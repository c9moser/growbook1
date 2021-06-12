//           growlogdialog.cc
//  Mi Mai 12 20:23:37 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// growlogdialog.cc
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

#include "growlogdialog.h"
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>

#include <gtkmm/label.h>
#include <gtkmm/grid.h>
#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/separator.h>
#include <cassert>

#include "strainchooser.h"
#include "error.h"

/*******************************************************************************
 * GrowlogDialogStrainColumns
 ******************************************************************************/

GrowlogDialogStrainColumns::GrowlogDialogStrainColumns()
{
	add(column_id);
	add(column_breeder);
	add(column_name);
	add(column_changed);
}

GrowlogDialogStrainColumns::~GrowlogDialogStrainColumns()
{}

/*******************************************************************************
 * GrowlogDialog
 ******************************************************************************/

const char GrowlogDialog::TITLE[] = N_("Growbook: Growlog");

GrowlogDialog::GrowlogDialog(const Glib::RefPtr<Database> &db,
                             const Glib::RefPtr<Growlog> &growlog):
	Gtk::Dialog{TITLE},
	m_database_{db},
	m_growlog_{growlog},
	m_update_database_{true},
	m_deleted_strains_{},
	m_strain_columns_{},
	m_title_entry_{},
	m_description_view_{},
	m_strain_view_{},
	m_add_button_{},
	m_remove_button_{},
	m_toolbar_{},
	m_notebook_{}
{
	assert(m_database_);
	
	_add_buttons();
	_add_widgets();

	set_default_size(300,300);
	show_all();
}

GrowlogDialog::GrowlogDialog(Gtk::Window &parent,
                             const Glib::RefPtr<Database> &db,
                             const Glib::RefPtr<Growlog> &growlog):
	Gtk::Dialog{TITLE,parent},
	m_database_{db},
	m_growlog_{growlog},
	m_update_database_{true},
	m_deleted_strains_{},
	m_strain_columns_{},
	m_title_entry_{},
	m_description_view_{},
	m_strain_view_{},
	m_add_button_{},
	m_remove_button_{},
	m_toolbar_{},
	m_notebook_{}
{
	assert(m_database_);

	_add_buttons();
	_add_widgets();

	set_default_size(300,300);
	show_all();
}

GrowlogDialog::~GrowlogDialog()
{}

void
GrowlogDialog::_add_buttons()
{
	add_button(_("Apply"), Gtk::RESPONSE_APPLY);
	add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
}

void
GrowlogDialog::_add_widgets()
{
	
	Glib::RefPtr<Gtk::ListStore> strain_model = Gtk::ListStore::create(m_strain_columns_);
	
	uint64_t id = 0;
	if (m_growlog_) {
		id = m_growlog_->get_id();
		m_title_entry_.set_text(m_growlog_->get_title());
		m_description_view_.get_buffer()->set_text(m_growlog_->get_description());

		std::list<Glib::RefPtr<Strain> > strains{m_database_->get_strains_for_growlog(m_growlog_)};
		for (auto iter = strains.begin(); iter != strains.end(); ++iter) {
			Glib::RefPtr<Strain> strain = *iter;
			Gtk::TreeModel::iterator model_iter = strain_model->append();
			Gtk::TreeModel::Row row = *model_iter;
			row[m_strain_columns_.column_id] = strain->get_id();
			row[m_strain_columns_.column_breeder] = strain->get_breeder_name();
			row[m_strain_columns_.column_name] = strain->get_name();
			row[m_strain_columns_.column_changed] = false;
		}		
	}

	Gtk::Box *vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	Gtk::Grid *grid = Gtk::manage(new Gtk::Grid());

	Gtk::Label *label = Gtk::manage(new Gtk::Label(_("ID:")));
	grid->attach(*label,0,0,1,1);
	label = Gtk::manage(new Gtk::Label(std::to_string(id)));
	grid->attach(*label,1,0,1,1);

	label = Gtk::manage(new Gtk::Label(_("Title:")));
	grid->attach(*label,0,1,1,1);
	grid->attach(m_title_entry_,1,1,1,1);

	vbox->pack_start(*grid,false,false,0);

	vbox->pack_start(*manage(new Gtk::Separator(Gtk::ORIENTATION_VERTICAL)),false,false,3);
	Gtk::ScrolledWindow *scrolled = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	m_description_view_.set_wrap_mode(Gtk::WRAP_WORD);
	scrolled->add(m_description_view_);
	vbox->pack_start(*scrolled);

	m_notebook_.append_page (*vbox,_("Growlog"));

	vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));

	m_toolbar_.set_icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
	m_add_button_.set_icon_name("list-add");
	m_add_button_.signal_clicked().connect(sigc::mem_fun(*this,&GrowlogDialog::on_add_clicked));
	m_toolbar_.append(m_add_button_);
	
	m_remove_button_.set_icon_name("list-remove");
	m_toolbar_.append(m_remove_button_);
	m_remove_button_.signal_clicked().connect(sigc::mem_fun(*this,&GrowlogDialog::on_remove_clicked));
	vbox->pack_start(m_toolbar_,false,false,0);
	
	scrolled = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	m_strain_view_.set_model(strain_model);
	m_strain_view_.append_column(_("Breeder"),m_strain_columns_.column_breeder);
	m_strain_view_.set_border_width(5);
	
	Gtk::TreeView::Column *col = m_strain_view_.get_column(0);
	if (col)
		col->set_sort_column (m_strain_columns_.column_breeder);

	m_strain_view_.append_column(_("Strain"),m_strain_columns_.column_name);
	col = m_strain_view_.get_column(1);
	if (col)
		col->set_sort_column (m_strain_columns_.column_name);
	
	m_strain_view_.get_selection()->signal_changed().connect(sigc::mem_fun(*this,&GrowlogDialog::on_strain_selection_changed));
	on_strain_selection_changed();
	scrolled->add(m_strain_view_);
	vbox->pack_start(*scrolled,true,true,0);

	m_notebook_.append_page(*vbox,_("Strains"));
	
	get_content_area()->pack_start(m_notebook_,true,true,0);
}

Glib::RefPtr<Growlog>
GrowlogDialog::get_growlog()
{
	return m_growlog_;
}

Glib::RefPtr<const Growlog>
GrowlogDialog::get_growlog() const
{
	return Glib::RefPtr<const Growlog>::cast_const(m_growlog_);
}

bool
GrowlogDialog::get_update_database() const
{
	return m_update_database_;
}

void
GrowlogDialog::set_update_database(bool b)
{
	m_update_database_ = b;
}

void
GrowlogDialog::on_add_clicked()
{
	typedef Gtk::TreeModel::Children Children;
	
	StrainChooserDialog dialog{m_database_};
	int response = dialog.run();

	if (response == Gtk::RESPONSE_APPLY) {
		Glib::RefPtr<Strain> strain = dialog.get_selected_strain();
		if (!strain)
			return;
		
		// check if strain already exists
		Glib::RefPtr<Gtk::ListStore> model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_strain_view_.get_model());
		assert(model);
		
		Children children = model->children();
		for (Children::iterator iter = children.begin(); iter != children.end(); ++iter) {
			Gtk::TreeModel::Row row = *iter;
			uint64_t id = row[m_strain_columns_.column_id];
			if (id == strain->get_id())
				return;			
		}

		// check if strain has been deleted
		for (auto iter = m_deleted_strains_.begin(); iter != m_deleted_strains_.end(); ++iter) {
			if (strain->get_id() == *iter) {
				Gtk::TreeModel::iterator tree_iter = model->append();
				Gtk::TreeModel::Row row = *tree_iter;

				row[m_strain_columns_.column_id] = strain->get_id();
				row[m_strain_columns_.column_breeder] = strain->get_breeder_name();
				row[m_strain_columns_.column_name] = strain->get_name();
				row[m_strain_columns_.column_changed] = false;
				return;
			}
		}
		Gtk::TreeModel::iterator iter = model->append();
		Gtk::TreeModel::Row row = *iter;

		row[m_strain_columns_.column_id] = strain->get_id();
		row[m_strain_columns_.column_breeder] = strain->get_breeder_name();
		row[m_strain_columns_.column_name] = strain->get_name();
		row[m_strain_columns_.column_changed] = true;
	}
}

void
GrowlogDialog::on_remove_clicked()
{
	Glib::RefPtr<Gtk::ListStore> model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_strain_view_.get_model());
	assert(model);
	
	Gtk::TreeModel::iterator iter = m_strain_view_.get_selection()->get_selected();
	
	if (!iter)
		return;
	Gtk::TreeModel::Row row = *iter;
	bool changed = row[m_strain_columns_.column_changed];
	if (!changed) {
		uint64_t id = row[m_strain_columns_.column_id];
		m_deleted_strains_.push_back(id);
	}
	model->erase(iter);
}

void
GrowlogDialog::on_strain_selection_changed()
{
	Gtk::TreeModel::iterator iter = m_strain_view_.get_selection()->get_selected();
	if (iter) {
		m_remove_button_.set_sensitive(true);
	} else {
		m_remove_button_.set_sensitive(false);
	}
}

void
GrowlogDialog::on_response(int response_id)
{
	typedef Gtk::TreeModel::Children Children;
	
	if (response_id == Gtk::RESPONSE_APPLY) {
		if (m_growlog_) {
			m_growlog_->set_title(m_title_entry_.get_text());
			m_growlog_->set_description(m_description_view_.get_buffer()->get_text(false));
		} else {
			m_growlog_ = Growlog::create(m_title_entry_.get_text(),
			                             m_description_view_.get_buffer()->get_text(false));
		}
		if (m_update_database_) {
			try {
				m_database_->add_growlog(m_growlog_);
			} catch (DatabaseError &ex) {
				Gtk::MessageDialog dialog(*this,
				                          ex.what(),
				                          false,
				                          Gtk::MESSAGE_ERROR,
				                          Gtk::BUTTONS_OK,
				                          false);
				dialog.run();
				dialog.hide();
				return;
			}
			if (!m_growlog_->get_id()) {
				m_growlog_ = m_database_->get_growlog(m_growlog_->get_title());
			}
			for (auto iter = m_deleted_strains_.begin(); iter != m_deleted_strains_.end(); ++iter) {
				m_database_->remove_strain_for_growlog(m_growlog_->get_id(),*iter);
			}

			Glib::RefPtr<Gtk::ListStore> model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_strain_view_.get_model());
			Children children = model->children();
			for (Children::iterator iter = children.begin(); iter != children.end(); ++iter) {
				Gtk::TreeModel::Row row = *iter;
				bool changed = row[m_strain_columns_.column_changed];
				if (changed) {
					uint64_t strain_id = row[m_strain_columns_.column_id];
					m_database_->add_strain_for_growlog (m_growlog_->get_id(),strain_id);
				}
			}
		}
	}
	Gtk::Dialog::on_response(response_id);
}
