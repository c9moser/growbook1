/***************************************************************************
 *            strainview.h
 *
 *  Sa Mai 01 18:24:04 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * strainview.h
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

#ifndef __STRAINVIEW_H__
#define __STRAINVIEW_H__

#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/textview.h>

#include "browserpage.h"
#include "database.h"

class StrainView:
	public BrowserPage
{
	 public:
		 static const char TYPE[]; 
	 private:
		 Glib::RefPtr<Strain> m_strain_;
		 Gtk::Toolbar m_toolbar_;
		 Gtk::ToolButton m_breeder_homepage_button_;
		 Gtk::ToolButton m_homepage_button_;
		 Gtk::ToolButton m_seedfinder_button_;
		 Gtk::ToolButton m_refresh_button_;
		 Gtk::TextView m_textview_;
		 
	 public:
		 StrainView(const Glib::RefPtr<Database> &database,
		            const Glib::RefPtr<Strain> &strain);
		 virtual ~StrainView();

	private:
		 Glib::RefPtr<Gtk::TextBuffer> _create_textbuffer();
		 
	public:
		 Glib::RefPtr<Strain> get_strain();
		 Glib::RefPtr<const Strain> get_strain() const;
		 
	protected:
		 virtual uint64_t get_id_vfunc() const;
		 virtual Glib::ustring get_title_vfunc() const;
		 
		 virtual void on_refresh();

	private:
		 void on_breeder_homepage_clicked();
		 void on_homepage_clicked();
		 void on_seedfinder_clicked();
};

#endif /* __STRAINVIEW_H__ */
