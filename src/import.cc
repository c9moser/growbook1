//           import.cc
//  Sa Mai 29 11:37:55 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// import.cc
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

#include "import.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm.h>
#include <glibmm/i18n.h>

#include <cassert>

#include "application.h"

/*******************************************************************************
 * Importer
 ******************************************************************************/
Importer::Importer(const Glib::RefPtr<Database> &db,
                   const std::string &filename):
	m_database_{db},
	m_filename_{filename}
{
}

Importer::~Importer()
{
}

Glib::RefPtr<Importer>
Importer::create(const Glib::RefPtr<Database> &db,
                 const std::string &filename)
{
	return Glib::RefPtr<Importer>(new Importer(db,filename));
}

void
Importer::import_db()
{
	this->import_db(*app->get_appwindow());
}

void
Importer::import_db(Gtk::Window &parent)
{
}

/*******************************************************************************
 * ImportDialog
 ******************************************************************************/

const char ImportDialog::TITLE[] = N_("GrowBook: Import");

ImportDialog::ImportDialog(const Glib::RefPtr<Database> &db):
	Gtk::FileChooserDialog{_(TITLE),Gtk::FILE_CHOOSER_ACTION_OPEN},
	m_database_{db}
{
	assert(m_database_);

	_add_buttons();
	_configure();

	show_all();
}

ImportDialog::ImportDialog(Gtk::Window &parent,
                           const Glib::RefPtr<Database> &db):
	Gtk::FileChooserDialog{parent,_(TITLE),Gtk::FILE_CHOOSER_ACTION_OPEN},
	m_database_{db}
{
	assert(m_database_);

	_add_buttons();
	_configure();

	show_all();
}

void
ImportDialog::_add_buttons()
{
	add_button(_("Apply"), Gtk::RESPONSE_APPLY);
	add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
}

void
ImportDialog::_configure()
{
	Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
	filter->set_name(_("All GrowBook files"));
	filter->add_pattern("*.db");
	filter->add_pattern("*.growbook");
	add_filter(filter);
	set_filter(filter);

	filter = Gtk::FileFilter::create();
	filter->set_name(_("Database files"));
	filter->add_pattern("*.db");
	add_filter(filter);

	filter = Gtk::FileFilter::create();
	filter->set_name(_("Growbook files"));
	filter->add_pattern("*.growbook");
	add_filter(filter);

	filter = Gtk::FileFilter::create();
	filter->set_name(_("All files"));
	filter->add_pattern("*");
	add_filter(filter);
		
	set_current_folder(Glib::get_user_special_dir(Glib::USER_DIRECTORY_DOCUMENTS));	
}

Glib::RefPtr<Importer>
ImportDialog::get_importer()
{
	std::string filename = get_filename();
	if (filename.empty())
		return Glib::RefPtr<Importer>();
	
	return Importer::create(m_database_,filename);
}