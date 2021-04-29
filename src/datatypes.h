/***************************************************************************
 *            datatypes.h
 *
 *  Do April 29 17:27:28 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * datatypes.h
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
#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#include <refclass.h>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <string>
#include <cstdint>

class Breeder:
	public RefClass
{
	 private:
		 uint64_t m_id_;
		 Glib::ustring m_name_;
		 std::string m_homepage_;
		 
	 private:
		 Breeder(const Breeder &src) = delete;
		 Breeder& operator = (const Breeder &src) = delete;

	 protected:
		 Breeder(const Glib::ustring &name,
		         const std::string &homepage = "");
		 Breeder(uint64_t id, 
		         const Glib::ustring &name,
		         const std::string &homepage = "");
	 public:
		 virtual ~Breeder();

	 public:
		 static Glib::RefPtr<Breeder> create(const Glib::ustring &name,
		                                     const std::string &homepage = "");
		 static Glib::RefPtr<Breeder> create(uint64_t id,
		                                     const Glib::ustring &name,
		                                     const std::string &homepage = "");
	 public:
	 	 uint64_t get_id() const;
		 
		 Glib::ustring get_name() const;
		 void set_name(const Glib::ustring &name);

		 std::string get_homepage() const;
		 void set_homepage(const std::string &homepage);
};



#endif /* __DATATYPES_H__ */
