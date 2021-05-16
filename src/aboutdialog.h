/***************************************************************************
 *            aboutdialog.h
 *
 *  Do April 29 14:13:14 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * aboutdialog.h
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
#ifndef __ABOUTDIALOG_H__
#define __ABOUTDIALOG_H__

#include <gtkmm/aboutdialog.h>

class AboutDialog:
	public Gtk::AboutDialog
{
	 public:
		 AboutDialog();
		 virtual ~AboutDialog();

	private:
		std::vector<Glib::ustring> _get_authors();
		Glib::ustring _get_translator_credits();
};

#endif /* __ABOUTDIALOG_H__ */
