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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "export.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>
#include <glibmm.h>

#include <gtkmm/messagedialog.h>

#include <cassert>
#include <iostream>
#include <fstream>
#include <time.h>

#include <unistd.h>

#include "application.h"

struct indent {
	unsigned depth;
	inline indent(unsigned i):depth(i) {};
};

std::fstream& operator<<(std::fstream &f,  const indent &in)
{
	for (unsigned i=0; i<in.depth; ++i) {
		f << "  ";
	}
	return f;
}

/*******************************************************************************
 * Exporter
 ******************************************************************************/

Exporter::Exporter(const Glib::RefPtr<Database> &database,
                   const std::string &filename):
	m_database_(database),
	m_filename_(filename)
{
	assert(m_database_->is_connected());
}

Exporter::~Exporter()
{
}

Glib::RefPtr<Database>
Exporter::get_database()
{
	return m_database_;
}

Glib::RefPtr<const Database>
Exporter::get_database() const
{
	return Glib::RefPtr<const Database>::cast_const(m_database_);
}

void
Exporter::set_filename(const std::string &filename)
{
	m_filename_ = filename;
}

std::string
Exporter::get_filename() const
{
	return m_filename_;
}

bool
Exporter::file_exists() const
{
	return Glib::file_test(m_filename_,Glib::FILE_TEST_EXISTS);
}


void
Exporter::export_db()
{
	export_vfunc(*app->get_appwindow());
}

void
Exporter::export_db(Gtk::Window &parent)
{
	export_vfunc(parent);
}

/*******************************************************************************
 * XML_Exporter
 ******************************************************************************/

XML_Exporter::XML_Exporter(const Glib::RefPtr<Database> &database,
                           const std::string &filename):
	Exporter(database,filename)
{
}

XML_Exporter::~XML_Exporter()
{
}

Glib::RefPtr<XML_Exporter>
XML_Exporter::create(const Glib::RefPtr<Database> &database,
                     const std::string &filename)
{
	return Glib::RefPtr<XML_Exporter>(new XML_Exporter(database,filename));
}

