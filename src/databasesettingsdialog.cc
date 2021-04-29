//           databasesettingsdialog.cc
//  So April 25 12:37:48 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// databasesettingsdialog.cc
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

#include "databasesettingsdialog.h"

#include <cassert>
#include <glibmm/i18n.h>
#include <gtkmm/grid.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>

#include <cstdio>

#include "database.h"
#include "application.h"
#include "error.h"

const char DatabaseSettingsDialog::TITLE[] = N_("GrowBook: Database Settings");
const char DatabaseSettingsDialog::HOST_LABEL[] = N_("Host:");
const char DatabaseSettingsDialog::PORT_LABEL[] = N_("Port:");
const char DatabaseSettingsDialog::USER_LABEL[] = N_("User:");
const char DatabaseSettingsDialog::PASSWORD_LABEL[] = N_("Password:");
const char DatabaseSettingsDialog::PASSWORD_CHECKBUTTON[] = N_("Ask for Password?");
const char DatabaseSettingsDialog::CREATE_BUTTON[] = N_("Create Database");
const char DatabaseSettingsDialog::TEST_BUTTON[] = N_("Test Connection");

DatabaseSettingsDialog::DatabaseSettingsDialog(const Glib::RefPtr<DatabaseSettings> &settings):
	Gtk::Dialog{_(TITLE)},
	m_settings_{settings},
	m_update_settings_{true},
	m_flags_{DB_FLAGS_NONE},
	m_engine_combobox_{},
	m_dbname_entry_{},
	m_dbname_button_{},
	m_host_label_{_(HOST_LABEL)},
	m_host_entry_{},
	m_port_label_{_(PORT_LABEL)},
	m_port_entry_{},
	m_user_label_{_(USER_LABEL)},
	m_user_entry_{},
	m_password_label_{_(PASSWORD_LABEL)},
	m_password_entry_{},
	m_password_checkbutton_{_(PASSWORD_CHECKBUTTON)},
	m_test_button_{_(TEST_BUTTON)},
	m_create_button_{_(CREATE_BUTTON)}
{
	assert(m_settings_);

	m_flags_ = m_settings_->get_flags();
	
	_add_buttons();
	_add_widgets();
	
	show_all();
}

DatabaseSettingsDialog::DatabaseSettingsDialog(Gtk::Window &parent,
                                               const Glib::RefPtr<DatabaseSettings> &settings):
	Gtk::Dialog{_(TITLE), parent},
	m_settings_{settings},
	m_update_settings_{true},
	m_flags_{DB_FLAGS_NONE},
	m_engine_combobox_{},
	m_dbname_entry_{},
	m_dbname_button_{},
	m_host_label_{_(HOST_LABEL)},
	m_host_entry_{},
	m_port_label_{_(PORT_LABEL)},
	m_port_entry_{},
	m_user_label_{_(USER_LABEL)},
	m_user_entry_{},
	m_password_label_{_(PASSWORD_LABEL)},
	m_password_entry_{},
	m_password_checkbutton_{_(PASSWORD_CHECKBUTTON)},
	m_test_button_{_(TEST_BUTTON)},
	m_create_button_{_(CREATE_BUTTON)}
{
	assert(m_settings_);
	
	m_flags_ = m_settings_->get_flags();
	
	_add_buttons();
	_add_widgets();
	
	show_all();
}

DatabaseSettingsDialog::~DatabaseSettingsDialog()
{
}

void
DatabaseSettingsDialog::_add_buttons()
{
	add_button(_("Apply"),Gtk::RESPONSE_APPLY);
	add_button(_("Cancel"),Gtk::RESPONSE_CANCEL);
}

