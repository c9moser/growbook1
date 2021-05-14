//           growlogview.cc
//  Di Mai 11 16:41:03 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// growlogview.cc
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

#include "growlogview.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>

#include <gtkmm/separatortoolitem.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/messagedialog.h>

#include <cassert>

#include "application.h"
#include "growlogdialog.h"
#include "growlogentrydialog.h"

/*******************************************************************************
 * GrowlogViewStrainColumns
 ******************************************************************************/

GrowlogViewStrainColumns::GrowlogViewStrainColumns():
	Gtk::TreeModelColumnRecord{},
	column_id{},
	column_breeder{},
	column_name{}
{
	add(column_id);
	add(column_breeder);
	add(column_name);
}

GrowlogViewStrainColumns::~GrowlogViewStrainColumns()
{}

/*******************************************************************************
 * GrowlogViewEntryColumns
 ******************************************************************************/

GrowlogViewEntryColumns::GrowlogViewEntryColumns():
	Gtk::TreeModelColumnRecord{},
	column_id{},
	column_text{},
	column_datetime{},
	column_created_on{}
{
	add(column_id);
	add(column_text);
	add(column_datetime);
	add(column_created_on);
}

GrowlogViewEntryColumns::~GrowlogViewEntryColumns()
{}

/*******************************************************************************
 * GrowlogViewStrainView
 ******************************************************************************/

GrowlogViewStrainView::GrowlogViewStrainView(const Glib::RefPtr<Database> &db,
                                             const Glib::RefPtr<Growlog> &growlog):
	Gtk::TreeView{},
	columns{},
	m_database_{db},
	m_growlog_{growlog}
{
	assert(m_database_);
	assert(m_growlog_);

	set_model(_create_model());
	append_column(_("Breeder"),columns.column_breeder);
	append_column(_("Strain"),columns.column_name);

	show();
}

GrowlogViewStrainView::~GrowlogViewStrainView()
{
}

Glib::RefPtr<Gtk::ListStore>
GrowlogViewStrainView::_create_model()
{
	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(columns);
	
	std::list<Glib::RefPtr<Strain> > strains{m_database_->get_strains_for_growlog(m_growlog_)};

	for (auto iter = strains.begin(); iter != strains.end(); ++iter) {
		Glib::RefPtr<Strain> strain = *iter;
		Gtk::TreeModel::iterator model_iter = model->append();
		Gtk::TreeModel::Row row = *model_iter;
		
		row[columns.column_id] = strain->get_id();
		row[columns.column_breeder] = strain->get_breeder_name();
		row[columns.column_name] = strain->get_name();
	}

	return model;
}

Glib::RefPtr<Database>
GrowlogViewStrainView::get_database()
{
	return m_database_;
}

Glib::RefPtr<const Database>
GrowlogViewStrainView::get_database() const
{
	return Glib::RefPtr<const Database>::cast_const(m_database_);
}

Glib::RefPtr<Growlog>
GrowlogViewStrainView::get_growlog()
{
	return m_growlog_;
}

Glib::RefPtr<const Growlog>
GrowlogViewStrainView::get_growlog() const
{
	return Glib::RefPtr<const Growlog>::cast_const(m_growlog_);
}

void
GrowlogViewStrainView::set_growlog(const Glib::RefPtr<Growlog> &growlog)
{
	if (!growlog)
		return;
	
	if (m_growlog_ != growlog)
		m_growlog_ = growlog;
	
	set_model(_create_model());
	show();
}

/*******************************************************************************
 * GrowlogViewEntryView
 ******************************************************************************/

GrowlogViewEntryView::GrowlogViewEntryView(const Glib::RefPtr<Database> &db,
                                           const Glib::RefPtr<Growlog> &growlog):
	Gtk::TreeView{},
	columns{},
	m_database_{db},
	m_growlog_{growlog}
{
	assert(m_database_);
	assert(m_growlog_);

	set_model(_create_model());

	append_column("Created on",columns.column_datetime);
	append_column("Text",columns.column_text);
}

GrowlogViewEntryView::~GrowlogViewEntryView()
{
}

