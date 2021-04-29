/***************************************************************************
 *            error.h
 *
 *  Mi April 21 20:39:55 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * error.h
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
#ifndef __ERROR_H__
#define __ERROR_H__

#include <exception>
#include <glibmm/ustring.h>

class DatabaseError:
	public std::exception
{
	 private:
		 int m_ec_;
		 Glib::ustring m_msg_;
			 
	 public:
		 DatabaseError(const Glib::ustring &message) noexcept;
		 DatabaseError(int error_code, const Glib::ustring &message) noexcept;
		 DatabaseError(const DatabaseError &src) noexcept;
		 virtual ~DatabaseError();

		 DatabaseError& operator=(const DatabaseError &src) noexcept;
		 
		 virtual const char *what()const noexcept;
		 virtual Glib::ustring get_message() const noexcept;
		 virtual int get_error_code() const noexcept;
};

#endif /* __ERROR_H__ */