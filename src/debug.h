/***************************************************************************
 *            import.h
 *
 *  Sa Mai 29 11:37:55 2021
 *  Copyright  2021  Christian Moser
 *  <user@host>
 ****************************************************************************/
/*
 * import.h
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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <cstdio>
#include <cstdarg>


#define DEBUG(fmt,args...) __debug__(fmt,args)

inline void __debug__(const char *fmt,...) 
{
	va_list args;
	va_start(args,fmt);
	fprintf(stderr,"DEBUG> ");
	vfprintf(stderr,fmt,args);
	va_end(args);
}
#endif /* __DEBUG_H__ */