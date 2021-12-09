//           xml_importer.cc
//  Mi Dezember 08 13:54:26 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// xml_importer.cc
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
#include <glibmm/i18n.h>

#include "xml_importer.h"
#include <glibmm/markup.h>
#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/messagedialog.h>

#include <fstream>
#include <ctime>
#include <cstdio>
#include <cassert>

enum MarkupElement {
	MARKUP_UNKNOWN = -1,
	MARKUP_NONE = 0,
	MARKUP_GROWBOOK,
	MARKUP_GB_BREEDERS,
	MARKUP_GB_BREEDERS_BREEDER,
	MARKUP_GB_BREEDERS_BREEDER_NAME,
	MARKUP_GB_BREEDERS_BREEDER_HOMEPAGE,
	MARKUP_GB_BREEDERS_BREEDER_STRAINS,
	MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN,
	MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_NAME,
	MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_HOMEPAGE,
	MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_SEEDFINDER,
	MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_INFO,
	MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_DESCRIPTION,
	MARKUP_GB_GROWLOGS,
	MARKUP_GB_GROWLOGS_GROWLOG,
	MARKUP_GB_GROWLOGS_GROWLOG_TITLE,
	MARKUP_GB_GROWLOGS_GROWLOG_CREATED,
	MARKUP_GB_GROWLOGS_GROWLOG_FLOWER,
	MARKUP_GB_GROWLOGS_GROWLOG_FINISHED,
	MARKUP_GB_GROWLOGS_GROWLOG_DESCRIPTION,
	MARKUP_GB_GROWLOGS_GROWLOG_STRAINS,
	MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN,
	MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN_BREEDER,
	MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN_NAME,
	MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES,
	MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY,
	MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY_CREATED,
	MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY_TEXT
};

enum BreederMode {
	BREEDER_MODE_UNKNOWN,
	BREEDER_MODE_UPDATE,
	BREEDER_MODE_UPDATE_ALL,
	BREEDER_MODE_MERGE,
	BREEDER_MODE_MERGE_ALL
};

struct MarkupNode {
	MarkupElement element;
	MarkupNode *parent;

	inline MarkupNode(): element(MARKUP_NONE), parent(nullptr) {};
	inline MarkupNode(MarkupElement e,MarkupNode *p):
		element(e),
		parent(p)
	{};		
};

class MarkupParser:
	public Glib::Markup::Parser
{
	private:
		Gtk::Window *m_parent_;
		Glib::RefPtr<Database> m_database_;
		MarkupNode  *m_node_;

		BreederMode m_breeder_mode_;
		bool m_breeder_exists_;
		Glib::RefPtr<Breeder> m_breeder_;
		Glib::RefPtr<Strain> m_strain_;

		bool m_growlog_ignore_;
		Glib::ustring m_growlog_breeder_;
		Glib::ustring m_growlog_strain_;
		Glib::ustring m_growlog_title_;
		Glib::ustring m_growlog_created_on_;
		Glib::RefPtr<Growlog> m_growlog_;
		Glib::ustring m_growlog_entry_created_on_;
		Glib::ustring m_growlog_entry_text_;
		
	public:
		MarkupParser(Gtk::Window &parent,
		             const Glib::RefPtr<Database> &database);
		virtual ~MarkupParser();

	private:
		Glib::ustring _rename_growlog(const Glib::ustring &title);

		void _create_growlog();
		time_t _parse_date(const Glib::ustring &date);
		time_t _parse_datetime(const Glib::ustring &datetime);
		
	protected:
		virtual void on_start_element(Glib::Markup::ParseContext &context,
		                              const Glib::ustring &element_name);
		virtual void on_end_element(Glib::Markup::ParseContext &context,
		                            const Glib::ustring &emelent_name);
		virtual void on_text(Glib::Markup::ParseContext &context,
		                     const Glib::ustring &text);
		virtual void on_passthrough(Glib::Markup::ParseContext &context,
		                            const Glib::ustring &passthrough_text);
		virtual void on_error(Glib::Markup::ParseContext &context,
		                      const Glib::MarkupError &error);
};

/******************************************************************************/

