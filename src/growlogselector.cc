//           growlogselector.cc
//  Mo Mai 10 17:01:26 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// growlogselector.cc
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

#include "growlogselector.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/messagedialog.h>

#include "growlogview.h"
#include "application.h"
#include "growlogdialog.h"

/*******************************************************************************
 * GrowlogSelectorColumns
 ******************************************************************************/

GrowlogSelectorColumns::GrowlogSelectorColumns()
{
	add(column_id);
	add(column_title);
}

GrowlogSelectorColumns::~GrowlogSelectorColumns()
{
}

/*******************************************************************************
 * GrowlogSelectorTreeView
 ******************************************************************************/

GrowlogSelectorTreeView::GrowlogSelectorTreeView(const Glib::RefPtr<Database> &db):
	Gtk::TreeView{},
	columns{},
	m_database_{db},
	m_refresh_menuitem_{_("Refresh")},
	m_open_menuitem_{_("Open")},
	m_new_menuitem_{_("New Growlog")},
	m_edit_menuitem_{_("Edit")},
	m_delete_menuitem_{_("Delete")},
	m_popup_menu_{}
{
	set_model(_create_model());
	append_column (_("Title"),columns.column_title);
	set_headers_visible (false);

	m_popup_menu_.attach_to_widget(*this);

	m_refresh_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&GrowlogSelectorTreeView::refresh));
	m_popup_menu_.append(m_refresh_menuitem_);

	m_popup_menu_.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));

	m_open_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&GrowlogSelectorTreeView::on_open));
	m_popup_menu_.append(m_open_menuitem_);
	
	m_popup_menu_.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	
	m_new_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&GrowlogSelectorTreeView::on_new));
	m_popup_menu_.append(m_new_menuitem_);
	
	m_edit_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&GrowlogSelectorTreeView::on_edit));
	m_popup_menu_.append(m_edit_menuitem_);

	m_delete_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&GrowlogSelectorTreeView::on_delete));
	m_popup_menu_.append(m_delete_menuitem_);
	
	m_popup_menu_.show_all();
}

GrowlogSelectorTreeView::~GrowlogSelectorTreeView()
{
}

Glib::RefPtr<Gtk::TreeStore>
GrowlogSelectorTreeView::_create_model()
{
	Glib::RefPtr<Gtk::TreeStore> model = Gtk::TreeStore::create(columns);

	Gtk::TreeModel::iterator parent_iter = model->append();
	Gtk::TreeModel::Row parent_row = *parent_iter;

	// ongoing growlogs
	parent_row[columns.column_id] = 0;
	parent_row[columns.column_title] = _("Ongoing Growlogs");
	std::list<Glib::RefPtr<Growlog> > ongoing_growlogs{m_database_->get_ongoing_growlogs()};
	for (auto gl_iter = ongoing_growlogs.begin(); gl_iter != ongoing_growlogs.end(); ++gl_iter) {
		Glib::RefPtr<Growlog> growlog = *gl_iter;
		Gtk::TreeModel::iterator iter = model->append(parent_row.children());
		Gtk::TreeModel::Row row = *iter;
		row[columns.column_id] = growlog->get_id();
		row[columns.column_title] = growlog->get_title();
	}
	ongoing_growlogs.clear();

	// finished growlogs
	parent_iter = model->append();
	parent_row = *parent_iter;
	parent_row[columns.column_id] = 0;
	parent_row[columns.column_title] = _("Finished Growlogs");
	std::list<Glib::RefPtr<Growlog> > finished_growlogs{m_database_->get_finished_growlogs()};
	for (auto gl_iter = finished_growlogs.begin(); gl_iter != finished_growlogs.end(); ++gl_iter) {
		Glib::RefPtr<Growlog> growlog = *gl_iter;
		Gtk::TreeModel::iterator iter = model->append(parent_row.children());
		Gtk::TreeModel::Row row = *iter;
		row[columns.column_id] = growlog->get_id();
		row[columns.column_title] = growlog->get_title();
	}
	finished_growlogs.clear();

	// growlogs per strain
	parent_iter = model->append();
	parent_row = *parent_iter;
	parent_row[columns.column_id] = 0;
	parent_row[columns.column_title] = _("Strains");

	std::list<Glib::RefPtr<Breeder> > breeders = m_database_->get_breeders();
	for (auto b_iter = breeders.begin(); b_iter != breeders.end(); ++b_iter) {
		Glib::RefPtr<Breeder> breeder = *b_iter;
		Gtk::TreeModel::iterator breeder_iter;
		
		std::list<Glib::RefPtr<Strain> > strains{m_database_->get_strains_for_breeder(breeder)};
		for (auto s_iter = strains.begin(); s_iter != strains.end(); ++s_iter) {
			Glib::RefPtr<Strain> strain = *s_iter;
			Gtk::TreeModel::iterator strain_iter;
			
			std::list<Glib::RefPtr<Growlog> > growlogs{m_database_->get_growlogs_for_strain(strain)};
			for (auto gl_iter = growlogs.begin(); gl_iter != growlogs.end(); ++gl_iter) {
				Glib::RefPtr<Growlog> growlog = *gl_iter;
				if (!breeder_iter) {
					breeder_iter = model->append(parent_row.children());
					Gtk::TreeModel::Row row = *breeder_iter;
					row[columns.column_id] = 0;
					row[columns.column_title] = breeder->get_name(); 
				}
				if (!strain_iter) {
					strain_iter = model->append(breeder_iter->children());
					Gtk::TreeModel::Row row = *strain_iter;
					row[columns.column_id] = 0;
					row[columns.column_title] = strain->get_name();
				}
				Gtk::TreeModel::iterator iter = model->append(strain_iter->children());
				Gtk::TreeModel::Row row = *iter;
				row[columns.column_id] = growlog->get_id();
				row[columns.column_title] = growlog->get_title();
			}
		}
	}
	
	return model;
}