Glib::RefPtr<Gtk::ListStore>
GrowlogViewEntryView::_create_model()
{
	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(columns);

	std::list<Glib::RefPtr<GrowlogEntry> > entries{m_database_->get_growlog_entries(m_growlog_)};
	for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
		Glib::RefPtr<GrowlogEntry> entry = *iter;
		Gtk::TreeModel::iterator model_iter = model->append();
		Gtk::TreeModel::Row row = *model_iter;

		row[columns.column_id] = entry->get_id();
		row[columns.column_text] = entry->get_text();
		row[columns.column_created_on] = entry->get_created_on();
		
		time_t day = 24*60*60;
		Glib::ustring datetime = entry->get_created_on_format(app->get_settings()->get_datetime_format());
		datetime += "\n";
		datetime += _("Age [days]: ");
		datetime += std::to_string(entry->get_created_on()/day - m_growlog_->get_created_on()/day);
		if (m_growlog_->get_flower_on() && (entry->get_created_on() >= m_growlog_->get_flower_on())) {
			datetime += "\n";
			datetime += _("Flowering [days]: ");
			datetime += std::to_string(entry->get_created_on()/day - m_growlog_->get_flower_on()/day);
		}
		row[columns.column_datetime] = datetime; 
	}
	
	return model;
}

Glib::RefPtr<Database>
GrowlogViewEntryView::get_database()
{
	return m_database_;
}

Glib::RefPtr<const Database>
GrowlogViewEntryView::get_database() const
{
	return Glib::RefPtr<const Database>::cast_const(m_database_);
}

Glib::RefPtr<Growlog>
GrowlogViewEntryView::get_growlog()
{
	return m_growlog_;
}

Glib::RefPtr<const Growlog>
GrowlogViewEntryView::get_growlog() const
{
	return Glib::RefPtr<const Growlog>::cast_const(m_growlog_);
}

void
GrowlogViewEntryView::set_growlog(const Glib::RefPtr<Growlog> &growlog)
{
	if (!growlog)
		return;

	if (growlog != m_growlog_)
		m_growlog_ = growlog;
	
	set_model(_create_model());
	show();
}

/*******************************************************************************
 * GrowlogView
 ******************************************************************************/

const char GrowlogView::TYPE[] = "growbook-growlog";

GrowlogView::GrowlogView(const Glib::RefPtr<Database> &db,
                         const Glib::RefPtr<Growlog> &growlog):
	BrowserPage{db,TYPE},
	m_growlog_{growlog},
	m_refresh_button_{_("Refresh")},
	m_edit_button_{_("Edit")},
	m_add_logentry_button_{_("Add log-entry")},
	m_edit_logentry_button_{_("Edit log-entry")},
	m_remove_logentry_button_{_("Remove log-entry")},
	m_toolbar_{},
	m_strain_view_{db,growlog},
	m_entry_view_{db,growlog}
{
	assert(m_growlog_);

	// Toolbar /////////////////////////////////////////////////////////////////
	m_toolbar_.set_icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
	
	m_refresh_button_.signal_clicked().connect(sigc::mem_fun(*this,&GrowlogView::refresh));
	m_refresh_button_.set_icon_name("view-refresh");
	m_refresh_button_.set_tooltip_text(_("Refresh view."));
	m_toolbar_.append(m_refresh_button_);

	m_toolbar_.append(*Gtk::manage(new Gtk::SeparatorToolItem()));
	
	m_edit_button_.signal_clicked().connect(sigc::mem_fun(*this,&GrowlogView::on_edit));
	m_edit_button_.set_icon_name("gtk-edit");
	m_edit_button_.set_tooltip_text(_("Edit Growlog."));
	m_toolbar_.append(m_edit_button_);

	//m_flower_button_.signal_clicked().connect(sigc::mem_fun(*this,&GrowlogView::on_flower));
	m_flower_button_.set_icon_name("flower-icon");
	m_flower_button_.set_tooltip_text("Start flowering.");
	if (m_growlog_->get_flower_on())
		m_flower_button_.set_sensitive(false);
	m_toolbar_.append(m_flower_button_);

	//m_finish_button_.signal_clicked().connect(sigc::mem_fun(*this,&GrowlogView::on_finish));
	m_finish_button_.set_icon_name("process-stop");
	m_finish_button_.set_tooltip_text("Finish growlog.");
	if (m_growlog_->get_flower_on()) {
		m_finish_button_.set_sensitive(false);
	}
	m_toolbar_.append(m_finish_button_);
	
	
	m_toolbar_.append(*Gtk::manage(new Gtk::SeparatorToolItem()));

	m_add_logentry_button_.signal_clicked().connect(sigc::mem_fun(*this,&GrowlogView::on_add_logentry));
	m_add_logentry_button_.set_icon_name("list-add");
	m_add_logentry_button_.set_tooltip_text (_("Add Growlog-entry."));
	m_toolbar_.append(m_add_logentry_button_);

	m_edit_logentry_button_.signal_clicked().connect(sigc::mem_fun(*this,&GrowlogView::on_edit_logentry));
	m_edit_logentry_button_.set_icon_name("gtk-edit");
	m_edit_logentry_button_.set_tooltip_text(_("Edit selected Growlog-entry."));
	m_toolbar_.append(m_edit_logentry_button_);

	m_remove_logentry_button_.signal_clicked().connect(sigc::mem_fun(*this,&GrowlogView::on_remove_logentry));
	m_remove_logentry_button_.set_icon_name("list-remove");
	m_remove_logentry_button_.set_tooltip_text(_("Delete selected Growlog-entry."));
	m_toolbar_.append(m_remove_logentry_button_);

	Gtk::TreeModel::iterator iter = m_entry_view_.get_selection()->get_selected();
	if (!iter || m_growlog_->get_finished_on()) {
		m_edit_logentry_button_.set_sensitive(false);
		m_remove_logentry_button_.set_sensitive(false);
		
	}
	if (m_growlog_->get_finished_on()) {
		m_add_logentry_button_.set_sensitive(false);
	}
		
	pack_start(m_toolbar_,false,false,0);

	// ScrolledWindow //////////////////////////////////////////////////////////
	Gtk::Box *box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	Gtk::ScrolledWindow *scrolled = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	scrolled->add(*box);
	pack_start(*scrolled,true,true,0);

	// TextView ////////////////////////////////////////////////////////////////
	m_textview_.set_buffer(_create_textbuffer());
	m_textview_.set_wrap_mode(Gtk::WRAP_WORD);
	m_textview_.set_border_width (5);
	m_textview_.set_editable(false);
	box->pack_start(m_textview_,false,false,0);

	// StrainView //////////////////////////////////////////////////////////////
	box->pack_start(m_strain_view_,false,false,0);
	
	// EntryView ///////////////////////////////////////////////////////////////
	m_entry_view_.get_selection()->signal_changed().connect(sigc::mem_fun(*this,&GrowlogView::on_entry_view_selection_changed));
	box->pack_start(m_entry_view_,true,true,0);

	show_all();	
}

