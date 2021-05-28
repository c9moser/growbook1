//           export.cc
//  Mo Mai 24 13:53:48 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// export.cc
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

#include "export.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>
#include <glibmm.h>

#include <gtkmm/messagedialog.h>

#include <cassert>
#include <iostream>
#include <time.h>

#include <unistd.h>

#include "application.h"

/*******************************************************************************
 * Exporter
 ******************************************************************************/

Exporter::Exporter(const Glib::RefPtr<Database> &db,
                   const std::string &filename):
	m_database_(db),
	m_filename_(filename)
{
	assert(!m_filename_.empty());
}

Exporter::~Exporter()
{
}

Glib::RefPtr<Exporter>
Exporter::create(const Glib::RefPtr<Database> &db,
                 const std::string &filename)
{
	return Glib::RefPtr<Exporter>(new Exporter(db,filename));
}
                              

bool
Exporter::file_exists() const
{
	return Glib::file_test(m_filename_,Glib::FILE_TEST_EXISTS);
}

std::string
Exporter::get_filename() const
{
	return m_filename_;
}

void
Exporter::set_filename(const std::string &filename)
{
	m_filename_ = filename;
}

void
Exporter::export_db()
{
	this->export_db(*app->get_appwindow());
}

void
Exporter::export_db(Gtk::Window &parent)
{
	m_strain_map_.clear();
	m_breeder_map_.clear();

	if (this->file_exists())
#ifdef NATIVE_WINDOWS
		_unlink(m_filename_.c_str());
#else
		unlink(m_filename_.c_str());
#endif

	Glib::RefPtr<DatabaseModule> dbmodule = db_get_module("sqlite3");
	Glib::RefPtr<DatabaseSettings> settings = DatabaseSettings::create("sqlite3",
	                                                                   m_filename_,
	                                                                   DB_NAME_IS_FILENAME);
	Glib::RefPtr<Database> db = dbmodule->create_database(settings);
	try {
		db->connect();
		db->create_database();
	} catch (DatabaseError ex) {
		Gtk::MessageDialog dialog(parent,ex.what(),false,Gtk::MESSAGE_ERROR,Gtk::BUTTONS_OK,true);
		dialog.run();
		dialog.hide();
	}
	
	_export_strains(parent,db);
	_export_growlogs(parent,db);
}

void
Exporter::_export_strains(Gtk::Window &parent,
                          const Glib::RefPtr<Database> &dbexport)
{
	std::list<Glib::RefPtr<Breeder> > breeder_list{m_database_->get_breeders()};

	for (auto breeder_iter = breeder_list.begin(); breeder_iter != breeder_list.end(); ++breeder_iter) {
		Glib::RefPtr<Breeder> breeder0 = *breeder_iter;
		Glib::RefPtr<Breeder> breeder1 = Breeder::create(breeder0->get_name(),
		                                                 breeder0->get_homepage());
		dbexport->add_breeder(breeder1);
		breeder1 = dbexport->get_breeder(breeder0->get_name());
		assert(breeder1);

		std::list<Glib::RefPtr<Strain> > strain_list{m_database_->get_strains_for_breeder(breeder0->get_id())};
		m_breeder_map_[breeder0->get_id()] = breeder1;

		for (auto strain_iter = strain_list.begin(); strain_iter != strain_list.end(); ++strain_iter) {
			Glib::RefPtr<Strain> strain0 = *strain_iter;
			Glib::RefPtr<Strain> strain1 = Strain::create(breeder1->get_id(),
			                                              breeder1->get_name(),
			                                              strain0->get_name(),
			                                              strain0->get_info(),
			                                              strain0->get_description(),
			                                              strain0->get_homepage(),
			                                              strain0->get_seedfinder());
			dbexport->add_strain(strain1);
			strain1  = dbexport->get_strain(strain1->get_breeder_name(),strain1->get_name());
			assert(strain1);
			
			m_strain_map_[strain0->get_id()] = strain1;			
		}
	}
}

