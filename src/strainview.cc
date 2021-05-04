//           strainview.cc
//  Sa Mai 01 18:24:04 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// strainview.cc
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

#include "strainview.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include <cassert>
#include <gtkmm/texttag.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/separatortoolitem.h>
#include <gtk/gtk.h>

#include "application.h"

const char StrainView::TYPE[] = "growbook-strain";

StrainView::StrainView(const Glib::RefPtr<Database> &db,
                       const Glib::RefPtr<Strain> &strain):
	BrowserPage{db,TYPE},
	m_strain_{strain},
	m_toolbar_{},
	m_breeder_homepage_button_{},
	m_homepage_button_{},
	m_seedfinder_button_{},
	m_refresh_button_{},
	m_textview_{}
{
	assert(m_strain_);

	m_toolbar_.set_icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);

	Glib::RefPtr<Breeder> breeder = get_database()->get_breeder(m_strain_->get_breeder_id());
	m_breeder_homepage_button_.set_icon_name("go-home");
	m_breeder_homepage_button_.set_tooltip_text(_("Breeder homepage"));
	m_breeder_homepage_button_.signal_clicked().connect(sigc::mem_fun(*this,&StrainView::on_breeder_homepage_clicked));
	if (breeder->get_homepage().empty())
		m_breeder_homepage_button_.set_sensitive(false);
	m_toolbar_.append(m_breeder_homepage_button_);

	m_toolbar_.append(*Gtk::manage(new Gtk::SeparatorToolItem()));
	
	m_homepage_button_.set_icon_name("go-home");
	m_homepage_button_.set_tooltip_text(_("Strain homepage"));
	m_homepage_button_.signal_clicked().connect(sigc::mem_fun(*this,&StrainView::on_homepage_clicked));
	if (m_strain_->get_homepage().empty())
		m_homepage_button_.set_sensitive(false);
	m_toolbar_.append(m_homepage_button_);

	m_seedfinder_button_.signal_clicked().connect(sigc::mem_fun(*this,&StrainView::on_seedfinder_clicked));
	m_seedfinder_button_.set_tooltip_text(_("Seedfinder.eu webpage"));
	if (m_strain_->get_seedfinder ().empty())
		m_seedfinder_button_.set_sensitive(false);
	m_toolbar_.append(m_seedfinder_button_);

	m_toolbar_.append(*Gtk::manage(new Gtk::SeparatorToolItem()));

	m_refresh_button_.set_icon_name("view-refresh");
	m_refresh_button_.signal_clicked().connect(sigc::mem_fun(*this,&BrowserPage::refresh));
	m_toolbar_.append(m_refresh_button_);
	
	pack_start(m_toolbar_,false,false,0);
	
	m_textview_.set_wrap_mode(Gtk::WRAP_WORD);
	m_textview_.set_editable(false);
	m_textview_.set_buffer(_create_textbuffer());
	m_textview_.set_border_width(5);

	Gtk::ScrolledWindow *scrolled = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	scrolled->add(m_textview_);
	pack_start(*scrolled,true,true,0);
}

StrainView::~StrainView()
{}

Glib::RefPtr<Gtk::TextBuffer>
StrainView::_create_textbuffer()
{
	Glib::RefPtr<Gtk::TextBuffer> buffer = Gtk::TextBuffer::create();
	Glib::RefPtr<Gtk::TextTagTable> tagtable = buffer->get_tag_table();

	Glib::RefPtr<Gtk::TextTag> tag_title = Gtk::TextTag::create("title");
	tag_title->property_scale() = 3.0;
	tag_title->property_weight() = PANGO_WEIGHT_BOLD;
	tagtable->add(tag_title);

	Glib::RefPtr<Gtk::TextTag> tag_paragraph = Gtk::TextTag::create("paragraph");
	tag_paragraph->property_scale() = 2.0;
	tag_paragraph->property_weight() = PANGO_WEIGHT_BOLD;
	tagtable->add(tag_paragraph);

	Glib::RefPtr<Gtk::TextTag> tag_bold = Gtk::TextTag::create("bold");
	tag_bold->property_weight() = PANGO_WEIGHT_BOLD;
	tagtable->add(tag_bold);

	buffer->insert_with_tag (buffer->begin(),get_title(),tag_title);
	buffer->insert(buffer->end(),"\n\n");
	if (!m_strain_->get_homepage().empty()) {
		buffer->insert_with_tag(buffer->end(),_("Homepage: "),tag_bold);
		buffer->insert(buffer->end(),m_strain_->get_homepage());
		buffer->insert(buffer->end(),"\n");
	}
	
	if (!m_strain_->get_seedfinder().empty()) {
		buffer->insert_with_tag(buffer->end(), _("SeedFinder.eu: "),tag_bold);
		buffer->insert(buffer->end(),m_strain_->get_seedfinder());
		buffer->insert(buffer->end(),"\n");
	}

	if (!m_strain_->get_info().empty()) {
		buffer->insert_with_tag(buffer->end(), _("Info\n"), tag_paragraph);
		buffer->insert(buffer->end(), m_strain_->get_info());
		buffer->insert(buffer->end(), "\n");
	}

	if (!m_strain_->get_description().empty()) {
		buffer->insert_with_tag(buffer->end(), _("Description\n"), tag_paragraph);
		buffer->insert(buffer->end(),m_strain_->get_description());
		buffer->insert(buffer->end(),"\n");
	}
	
	return buffer;
}

