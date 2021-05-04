//           straindialog.cc
//  Mo Mai 03 18:14:41 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// straindialog.cc
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

#include "straindialog.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>

#include <gtkmm/label.h>
#include <gtkmm/grid.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/messagedialog.h>

#include <cassert>

#include "error.h"

const char StrainDialog::TITLE[] = N_("GrowBook: Strain");

StrainDialog::StrainDialog(const Glib::RefPtr<Database> &db,
                           const Glib::RefPtr<Strain> &strain):
	Gtk::Dialog{TITLE},
	m_update_database_{true},
	m_database_{db},
	m_strain_{strain}
{
	assert(m_database_);
	assert(m_strain_);

	_add_buttons();
	_add_widgets();

	set_default_size(400,400);
	show_all();
}

StrainDialog::StrainDialog(Gtk::Window &parent,
                           const Glib::RefPtr<Database> &db,
                           const Glib::RefPtr<Strain> &strain):
	Gtk::Dialog{TITLE},
	m_update_database_{true},
	m_database_{db},
	m_strain_{strain}
{
	assert(m_database_);
	assert(m_strain_);

	_add_buttons();
	_add_widgets();

	set_default_size(400,400);
	show_all();
}

StrainDialog::~StrainDialog()
{}

void
StrainDialog::_add_buttons()
{
	add_button(_("Apply"),Gtk::RESPONSE_APPLY);
	add_button(_("Cancel"),Gtk::RESPONSE_CANCEL);
}

void
StrainDialog::_add_widgets()
{
	Gtk::Box *box = get_content_area();
	Gtk::Grid *grid = Gtk::manage(new Gtk::Grid());
	
	Gtk::Label *label=Gtk::manage(new Gtk::Label(_("Breeder:")));
	grid->attach(*label,0,0,1,1);
	label = Gtk::manage(new Gtk::Label(m_strain_->get_breeder_name()));
	grid->attach(*label,1,0,1,1);

	label = Gtk::manage(new Gtk::Label(_("Name:")));
	grid->attach(*label,0,1,1,1);
	m_name_entry_.set_text(m_strain_->get_name());
	grid->attach(m_name_entry_,1,1,1,1);

	label = Gtk::manage(new Gtk::Label(_("Homepage:")));
	grid->attach(*label,0,2,1,1);
	m_homepage_entry_.set_text(m_strain_->get_homepage());
	grid->attach(m_homepage_entry_,1,2,1,1);

	label = Gtk::manage(new Gtk::Label(_("Seedfinder.eu:")));
	grid->attach(*label,0,3,1,1);
	m_seedfinder_entry_.set_text(m_strain_->get_seedfinder());
	grid->attach(m_seedfinder_entry_,1,3,1,1);

	m_notebook_.append_page(*grid,_("Strain"));

	Gtk::ScrolledWindow *scrolled = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	m_info_textview_.set_wrap_mode(Gtk::WRAP_WORD);
	m_info_textview_.get_buffer()->set_text(m_strain_->get_info());
	scrolled->add(m_info_textview_);
	m_notebook_.append_page(*scrolled,_("Info"));

	scrolled = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	m_description_textview_.set_wrap_mode(Gtk::WRAP_WORD);
	m_description_textview_.get_buffer()->set_text(m_strain_->get_description());
	scrolled->add(m_description_textview_);
	m_notebook_.append_page(*scrolled,_("Description"));

	box->pack_start(m_notebook_,true,true,0);
}

bool
StrainDialog::get_update_database() const
{
	return m_update_database_;
}

void
StrainDialog::set_update_database(bool b)
{
	m_update_database_ = b;
}

Glib::RefPtr<Strain>
StrainDialog::get_strain()
{
	return m_strain_;
}

Glib::RefPtr<const Strain>
StrainDialog::get_strain() const
{
	return Glib::RefPtr<const Strain>::cast_const(m_strain_);
}

void
StrainDialog::on_response(int response_id)
{
	if (response_id == Gtk::RESPONSE_APPLY) {
		m_strain_->set_name(m_name_entry_.get_text());
		m_strain_->set_info(m_info_textview_.get_buffer()->get_text(false));
		m_strain_->set_description(m_description_textview_.get_buffer()->get_text(false));
		m_strain_->set_homepage(m_homepage_entry_.get_text().c_str());
		m_strain_->set_seedfinder(m_seedfinder_entry_.get_text().c_str());

		if (m_update_database_) {
			try {
				m_database_->add_strain(m_strain_);
			} catch (DatabaseError ex) {
				Gtk::MessageDialog dialog{*this,ex.what(),false,Gtk::MESSAGE_ERROR,Gtk::BUTTONS_OK,true};
				dialog.run();
				dialog.hide();
			}
		}
	}
	Gtk::Dialog::on_response(response_id);
}