void
DatabaseSettingsDialog::_add_widgets()
{
	Gtk::Box *box = get_content_area();
	
	Gtk::Grid *grid = Gtk::manage(new Gtk::Grid());

	// Database Engine
	Gtk::Label *label = Gtk::manage(new Gtk::Label(_("Database Engine:")));
	grid->attach(*label,0,0,1,1);
	std::list<Glib::RefPtr<DatabaseModule> > db_modules{db_get_modules()};
	for (auto iter = db_modules.begin(); iter != db_modules.end(); ++ iter) {
		m_engine_combobox_.append((*iter)->get_engine());
	}
	m_engine_combobox_.set_active_text(m_settings_->get_engine());
	m_engine_combobox_.signal_changed().connect(sigc::mem_fun(*this, &DatabaseSettingsDialog::on_engine_changed));
	grid->attach(m_engine_combobox_,1,0,2,1);

	// Database Name
	label = Gtk::manage(new Gtk::Label(_("Database Name:")));
	grid->attach(*label,0,1,1,1);
	m_dbname_entry_.set_text(m_settings_->get_dbname());
	grid->attach(m_dbname_entry_,1,1,1,1);
	m_dbname_button_.set_image_from_icon_name ("folder",Gtk::ICON_SIZE_MENU);
	m_dbname_button_.signal_clicked().connect(sigc::mem_fun(*this,&DatabaseSettingsDialog::on_dbname_clicked));
	if (m_flags_ & DB_NAME_IS_FILENAME) {
		m_dbname_button_.set_sensitive(true);
	} else {
		m_dbname_button_.set_sensitive(false);
	}
	grid->attach(m_dbname_button_,2,1,1,1);

	// Host
	if (m_flags_ & DB_HAS_HOST) {
		m_host_label_.set_sensitive(true);
		m_host_entry_.set_text(m_settings_->get_host());
		m_host_entry_.set_sensitive(true);
	} else {
		m_host_label_.set_sensitive(false);
		m_host_entry_.set_text("");
		m_host_entry_.set_sensitive(false);
	}
	
	grid->attach(m_host_label_,0,2,1,1);
	grid->attach(m_host_entry_,1,2,2,1);

	// Port
	if (m_flags_ & DB_HAS_PORT) {
		m_port_label_.set_sensitive(true);
		m_port_entry_.set_text(std::to_string(m_settings_->get_port()));
		m_port_entry_.set_sensitive(true);
	} else {
		m_port_label_.set_sensitive(false);
		m_port_entry_.set_text("0");
		m_port_entry_.set_sensitive(false);
	}
	grid->attach(m_port_label_,0,3,1,1);
	grid->attach(m_port_entry_,1,3,2,1);

	// Database User
	if (m_flags_ & DB_HAS_USER) {
		m_user_label_.set_sensitive(true);
		m_user_entry_.set_text(m_settings_->get_user());
		m_user_entry_.set_sensitive(true);
	} else {
		m_user_label_.set_sensitive(false);
		m_user_entry_.set_text("");
		m_user_entry_.set_sensitive(false);
	}
	grid->attach(m_user_label_,0,4,1,1);
	grid->attach(m_user_entry_,1,4,2,1);

	// Database Password
	if (m_flags_ & DB_HAS_PASSWORD) {
		m_password_label_.set_sensitive(true);
		m_password_entry_.set_text(m_settings_->get_password());
		m_password_entry_.set_sensitive(true);
		m_password_checkbutton_.set_active(m_settings_->get_ask_password());
		m_password_checkbutton_.set_sensitive(true);
	} else {
		m_password_label_.set_sensitive(false);
		m_password_entry_.set_text("");
		m_password_entry_.set_sensitive(false);
		m_password_checkbutton_.set_active(false);
		m_password_checkbutton_.set_sensitive(false);
	}
	m_password_entry_.set_visibility(false);
	grid->attach(m_password_label_,0,5,1,1);
	grid->attach(m_password_entry_,1,5,2,1);
	grid->attach(m_password_checkbutton_,0,6,3,1);

	Gtk::ButtonBox *buttonbox = Gtk::manage(new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL));
	m_test_button_.signal_clicked().connect(sigc::mem_fun(*this,&DatabaseSettingsDialog::on_test_connection_clicked));
	buttonbox->pack_start(m_test_button_,false,false,0);
	m_create_button_.signal_clicked().connect(sigc::mem_fun(*this,&DatabaseSettingsDialog::on_create_database_clicked));
	buttonbox->pack_start(m_create_button_,false,false,0);
	grid->attach(*buttonbox,0,7,3,1);
	
	box->pack_start(*grid,true,true,0);
}

bool
DatabaseSettingsDialog::get_update_settings() const
{
	return m_update_settings_;
}

void
DatabaseSettingsDialog::set_update_settings(bool b)
{
	m_update_settings_ = b;
}

