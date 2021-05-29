//           aboutdialog.cc
//  Do April 29 14:13:14 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// aboutdialog.cc
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
#include "aboutdialog.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h>
#include <glibmm.h>

#include "application.h"

AboutDialog::AboutDialog()
{
	set_program_name ("GrowBook");
	set_version (PACKAGE_VERSION);
	set_copyright("(C) 2021 Christan Moser");
	set_license_type(Gtk::LICENSE_GPL_3_0);
	set_authors(_get_authors());

	set_logo_icon_name ("emoji-nature-symbolic");
	set_translator_credits (_get_translator_credits ());
	show_all();
}

AboutDialog::~AboutDialog()
{
}

std::vector<Glib::ustring>
AboutDialog::_get_authors()
{
	std::vector<Glib::ustring> authors{1};

	authors[0] = "Chrsitian Moser";

	return authors;
}

Glib::ustring
AboutDialog::_get_translator_credits()
{
	Glib::ustring text = "[German] Christian Moser\n";

	return text;
}