Glib::RefPtr<Strain>
StrainView::get_strain()
{
	return m_strain_;
}

Glib::RefPtr<const Strain>
StrainView::get_strain() const
{
	return Glib::RefPtr<const Strain>::cast_const(m_strain_);
}

uint64_t
StrainView::get_id_vfunc() const
{
	return m_strain_->get_id();
}

Glib::ustring
StrainView::get_title_vfunc() const
{
	Glib::ustring title = m_strain_->get_breeder_name();
	title += " - ";
	title += m_strain_->get_name();

	return title;
}

void
StrainView::on_refresh()
{
	Glib::RefPtr<Strain> strain = get_database()->get_strain(strain->get_id());
	if (strain)
		m_strain_ = strain;

	Glib::RefPtr<Breeder> breeder = get_database()->get_breeder(m_strain_->get_breeder_id());
	if (breeder && !breeder->get_homepage().empty()) {
		m_breeder_homepage_button_.set_sensitive(true);
	} else {
		m_breeder_homepage_button_.set_sensitive(false);
	}

	if (!m_strain_->get_homepage().empty()) {
		m_homepage_button_.set_sensitive(true);
	} else {
		m_homepage_button_.set_sensitive(false);
	}

	if (!m_strain_->get_seedfinder().empty()) {
		m_seedfinder_button_.set_sensitive(true);
	} else {
		m_seedfinder_button_.set_sensitive(false);
	}
	m_textview_.set_buffer(_create_textbuffer());
	title_changed();
	show_all();
}

void
StrainView::on_homepage_clicked()
{
	Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();
	GtkWindow *gtk_window = window->gobj();
	GError *error = nullptr;
	std::string uri = m_strain_->get_homepage();
	gtk_show_uri_on_window(gtk_window,
	                       uri.c_str(),
	                       gtk_get_current_event_time(),
	                       &error);
	if (error) {
		Gtk::MessageDialog dialog{*window,
		                          _("Unable to open URI!"),
		                          false,
		                          Gtk::MESSAGE_ERROR,
		                          Gtk::BUTTONS_OK,
		                          true};
		dialog.set_secondary_text (error->message);
		g_error_free(error);
		dialog.run();
		dialog.hide();
	}
}

void
StrainView::on_seedfinder_clicked()
{
	Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();
	GtkWindow *gtk_window = window->gobj();
	GError *error = nullptr;
	std::string uri = m_strain_->get_seedfinder();
	gtk_show_uri_on_window(gtk_window,
	                       uri.c_str(),
	                       gtk_get_current_event_time(),
	                       &error);
	if (error) {
		Gtk::MessageDialog dialog{*window,
		                          _("Unable to open URI!"),
		                          false,
		                          Gtk::MESSAGE_ERROR,
		                          Gtk::BUTTONS_OK,
		                          true};
		dialog.set_secondary_text (error->message);
		g_error_free(error);
		dialog.run();
		dialog.hide();
	}
}

void
StrainView::on_breeder_homepage_clicked()
{
	Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();
	GtkWindow *gtk_window = window->gobj();
	GError *error = nullptr;
	Glib::RefPtr<Breeder> breeder = get_database()->get_breeder(m_strain_->get_breeder_id());
	if (!breeder)
		return;
	
	std::string uri = breeder->get_homepage();
	gtk_show_uri_on_window(gtk_window,
	                       uri.c_str(),
	                       gtk_get_current_event_time(),
	                       &error);
	if (error) {
		Gtk::MessageDialog dialog{*window,
		                          _("Unable to open URI!"),
		                          false,
		                          Gtk::MESSAGE_ERROR,
		                          Gtk::BUTTONS_OK,
		                          true};
		dialog.set_secondary_text (error->message);
		g_error_free(error);
		dialog.run();
		dialog.hide();
	}
}