Glib::RefPtr<DatabaseSettings>
DatabaseSettingsDialog::get_settings()
{
	return m_settings_;
}

Glib::RefPtr<const DatabaseSettings>
DatabaseSettingsDialog::get_settings() const
{
	return Glib::RefPtr<const DatabaseSettings>::cast_const(m_settings_);
}

void
DatabaseSettingsDialog::on_response(int response_id)
{
	if (response_id == Gtk::RESPONSE_APPLY) {
		Glib::ustring engine = m_engine_combobox_.get_active_text();
		Glib::RefPtr<DatabaseModule> module = db_get_module(engine);
		
		if (!module) {
			// should never be reached
			char msg[1024];
			snprintf(msg,1024,_("Database-engine '%s' does not exist!"));
			Gtk::MessageDialog dialog{*this,msg,false,Gtk::MESSAGE_ERROR,Gtk::BUTTONS_OK,true};
			dialog.set_secondary_text (_("Please select another Database-engine."));
			dialog.run();
			dialog.hide();
			return;
		}
		
		
		m_settings_ = DatabaseSettings::create(engine,
		                                       m_dbname_entry_.get_text(),
		                                       m_host_entry_.get_text().c_str(),
		                                       static_cast<uint16_t>(std::stoi(m_port_entry_.get_text().c_str())),
		                                       m_user_entry_.get_text().c_str(),
		                                       m_password_entry_.get_text().c_str(),
		                                       m_password_checkbutton_.get_active(),
		                                       m_flags_);
		if (m_update_settings_) {
			Glib::RefPtr<Settings> settings = app->get_settings();
			settings->set_database_settings(m_settings_);
			settings->save();			
		}
	}
	
	Gtk::Dialog::on_response(response_id);
}

void
DatabaseSettingsDialog::on_engine_changed()
{
	Glib::ustring engine = m_engine_combobox_.get_active_text();
	Glib::RefPtr<DatabaseModule> module=db_get_module(engine);
	
	if (!module) {
		// should never be reached!
		m_engine_combobox_.set_active_text("sqlite3");
		module=db_get_module("sqlite3");
	}

	DatabaseSettingsFlags m_flags_ = module->get_defaults()->get_flags();

	if (m_settings_->get_engine() == engine) {
		m_dbname_entry_.set_text(m_settings_->get_dbname());
		m_host_entry_.set_text(m_settings_->get_host());
		m_port_entry_.set_text(std::to_string(m_settings_->get_port()));
		m_user_entry_.set_text(m_settings_->get_user());
		m_password_entry_.set_text(m_settings_->get_password());
		m_password_checkbutton_.set_active(m_settings_->get_ask_password());
	} else {
		Glib::RefPtr<DatabaseSettings> settings = module->get_defaults();
		m_dbname_entry_.set_text(settings->get_dbname());
		m_host_entry_.set_text(settings->get_host());
		m_port_entry_.set_text(std::to_string(settings->get_port()));
		m_user_entry_.set_text(settings->get_user());
		m_password_entry_.set_text(settings->get_password());
		m_password_checkbutton_.set_active(settings->get_ask_password());
	}

	if (m_flags_ & DB_NAME_IS_FILENAME) {
		m_dbname_button_.set_sensitive(true);
	} else {
		m_dbname_button_.set_sensitive(false);
	}

	if (m_flags_ & DB_HAS_HOST) {
		m_host_label_.set_sensitive(true);
		m_host_entry_.set_sensitive(true);
	} else {
		m_host_label_.set_sensitive(false);
		m_host_entry_.set_text("");
		m_host_entry_.set_sensitive(false);
	}

	if (m_flags_ & DB_HAS_PORT) {
		m_port_label_.set_sensitive(true);
		m_port_entry_.set_sensitive(true);
	} else {
		m_port_label_.set_sensitive (false);
		m_port_entry_.set_text("0");
		m_port_entry_.set_sensitive (false);
	}

	if (m_flags_ & DB_HAS_USER) {
		m_user_label_.set_sensitive(true);
		m_user_entry_.set_sensitive(true);
	} else {
		m_user_label_.set_sensitive(false);
		m_user_entry_.set_text("");
		m_user_entry_.set_sensitive(false);
	}

	if (m_flags_ & DB_HAS_PASSWORD) {
		m_password_label_.set_sensitive(true);
		m_password_entry_.set_sensitive(true);
		m_password_checkbutton_.set_sensitive(true);
	} else {
		m_password_label_.set_sensitive(false);
		m_password_entry_.set_text("");
		m_password_entry_.set_sensitive(false);
		m_password_checkbutton_.set_active(false);
		m_password_checkbutton_.set_sensitive(false);
	}
	show_all();
}

