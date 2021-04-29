// refclass.cc
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
#include "refclass.h"

RefClass::RefClass():
	m_ref_count_(1)
{
}

RefClass::~RefClass()
{}

void
RefClass::reference() const
{
	++m_ref_count_;
}

void
RefClass::unreference() const
{
	if (0 == --m_ref_count_) {
		delete this;
	}
}

unsigned long
RefClass::get_ref_count() const
{
	return m_ref_count_;
}

