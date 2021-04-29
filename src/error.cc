//           error.cc
//  Mi April 21 20:39:55 2021
//  Copyright  2021  Christian Moser
//  <user@host>
// error.cc
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

#include "error.h"

/*******************************************************************************
 * DatabaseError
 ******************************************************************************/

DatabaseError::DatabaseError(const DatabaseError &src) noexcept:
	std::exception(),
	m_ec_(src.m_ec_),
	m_msg_(src.m_msg_)
{}

DatabaseError::DatabaseError(const Glib::ustring &msg) noexcept:
	std::exception(),
	m_ec_(-1),
	m_msg_(msg)
{}

DatabaseError::DatabaseError(int ec, const Glib::ustring &msg) noexcept:
	std::exception(),
	m_ec_(ec),
	m_msg_(msg)
{}

DatabaseError::~DatabaseError()
{}

DatabaseError&
DatabaseError::operator = (const DatabaseError &src) noexcept
{
	m_ec_ = src.m_ec_;
	m_msg_ = src.m_msg_;
	return *this;
}

const char*
DatabaseError::what() const noexcept
{
	return m_msg_.c_str();
}

Glib::ustring
DatabaseError::get_message() const noexcept
{
	return m_msg_;
}

int
DatabaseError::get_error_code() const noexcept
{
	return m_ec_;
}
