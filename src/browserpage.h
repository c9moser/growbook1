/***************************************************************************
 *            browserpage.h
 *
 *  Sa Mai 01 17:45:23 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * browserpage.h
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

#ifndef __BROWSERPAGE_H__
#define __BROWSERPAGE_H__

#include <gtkmm/box.h>
#include <sigc++/sigc++.h>

#include "database.h"

class BrowserPage:
	public Gtk::Box
{
	 private:
		 Glib::RefPtr<Database> m_database_;
		 Glib::ustring m_type_;

		 sigc::signal0<void> m_signal_title_changed_;
		 sigc::signal0<void> m_signal_refresh_;
		 
	 public:
		 BrowserPage(const Glib::RefPtr<Database> &database,
		             const Glib::ustring &type);

		 virtual ~BrowserPage();

	 public:
		 uint64_t get_id() const;
		 Glib::ustring get_type() const;
		 Glib::ustring get_title() const;

		 Glib::RefPtr<Database> get_database();
		 Glib::RefPtr<const Database> get_database() const;

		 void refresh();
		 
		 sigc::signal0<void> signal_title_changed();
		 sigc::signal0<void> signal_refresh();
	protected:
		 void title_changed();
		 
		 virtual uint64_t get_id_vfunc() const = 0;
		 virtual Glib::ustring get_title_vfunc() const = 0;
		 virtual void on_refresh();
		 virtual void on_title_changed();
};


#endif /* __BROWSERPAGE_H__ */

