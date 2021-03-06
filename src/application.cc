//           application.cc
//  Di April 20 16:14:03 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// application.cc
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

#include "application.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ENABLE_NLS
# include <libintl.h>
#endif

#include <glibmm/i18n.h>
#include <glibmm.h>
#include <gtkmm/messagedialog.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "appwindow.h"
#include "databasesettingsdialog.h"
#include "error.h"

Glib::RefPtr<Application> app{};

Application::Application(int argc,char **argv):
	Gtk::Application(argc,argv, "growbook.org"),
	m_settings_{Settings::create(argc,argv)},
	m_database_{},
	m_appwindow_{nullptr}
{
	setlocale(LC_ALL,"");
	bindtextdomain(GETTEXT_PACKAGE,m_settings_->get_locale_dir ().c_str());
	textdomain(GETTEXT_PACKAGE);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset(GETTEXT_PACKAGE,"utf-8");
#endif

	m_settings_->load();
}

Application::~Application()
{
	if (m_appwindow_)
		delete m_appwindow_;
}

Glib::RefPtr<Application>
Application::create(int argc, char **argv)
{
	return Glib::RefPtr<Application>(new Application(argc,argv));
}

void
Application::on_activate()
{
	if (m_settings_->get_first_run()) {
		DatabaseSettingsDialog dialog{m_settings_->get_database_settings()};
		dialog.run();
		dialog.hide();
	}

	bool create_db = false;
	
	Glib::RefPtr<DatabaseSettings> dbsettings = m_settings_->get_database_settings();
	if (dbsettings->get_dbname_is_filename () 
	    && ! Glib::file_test(dbsettings->get_dbname(),Glib::FILE_TEST_EXISTS)) {
		create_db=true;
	}
	
	if (dbsettings->get_ask_password () && dbsettings->get_password().empty()) {
		int retry=0;
		bool connected = false;
		DatabaseSettingsDialog dialog{dbsettings};
		while ((retry < 3) && !connected) {
			dialog.present();
			int response = dialog.run();
			if (response == Gtk::RESPONSE_APPLY) {
				Glib::RefPtr<DatabaseSettings> db_settings = dialog.get_settings();
				Glib::RefPtr<DatabaseModule> db_module = db_get_module(db_settings->get_engine());
				if (!db_module) {
					fprintf(stderr, _("No database engine '%s'!\n"),db_settings->get_engine().c_str());
					exit(EXIT_FAILURE);
				}
				m_database_ = db_module->create_database(db_settings);
				try {
					m_database_->connect();
				} catch (DatabaseError ex) {
					fprintf(stderr,"%s",ex.what());
					++retry;
					continue;
				}
				connected=m_database_->is_connected();
				++retry;
				if (connected) break;
			} else {
				exit(EXIT_SUCCESS);
			}
		}
		dialog.hide();
		if (!m_database_->is_connected ()) {
			Gtk::MessageDialog dialog(_("Could not connect to database!"),
			                   		  false,
			                          Gtk::MESSAGE_ERROR,
			                          Gtk::BUTTONS_OK,
			                          false);
			dialog.run();
			dialog.hide();
			exit(EXIT_FAILURE);
		}
	}  else {
		Glib::RefPtr<DatabaseModule> module = db_get_module(m_settings_->get_database_settings()->get_engine());

		assert(module);

		m_database_ = module->create_database(m_settings_->get_database_settings());
		try {
			m_database_->connect();
		} catch (DatabaseError ex) {
			Gtk::MessageDialog dialog(_("Unable to connect to database!"),
			                          false,
			                          Gtk::MESSAGE_ERROR,
			                          Gtk::BUTTONS_OK,
			                          false);
			dialog.set_secondary_text (ex.get_message());
			dialog.run();
			dialog.hide();

			DatabaseSettingsDialog db_dialog(m_settings_->get_database_settings());
			if (db_dialog.run() == Gtk::RESPONSE_APPLY) {
				Glib::RefPtr<DatabaseSettings> db_settings;
				Glib::RefPtr<DatabaseModule> db_module = db_get_module(db_settings->get_engine());

				assert(module);

				m_settings_->set_database_settings (db_settings);
				m_database_ = db_module->create_database(db_settings);
				assert(m_database_);
				try {
					m_database_->connect();
				} catch (DatabaseError ex) {
					Gtk::MessageDialog dlg(_("Could not connect to database!"),
					                       false,
					                       Gtk::MESSAGE_ERROR,
					                       Gtk::BUTTONS_OK,
					                       false);
					dlg.set_secondary_text (ex.get_message());
					dlg.run();
					dlg.hide();
					exit(EXIT_FAILURE);
				}
				
			} else {
				exit(EXIT_SUCCESS);
			}
		}
	}
	if (create_db)
		m_database_->create_database();
		
	if (!m_appwindow_) {
		m_appwindow_ = new AppWindow(m_settings_,m_database_);
		m_appwindow_->signal_hide().connect(sigc::bind<AppWindow*>(sigc::mem_fun(*this,&Application::on_hide_window),
		                                                          m_appwindow_));
		add_window(*m_appwindow_);
	}
	m_appwindow_->present();	
}

void
Application::on_hide_window(AppWindow *window)
{
	if (m_appwindow_ == window) 
		m_appwindow_ = nullptr;
	delete window;
}

AppWindow*
Application::get_appwindow()
{
	return m_appwindow_;
}

const AppWindow*
Application::get_appwindow() const
{
	return m_appwindow_;
}

Glib::RefPtr<Settings>
Application::get_settings()
{
	return m_settings_;
}

Glib::RefPtr<const Settings>
Application::get_settings() const
{
	return Glib::RefPtr<const Settings>::cast_const(m_settings_);
}

Glib::RefPtr<Database>
Application::get_database()
{
	return m_database_;
}

Glib::RefPtr<const Database>
Application::get_database() const
{
	return Glib::RefPtr<const Database>::cast_const(m_database_);
}
