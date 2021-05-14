/***************************************************************************
 *            growlogentrydialog.h
 *
 *  Do Mai 13 21:27:48 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * growlogentrydialog.h
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

#ifndef __GROWLOGENTRYDIALOG_H__
#define __GROWLOGENTRYDIALOG_H__

#include <gtkmm/dialog.h>
#include <gtkmm/textview.h>

#include "database.h"

class GrowlogEntryDialog:
	public Gtk::Dialog
{
	 private:
		 static const char TITLE[];
	 private:
		 Glib::RefPtr<Database> m_database_;
		 Glib::RefPtr<GrowlogEntry> m_entry_;
		 Gtk::TextView m_textview_;
		 
	 public:
		 GrowlogEntryDialog(const Glib::RefPtr<Database> &database,
		                    const Glib::RefPtr<GrowlogEntry> &entry);
		 GrowlogEntryDialog(const Glib::RefPtr<Database> &database,
		                    const Glib::RefPtr<Growlog> &growlog);
		 GrowlogEntryDialog(Gtk::Window &parent,
		                    const Glib::RefPtr<Database> &database,
		                    const Glib::RefPtr<GrowlogEntry> &entry);
		 GrowlogEntryDialog(Gtk::Window &parent,
		                    const Glib::RefPtr<Database> &database,
		                    const Glib::RefPtr<Growlog> &growlog);
		 virtual ~GrowlogEntryDialog();

	private:
		 void _add_buttons();
		 void _add_widgets();

	protected:
		 virtual void on_response(int response_id);
};

#endif /* __GROWLOGENTRYDIALOG_H__ */
