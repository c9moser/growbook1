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

#include <gtkmm/messagedialog.h>

#include <cassert>

#include "application.h"
#include "error.h"
#include "growlogdialog.h"

/*******************************************************************************
 * Importer
 ******************************************************************************/
Importer::Importer(const Glib::RefPtr<Database> &db,
                   const std::string &filename):
	m_database_{db},
	m_filename_{filename}
{
	assert(m_database_);
	assert(!m_filename_.empty());
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
	m_breeder_map_.clear();
	m_strain_map_.clear();
	m_growlog_map_.clear();
	
	Glib::RefPtr<DatabaseModule> module = db_get_module("sqlite3");
	assert(module);

	Glib::RefPtr<DatabaseSettings> import_settings = DatabaseSettings::create("sqlite3",
	                                                                          m_filename_,
	                                                                          module->get_defaults()->get_flags());
	Glib::RefPtr<Database> import_db = module->create_database(import_settings);
	assert(import_db);
	
	try {
		import_db->connect();
	} catch (DatabaseError ex) {
		Gtk::MessageDialog dialog(parent,
		                          ex.what(),
		                          false,
		                          Gtk::MESSAGE_ERROR,
		                          Gtk::BUTTONS_OK,
		                          true);
		dialog.run();
		dialog.hide();
		return;
	}

	int response = RESPONSE_NONE;
	
	//// import breeders ///////////////////////////////////////////////////////
	std::list<Glib::RefPtr<Breeder> > breeder_list = import_db->get_breeders();
	for (auto breeder_iter = breeder_list.begin(); breeder_iter != breeder_list.end(); ++breeder_iter) {
		Glib::RefPtr<Breeder> import_breeder = *breeder_iter;
		Glib::RefPtr<Breeder> breeder = m_database_->get_breeder(import_breeder->get_name());

		if (breeder && response != RESPONSE_UPDATE_ALL && response != RESPONSE_MERGE_ALL) {
			Gtk::MessageDialog dialog(parent,
			                          _("Breeder already exists! How do you want to proceed?"),
			                          false,
			                          Gtk::MESSAGE_QUESTION,
			                          Gtk::BUTTONS_NONE,
			                          true);
			dialog.add_button(_("Update"),RESPONSE_UPDATE);
			dialog.add_button(_("Update all"),RESPONSE_UPDATE_ALL);
			dialog.add_button(_("Merge"), RESPONSE_MERGE);
			dialog.add_button(_("Merge all"), RESPONSE_MERGE_ALL);

			do {
				response = dialog.run();
			} while (response == Gtk::RESPONSE_DELETE_EVENT);
		}
		if (breeder) {
			switch (response) {
				case RESPONSE_UPDATE:
				case RESPONSE_UPDATE_ALL:
					breeder->set_homepage(import_breeder->get_homepage());
					m_database_->add_breeder(breeder);
					break;
				case RESPONSE_MERGE:
				case RESPONSE_MERGE_ALL:
					/* do nothing */
					break;
				default:
					/*should never be reached */
					break;
			}
		} else {
			breeder = Breeder::create(import_breeder->get_name(),
			                          import_breeder->get_homepage());
			m_database_->add_breeder(breeder);
			breeder = m_database_->get_breeder(breeder->get_name());
		}
		m_breeder_map_[import_breeder->get_id()] = breeder->get_id();

		std::list<Glib::RefPtr<Strain> > strain_list{import_db->get_strains_for_breeder (import_breeder->get_id())};
		for (auto strain_iter = strain_list.begin(); strain_iter != strain_list.end(); ++strain_iter) {
			Glib::RefPtr<Strain> import_strain = *strain_iter;
			Glib::RefPtr<Strain> strain = m_database_->get_strain(import_strain->get_breeder_name(),
			                                                      import_strain->get_name());
			if (strain) {
				switch (response) {
					case RESPONSE_UPDATE:
					case RESPONSE_UPDATE_ALL:
						strain->set_info(import_strain->get_info());
						strain->set_description(import_strain->get_description());
						strain->set_homepage(import_strain->get_homepage());
						strain->set_seedfinder(import_strain->get_seedfinder());
						m_database_->add_strain(strain);
						break;
					case RESPONSE_MERGE:
					case RESPONSE_MERGE_ALL:
						break;
					default:
					 /*should never be reached */
					  break;
				}
			} else {
				strain = Strain::create(breeder->get_id(),
				                        breeder->get_name(),
				                        import_strain->get_name(),
				                        import_strain->get_info(),
				                        import_strain->get_description(),
				                        import_strain->get_homepage(),
				                        import_strain->get_seedfinder());
				m_database_->add_strain(strain);
				strain = m_database_->get_strain(breeder->get_name(),strain->get_name());
				assert(strain);
			}
			m_strain_map_[import_strain->get_id()] = strain->get_id();
		}		
	}

	//// import Growlogs ///////////////////////////////////////////////////////
	std::list<Glib::RefPtr<Growlog> > growlog_list{import_db->get_growlogs()};

	response = RESPONSE_NONE;
	
	for (auto growlog_iter = growlog_list.begin(); growlog_iter != growlog_list.end(); ++growlog_iter) {
		Glib::RefPtr<Growlog> import_growlog = *growlog_iter;
		Glib::RefPtr<Growlog> growlog = m_database_->get_growlog(import_growlog->get_title());

		if (growlog) {
			Gtk::MessageDialog dialog(parent,
			                          _("Growlog already exists! How do you want to proceed?"),
			                          false,
			                          Gtk::MESSAGE_QUESTION,
			                          Gtk::BUTTONS_NONE,
			                          true);
			dialog.add_button(_("Edit"), RESPONSE_EDIT);
			dialog.add_button(_("Ignore"), RESPONSE_IGNORE);
			do {
				response = dialog.run();
			} while (Gtk::RESPONSE_DELETE_EVENT);

			if (response == RESPONSE_IGNORE) {
				response = RESPONSE_NONE;
				continue;
			} else if (response = RESPONSE_EDIT) {
				GrowlogDialog growlog_dialog(parent, import_db, import_growlog);

				do {
					response = growlog_dialog.run();
					if (response == Gtk::RESPONSE_APPLY) {
						import_growlog = growlog_dialog.get_growlog();
						growlog = m_database_->get_growlog(import_growlog->get_title());
					} else if (response == Gtk::RESPONSE_CANCEL) {
						response = RESPONSE_IGNORE;
						growlog = Glib::RefPtr<Growlog>();
					}
				} while (growlog || response == Gtk::RESPONSE_DELETE_EVENT);
				if (response == RESPONSE_IGNORE) {
					response = RESPONSE_NONE;
					continue;
				}
				growlog = Growlog::create(import_growlog->get_title(),
				                          import_growlog->get_description(),
				                          import_growlog->get_created_on(),
				                          import_growlog->get_flower_on(),
				                          import_growlog->get_finished_on());
				m_database_->add_growlog(growlog);
				growlog = m_database_->get_growlog(growlog->get_title());
				assert(growlog);
			}			
		} else {
			growlog = Growlog::create(import_growlog->get_title(),
			                          import_growlog->get_description(),
			                          import_growlog->get_created_on(),
			                          import_growlog->get_flower_on(),
			                          import_growlog->get_finished_on());
			m_database_->add_growlog(growlog);
			growlog = m_database_->get_growlog(growlog->get_title());
			assert(growlog);
		}

		m_growlog_map_[import_growlog->get_id()] = growlog->get_id();
		
		std::list<Glib::RefPtr<Strain> > strain_list{import_db->get_strains_for_growlog(import_growlog)};
		for (auto strain_iter = strain_list.begin(); strain_iter != strain_list.end(); ++strain_iter) {
			Glib::RefPtr<Strain> import_strain = *strain_iter;
			m_database_->add_strain_for_growlog (growlog->get_id(),
			                                     m_strain_map_[import_strain->get_id()]);
		}

		std::list<Glib::RefPtr<GrowlogEntry> > entry_list{import_db->get_growlog_entries(import_growlog->get_id())};
		for (auto entry_iter = entry_list.begin(); entry_iter != entry_list.end(); ++entry_iter) {
			Glib::RefPtr<GrowlogEntry> import_entry = *entry_iter;
			Glib::RefPtr<GrowlogEntry> entry = GrowlogEntry::create(growlog->get_id(),
			                                                        import_entry->get_text(),
			                                                        import_entry->get_created_on());
			m_database_->add_growlog_entry(entry);
		}
	}
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