void
GrowlogSelectorTreeView::refresh()
{
	set_model(_create_model());
	show();
}

Glib::RefPtr<Database>
GrowlogSelectorTreeView::get_database()
{
	return m_database_;
}

Glib::RefPtr<const Database>
GrowlogSelectorTreeView::get_database() const
{
	return Glib::RefPtr<const Database>::cast_const(m_database_);
}

bool
GrowlogSelectorTreeView::on_button_press_event(GdkEventButton *event) 
{
	bool ret = Gtk::TreeView::on_button_press_event(event);
	
	if (event->button == 3 && event->type == GDK_BUTTON_PRESS) {
		Gtk::TreeModel::iterator iter = get_selection()->get_selected();
		if (iter && (*iter)[columns.column_id]) {
			m_open_menuitem_.set_sensitive(true);
			m_edit_menuitem_.set_sensitive(true);
			m_delete_menuitem_.set_sensitive (true);
		} else {
			m_open_menuitem_.set_sensitive(false);
			m_edit_menuitem_.set_sensitive(false);
			m_delete_menuitem_.set_sensitive (false);
		}

		m_popup_menu_.popup(event->button,event->time);
	}
	return ret;
}

void
GrowlogSelectorTreeView::on_row_activated(const Gtk::TreeModel::Path &path,
                                          Gtk::TreeViewColumn *column)
{
	Glib::RefPtr<Gtk::TreeStore> model = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(get_model());
	Gtk::TreeModel::iterator iter = model->get_iter(path);
	Gtk::TreeModel::Row row = *iter;
	uint64_t id = row[columns.column_id];
	if (id) {
		Glib::RefPtr<Growlog> growlog = m_database_->get_growlog(id);
		if (!growlog) 
			return;

		GrowlogView *glv = Gtk::manage(new GrowlogView(m_database_,growlog));
		AppWindow *window = dynamic_cast<AppWindow*>(get_toplevel());
		if (!window)
			app->get_appwindow();

		if (window->add_browser_page(*glv) == -1) 
			delete glv;
	}
}

