/***************************************************************************
 *            refclass.h
 *
 *  Di April 20 16:34:04 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * refclass.h
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

#ifndef __REFCLASS_H__
#define __REFCLASS_H__

class RefClass
{
	 private:
		 mutable unsigned long m_ref_count_;

	private:
		 RefClass(const RefClass &src) = delete;
		 RefClass& operator=(const RefClass &src) = delete;
		 
	 protected:
		 RefClass();
	public:
		 virtual ~RefClass();

		 void reference() const;
		 void unreference() const;
		 unsigned long get_ref_count() const; 
};


#endif /* __REFCLASS_H__ */

