//           datatypes.cc
//  Do April 29 17:27:28 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// datatypes.cc
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

#include "datatypes.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cassert>

/*******************************************************************************
 * Breeder
 ******************************************************************************/

Breeder::Breeder(const Glib::ustring &name,
                 const std::string &homepage):
	RefClass{},
	m_id_{0},
	m_name_{name},
	m_homepage_{homepage}
{
	assert(!m_name_.empty());
}

Breeder::Breeder(uint64_t id, 
                 const Glib::ustring &name,
                 const std::string &homepage):
	RefClass{},
	m_id_{id},
	m_name_{name},
	m_homepage_{homepage}
{
	assert(m_id_);
	assert(!m_name_.empty());
}

Breeder::~Breeder()
{}

Glib::RefPtr<Breeder>
Breeder::create(const Glib::ustring &name,
                const std::string &homepage)
{
	return Glib::RefPtr<Breeder>(new Breeder(name,homepage));
}

Glib::RefPtr<Breeder>
Breeder::create(uint64_t id, 
                const Glib::ustring &name,
                const std::string &homepage)
{
	return Glib::RefPtr<Breeder>(new Breeder(id,name,homepage));
}

uint64_t
Breeder::get_id() const
{
	return m_id_;
}

Glib::ustring
Breeder::get_name() const
{
	return m_name_;
}

void
Breeder::set_name(const Glib::ustring &name)
{
	assert(!name.empty());
	
	m_name_ = name;
}

std::string
Breeder::get_homepage() const
{
	return m_homepage_;
}

void
Breeder::set_homepage(const std::string &homepage)
{
	m_homepage_ = homepage;
}