void
DatabaseSettingsDialog::on_dbname_clicked()
{
	Gtk::FileChooserDialog dialog{*this,_("GrowBook: Database"),Gtk::FILE_CHOOSER_ACTION_SAVE};
	dialog.add_button(_("Apply"),Gtk::RESPONSE_APPLY);
	dialog.add_button(_("Cancel"),Gtk::RESPONSE_CANCEL);
	dialog.set_local_only(true);
	dialog.set_select_multiple(false);
	dialog.set_create_folders(true);
	dialog.set_filename(m_dbname_entry_.get_text().c_str());
	int result = dialog.run();
	if (result == Gtk::RESPONSE_APPLY) {
		m_dbname_entry_.set_text(dialog.get_filename());
		m_dbname_entry_.show();
	}
	dialog.hide();
}

void
DatabaseSettingsDialog::on_test_connection_clicked()
{
	Glib::RefPtr<DatabaseModule> module = db_get_module (m_engine_combobox_.get_active_text());

	assert(module);
	
	Glib::RefPtr<DatabaseSettings> settings = DatabaseSettings::create(m_engine_combobox_.get_active_text(),
	                                                                   m_dbname_entry_.get_text().c_str(),
	                                                                   m_host_entry_.get_text().c_str(),
	                                                                   static_cast<uint16_t>(std::stoi(m_port_entry_.get_text().c_str())),
	                                                                   m_user_entry_.get_text().c_str(),
	                                                                   m_password_entry_.get_text().c_str(),
	                                                                   m_password_checkbutton_.get_active(),
	                                                                   m_flags_);
	Glib::RefPtr<Database> db = module->create_database(settings);

	assert(db);
	
	if (db->test_connection()) {
		Gtk::MessageDialog dialog(*this,
		                          _("Successfully connected to database!"),
		                          false,
		                          Gtk::MESSAGE_INFO,
		                          Gtk::BUTTONS_OK,
		                          false);
		int result = dialog.run();
		return;
	}
	Gtk::MessageDialog dialog{*this,
	                          _("Connecting to database failed!"),
	                          false,
	                          Gtk::MESSAGE_ERROR,
	                          Gtk::BUTTONS_OK,
	                          false};
	dialog.run();
}

void
DatabaseSettingsDialog::on_create_database_clicked()
{
	Glib::RefPtr<DatabaseModule> module = db_get_module(m_engine_combobox_.get_active_text());

	assert(module);
	
	Glib::RefPtr<DatabaseSettings> settings = DatabaseSettings::create(m_engine_combobox_.get_active_text(),
	                                                                   m_dbname_entry_.get_text().c_str(),
	                                                                   m_host_entry_.get_text().c_str(),
	                                                                   static_cast<uint16_t>(std::stoi(m_port_entry_.get_text().c_str())),
	                                                                   m_user_entry_.get_text().c_str(),
	                                                                   m_password_entry_.get_text().c_str(),
	                                                                   m_password_checkbutton_.get_active(),
	                                                                   m_flags_);

	Glib::RefPtr<Database> db = module->create_database(settings);

	assert(db);

	try {
		db->connect();
		db->create_database();
		db->close();
	} catch (DatabaseError ex) {
		Glib::ustring msg = _("Unable to create database!");
		
		Gtk::MessageDialog dialog{*this,msg,false,Gtk::MESSAGE_ERROR,Gtk::BUTTONS_OK,true};
		dialog.set_secondary_text (ex.get_message());
		dialog.run();
		dialog.hide();
		return;
	}
	Glib::ustring msg = _("Database successfully created!");
	Gtk::MessageDialog dialog{*this,msg,false,Gtk::MESSAGE_INFO,Gtk::BUTTONS_OK,true};
	dialog.run();
	dialog.hide();
}
