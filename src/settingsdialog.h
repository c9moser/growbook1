/***************************************************************************
 *            settingsdialog.h
 *
 *  So April 25 21:15:06 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * settingsdialog.h
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
#ifndef __SETTINGSDIALOG_H__
#define __SETTINGSDIALOG_H__

#include <gtkmm/dialog.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>

#include "settings.h"

class SettingsDialog:
	public Gtk::Dialog
{
	 private:
		 static const char TITLE[];
		 static const char OPEN_ONGOING_GROWLOGS_CHECKBUTTON[];
		 static const char DATABASE_SETTINGS_BUTTON[];
	 private:
		 Glib::RefPtr<Settings> m_settings_;

		 Gtk::CheckButton m_open_ongoing_growlogs_checkbutton_;
		 Gtk::Entry m_date_format_entry_;
		 Gtk::Entry m_datetime_format_entry_;
		 Gtk::Button m_database_settings_button_;
		 
	 public:
		 SettingsDialog(const Glib::RefPtr<Settings> &settings);
		 SettingsDialog(Gtk::Window &parent,
		                const Glib::RefPtr<Settings> &settings);
		 virtual ~SettingsDialog();

	 private:
		 void _add_buttons();
		 void _add_widgets();
		 
	 public:
		 Glib::RefPtr<Settings> get_settings();
		 Glib::RefPtr<const Settings> get_settings() const;

		 Glib::ustring get_date_format() const;
		 Glib::ustring get_datetime_format() const;
			 
	 protected:
		 virtual void on_response(int response_id) override;

	private:
		 void on_database_settings_clicked();
};

#endif /* __SETTINGSDIALOG_H__ */

