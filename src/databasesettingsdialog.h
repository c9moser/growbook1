/***************************************************************************
 *            databasesettingsdialog.h
 *
 *  So April 25 12:37:48 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * databasesettingsdialog.h
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
#ifndef __DATABASESETTINGSDIALOG_H__
#define __DATABASESETTINGSDIALOG_H__

#include <gtkmm/dialog.h>

#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>

#include "settings.h"

class DatabaseSettingsDialog:
	public Gtk::Dialog
{
	 private:
		 static const char TITLE[];
		 static const char HOST_LABEL[];
		 static const char PORT_LABEL[];
		 static const char USER_LABEL[];
		 static const char PASSWORD_LABEL[];
		 static const char PASSWORD_CHECKBUTTON[];
		 static const char CREATE_BUTTON[];
		 static const char TEST_BUTTON[];
		 
	 private:
		 Glib::RefPtr<DatabaseSettings> m_settings_;
		 bool m_update_settings_;
		 DatabaseSettingsFlags m_flags_;
		 
		 Gtk::ComboBoxText m_engine_combobox_;
		 Gtk::Entry m_dbname_entry_;
		 Gtk::Button m_dbname_button_;
		 Gtk::Label m_host_label_;
		 Gtk::Entry m_host_entry_;
		 Gtk::Label m_port_label_;
		 Gtk::Entry m_port_entry_;
		 Gtk::Label m_user_label_;
		 Gtk::Entry m_user_entry_;
		 Gtk::Label m_password_label_;
		 Gtk::Entry m_password_entry_;
		 Gtk::CheckButton m_password_checkbutton_;
		 Gtk::Button m_test_button_;
		 Gtk::Button m_create_button_;
			 
	public:
		 DatabaseSettingsDialog(const Glib::RefPtr<DatabaseSettings> &settings);
		 DatabaseSettingsDialog(Gtk::Window &parent,
		                        const Glib::RefPtr<DatabaseSettings> &settings);
		 
		 virtual ~DatabaseSettingsDialog();

	private:
		 void _add_buttons();
		 void _add_widgets();
		 
	public:
		 bool get_update_settings() const;
		 void set_update_settings(bool set=true);

		 Glib::RefPtr<DatabaseSettings> get_settings();
		 Glib::RefPtr<const DatabaseSettings> get_settings() const;
		 
	protected:
		 virtual void on_response(int response_id) override;

	private:
		 void on_engine_changed();
		 void on_dbname_clicked();
		 void on_test_connection_clicked();
		 void on_create_database_clicked();
		 
};
#endif
