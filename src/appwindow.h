/***************************************************************************
 *            appwindow.h
 *
 *  Di April 20 16:18:53 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * appwindow.h
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

#ifndef __APPWINDOW_H__
#define __APPWINDOW_H__

#include <gtkmm/applicationwindow.h>
#include <gtkmm/menubar.h>

#include "settings.h"

class AppWindow:
	public Gtk::ApplicationWindow
{
	 private:
		 Glib::RefPtr<Settings> m_settings_;
		 Gtk::MenuBar m_menubar_;
		 
	 public:
		 AppWindow(const Glib::RefPtr<Settings> &settings);
		 virtual ~AppWindow();
	private:
		 void _add_menu();

		 void on_database_settings();
		 void on_preferences();
		 void on_about();
};

#endif /* __APPWINDOW_H__ */