void
XML_Exporter::export_vfunc(Gtk::Window &parent)
{
	
	std::fstream of;
	if (Glib::file_test(get_filename(),Glib::FILE_TEST_EXISTS)) { 
		of.open(get_filename(), std::fstream::out | std::fstream::trunc);
	} else {
		of.open(get_filename(), std::fstream::out);
	}
	
	if (!of.is_open()) {
		Gtk::MessageDialog dialog(parent,
		                   		  _("Unable to open file for writing!"),
		                          false,
		                          Gtk::MESSAGE_ERROR,
		                          Gtk::BUTTONS_OK,
		                          true);
		dialog.run();
		dialog.hide();
		return;		                   
	}

	of << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	of << "<growbook>\n";

	// Export Breeder;
	of << indent(1) << "<breeders>\n";
	std::list<Glib::RefPtr<Breeder> > breeders(get_database()->get_breeders());
	for (auto iter = breeders.begin(); iter != breeders.end(); ++iter) {
		Glib::RefPtr<Breeder> b = *iter;
		of << indent(2) << "<breeder>\n";
		of << indent(3) << "<name>" << escape_text(b->get_name())<< "</name>\n";
		if (!b->get_homepage().empty()) {
			of << indent(3) << "<homepage>" << escape_text(b->get_homepage()) << "</homepage>\n";
		}

		of << indent(3) << "<strains>\n";
		std::list<Glib::RefPtr<Strain> > strains(get_database()->get_strains_for_breeder(b->get_id()));
		// export strains
		for (auto iter = strains.begin(); iter != strains.end(); ++iter) {
			Glib::RefPtr<Strain> s = *iter;
			of << indent(4) << "<strain>\n";
			of << indent(5) << "<name>" << escape_text(s->get_name()) << "</name>\n";
			if (!s->get_homepage().empty()) {
				of << indent(5) << "<homepage>" << escape_text(s->get_homepage()) << "</homepage>\n";
			}			
			if (!s->get_seedfinder().empty()) {
				of << indent(5) << "<seedfinder>" << escape_text(s->get_seedfinder()) << "</seedfinder>\n";
			}
			if (!s->get_info().empty()) {
				of << indent(5) << "<info><![CDATA[" << s->get_info() << "]]></info>\n";
			}
			if (!s->get_description().empty()) {
				of << indent(5) << "<description><![CDATA[" 
					<< s->get_description() 
					<< "]]></description>\n";
			}
			// end strain
			of << indent(4) << "</strain>\n";
		}
		of << indent(3) << "</strains>\n";

		// end breeder
		of << indent(2) << "</breeder>\n";
	}
	// end breeders
	breeders.clear();
	of << indent(1) << "</breeders>" << std::endl;
	
	// export growlogs
	
	of << indent(1) << "<growlogs>\n";
	std::list<Glib::RefPtr<Growlog> > growlogs(get_database()->get_growlogs());
	for (auto iter = growlogs.begin(); iter != growlogs.end(); ++iter) {
		Glib::RefPtr<Growlog> gl = *iter;
		
		of << indent(2) << "<growlog>\n";
		of << indent(3) << "<title>" << escape_text(gl->get_title()) << "</title>\n";
		of << indent(3) << "<created_on>" 
			<< escape_text(gl->get_created_on_format()) 
			<< "</created_on>\n";

		if (gl->get_flower_on()) {
			of << indent(3) << "<flower_on>" 
				<< escape_text(gl->get_flower_on_format()) 
				<< "</flower_on>\n";
		}
			
		if (gl->get_finished_on()) {
			of << indent(3) << "<finished_on>" 
				<< escape_text(gl->get_finished_on_format()) 
				<< "</finished_on>\n";
		}
		
		if (!gl->get_description().empty()) { 
			of << indent(3) << "<description><![CDATA[" 
				<< gl->get_description() 
				<< "]]></description>\n";
		}

		of << indent(3) << "<strains>\n";
		std::list<Glib::RefPtr<Strain> > strains 
			= get_database()->get_strains_for_growlog(gl->get_id());
		for (auto iter = strains.begin(); iter != strains.end(); ++iter) {
			Glib::RefPtr<Strain> strain = *iter;
			of << indent(4) << "<strain>\n";

			of << indent(5) << "<breeder>" 
				<< escape_text(strain->get_breeder_name())
				<< "</breeder>\n";

			of << indent(5) << "<name>"
				<< escape_text(strain->get_name())
				<< "</name>\n";
			
			of << indent(4) << "</strain>\n";
		}
		of << indent(3) << "</strains>\n";

		of << indent(3) << "<entries>\n";
		std::list<Glib::RefPtr<GrowlogEntry> > entries 
			= get_database()->get_growlog_entries(gl);
		for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
			Glib::RefPtr<GrowlogEntry> entry = *iter;
			of << indent(4) << "<entry>\n";

			of << indent(5) << "<created_on>"
				<< escape_text(entry->get_created_on_format())
				<< "</created_on>\n";

			of << indent(5) << "<text><![CDATA["
				<< entry->get_text()
				<< "]]></text>\n";

			of << indent(4) << "</entry>\n";
		}
		of << indent(3) << "</entries>\n";
		
		of << indent(2) << "</growlog>\n";
	}
	of << indent(1) << "</growlogs>\n";

	of << "</growbook>" << std::endl;

	of.close();
}

Glib::ustring
XML_Exporter::escape_text(const Glib::ustring &txt) const
{
	return Glib::Markup::escape_text(txt); 
}

/*******************************************************************************
 * DB_Exporter
 ******************************************************************************/

DB_Exporter::DB_Exporter(const Glib::RefPtr<Database> &database,
                   const std::string &filename):
	Exporter(database,filename)
{
}

DB_Exporter::~DB_Exporter()
{
}

Glib::RefPtr<DB_Exporter>
DB_Exporter::create(const Glib::RefPtr<Database> &db,
                    const std::string &filename)
{
	return Glib::RefPtr<DB_Exporter>(new DB_Exporter(db,filename));
}
                              