class RenameGrowlogDialog:
	public Gtk::Dialog
{
	private:
		Glib::RefPtr<Database> m_database_;
		Gtk::Entry m_title_entry_;
	public:
		RenameGrowlogDialog(Gtk::Window &parent,
		                    const Glib::RefPtr<Database> &database,
		                    const Glib::ustring &title);
		virtual ~ RenameGrowlogDialog();

	public:
		void set_growlog_title(const Glib::ustring &title);
		Glib::ustring get_growlog_title() const;

		bool get_is_renamed() const;
};

/*******************************************************************************
 * RenameGrowlogDialog
 ******************************************************************************/
RenameGrowlogDialog::RenameGrowlogDialog(Gtk::Window &parent,
                                         const Glib::RefPtr<Database> &db,
                                         const Glib::ustring &title):
	Gtk::Dialog(_("Rename Growlog"),parent),
	m_database_(db),
	m_title_entry_()
{
	m_title_entry_.set_text(title);


	Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox());
	Gtk::Label *label = Gtk::manage(new Gtk::Label(_("Title")));
	hbox->pack_start(*label);
	hbox->pack_start(m_title_entry_);

	get_content_area()->pack_start(*hbox);

	add_button(_("OK"), Gtk::RESPONSE_OK);
}
                                         
RenameGrowlogDialog::~RenameGrowlogDialog()
{}

void
RenameGrowlogDialog::set_growlog_title(const Glib::ustring &title)
{
	m_title_entry_.set_text(title);
}

Glib::ustring
RenameGrowlogDialog::get_growlog_title() const
{
	return m_title_entry_.get_text();
}

bool
RenameGrowlogDialog::get_is_renamed() const
{
	Glib::RefPtr<Growlog> gl = m_database_->get_growlog(get_growlog_title());
	if (gl)
		return false;
	return true;
}

/*******************************************************************************
 * MarkupParser
 ******************************************************************************/

MarkupParser::MarkupParser(Gtk::Window &parent,
                           const Glib::RefPtr<Database> &db):
	Glib::Markup::Parser(),
	m_parent_(&parent),
	m_database_(db),
	m_node_(new MarkupNode),
	m_breeder_exists_(false),
	m_breeder_mode_(BREEDER_MODE_UNKNOWN),
	m_breeder_(),
	m_strain_(),
	m_growlog_ignore_(false),
	m_growlog_breeder_(),
	m_growlog_strain_(),
	m_growlog_title_(),
	m_growlog_created_on_(),
	m_growlog_(),
	m_growlog_entry_created_on_(),
	m_growlog_entry_text_()
{
}

MarkupParser::~MarkupParser()
{
}

Glib::ustring
MarkupParser::_rename_growlog(const Glib::ustring &title)
{
	RenameGrowlogDialog dialog(*m_parent_,m_database_,title);
	do {
		dialog.present();
		dialog.run();
	} while(!dialog.get_is_renamed());
	dialog.hide();
	return dialog.get_growlog_title();
}

time_t
MarkupParser::_parse_datetime(const Glib::ustring &dt)
{
	if (dt.empty())
		return 0;
	    
	tm datetime;
	strptime(dt.c_str(),DATETIME_ISO_FORMAT,&datetime);
	return mktime(&datetime);
}

time_t
MarkupParser::_parse_date(const Glib::ustring &d)
{
	if (d.empty())
		return 0;

	tm datetime;
	strptime(d.c_str(),DATE_ISO_FORMAT,&datetime);
	return mktime(&datetime);
}

void
MarkupParser::_create_growlog()
{
	if (m_growlog_)
		return;
	
	time_t created_on = 0;
	if (!m_growlog_created_on_.empty())
		created_on = _parse_datetime(m_growlog_created_on_);
		
	if (!m_growlog_ && created_on && !m_growlog_title_.empty()) {
		Glib::RefPtr<Growlog> gl = Growlog::create(m_growlog_title_,"",created_on);
		m_database_->add_growlog(gl);
		m_growlog_ = m_database_->get_growlog(m_growlog_title_);
	}
}

