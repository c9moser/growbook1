/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.cc
 * Copyright (C) 2021 Christian Moser <c9moser@gmx.at>
 * 
 * GrowBook is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GrowBook is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtkmm.h>
#include <iostream>

#include "config.h"


#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

#include "application.h"
#include "database.h"

int
main (int argc, char *argv[])
{
	db_init();
	app = Application::create(argc,argv);
	app->run();
	return 0;
}

