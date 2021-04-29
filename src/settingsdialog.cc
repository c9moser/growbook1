//           settingsdialog.cc
//  So April 25 21:15:06 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// settingsdialog.cc
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

#include "settingsdialog.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glibmm/i18n.h>

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/buttonbox.h>

#include "databasesettingsdialog.h"

const char SettingsDialog::TITLE[] = N_("GrowBook: Preferences");
const char SettingsDialog::OPEN_ONGOING_GROWLOGS_CHECKBUTTON[] = N_("Open ongoing Growlogs");
const char SettingsDialog::DATABASE_SETTINGS_BUTTON[] = N_("Database Settings");

SettingsDialog::SettingsDialog(const Glib::RefPtr<Settings> &settings):
	Gtk::Dialog{_(TITLE)},
	m_settings_{settings},
	m_open_ongoing_growlogs_checkbutton_{_(OPEN_ONGOING_GROWLOGS_CHECKBUTTON)},
	m_date_format_entry_{},
	m_datetime_format_entry_{},
	m_database_settings_button_{DATABASE_SETTINGS_BUTTON}
{
	_add_buttons();
	_add_widgets();
	show_all();
}

SettingsDialog::SettingsDialog(Gtk::Window &parent,
                               const Glib::RefPtr<Settings> &settings):
	Gtk::Dialog{_(TITLE),parent},
	m_settings_{settings},
	m_open_ongoing_growlogs_checkbutton_{_(OPEN_ONGOING_GROWLOGS_CHECKBUTTON)},
	m_date_format_entry_{},
	m_datetime_format_entry_{},
	m_database_settings_button_{DATABASE_SETTINGS_BUTTON}
{
	_add_buttons();
	_add_widgets();
	show_all();
}

SettingsDialog::~SettingsDialog()
{
}

void
SettingsDialog::_add_buttons()
{
	add_button(_("Apply"), Gtk::RESPONSE_APPLY);
	add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
}

void
SettingsDialog::_add_widgets()
{
	Gtk::Box *box = get_content_area ();
	Gtk::Grid *grid = Gtk::manage(new Gtk::Grid());
	m_open_ongoing_growlogs_checkbutton_.set_active(m_settings_->get_open_ongoing_growlogs());
	grid->attach(m_open_ongoing_growlogs_checkbutton_,0,0,2,1);

	Gtk::Label *label = Gtk::manage(new Gtk::Label(_("Date format:")));
	grid->attach(*label,0,1,1,1);
	m_date_format_entry_.set_text(m_settings_->get_date_format());
	grid->attach(m_date_format_entry_,1,1,1,1);

	label = Gtk::manage(new Gtk::Label(_("Datetime format:")));
	grid->attach(*label,0,2,1,1);
	m_datetime_format_entry_.set_text(m_settings_->get_datetime_format());
	grid->attach(m_datetime_format_entry_,1,2,1,1);

	Gtk::ButtonBox *buttonbox = Gtk::manage(new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL));
	m_database_settings_button_.signal_clicked().connect(sigc::mem_fun(*this,&SettingsDialog::on_database_settings_clicked));
	buttonbox->pack_start(m_database_settings_button_,false,false,0);
	grid->attach(*buttonbox,0,3,2,1);

	box->pack_start(*grid,true,true,0);
}

void
SettingsDialog::on_database_settings_clicked()
{
	DatabaseSettingsDialog dialog{*this,m_settings_->get_database_settings()};
	dialog.set_update_settings(false);
	int response = dialog.run();
	if (response == Gtk::RESPONSE_APPLY) {
		m_settings_->set_database_settings(dialog.get_settings());
	}
}

void
SettingsDialog::on_response(int response_id)
{
	if (response_id == Gtk::RESPONSE_APPLY) {
		m_settings_->set_open_ongoing_growlogs(m_open_ongoing_growlogs_checkbutton_.get_active());
		m_settings_->set_date_format (m_date_format_entry_.get_text());
		m_settings_->set_datetime_format (m_datetime_format_entry_.get_text());
		m_settings_->save();
	}
	Gtk::Dialog::on_response(response_id);
}
