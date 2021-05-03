//           strainselector.cc
//  Sa Mai 01 15:09:58 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// strainselector.cc
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

#include "strainselector.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>
#include <gtkmm/separatormenuitem.h>

#include "strainview.h"
#include "application.h"
#include "breederdialog.h" 


/*******************************************************************************
 * StrainSelectorColumns
 ******************************************************************************/
StrainSelectorColumns::StrainSelectorColumns():
	Gtk::TreeModelColumnRecord{},
	column_id{},
	column_breeder_id{},
	column_name{}
{
	add(column_id);
	add(column_breeder_id);
	add(column_name);
}

StrainSelectorColumns::~StrainSelectorColumns()
{}

/*******************************************************************************
 * StrainSelectorTreeView
 ******************************************************************************/
StrainSelectorTreeView::StrainSelectorTreeView(const Glib::RefPtr<Database> &db):
	Gtk::TreeView{},
	columns{},
	m_database_{db},
	m_refresh_menuitem_{_("Refresh")},
	m_open_menuitem_{_("Open")},
	m_add_breeder_menuitem_{_("Add Breeder")},
	m_add_strain_menuitem_{_("Add Strain")},
	m_edit_menuitem_{_("Edit")},
	m_delete_menuitem_{_("Delete")},
	m_popup_menu_{}
{
	set_model(_create_model());
	append_column ("Name",columns.column_name);
	set_headers_visible(false);
	
	m_popup_menu_.attach_to_widget(*this);

	m_refresh_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&StrainSelectorTreeView::refresh));
	m_popup_menu_.append(m_refresh_menuitem_);

	m_popup_menu_.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	
	m_open_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&StrainSelectorTreeView::on_open));
	m_popup_menu_.append(m_open_menuitem_);
	
	m_add_breeder_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&StrainSelectorTreeView::on_add_breeder));
	m_popup_menu_.append(m_add_breeder_menuitem_);

	m_add_strain_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&StrainSelectorTreeView::on_add_strain));
	m_popup_menu_.append(m_add_strain_menuitem_);

	m_popup_menu_.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));

	m_edit_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&StrainSelectorTreeView::on_edit));
	m_popup_menu_.append(m_edit_menuitem_);

	m_popup_menu_.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));

	m_delete_menuitem_.signal_activate().connect(sigc::mem_fun(*this,&StrainSelectorTreeView::on_delete));
	m_popup_menu_.append(m_delete_menuitem_);
	m_popup_menu_.show_all();
}

StrainSelectorTreeView::~StrainSelectorTreeView()
{
}

Glib::RefPtr<Gtk::TreeStore>
StrainSelectorTreeView::_create_model()
{
	Glib::RefPtr<Gtk::TreeStore> model = Gtk::TreeStore::create(columns);

	std::list<Glib::RefPtr<Breeder> > breeders{m_database_->get_breeders()};
	for (auto breeder_iter = breeders.begin(); breeder_iter != breeders.end(); ++breeder_iter) {
		Gtk::TreeModel::iterator model_iter = model->append();
		Gtk::TreeModel::Row row = *model_iter;
		row[columns.column_id] = 0;
		row[columns.column_breeder_id] = (*breeder_iter)->get_id();
		row[columns.column_name] = (*breeder_iter)->get_name();
		
		std::list<Glib::RefPtr<Strain> > strains{m_database_->get_strains_for_breeder(*breeder_iter)};
		for (auto strain_iter = strains.begin(); strain_iter != strains.end(); ++strain_iter) {
			Gtk::TreeModel::iterator child_iter = model->append(model_iter->children());
			Gtk::TreeModel::Row row = *child_iter;
			Glib::RefPtr<Strain> strain = *strain_iter;
			row[columns.column_id] = strain->get_id();
			row[columns.column_breeder_id] = strain->get_breeder_id();
			row[columns.column_name] = strain->get_name();
		}
	}
	return model;
}

Glib::RefPtr<Database>
StrainSelectorTreeView::get_database()
{
	return m_database_;
}

Glib::RefPtr<const Database>
StrainSelectorTreeView::get_database() const
{
	return Glib::RefPtr<const Database>::cast_const(m_database_);
}

void
StrainSelectorTreeView::refresh()
{
	set_model(this->_create_model());
	show();
}