void
DB_Exporter::export_vfunc(Gtk::Window &parent)
{
	m_strain_map_.clear();
	m_breeder_map_.clear();

	if (this->file_exists())
#ifdef NATIVE_WINDOWS
		_unlink(get_filename().c_str());
#else
		unlink(get_filename().c_str());
#endif

	Glib::RefPtr<DatabaseModule> dbmodule = db_get_module("sqlite3");
	Glib::RefPtr<DatabaseSettings> settings = DatabaseSettings::create("sqlite3",
	                                                                   get_filename(),
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
DB_Exporter::_export_strains(Gtk::Window &parent,
                          const Glib::RefPtr<Database> &dbexport)
{
	std::list<Glib::RefPtr<Breeder> > breeder_list{get_database()->get_breeders()};

	for (auto breeder_iter = breeder_list.begin(); breeder_iter != breeder_list.end(); ++breeder_iter) {
		Glib::RefPtr<Breeder> breeder0 = *breeder_iter;
		Glib::RefPtr<Breeder> breeder1 = Breeder::create(breeder0->get_name(),
		                                                 breeder0->get_homepage());
		dbexport->add_breeder(breeder1);
		breeder1 = dbexport->get_breeder(breeder0->get_name());
		assert(breeder1);

		std::list<Glib::RefPtr<Strain> > strain_list{get_database()->get_strains_for_breeder(breeder0->get_id())};
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
DB_Exporter::_export_growlogs(Gtk::Window &parent,
                           const Glib::RefPtr<Database> &dbexport)
{
	std::list<Glib::RefPtr<Growlog> > growlog_list{get_database()->get_growlogs()};

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

		std::list<Glib::RefPtr<Strain> > strain_list{get_database()->get_strains_for_growlog(growlog0)};

		for (auto strain_iter = strain_list.begin(); strain_iter != strain_list.end(); ++strain_iter) {
			Glib::RefPtr<Strain> strain0 = *strain_iter;
			Glib::RefPtr<Strain> strain1 = m_strain_map_[strain0->get_id()];

			dbexport->add_strain_for_growlog(growlog1,strain1);
		}

		std::list<Glib::RefPtr<GrowlogEntry> > growlog_entry_list{get_database()->get_growlog_entries(growlog0)};

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

enum ExportFilter {
	EXPORT_FILTER_XML,
	EXPORT_FILTER_DB
};

static const char *EXPORT_FILTER[] {
	N_("Growbook File"),
	N_("SQLite3 Database")
};

void
ExportDialog::_configure()
{
	set_create_folders(true);

	Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
	filter->set_name(_(EXPORT_FILTER[EXPORT_FILTER_XML]));
	filter->add_pattern("*.growbook");
	add_filter(filter);

	filter = Gtk::FileFilter::create();
	filter->set_name(EXPORT_FILTER[EXPORT_FILTER_DB]);
	filter->add_pattern("*.db");
	add_filter(filter);
	
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
	set_current_name(filename);
}

static bool _has_ending(const std::string &s, const std::string &end) {
	if (s.length() >= end.length())
		return (0 == s.compare(s.length() - end.length(), end.length(), end));
		
	return false;
}

Glib::RefPtr<Exporter>
ExportDialog::get_exporter()
{
	std::string filename = get_filename();
	Glib::RefPtr<Exporter> exporter{};
	if (filename.empty())
		return exporter;

	if (Glib::file_test(filename, Glib::FILE_TEST_EXISTS)) {
#ifdef NATIVE_WINDOWS
		Glib::ustring msg = _("File already exists!");
#else
		Glib::ustring msg = Glib::ustring::sprintf(_("File \"%s\" already exists!"),filename);
#endif // NATIVE_WINDOWS
		Gtk::MessageDialog dialog(*this,msg,false,Gtk::MESSAGE_QUESTION,Gtk::BUTTONS_YES_NO,true);
		dialog.set_secondary_text(_("Do you want to overwrite the file?"));
		if (dialog.run() == Gtk::RESPONSE_YES) {
#ifdef NATIVE_WINDOWS
			_unlink(filename.c_str());
#else
			unlink(filename.c_str());
#endif // NATIVE_WINDOWS
		} else {
			return exporter;
		}
	}
	Glib::RefPtr<Gtk::FileFilter> filter = get_filter();
	if (filter->get_name() == _(EXPORT_FILTER[EXPORT_FILTER_XML])) {
		if (!_has_ending(filename, ".growbook"))
			filename += ".growbook";
		exporter = XML_Exporter::create(m_database_,filename);
	} else if (filter->get_name() == _(EXPORT_FILTER[EXPORT_FILTER_DB])) {
		if (!_has_ending(filename, ".db"))
			filename += ".db";
		exporter = DB_Exporter::create(m_database_, filename);		
	}
	return exporter;
}