void
GrowlogSelectorTreeView::on_open()
{
	Gtk::TreeModel::iterator iter = get_selection()->get_selected();
	if (!iter)
		return;
	Gtk::TreeModel::Row row = *iter;
	uint64_t id = row[columns.column_id];
	if (!id)
		return;

	Glib::RefPtr<Growlog> growlog = m_database_->get_growlog(id);
	if (!growlog)
		return;
	
	AppWindow *window = dynamic_cast<AppWindow*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();

	GrowlogView *glv = Gtk::manage(new GrowlogView(m_database_,growlog));
	if (window->add_browser_page(*glv) == -1)
		delete glv;
}

void
GrowlogSelectorTreeView::on_new()
{
	AppWindow *window = dynamic_cast<AppWindow*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();
	
	GrowlogDialog dialog{*window,m_database_};
	int response = dialog.run();

	if (response == Gtk::RESPONSE_APPLY) {
		Glib::RefPtr<Growlog> growlog = dialog.get_growlog();
		if (growlog) {
			GrowlogView *glv = Gtk::manage(new GrowlogView(m_database_,growlog));
			if (window->add_browser_page(*glv) == -1)
			    delete glv;
		}
		refresh();
	}
}

void
GrowlogSelectorTreeView::on_edit()
{
	Gtk::TreeModel::iterator iter = get_selection()->get_selected();
	if (!iter)
		return;
	Gtk::TreeModel::Row row = *iter;
	uint64_t id = row[columns.column_id];
	
	if (!id)
		return;
	Glib::RefPtr<Growlog> growlog = m_database_->get_growlog(id);
	if (!growlog)
		return;
	
	AppWindow *window = dynamic_cast<AppWindow*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();
	 	
	GrowlogDialog dialog(*window,m_database_,growlog);
	int response = dialog.run();
	dialog.hide();
	
	if (response == Gtk::RESPONSE_APPLY) {
		growlog = dialog.get_growlog();
		GrowlogView *glv = Gtk::manage(new GrowlogView(m_database_,growlog));
		if (window->add_browser_page (*glv) == -1)
			delete glv;
		refresh();
	}
}

void
GrowlogSelectorTreeView::on_delete()
{
	Gtk::TreeModel::iterator iter = get_selection()->get_selected();
	if (!iter || !(*iter)[columns.column_id])
		return;
	Gtk::TreeModel::Row row = *iter;
	uint64_t id = row[columns.column_id];
	
 	Glib::RefPtr<Growlog> growlog = m_database_->get_growlog(id);
	
	AppWindow *window = dynamic_cast<AppWindow*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();

	
	Gtk::MessageDialog dialog(*window,
	                          _("Do you really want to delete the selected  growlog?"),
	                          false,
	                          Gtk::MESSAGE_QUESTION,
	                          Gtk::BUTTONS_YES_NO,
	                          true);
	int response = dialog.run();
	if (response == Gtk::RESPONSE_YES) {
		std::list<Glib::RefPtr<Strain> > strains{m_database_->get_strains_for_growlog(growlog)};
		for (auto iter = strains.begin(); iter != strains.end(); ++iter) {
			m_database_->remove_strain_for_growlog (growlog,*iter);
		}
		strains.clear();

		std::list<Glib::RefPtr<GrowlogEntry> > entries{m_database_->get_growlog_entries (growlog)};
		for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
			m_database_->remove_growlog_entry(*iter);
		}
		entries.clear();

		m_database_->remove_growlog(growlog);
		refresh();
	}
}

/*******************************************************************************
 * GrowlogSelector
 ******************************************************************************/

GrowlogSelector::GrowlogSelector(const Glib::RefPtr<Database> &db):
	Gtk::ScrolledWindow{},
	m_treeview_{db}
{
	add(m_treeview_);
	show();
}

GrowlogSelector::~GrowlogSelector()
{
}

GrowlogSelectorTreeView*
GrowlogSelector::get_treeview()
{
	return &m_treeview_;
}

const GrowlogSelectorTreeView*
GrowlogSelector::get_treeview () const
{
	return &m_treeview_;
}

Glib::RefPtr<Database>
GrowlogSelector::get_database()
{
	return m_treeview_.get_database();
}

Glib::RefPtr<const Database>
GrowlogSelector::get_database() const
{
	return m_treeview_.get_database();
}

void
GrowlogSelector::refresh()
{
	m_treeview_.refresh();
	show_all();
}