void
Exporter::_export_growlogs(Gtk::Window &parent,
                           const Glib::RefPtr<Database> &dbexport)
{
	std::list<Glib::RefPtr<Growlog> > growlog_list{m_database_->get_growlogs()};

	for (auto growlog_iter = growlog_list.begin(); growlog_iter != growlog_list.end(); ++growlog_iter) {
		Glib::RefPtr<Growlog> growlog0 = *growlog_iter;
		Glib::RefPtr<Growlog> growlog1 = Growlog::create(growlog0->get_title(),
		                                                 growlog0->get_description(),
		                                                 growlog0->get_created_on(),
		                                                 growlog0->get_flower_on(),
		                                                 growlog0->get_finished_on());
		dbexport->add_growlog(growlog1);
		growlog1 = dbexport->get_growlog(growlog1->get_title());
		assert(growlog1);
		
		m_growlog_map_[growlog0->get_id()] = growlog1;

		std::list<Glib::RefPtr<Strain> > strain_list{m_database_->get_strains_for_growlog(growlog0)};

		for (auto strain_iter = strain_list.begin(); strain_iter != strain_list.end(); ++strain_iter) {
			Glib::RefPtr<Strain> strain0 = *strain_iter;
			Glib::RefPtr<Strain> strain1 = m_strain_map_[strain0->get_id()];

			dbexport->add_strain_for_growlog(growlog1,strain1);
		}

		std::list<Glib::RefPtr<GrowlogEntry> > growlog_entry_list{m_database_->get_growlog_entries(growlog0)};

		for (auto iter = growlog_entry_list.begin(); iter != growlog_entry_list.end(); ++iter) {
			Glib::RefPtr<GrowlogEntry> entry0 = *iter;
			Glib::RefPtr<GrowlogEntry> entry1 = GrowlogEntry::create(growlog1->get_id(),
			                                                         entry0->get_text(),
			                                                         entry0->get_created_on());
			dbexport->add_growlog_entry(entry1);
		}
	}
}

/*******************************************************************************
 * ExportDialog
 ******************************************************************************/

const char ExportDialog::TITLE[] = N_("GrowBook: Export");

ExportDialog::ExportDialog(const Glib::RefPtr<Database> &db):
	Gtk::FileChooserDialog{_(TITLE),Gtk::FILE_CHOOSER_ACTION_SAVE},
	m_database_{db}
{
	_add_buttons();
	_configure();

	show_all();
}

ExportDialog::ExportDialog(Gtk::Window &parent,
                           const Glib::RefPtr<Database> &db):
	Gtk::FileChooserDialog{parent,_(TITLE),Gtk::FILE_CHOOSER_ACTION_SAVE},
	m_database_{db}
{
	_add_buttons();
	_configure();

	show_all();
}

ExportDialog::~ExportDialog()
{
}

void
ExportDialog::_add_buttons()
{
	add_button(_("Apply"), Gtk::RESPONSE_APPLY);
	add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
}

void
ExportDialog::_configure()
{
	set_create_folders(true);
	
	time_t t = time(nullptr);
	tm *datetime;
	
#ifdef NATIVE_WINDOWS
	datetime = localtime(&t);
#else
	datetime = new tm;
	if (!localtime_r(&t,datetime))
		std::cerr << "Time conversion failed!" << std::endl;
#endif
	char date[12];
	strftime(date,12,DATE_ISO_FORMAT,datetime);
	
	std::string filename = date;
	filename += ".growbook";

	set_filename(Glib::build_filename(Glib::get_user_special_dir (Glib::USER_DIRECTORY_DOCUMENTS),
	                                  filename));
}

Glib::RefPtr<Exporter>
ExportDialog::get_exporter()
{
	std::string filename = get_filename();
	
	if (filename.empty())
		return Glib::RefPtr<Exporter>();
	
	return Exporter::create(m_database_,filename);
}