void
MarkupParser::on_start_element(Glib::Markup::ParseContext &context,
                               const Glib::ustring &element)
{
	switch (m_node_->element) {
		case MARKUP_NONE:
			if (element == "growbook") {
				m_node_ = new MarkupNode(MARKUP_GROWBOOK,m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
			break;
		case MARKUP_GROWBOOK:
			if (element == "breeders") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS,m_node_);
			} else if (element == "growlogs") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS,m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
			break;
		case MARKUP_GB_BREEDERS:
			if (element == "breeder") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER,m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
			break;
		case MARKUP_GB_BREEDERS_BREEDER:
			if (element == "name") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER_NAME,m_node_);
			} else if (element == "homepage") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER_HOMEPAGE,m_node_);
			} else if (element == "strains") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER_STRAINS,m_node_);
			}
			break;
		case MARKUP_GB_BREEDERS_BREEDER_STRAINS:
			if (element == "strain") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN,
				                         m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
			break;
		case MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN:
			if (element == "name") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_NAME,
				                         m_node_);
			} else if (element == "homepage") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_HOMEPAGE,
				                         m_node_);
			} else if (element == "seedfinder") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_SEEDFINDER,
				                         m_node_);
			} else if (element == "info") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_INFO,
				                         m_node_);
			} else if (element == "description") {
				m_node_ = new MarkupNode(MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_DESCRIPTION,
				                         m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
			break;
		case MARKUP_GB_GROWLOGS:
			if (element == "growlog") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG,m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
		case MARKUP_GB_GROWLOGS_GROWLOG:
			if (element == "title") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_TITLE,
				                         m_node_);
			} else if (element == "created_on") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_CREATED,
				                         m_node_);
			} else if (element == "flower_on") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_FLOWER,
				                         m_node_);
			} else if (element == "finished_on") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_FINISHED,
				                         m_node_);
			} else if (element == "description") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_DESCRIPTION,
				                         m_node_);
			} else if (element == "strains") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_STRAINS,
				                         m_node_);
			} else if (element == "entries") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES,
				                         m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_STRAINS:
			if (element == "strain") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN,
				                         m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN:
			if (element == "breeder") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN_BREEDER,
				                         m_node_);
			} else if (element == "name") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN_NAME,
				                         m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES:
			if (element == "entry") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY,
				                         m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY:
			if (element == "created_on") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY_CREATED,
				                         m_node_);
			} else if (element == "text") {
				m_node_ = new MarkupNode(MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY_TEXT,
				                         m_node_);
			} else {
				m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			}			
		case MARKUP_UNKNOWN:
		default:
			m_node_ = new MarkupNode(MARKUP_UNKNOWN,m_node_);
			break;
	};
} // MarkupParser::on_start_element()

void
MarkupParser::on_end_element(Glib::Markup::ParseContext &context,
                             const Glib::ustring &element)
{
	switch (m_node_->element) {
		case MARKUP_GB_BREEDERS_BREEDER:
			assert(element == "breeder");
			assert(m_breeder_);
			
			if (m_breeder_mode_ == BREEDER_MODE_UPDATE 
			    || m_breeder_mode_ == BREEDER_MODE_UPDATE_ALL
			    || !m_breeder_exists_) {
				m_database_->add_breeder(m_breeder_);
			}
			m_breeder_exists_ = false;
			
			if (m_breeder_mode_ != BREEDER_MODE_UPDATE_ALL
			    && m_breeder_mode_ != BREEDER_MODE_MERGE_ALL)
				m_breeder_mode_ = BREEDER_MODE_UNKNOWN;

			m_breeder_ = Glib::RefPtr<Breeder>();
			break;
		case MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN:
			if (!m_strain_->get_id()
			    || m_breeder_mode_ == BREEDER_MODE_UPDATE
			    || m_breeder_mode_ == BREEDER_MODE_UPDATE_ALL) {
				
				m_database_->add_strain(m_strain_);
			}
			m_strain_ = Glib::RefPtr<Strain>();
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG:
			if (!m_growlog_ignore_ && m_growlog_)
				m_database_->add_growlog(m_growlog_);
			m_growlog_ignore_ = false;
			m_growlog_title_.clear();
			m_growlog_created_on_.clear();
			m_growlog_.clear();
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_TITLE:
		case MARKUP_GB_GROWLOGS_GROWLOG_CREATED:
			if (!m_growlog_ignore_ 
			    	&& !m_growlog_title_.empty() 
			    	&& !m_growlog_created_on_.empty() 
			    	&& !m_growlog_) {
				_create_growlog();
			}
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN:
			if (!m_growlog_ignore_ && m_growlog_ 
			   		&& !m_growlog_breeder_.empty() 
			    	&& !m_growlog_strain_.empty()) {
				Glib::RefPtr<Strain> strain = m_database_->get_strain(m_growlog_breeder_,
				                                                      m_growlog_strain_);
				m_database_->add_strain_for_growlog(m_growlog_,strain);
			}
			m_growlog_breeder_.clear();
			m_growlog_strain_.clear();
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY:
			if (!m_growlog_ignore_ && !m_growlog_entry_created_on_.empty() && m_growlog_) {
				time_t created_on = _parse_datetime(m_growlog_entry_created_on_);
				Glib::RefPtr<GrowlogEntry> entry = GrowlogEntry::create(m_growlog_->get_id(),
				                                                        m_growlog_entry_text_,
				                                                        created_on);
			}
			m_growlog_entry_created_on_.clear();
			m_growlog_entry_text_.clear();
		default:
			break;
	}
	MarkupNode *node = m_node_;
	m_node_ = m_node_->parent;
	delete node;
} // MarkupParser::on_end_element()

