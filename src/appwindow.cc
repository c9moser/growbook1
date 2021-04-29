//           appwindow.cc
//  Di April 20 16:18:53 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// appwindow.cc
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

#include "appwindow.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glibmm/i18n.h>
#include <gtkmm/box.h>
#include <gtkmm/menu.h>
#include <gtkmm/messagedialog.h>

#include "databasesettingsdialog.h"
#include "settingsdialog.h"
#include "aboutdialog.h"

AppWindow::AppWindow(const Glib::RefPtr<Settings> &settings):
	Gtk::ApplicationWindow{},
	m_settings_{settings},
	m_menubar_{}
{
	Gtk::Box *box = Gtk::manage(new Gtk::Box());
	_add_menu();
	box->pack_start(m_menubar_,false,false,0);

	add(*box);
	set_title(_("GrowBook"));
	show_all();
}

AppWindow::~AppWindow()
{
}

void
AppWindow::_add_menu()
{
	// File menu
	Gtk::MenuItem *menuitem_file = Gtk::manage(new Gtk::MenuItem(_("File")));
	Gtk::Menu *submenu_file = Gtk::manage(new Gtk::Menu());
	menuitem_file->set_submenu(*submenu_file);

	Gtk::MenuItem *menuitem = Gtk::manage(new Gtk::MenuItem(_("Quit")));
	menuitem->signal_activate().connect(sigc::mem_fun(*this,&AppWindow::hide));
	submenu_file->append(*menuitem);

	m_menubar_.append(*menuitem_file);
	
	//Edit Menu
	Gtk::MenuItem *menuitem_edit = Gtk::manage(new Gtk::MenuItem(_("Edit")));
	Gtk::Menu *submenu_edit = Gtk::manage(new Gtk::Menu());
	menuitem_edit->set_submenu(*submenu_edit);

	menuitem = Gtk::manage(new Gtk::MenuItem(_("Database Settings")));
	menuitem->signal_activate().connect(sigc::mem_fun(*this,&AppWindow::on_database_settings));
	submenu_edit->append(*menuitem);

	menuitem = Gtk::manage(new Gtk::MenuItem(_("Preferences")));
	menuitem->signal_activate().connect(sigc::mem_fun(*this,&AppWindow::on_preferences));
	submenu_edit->append(*menuitem);

	m_menubar_.append(*menuitem_edit);

	// Help menu
	Gtk::MenuItem *menuitem_help = Gtk::manage(new Gtk::MenuItem(_("Help")));
	Gtk::Menu *submenu_help = Gtk::manage(new Gtk::Menu());
	menuitem_help->set_submenu(*submenu_help);

	menuitem = Gtk::manage(new Gtk::MenuItem(_("About")));
	menuitem->signal_activate().connect(sigc::mem_fun(*this,&AppWindow::on_about));
	submenu_help->append(*menuitem);

	m_menubar_.append(*menuitem_help);
}

void
AppWindow::on_database_settings()
{
	DatabaseSettingsDialog dialog(*this,m_settings_->get_database_settings());
	
	int response = dialog.run();
	dialog.hide();
	if (response == Gtk::RESPONSE_APPLY) {
		Gtk::MessageDialog msg_dialog{*this,
		                              _("GrowBook needs to be restarted for changes to take effect"),
		                              false,
		                              Gtk::MESSAGE_INFO,
		                              Gtk::BUTTONS_OK,
		                              true};
		msg_dialog.run();
		msg_dialog.hide();
	}
}

void
AppWindow::on_preferences()
{
	SettingsDialog dialog{*this,m_settings_};
	int response = dialog.run();
	dialog.hide();
	if (response == Gtk::RESPONSE_APPLY) {
		Gtk::MessageDialog msg_dialog{*this,
		                              _("GrowBook needs to be restarted for Database-settings changes to take effect!"),
		                              false,
		                              Gtk::MESSAGE_INFO,
		                              Gtk::BUTTONS_OK,
		                              true};
		msg_dialog.run();
		msg_dialog.hide();		
	}
	
}

void
AppWindow::on_about()
{
	AboutDialog dialog{};
	dialog.run();
	dialog.hide();
}