GrowlogView::~GrowlogView()
{
}

Glib::RefPtr<Gtk::TextBuffer>
GrowlogView::_create_textbuffer()
{
	Glib::RefPtr<Gtk::TextBuffer> buffer = Gtk::TextBuffer::create();
	Glib::RefPtr<Gtk::TextTagTable> tag_table = buffer->get_tag_table();
	
	Glib::RefPtr<Gtk::TextTag> title_tag = Gtk::TextTag::create("title");
	title_tag->property_weight() = PANGO_WEIGHT_BOLD;
	title_tag->property_scale() = 3.0;
	tag_table->add(title_tag);
	
	Glib::RefPtr<Gtk::TextTag> paragraph_tag = Gtk::TextTag::create("paragraph");
	paragraph_tag->property_weight() = PANGO_WEIGHT_BOLD;
	paragraph_tag->property_scale() = 2.0;
	tag_table->add(paragraph_tag);
	
	Glib::RefPtr<Gtk::TextTag> bold_tag = Gtk::TextTag::create("bold");
	bold_tag->property_weight() = PANGO_WEIGHT_BOLD;
	tag_table->add(bold_tag);

	buffer->insert_with_tag(buffer->begin(),m_growlog_->get_title(),title_tag);
	buffer->insert(buffer->end(),"\n\n");

	buffer->insert_with_tag(buffer->end(),_("Created on: "),bold_tag);
	buffer->insert(buffer->end(),m_growlog_->get_created_on_format(app->get_settings()->get_datetime_format()));
	buffer->insert(buffer->end(),"\n");

	time_t current_time = time(nullptr);
	time_t day = 24*60*60;
	

	if (m_growlog_->get_flower_on()) {
		buffer->insert_with_tag(buffer->end(),_("Started flowering on: "),bold_tag);
		buffer->insert(buffer->end(),m_growlog_->get_flower_on_format(app->get_settings()->get_date_format()));
		buffer->insert(buffer->end(),"\n");
		buffer->insert_with_tag(buffer->end(),_("Flowering-days: "),bold_tag);
		time_t flowering;
		if (m_growlog_->get_finished_on()) {
			flowering = (m_growlog_->get_finished_on()/day - m_growlog_->get_flower_on()/day);
		} else {
			flowering = (current_time/day - m_growlog_->get_flower_on()/day);
		}
		buffer->insert(buffer->end(),std::to_string(flowering));
		buffer->insert(buffer->end(),"\n");
	}

	time_t age;
	if (m_growlog_->get_finished_on()) {
		age = (m_growlog_->get_finished_on()/day - m_growlog_->get_created_on()/day);

		buffer->insert_with_tag(buffer->end(),_("Finished on: "),bold_tag);
		buffer->insert(buffer->end(),m_growlog_->get_finished_on_format(app->get_settings()->get_datetime_format()));
		buffer->insert(buffer->end(),"\n");
	} else {
		age = current_time/day - m_growlog_->get_created_on()/day;
	}
	buffer->insert_with_tag(buffer->end(),_("Age: "),bold_tag);
	buffer->insert(buffer->end(),std::to_string(age));
	buffer->insert(buffer->end(),_(" day(s)"));
	buffer->insert(buffer->end(),"\n\n");

	buffer->insert_with_tag(buffer->end(),_("Description"),paragraph_tag);
	buffer->insert(buffer->end(),"\n");
	buffer->insert(buffer->end(),m_growlog_->get_description());
	buffer->insert(buffer->end(),"\n");
	return buffer;
}