void
MarkupParser::on_text(Glib::Markup::ParseContext &context,
                      const Glib::ustring &text)
{
	switch (m_node_->element) {
		case MARKUP_GB_BREEDERS_BREEDER_NAME:
			if (!m_breeder_) {
				m_breeder_ = m_database_->get_breeder(text);
				if (!m_breeder_) {
					Glib::RefPtr<Breeder> b = Breeder::create(text,"");
					m_database_->add_breeder(b);
					m_breeder_ = m_database_->get_breeder(text);
					m_breeder_exists_ = false;
					if (m_breeder_mode_ == BREEDER_MODE_UNKNOWN)
						m_breeder_mode_ = BREEDER_MODE_UPDATE;
 				} else {
					m_breeder_exists_ = true;
					if (m_breeder_mode_ != BREEDER_MODE_UPDATE_ALL 
						&& m_breeder_mode_ != BREEDER_MODE_MERGE_ALL) {
						
						Glib::ustring msg_fmt = 
							_("Breeder \"%s\" already exists!\nHow do you want to proceed?");
						size_t msg_size = msg_fmt.bytes() + text.bytes() + 1;
						char *msg = new char[msg_size];
						msg[msg_size-1]='\0';
						snprintf(msg,msg_size,msg_fmt.c_str(),text.c_str());
						Gtk::MessageDialog dialog(*m_parent_,
					    	                      (const char*) msg,
					    	                      false,
					    	                      Gtk::MESSAGE_QUESTION,
					    	                      Gtk::BUTTONS_NONE,
					    	                      true);
						delete[] msg;
						dialog.add_button(_("Merge"),BREEDER_MODE_MERGE);
						dialog.add_button(_("Merge All"),BREEDER_MODE_MERGE_ALL);
						dialog.add_button(_("Update"),BREEDER_MODE_UPDATE);
						dialog.add_button(_("Update All"),BREEDER_MODE_UPDATE);
						
						int result = BREEDER_MODE_UNKNOWN;
						
						do {
							dialog.present();
							result = dialog.run();
						} while (result == Gtk::RESPONSE_DELETE_EVENT);
						m_breeder_mode_ = (BreederMode) result;
					}
				} 
			} else {
				fprintf (stderr,"XML LOGIC ERROR!\n");
			}
			break;
		case MARKUP_GB_BREEDERS_BREEDER_HOMEPAGE:
			if (m_breeder_)
				m_breeder_->set_homepage(text);
			break;
		case MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_NAME:
			if (!m_breeder_)
				break;
			
			m_strain_ = m_database_->get_strain(m_breeder_->get_name(), text);
			if (!m_strain_) {
				m_strain_ = Strain::create(m_breeder_->get_id(),
				                           m_breeder_->get_name(),
				                           text,
				                           "",
				                           "",
				                           "",
				                           "");
			}
			break;
		case MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_HOMEPAGE:
			if (m_strain_)
				m_strain_->set_homepage(text);
			break;
		case MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_SEEDFINDER:
			if (m_strain_)
				m_strain_->set_seedfinder(text);
		case MARKUP_GB_GROWLOGS_GROWLOG_TITLE:
			m_growlog_ = m_database_->get_growlog(text);
			if (m_growlog_) {
				Glib::ustring fmt = _("Growlog \"%s\" already exists!\nHow do you want to proceed?");
				size_t msg_size = fmt.bytes() + text.bytes() + 1;
				char *msg = new char[msg_size];
				msg [msg_size -1] = '\0';
				snprintf(msg,msg_size,fmt.c_str(),text.c_str());
				
				Gtk::MessageDialog dialog(*m_parent_,
				                          (const char*) msg,
				                          false,
				                          Gtk::MESSAGE_QUESTION,
				                          Gtk::BUTTONS_NONE,
				                          true);
				delete[] msg;
				dialog.add_button(_("Rename"),Gtk::RESPONSE_APPLY);
				dialog.add_button(_("Skip"),Gtk::RESPONSE_CANCEL);

				int result;
				do {
					dialog.present();
					result = dialog.run();
				} while (result == Gtk::RESPONSE_DELETE_EVENT);
				dialog.hide();
				if (result == Gtk::RESPONSE_APPLY) 
					m_growlog_title_ = _rename_growlog(text);
				else
					m_growlog_ignore_ = true;
			}
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_CREATED:
			if (!m_growlog_ignore_)
				m_growlog_created_on_ = text;
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_FLOWER:
			if (m_growlog_)
				m_growlog_->set_flower_on(_parse_date(text));
		case MARKUP_GB_GROWLOGS_GROWLOG_FINISHED:
			if (m_growlog_)
				m_growlog_->set_finished_on(_parse_datetime(text));
		case MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN_BREEDER:
			m_growlog_breeder_ = text;
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_STRAINS_STRAIN_NAME:
			m_growlog_strain_ = text;
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY_CREATED:
			m_growlog_entry_created_on_ = text;
		default:
			break;
	}
} // MarkupParser::on_text()

