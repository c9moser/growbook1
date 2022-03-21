/***************************************************************************
 *            xml_importer.h
 *
 *  Mi Dezember 08 13:54:26 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * xml_importer.h
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

#ifndef __XML_IMPORTER_H__
#define __XML_IMPORTER_H__

#include "import.h"

class XML_Importer:
	public Importer
{
	 private:
		 XML_Importer(const XML_Importer &src) = delete;
		 XML_Importer& operator=(const XML_Importer &src) = delete;

	protected:
		 XML_Importer(const Glib::RefPtr<Database> &database,
		              const std::string &filename);

	public:
		 virtual ~XML_Importer();

		 static Glib::RefPtr<XML_Importer> create(const Glib::RefPtr<Database> &database,
		                                          const std::string &filename);

	protected:
		 void import_vfunc(Gtk::Window &parent);		 
};


#endif /* __XML_IMPORTER_H__ */