void
StrainSelectorTreeView::on_row_activated(const Gtk::TreeModel::Path &path,
                                         Gtk::TreeViewColumn *column)
{
	Gtk::TreeView::on_row_activated (path,column);
	
	Gtk::TreeModel::iterator iter = get_model()->get_iter(path);
	Gtk::TreeModel::Row row = *iter;

	uint64_t id = row[columns.column_id];

	if (id) {
		Glib::RefPtr<Strain> strain = m_database_->get_strain(id);
		if (strain) {
			StrainView *strain_view = Gtk::manage(new StrainView(m_database_,strain));
			AppWindow *appwindow = dynamic_cast<AppWindow*>(get_toplevel());
			if (!appwindow)
				appwindow = app->get_appwindow();
			
			int pagenum = appwindow->add_browser_page(*strain_view);
			if (pagenum == -1)
				delete strain_view;
		}
	}
}

bool
StrainSelectorTreeView::on_button_press_event(GdkEventButton *event)
{
	bool ret = Gtk::TreeView::on_button_press_event (event);
	if (event->button == 3 && event->type == GDK_BUTTON_PRESS) {
		Gtk::TreeModel::iterator iter = get_selection()->get_selected();
		if (iter) {
			Gtk::TreeModel::Row row = *iter;
			if (row[columns.column_breeder_id]) {
				m_add_strain_menuitem_.set_sensitive(true);
			} else {
				m_add_strain_menuitem_.set_sensitive(false);
			}
			if (row[columns.column_breeder_id] || row[columns.column_id]) {
				m_edit_menuitem_.set_sensitive(true);
				m_delete_menuitem_.set_sensitive(true);
			} else {
				m_edit_menuitem_.set_sensitive(false);
				m_delete_menuitem_.set_sensitive(false);
			}
			if (row[columns.column_id]) {
				m_open_menuitem_.set_sensitive(true);
			} else {
				m_open_menuitem_.set_sensitive(false);
			}
		} else {
			m_open_menuitem_.set_sensitive(false);
			m_add_strain_menuitem_.set_sensitive(false);
			m_edit_menuitem_.set_sensitive(false);
			m_delete_menuitem_.set_sensitive(false);
		}
		m_popup_menu_.popup(event->button,event->time);
	}
	return ret;
}

void
StrainSelectorTreeView::on_open()
{
	Gtk::TreeModel::iterator iter = get_selection()->get_selected();
	if (!iter || !(*iter)[columns.column_id])
		return;

	uint64_t id = (*iter)[columns.column_id];
	Glib::RefPtr<Strain> strain = m_database_->get_strain(id);
	if (!strain)
		return;

	AppWindow *window = dynamic_cast<AppWindow*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();
	
	StrainView *sv = Gtk::manage(new StrainView(m_database_,strain));
	int pagenum = window->add_browser_page(*sv);
	if (pagenum == -1)
		delete sv;
}

void
StrainSelectorTreeView::on_add_breeder()
{
	Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();
	
	BreederDialog dialog{*window,m_database_};
	if (dialog.run() == Gtk::RESPONSE_APPLY) {
		refresh();
	}		
	dialog.hide();
}

void
StrainSelectorTreeView::on_add_strain()
{
}

void
StrainSelectorTreeView::on_edit()
{
	Gtk::TreeModel::iterator iter = get_selection()->get_selected();
	if (!iter)
		return;

	Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_toplevel());
	if (!window)
		window = app->get_appwindow();
	
	Gtk::TreeModel::Row row = *iter;
	if (row[columns.column_id]) {
		//TODO
	} else if (row[columns.column_breeder_id]) {
		Glib::RefPtr<Breeder> breeder = m_database_->get_breeder(row[columns.column_breeder_id]);
		if (!breeder) return;

		BreederDialog dialog{*window,m_database_,breeder};
		int response = dialog.run();
		dialog.hide();
		if (response == Gtk::RESPONSE_APPLY) {
			refresh();
		}
	}
}

void
StrainSelectorTreeView::on_delete()
{
	Gtk::TreeModel::iterator iter = get_selection()->get_selected();
	if (!iter)
		return;
	Gtk::TreeModel::Row row = *iter;
}
                                  
/*******************************************************************************
 * StrainSelector
 ******************************************************************************/

StrainSelector::StrainSelector(const Glib::RefPtr<Database> &db):
	Gtk::ScrolledWindow{},
	m_treeview_{db}
{
	set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
	add(m_treeview_);
}

StrainSelector::~StrainSelector()
{
}

StrainSelector::TreeView*
StrainSelector::get_treeview()
{
	return &m_treeview_;
}

const StrainSelector::TreeView*
StrainSelector::get_treeview() const
{
	return &m_treeview_;
}

Glib::RefPtr<Database>
StrainSelector::get_database()
{
	return m_treeview_.get_database();
}

Glib::RefPtr<const Database>
StrainSelector::get_database() const
{
	return m_treeview_.get_database();
}

void
StrainSelector::refresh()
{
	m_treeview_.refresh();
}
