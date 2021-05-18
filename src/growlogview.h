/***************************************************************************
 *            growlogview.h
 *
 *  Di Mai 11 16:41:03 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * growlogview.h
 *
 * Copyright (C) 2021 - Christian Moser
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GROWLOGVIEW_H__
#define __GROWLOGVIEW_H__

#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#include "browserpage.h"

class GrowlogViewStrainColumns:
	public Gtk::TreeModelColumnRecord
{
	 public:
		 Gtk::TreeModelColumn<uint64_t> column_id;
		 Gtk::TreeModelColumn<Glib::ustring> column_breeder;
		 Gtk::TreeModelColumn<Glib::ustring> column_name;

	public:
		 GrowlogViewStrainColumns();
		 virtual ~GrowlogViewStrainColumns();
};

class GrowlogViewEntryColumns:
	public Gtk::TreeModelColumnRecord
{
	public:
		Gtk::TreeModelColumn<uint64_t> column_id;
		Gtk::TreeModelColumn<Glib::ustring> column_text;
		Gtk::TreeModelColumn<Glib::ustring> column_datetime;
		Gtk::TreeModelColumn<time_t> column_created_on;

	public:
		GrowlogViewEntryColumns();
		virtual ~GrowlogViewEntryColumns();
};

class GrowlogViewStrainView:
	public Gtk::TreeView
{
	public:
		using Columns = GrowlogViewStrainColumns;

	public:
		Columns columns;
	private:
		Glib::RefPtr<Database> m_database_;
		Glib::RefPtr<Growlog> m_growlog_;
		
	public:
		GrowlogViewStrainView(const Glib::RefPtr<Database> &database,
		                      const Glib::RefPtr<Growlog> &growlog);
		virtual ~GrowlogViewStrainView();

	private:
		Glib::RefPtr<Gtk::ListStore> _create_model();
		
	public:
		Glib::RefPtr<Database> get_database();
		Glib::RefPtr<const Database> get_database() const;
		
		void set_growlog(const Glib::RefPtr<Growlog> &growlog);
		Glib::RefPtr<Growlog> get_growlog();
		Glib::RefPtr<const Growlog> get_growlog() const;		
};

class GrowlogViewEntryView:
	public Gtk::TreeView
{
	public:
		using Columns = GrowlogViewEntryColumns;

	public:
		Columns columns;

	private:
		Glib::RefPtr<Database> m_database_;
		Glib::RefPtr<Growlog> m_growlog_;
		
	public:
		GrowlogViewEntryView(const Glib::RefPtr<Database> &database,
		                     const Glib::RefPtr<Growlog> &growlog);
		virtual ~GrowlogViewEntryView();

	private:
		Glib::RefPtr<Gtk::ListStore> _create_model();

	public:
		Glib::RefPtr<Database> get_database();
		Glib::RefPtr<const Database> get_database() const;
		
		Glib::RefPtr<Growlog> get_growlog();
		Glib::RefPtr<const Growlog> get_growlog() const;
		void set_growlog(const Glib::RefPtr<Growlog> &growlog);
};

class GrowlogView:
	public BrowserPage
{
	 public:
		using StrainColumns = GrowlogViewStrainColumns;
		using StrainView = GrowlogViewStrainView;
		using EntryColumns = GrowlogViewEntryColumns;
		using EntryView = GrowlogViewEntryView;
		
	 public:
		 static const char TYPE[];

	 private:
		 Glib::RefPtr<Growlog> m_growlog_;

		 Gtk::ToolButton m_refresh_button_;
		 Gtk::ToolButton m_edit_button_;
		 Gtk::ToolButton m_flower_button_;
		 Gtk::ToolButton m_finish_button_;
		 Gtk::ToolButton m_add_logentry_button_;
		 Gtk::ToolButton m_edit_logentry_button_;
		 Gtk::ToolButton m_remove_logentry_button_;
		 Gtk::Toolbar m_toolbar_;

		 Gtk::TextView m_textview_;
		 StrainView m_strain_view_;
		 EntryView m_entry_view_;
		 
	 public:
		 GrowlogView(const Glib::RefPtr<Database> &database,
		             const Glib::RefPtr<Growlog> &growlog);
		 virtual ~GrowlogView();

	private:
		 Glib::RefPtr<Gtk::TextBuffer> _create_textbuffer();
		 
		 
	protected:
		 virtual uint64_t get_id_vfunc() const override;
		 virtual Glib::ustring get_title_vfunc() const override;

		 virtual void on_refresh() override;

	private:
		 void on_edit();
		 void on_add_logentry();
		 void on_edit_logentry();
		 void on_remove_logentry();
		 void on_entry_view_selection_changed();
		 void on_flower();
		 void on_finish();

		 void on_strain_view_row_activated(const Gtk::TreeModel::Path &path,
		                                   Gtk::TreeViewColumn *column);
		 void on_entry_view_row_activated(const Gtk::TreeModel::Path &path,
		                                  Gtk::TreeViewColumn *column);
};


#endif /* __GROWLOGVIEW_H__ */

