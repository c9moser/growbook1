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

	import_strains(parent,import_db);
	import_growlogs(parent,import_db);
}


void
Importer::import_strains(Gtk::Window &parent,
                         const Glib::RefPtr<Database> &import_db)
{
	assert(import_db);

	int response = RESPONSE_NONE;

	std::list<Glib::RefPtr<Breeder> > breeders{import_db->get_breeders()};
	for (auto breeder_iter = breeders.begin(); breeder_iter != breeders.end(); ++breeder_iter) {
		Glib::RefPtr<Breeder> import_breeder = *breeder_iter;
		Glib::RefPtr<Breeder> breeder = m_database_->get_breeder(import_breeder->get_name());

		if (breeder && response != RESPONSE_UPDATE_ALL && !response != RESPONSE_MERGE_ALL) {
			Gtk::MessageDialog dialog(parent,
			                          "Breeder already exists! How do you want to perceed?",
			                          false,
			                          Gtk::MESSAGE_QUESTION,
			                          Gtk::BUTTONS_NONE,
			                          true);
			dialog.add_button(_("Merge"), RESPONSE_MERGE);
			dialog.add_button(_("Merge All"), RESPONSE_MERGE_ALL);
			dialog.add_button(_("Update"), RESPONSE_UPDATE);
			dialog.add_button(_("Update All"), RESPONSE_UPDATE_ALL);
			
			response = dialog.run();
			dialog.hide();
			if (response == Gtk::RESPONSE_DELETE_EVENT)
				return;
		}
		if (!breeder) {
			breeder = Breeder::create(import_breeder->get_name(),
			                         import_breeder->get_homepage());
			m_database_->add_breeder(breeder);
			breeder = m_database_->get_breeder(import_breeder->get_name());
			assert(breeder);
		} if (response == RESPONSE_UPDATE || response == RESPONSE_UPDATE_ALL) {
			breeder->set_homepage(import_breeder->get_homepage());
			m_database_->add_breeder(breeder);
		}
		assert(breeder);
		m_breeder_map_[import_breeder->get_id()] = breeder;

		std::list<Glib::RefPtr<Strain> > strains = import_db->get_strains_for_breeder(import_breeder);

		for (auto strain_iter = strains.begin(); strain_iter != strains.end(); ++strain_iter) {
			Glib::RefPtr<Strain> import_strain = *strain_iter;
			Glib::RefPtr<Strain> strain = m_database_->get_strain(breeder->get_name(),import_strain->get_name());
			if (!strain) {
				strain = Strain::create(breeder->get_id(),
				                        breeder->get_name(),
				                        import_strain->get_name(),
				                        import_strain->get_info(),
				                        import_strain->get_description(),
				                        import_strain->get_homepage(),
				                        import_strain->get_seedfinder());
				m_database_->add_strain(strain);
				strain = m_database_->get_strain(breeder->get_name(),
				                                 import_strain->get_name());
			} else if (response == RESPONSE_UPDATE || response == RESPONSE_UPDATE_ALL) {
				strain->set_info(import_strain->get_info());
				strain->set_description(import_strain->get_description());
				strain->set_homepage(import_strain->get_homepage());
				strain->set_seedfinder(import_strain->get_seedfinder());
				m_database_->add_strain(strain);
			}
			assert(strain);
			m_strain_map_[import_strain->get_id()] = strain;
		}
	}
}

void
Importer::import_growlogs(Gtk::Window &parent,
                          const Glib::RefPtr<Database> &import_db)
{
	int response = RESPONSE_NONE;
	std::list<Glib::RefPtr<Growlog> > growlogs = import_db->get_growlogs();
	for (auto growlog_iter = growlogs.begin(); growlog_iter != growlogs.end(); ++growlog_iter) {
		Glib::RefPtr<Growlog> import_growlog = *growlog_iter;
		Glib::RefPtr<Growlog> growlog = m_database_->get_growlog(import_growlog->get_title());
		if (growlog && response != RESPONSE_IGNORE_ALL) {
			Gtk::MessageDialog dialog(parent,
			                          _("Growlog exists already! How do you want to perceed?"),
			                          false,
			                          Gtk::MESSAGE_QUESTION,
			                          Gtk::BUTTONS_NONE,
			                          true);
			dialog.add_button(_("Edit"), RESPONSE_EDIT);
			dialog.add_button(_("Ignore"), RESPONSE_IGNORE);
			dialog.add_button(_("Ignore All"), RESPONSE_IGNORE_ALL);

			if (response == RESPONSE_EDIT) {
				bool edit_done = false;
				while (!edit_done) {
					GrowlogDialog growlog_dialog{import_db,import_growlog};
					growlog_dialog.set_update_database(false);

					int res = growlog_dialog.run();
					growlog_dialog.hide();
					if (res == Gtk::RESPONSE_APPLY) {
						import_growlog = growlog_dialog.get_growlog();
						growlog = m_database_->get_growlog(import_growlog->get_title());
						if (!growlog)
							edit_done = true;
					}
				}
			} else if (response == RESPONSE_IGNORE || response == RESPONSE_IGNORE_ALL) {
				continue;
			}
		}
		growlog = Growlog::create(import_growlog->get_title(),
		                          import_growlog->get_description(),
		                          import_growlog->get_created_on(),
		                          import_growlog->get_flower_on(),
		                          import_growlog->get_finished_on());
		m_database_->add_growlog(growlog);
		growlog = m_database_->get_growlog(import_growlog->get_title());
		assert(growlog);
		
		m_growlog_map_[import_growlog->get_id()] = growlog;

		std::list<Glib::RefPtr<GrowlogEntry> > entries = import_db->get_growlog_entries(import_growlog->get_id());
		for (auto entry_iter = entries.begin(); entry_iter != entries.end(); ++entry_iter) {
			Glib::RefPtr<GrowlogEntry> import_entry = *entry_iter;
			Glib::RefPtr<GrowlogEntry> entry = GrowlogEntry::create(growlog->get_id(),
			                                                        import_entry->get_text(),
			                                                        import_entry->get_created_on());
			m_database_->add_growlog_entry(entry);
		}

		std::list<Glib::RefPtr<Strain> > strains = import_db->get_strains_for_growlog(import_growlog);
		for (auto strain_iter = strains.begin(); strain_iter != strains.end(); ++strain_iter) {
			Glib::RefPtr<Strain> import_strain = *strain_iter;
			Glib::RefPtr<Strain> strain = m_strain_map_[import_strain->get_id()];
			m_database_->add_strain_for_growlog(growlog,strain);
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