uint64_t
GrowlogView::get_id_vfunc() const
{
	return m_growlog_->get_id();
}

Glib::ustring
GrowlogView::get_title_vfunc() const
{
	return m_growlog_->get_title();
}

void
GrowlogView::on_refresh()
{
	Glib::RefPtr<Growlog> growlog = get_database()->get_growlog(m_growlog_->get_id());
	if (growlog)
		m_growlog_ = growlog;

	if (m_growlog_->get_finished_on()) {
		m_add_logentry_button_.set_sensitive(false);
		m_edit_logentry_button_.set_sensitive(false);
		m_remove_logentry_button_.set_sensitive(false);
	} else {
		m_add_logentry_button_.set_sensitive(true);
	}
	m_textview_.set_buffer(_create_textbuffer());
	m_strain_view_.set_growlog(m_growlog_);
	m_entry_view_.set_growlog(m_growlog_);
	on_entry_view_selection_changed();
	show_all();
}

void
GrowlogView::on_entry_view_selection_changed()
{
	Gtk::TreeModel::iterator iter = m_entry_view_.get_selection()->get_selected();
	if (!m_growlog_->get_finished_on() && iter) {
		m_edit_logentry_button_.set_sensitive(true);
		m_remove_logentry_button_.set_sensitive(true);
	} else {
		m_edit_logentry_button_.set_sensitive(false);
		m_remove_logentry_button_.set_sensitive(false);
	}	
}

void
GrowlogView::on_edit()
{
	AppWindow *window = dynamic_cast<AppWindow*>(get_toplevel());
	GrowlogDialog dialog{*window,get_database(),m_growlog_};
	int response = dialog.run();

	if (response == Gtk::RESPONSE_APPLY) {
		refresh();
	}
}

void
GrowlogView::on_add_logentry()
{
	Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();
	
	GrowlogEntryDialog dialog(*window,get_database(),m_growlog_);
	int response = dialog.run();
	
	if (response == Gtk::RESPONSE_APPLY) {
		refresh();
	}
}

void
GrowlogView::on_edit_logentry()
{
	Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();

	Gtk::TreeModel::iterator iter = m_entry_view_.get_selection()->get_selected();
	if (!iter)
		return;
	Gtk::TreeModel::Row row = *iter;

	Glib::RefPtr<GrowlogEntry> entry = GrowlogEntry::create(row[m_entry_view_.columns.column_id],
	                                                        m_growlog_->get_id(),
	                                                        row[m_entry_view_.columns.column_text],
	                                                        row[m_entry_view_.columns.column_created_on]);
	GrowlogEntryDialog dialog(*window,get_database(),entry);
	int response = dialog.run();
	if (response == Gtk::RESPONSE_APPLY) {
		refresh();
	}
}

void
GrowlogView::on_remove_logentry()
{
	Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();

	
	Gtk::TreeModel::iterator iter = m_entry_view_.get_selection()->get_selected();
	if (!iter)
		return;

	Gtk::MessageDialog dialog{*window,
	                          _("Do you really want to delete the selected growlog-entry?"),
	                          false,
	                          Gtk::MESSAGE_QUESTION,
	                          Gtk::BUTTONS_YES_NO,
	                          true};
	int response = dialog.run();
	if (response == Gtk::RESPONSE_YES) {
		Gtk::TreeModel::Row row = *iter;
		get_database()->remove_growlog_entry(row[m_entry_view_.columns.column_id]);
		refresh();
	}
	
}

                              
