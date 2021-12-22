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
#include <gtkmm/paned.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/separatormenuitem.h>

#include <cassert>

#include "databasesettingsdialog.h"
#include "settingsdialog.h"
#include "aboutdialog.h"
#include "growlogview.h"
#include "application.h"
#include "export.h"
#include "import.h"

#include <iostream>

AppWindow::AppWindow(const Glib::RefPtr<Settings> &settings,
                     const Glib::RefPtr<Database> &database):
	Gtk::ApplicationWindow{},
	m_settings_{settings},
	m_database_{database},
	m_menubar_{},
	m_growlog_selector_{database},
	m_strain_selector_{database},
	m_selector_notebook_{},
	m_browser_notebook_{}
{
	assert(settings);
	assert(database);

	set_default_size(800,600);
	Gtk::Box *box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	_add_menu();
	box->pack_start(m_menubar_,false,false,0);

	Gtk::Paned *paned = Gtk::manage(new Gtk::Paned(Gtk::ORIENTATION_HORIZONTAL));
	m_selector_notebook_.append_page(m_growlog_selector_,_("Growlogs"));
	m_selector_notebook_.append_page(m_strain_selector_,_("Strains"));
	m_selector_notebook_.set_current_page(1);
	paned->add1(m_selector_notebook_);

	m_browser_notebook_.set_scrollable(true);
	// open ongoing growlogs
	if (settings->get_open_ongoing_growlogs()) {
		std::list<Glib::RefPtr<Growlog> > growlogs{m_database_->get_ongoing_growlogs()};
		for (auto iter = growlogs.begin(); iter != growlogs.end(); ++iter) {
			GrowlogView *glv = Gtk::manage(new GrowlogView(m_database_,*iter));
			if (add_browser_page(*glv) == -1)
				delete glv;
		}
	}
	paned->add2(m_browser_notebook_);
	
	box->pack_start(*paned,true,true,0);

	set_icon_name("emoji-nature-symbolic");
	
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

	Gtk::MenuItem *menuitem = Gtk::manage(new Gtk::MenuItem(_("Export")));
	menuitem->signal_activate().connect(sigc::mem_fun(*this,&AppWindow::on_export));
	submenu_file->append(*menuitem);

	menuitem = Gtk::manage(new Gtk::MenuItem(_("Import")));
	menuitem->signal_activate().connect(sigc::mem_fun(*this,&AppWindow::on_import));
	submenu_file->append(*menuitem);

	submenu_file->append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	
	menuitem = Gtk::manage(new Gtk::MenuItem(_("Quit")));
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

Gtk::Notebook*
AppWindow::get_selector_notebook()
{
	return &m_selector_notebook_;
}

const Gtk::Notebook*
AppWindow::get_selector_notebook() const
{
	return &m_selector_notebook_;
}

Gtk::Notebook*
AppWindow::get_browser_notebook()
{
	return &m_browser_notebook_;
}

const Gtk::Notebook*
AppWindow::get_browser_notebook() const
{
	return &m_browser_notebook_;
}

GrowlogSelector*
AppWindow::get_growlog_selector()
{
	return &m_growlog_selector_;
}

const GrowlogSelector*
AppWindow::get_growlog_selector() const
{
	return &m_growlog_selector_;
}

StrainSelector*
AppWindow::get_strain_selector()
{
	return &m_strain_selector_;
}

const StrainSelector*
AppWindow::get_strain_selector() const
{
	return &m_strain_selector_;
}

int
AppWindow::add_browser_page(Gtk::Widget &page,const Glib::ustring &title)
{
	page.show();
	
	Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox());
	Gtk::Label *label = Gtk::manage(new Gtk::Label(title));
	Gtk::Image *image=Gtk::manage(new Gtk::Image("window-close",Gtk::ICON_SIZE_MENU));
	Gtk::Button *button = Gtk::manage(new Gtk::Button());
	button->set_relief (Gtk::RELIEF_NONE);
	button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,&AppWindow::on_close_page),&page));
	hbox->pack_start(*label,false,false,0);
	hbox->pack_start(*button,false,false,0);

	
	int n =  m_browser_notebook_.append_page(page,*hbox);
	m_browser_notebook_.set_current_page (n);
	return n;
}

int
AppWindow::add_browser_page(BrowserPage &page)
{
	int n_pages = m_browser_notebook_.get_n_pages();
	for (int i = 0; i < n_pages; ++i) {
		BrowserPage *p = dynamic_cast<BrowserPage*>(m_browser_notebook_.get_nth_page (i));

		if (p && page.get_type() == p->get_type() && page.get_id() == p->get_id()) {
			p->refresh();
			m_browser_notebook_.set_current_page(i);
			m_browser_notebook_.show_all();
			return -1;
		}		
	}
	
	Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox());
	Gtk::Label *label = Gtk::manage(new Gtk::Label(page.get_title()));
	Gtk::Image *image=Gtk::manage(new Gtk::Image("window-close",Gtk::ICON_SIZE_MENU));
	Gtk::Button *button = Gtk::manage(new Gtk::Button());
	button->set_relief (Gtk::RELIEF_NONE);
	button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,&AppWindow::on_close_page),&page));
	button->add(*image);
	hbox->pack_start(*label,false,false,0);
	hbox->pack_start(*button,false,false,0);
	hbox->show_all();
	page.signal_title_changed().connect(sigc::bind(sigc::mem_fun(*this,&AppWindow::on_browser_title_changed),
	                                                label,&page));

	int n =  m_browser_notebook_.append_page(page,*hbox);
	m_browser_notebook_.show_all();
	m_browser_notebook_.set_current_page (n);
	return n;
}

void
AppWindow::on_close_page(Gtk::Widget *widget)
{
	m_browser_notebook_.remove_page(*widget);
}

void
AppWindow::on_browser_title_changed(Gtk::Label *label,BrowserPage *page)
{
	label->set_text(page->get_title());
}

void
AppWindow::on_export()
{
	ExportDialog dialog(*this,m_database_);
	int response = dialog.run();
	dialog.hide();
	if (response == Gtk::RESPONSE_APPLY) {
		Glib::RefPtr<Exporter> exporter = dialog.get_exporter();
		if (!exporter)
			return;
		exporter->export_db(*this);
	}
}

void
AppWindow::on_import()
{
	ImportDialog dialog(*this,m_database_);
	int response = dialog.run();
	dialog.hide();
	if (response == Gtk::RESPONSE_APPLY) {
		Glib::RefPtr<Importer> importer = dialog.get_importer();
		if (!importer)
			return;
		importer->import_db(*this);
	}
	m_growlog_selector_.refresh();
	m_strain_selector_.refresh();
}