void
MarkupParser::on_passthrough(Glib::Markup::ParseContext &context,
                             const Glib::ustring &text)
{
	switch (m_node_->element) {
		case MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_INFO:
			if (m_strain_)
				m_strain_->set_info(text);
			break;
		case MARKUP_GB_BREEDERS_BREEDER_STRAINS_STRAIN_DESCRIPTION:
			if (m_strain_)
				m_strain_->set_description(text);
			break;
		case MARKUP_GB_GROWLOGS_GROWLOG_DESCRIPTION:
			if (m_growlog_)
				m_growlog_->set_description(text);
		case MARKUP_GB_GROWLOGS_GROWLOG_ENTRIES_ENTRY_TEXT:
			m_growlog_entry_text_ = text;
		default:
			break;
	}
} // MarkupParser::on_passthrough()

void
MarkupParser::on_error(Glib::Markup::ParseContext &context,
                       const Glib::MarkupError &error)
{
}

/*******************************************************************************
 * XML_Importer
 ******************************************************************************/

XML_Importer::XML_Importer(const Glib::RefPtr<Database> &db,
                           const std::string &filename):
	Importer(db,filename)
{
}

XML_Importer::~XML_Importer()
{
}

Glib::RefPtr<XML_Importer>
XML_Importer::create(const Glib::RefPtr<Database> &db,
                     const std::string &filename)
{
	return Glib::RefPtr<XML_Importer>(new XML_Importer(db,filename));
}

void
XML_Importer::import_vfunc(Gtk::Window &parent)
{
	MarkupParser parser(parent,get_database());
	Glib::Markup::ParseContext context(parser);

	std::ifstream is(get_filename().c_str());
	if (is) {
		is.seekg(0, is.end);
		size_t len = is.tellg();
		is.seekg(0,is.beg);

		size_t bufsize = 100*1024*1024;
		if (len <= bufsize)
			bufsize = len;

		char *buf = new char[bufsize + 1];
		buf[bufsize] = '\0';

		do {
			if (bufsize < len) {
				is.read(buf,bufsize);
			} else {
				is.read(buf,len);
				buf[len]='\0';
			}
			len -= is.gcount();

			context.parse(buf);
		} while (is && len > 0);
	}
}
