//           browserpage.cc
//  Sa Mai 01 17:45:23 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// browserpage.cc
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

#include "browserpage.h"

BrowserPage::BrowserPage(const Glib::RefPtr<Database> &db,
                         const Glib::ustring &type):
	Gtk::Box{Gtk::ORIENTATION_VERTICAL},
	m_database_{db},
	m_type_{type},
	m_signal_title_changed_{},
	m_signal_refresh_{}
{
	m_signal_title_changed_.connect(sigc::mem_fun(*this,&BrowserPage::on_title_changed));
	m_signal_refresh_.connect(sigc::mem_fun(*this,&BrowserPage::on_refresh));
}

BrowserPage::~BrowserPage()
{
}

uint64_t
BrowserPage::get_id() const
{
	return this->get_id_vfunc ();
}

Glib::ustring
BrowserPage::get_title() const
{
	return this->get_title_vfunc();
}

Glib::ustring
BrowserPage::get_type() const
{
	return m_type_;
}

Glib::RefPtr<Database>
BrowserPage::get_database()
{
	return m_database_;
}

Glib::RefPtr<const Database>
BrowserPage::get_database() const
{
	return Glib::RefPtr<const Database>::cast_const(m_database_);
}

void
BrowserPage::refresh()
{
	m_signal_refresh_.emit();
}

sigc::signal0<void>
BrowserPage::signal_title_changed()
{
	return m_signal_title_changed_;
}

void
BrowserPage::title_changed()
{
	m_signal_title_changed_.emit();
}

void
BrowserPage::on_title_changed()
{
}

void
BrowserPage::on_refresh()
{
}
