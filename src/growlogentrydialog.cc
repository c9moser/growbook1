//           growlogentrydialog.cc
//  Do Mai 13 21:27:48 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// growlogentrydialog.cc
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

#include "growlogentrydialog.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/separator.h>
#include <gtkmm/messagedialog.h>

#include <cassert>

#include "application.h"
#include "error.h"

const char GrowlogEntryDialog::TITLE[] = N_("GrowBook: Growlog Entry");

GrowlogEntryDialog::GrowlogEntryDialog(const Glib::RefPtr<Database> &db,
                                       const Glib::RefPtr<GrowlogEntry> &entry):
	Gtk::Dialog{_(TITLE)},
	m_database_{db},
	m_entry_{entry}
{
	assert(m_database_);
	assert(m_entry_);
	assert(m_entry_->get_growlog_id());

	_add_buttons();
	_add_widgets();

	set_default_size(400,400);
	show_all();
}

GrowlogEntryDialog::GrowlogEntryDialog(const Glib::RefPtr<Database> &db,
                                       const Glib::RefPtr<Growlog> &growlog):
	Gtk::Dialog{_(TITLE)},
	m_database_{db},
	m_entry_{GrowlogEntry::create(growlog->get_id(),"",0)}
{
	assert(m_database_);
	assert(m_entry_);
	assert(m_entry_->get_growlog_id());

	_add_buttons();
	_add_widgets();

	set_default_size(400,400);
	show_all();
}

GrowlogEntryDialog::GrowlogEntryDialog(Gtk::Window &parent,
                                       const Glib::RefPtr<Database> &db,
                                       const Glib::RefPtr<GrowlogEntry> &entry):
	Gtk::Dialog{_(TITLE),parent},
	m_database_{db},
	m_entry_{entry}
{
	assert(m_database_);
	assert(m_entry_);
	assert(m_entry_->get_growlog_id());

	_add_buttons();
	_add_widgets();

	set_default_size(400,400);
	show_all();
}

GrowlogEntryDialog::GrowlogEntryDialog(Gtk::Window &parent,
                                       const Glib::RefPtr<Database> &db,
                                       const Glib::RefPtr<Growlog> &growlog):
	Gtk::Dialog{_(TITLE),parent},
	m_database_{db},
	m_entry_{GrowlogEntry::create(growlog->get_id(),"",0)}
{
	assert(m_database_);
	assert(m_entry_);
	assert(m_entry_->get_growlog_id());

	_add_buttons();
	_add_widgets();

	set_default_size(400,400);
	show_all();
}

GrowlogEntryDialog::~GrowlogEntryDialog()
{}

void
GrowlogEntryDialog::_add_buttons()
{
	add_button(_("Apply"),Gtk::RESPONSE_APPLY);
	add_button(_("Cancel"),Gtk::RESPONSE_CANCEL);
}

void
GrowlogEntryDialog::_add_widgets()
{
	Gtk::Box *box = get_content_area();

	Gtk::Grid *grid = Gtk::manage(new Gtk::Grid);
	
	Gtk::Label *label = Gtk::manage(new Gtk::Label(_("Growlog:")));
	grid->attach(*label,0,0,1,1);
	Glib::RefPtr<Growlog> growlog = m_database_->get_growlog(m_entry_->get_growlog_id());
	label = Gtk::manage(new Gtk::Label(growlog->get_title()));
	grid->attach(*label,1,0,1,1);

	label = Gtk::manage(new Gtk::Label(_("Created on:")));
	grid->attach(*label,0,1,1,1);
	label = Gtk::manage(new Gtk::Label(m_entry_->get_created_on_format(app->get_settings()->get_datetime_format())));
	grid->attach(*label,1,1,1,1);

	box->pack_start(*grid,false,false,3);

	box->pack_start(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_VERTICAL)), false,false,3);

	m_textview_.get_buffer()->set_text(m_entry_->get_text());
	m_textview_.set_wrap_mode(Gtk::WRAP_NONE);
	m_textview_.set_editable(true);
	Gtk::ScrolledWindow *scrolled = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	scrolled->add(m_textview_);

	box->pack_start(*scrolled,true,true,0);
}

void
GrowlogEntryDialog::on_response(int response_id)
{
	if (response_id == Gtk::RESPONSE_APPLY) {
		m_entry_->set_text(m_textview_.get_buffer()->get_text(false));
		try {
			m_database_->add_growlog_entry(m_entry_);
		} catch (DatabaseError &ex) {
			Gtk::MessageDialog dialog{*this,ex.what(),false,Gtk::MESSAGE_ERROR,Gtk::BUTTONS_OK,true};
			dialog.run();
			dialog.hide();
		}
	}
	Gtk::Dialog::on_response(response_id);
}
