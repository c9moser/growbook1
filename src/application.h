/***************************************************************************
 *            application.h
 *
 *  Di April 20 16:14:03 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * application.h
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

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <gtkmm/application.h>

#include "appwindow.h"
#include "settings.h"
#include "database.h"

class Application:
	public Gtk::Application
{
	 private:
		Glib::RefPtr<Settings> m_settings_;
		Glib::RefPtr<Database> m_database_;
	 	AppWindow *m_appwindow_;
	 
	 protected:
		 Application(int argc,char **argv);
	 public:
		 virtual ~Application();

	public:
		 static Glib::RefPtr<Application> create(int argc,char **argv);

	protected:
		 void on_activate();
	private:
		 void on_hide_window(AppWindow* appwindow);
		 
	public:
		 AppWindow* get_appwindow();
		 const AppWindow* get_appwindow() const;

		 Glib::RefPtr<Settings> get_settings();
		 Glib::RefPtr<const Settings> get_settings() const;

		 Glib::RefPtr<Database> get_database();
		 Glib::RefPtr<const Database> get_database() const;
};

extern Glib::RefPtr<Application> app;
#endif /* __APPLICATION_H__ */
