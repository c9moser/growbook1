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

//#ifdef HAVE_CONFIG_H
# include "config.h"
//#endif

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

/*******************************************************************************
 * Strain
 ******************************************************************************/

Strain::Strain(uint64_t breeder_id,
               const Glib::ustring &breeder_name,
               const Glib::ustring &name,
               const Glib::ustring &info,
               const Glib::ustring &desc,
               const std::string &homepage,
               const std::string &seedfinder):
	RefClass{},
	m_id_{0},
	m_breeder_id_{breeder_id},
	m_breeder_name_{breeder_name},
	m_name_{name},
	m_info_{info},
	m_description_{desc},
	m_homepage_{homepage},
	m_seedfinder_{seedfinder}
{}

Strain::Strain(uint64_t id,
               uint64_t breeder_id,
               const Glib::ustring &breeder_name,
               const Glib::ustring &name,
               const Glib::ustring &info,
               const Glib::ustring &desc,
               const std::string &homepage,
               const std::string &seedfinder):
	RefClass{},
	m_id_{id},
	m_breeder_id_{breeder_id},
	m_breeder_name_{breeder_name},
	m_name_{name},
	m_info_{info},
	m_description_{desc},
	m_homepage_{homepage},
	m_seedfinder_{seedfinder}
{}

Strain::~Strain()
{}

Glib::RefPtr<Strain>
Strain::create(uint64_t breeder_id,
               const Glib::ustring &breeder_name,
               const Glib::ustring &name,
               const Glib::ustring &info,
               const Glib::ustring &desc,
               const std::string &homepage,
               const std::string &seedfinder)
{
	return Glib::RefPtr<Strain>(new Strain(breeder_id,breeder_name,name,info,desc,homepage,seedfinder));
}

Glib::RefPtr<Strain>
Strain::create(uint64_t id,
               uint64_t breeder_id,
               const Glib::ustring &breeder_name,
               const Glib::ustring &name,
               const Glib::ustring &info,
               const Glib::ustring &desc,
               const std::string &homepage,
               const std::string &seedfinder)
{
	return Glib::RefPtr<Strain>(new Strain(id,breeder_id,breeder_name,name,info,desc,homepage,seedfinder));
}

uint64_t
Strain::get_id() const
{
	return m_id_;
}

uint64_t
Strain::get_breeder_id() const
{
	return m_breeder_id_;
}

Glib::ustring
Strain::get_breeder_name() const
{
	return m_breeder_name_;
}

void
Strain::set_breeder_name(const Glib::ustring &name)
{
	m_breeder_name_ = name;
}

Glib::ustring
Strain::get_name() const
{
	return m_name_;
}

void
Strain::set_name(const Glib::ustring &name)
{
	m_name_ = name;
}

Glib::ustring
Strain::get_info() const
{
	return m_info_;
}

void
Strain::set_info(const Glib::ustring &info)
{
	m_info_ = info;
}

Glib::ustring
Strain::get_description() const
{
	return m_description_;
}

void
Strain::set_description(const Glib::ustring &desc)
{
	m_description_ = desc;
}

std::string
Strain::get_homepage() const
{
	return m_homepage_;
}

void
Strain::set_homepage(const std::string &homepage)
{
	m_homepage_ = homepage;
}

std::string
Strain::get_seedfinder() const
{
	return m_seedfinder_;
}

void
Strain::set_seedfinder(const std::string &sf)
{
	m_seedfinder_ = sf;
}

/*******************************************************************************
 * Growlog
 ******************************************************************************/

Growlog::Growlog(const Glib::ustring &title,
                 const Glib::ustring &desc,
                 time_t created_on,
                 time_t flower_on,
                 time_t finished_on):
	RefClass{},
	m_id_{0},
	m_title_{title},
	m_description_{desc},
	m_created_on_{created_on ? created_on : time(0)},
	m_flower_on_{flower_on},
	m_finished_on_{finished_on}
{
}

Growlog::Growlog(uint64_t id,
                 const Glib::ustring &title,
                 const Glib::ustring &desc,
                 time_t created_on,
                 time_t flower_on,
                 time_t finished_on):
	RefClass{},
	m_id_{id},
	m_title_{title},
	m_description_{desc},
	m_created_on_{created_on ? created_on : time(0)},
	m_flower_on_{flower_on},
	m_finished_on_{finished_on}
{
}

Growlog::~Growlog()
{
}

Glib::RefPtr<Growlog>
Growlog::create(const Glib::ustring &title,
                const Glib::ustring &desc,
                time_t created_on,
                time_t flower_on,
                time_t finished_on)
{
	return Glib::RefPtr<Growlog>(new Growlog(title,desc,created_on,flower_on,finished_on));
}

Glib::RefPtr<Growlog>
Growlog::create(uint64_t id,
                const Glib::ustring &title,
                const Glib::ustring &desc,
                time_t created_on,
                time_t flower_on,
                time_t finished_on)
{
	return Glib::RefPtr<Growlog>(new Growlog(id,title,desc,created_on,flower_on,finished_on));
}

uint64_t
Growlog::get_id() const
{
	return m_id_;
}

Glib::ustring
Growlog::get_title() const
{
	return m_title_;
}

void
Growlog::set_title(const Glib::ustring &title)
{
	m_title_ = title;
}

Glib::ustring
Growlog::get_description() const
{
	return m_description_;
}

void
Growlog::set_description(const Glib::ustring &desc)
{
	m_description_ = desc;
}

time_t
Growlog::get_created_on() const
{
	return m_created_on_;
}

Glib::ustring
Growlog::get_created_on_format(const Glib::ustring &format) const
{
	
#ifdef NATIVE_WINDOWS
	tm *datetime = localtime(&m_created_on_);
	if (!datetime)
		return Glib::ustring;

	const size_t size = 100;
	char buf[size];
	buf[0] = '\1';
	buf[size-1] = '\0';
	size_t len = strftime(buf,size,format.c_str(),datetime);

	if (len == 0 && buf[0] != '\0') {
		//strftime went wrong
		return Glib::ustring();
	}
	Glib::ustring ret = buf;
	return ret;
#else // !NATIVE_WINDOWS
	tm datetime;
	
	if (localtime_r(&m_created_on_,&datetime) != &datetime) {
		//datetime went wrong
		return Glib::ustring();
	}
	const size_t size = 100;
	char buf[size];
	buf[0] = '\1';
	buf[size-1] = '\0';
	size_t len = strftime(buf,size,format.c_str(),&datetime);
	if (len == 0 && buf[0] != '\0') {
		//strftime went wrong
		return Glib::ustring();
	}
	Glib::ustring ret = buf;
	return ret;
#endif /*! NATIVE_WINDOWS */
}

time_t
Growlog::get_flower_on() const
{
	return m_flower_on_;
}

Glib::ustring
Growlog::get_flower_on_format(const Glib::ustring &format) const
{
	if (!m_flower_on_)
		return Glib::ustring();

#ifdef NATIVE_WINDOWS
	tm *datetime = localtime(&m_flower_on_);
	if (!datetime)
		return Glib::ustring;

	const size_t size = 100;
	char buf[size];
	buf[0] = '\1';
	buf[size-1] = '\0';
	size_t len = strftime(buf,size,format.c_str(),datetime);

	if (len == 0 && buf[0] != '\0') {
		//strftime went wrong
		return Glib::ustring();
	}
	Glib::ustring ret = buf;
	return ret;
#else // !NATIVE_WINDOWS

	tm datetime;
	if (localtime_r(&m_flower_on_,&datetime) != &datetime) {
		//datetime went wrong
		return Glib::ustring();
	}
	const size_t size = 100;
	char buf[size];
	buf[0] = '\1';
	buf[size-1] = '\0';
	size_t len = strftime(buf,size,format.c_str(),&datetime);
	if (len == 0 && buf[0] != '\0') {
		//strftime went wrong
		return Glib::ustring();
	}
	Glib::ustring ret = buf;
	
	return ret;
#endif // ! NATIVE_WINDOWS
}

void
Growlog::set_flower_on(time_t t)
{
	if (t == -1) {
		m_flower_on_ = time(0);
	} else {
		m_flower_on_ = t;
	}
}

time_t
Growlog::get_finished_on() const
{
	return m_finished_on_;
}

Glib::ustring
Growlog::get_finished_on_format(const Glib::ustring &format) const
{
	if (!m_finished_on_)
		return Glib::ustring();

#ifdef NATIVE_WINDOWS
	tm *datetime = localtime(&m_finished_on_);
	if (!datetime)
		return Glib::ustring;

	const size_t size = 100;
	char buf[size];
	buf[0] = '\1';
	buf[size-1] = '\0';
	size_t len = strftime(buf,size,format.c_str(),datetime);

	if (len == 0 && buf[0] != '\0') {
		//strftime went wrong
		return Glib::ustring();
	}
	Glib::ustring ret = buf;
	return ret;
#else // !NATIVE_WINDOWS

	tm datetime;
	if (localtime_r(&m_finished_on_,&datetime) != &datetime) {
		//datetime went wrong
		return Glib::ustring();
	}
	const size_t size = 100;
	char buf[size];
	buf[0] = '\1';
	buf[size-1] = '\0';
	size_t len = strftime(buf,size,format.c_str(),&datetime);
	if (len == 0 && buf[0] != '\0') {
		//strftime went wrong
		return Glib::ustring();
	}
	Glib::ustring ret = buf;
	return ret;
#endif // ! NATIVE_WINDOWS
}

void
Growlog::set_finished_on(time_t t)
{
	if (t == -1) {
		m_finished_on_ = time(0);
	} else {
		m_finished_on_ = t;
	}
}

/*******************************************************************************
 * GrowlogEntry
 ******************************************************************************/

GrowlogEntry::GrowlogEntry(uint64_t growlog_id,
                           const Glib::ustring &text,
                           time_t created_on):
	RefClass{},
	m_id_{0},
	m_growlog_id_{growlog_id},
	m_text_{text},
	m_created_on_{created_on ? created_on : time(0)}
{}

GrowlogEntry::GrowlogEntry(uint64_t id,
                           uint64_t growlog_id,
                           const Glib::ustring &text,
                           time_t created_on):
	RefClass{},
	m_id_{id},
	m_growlog_id_{growlog_id},
	m_text_{text},
	m_created_on_{created_on ? created_on : time(0)}
{}

GrowlogEntry::~GrowlogEntry()
{}

Glib::RefPtr<GrowlogEntry>
GrowlogEntry::create(uint64_t growlog_id,
                     const Glib::ustring &text,
                     time_t created_on)
{
	return Glib::RefPtr<GrowlogEntry>(new GrowlogEntry(growlog_id,text,created_on));
}

Glib::RefPtr<GrowlogEntry>
GrowlogEntry::create(uint64_t id,
                     uint64_t growlog_id,
                     const Glib::ustring &text,
                     time_t created_on)
{
	return Glib::RefPtr<GrowlogEntry>(new GrowlogEntry(id,growlog_id,text,created_on));
}

uint64_t 
GrowlogEntry::get_id() const
{
	return m_id_;
}

uint64_t
GrowlogEntry::get_growlog_id() const
{
	return m_growlog_id_;
}

Glib::ustring
GrowlogEntry::get_text() const
{
	return m_text_;
}

void
GrowlogEntry::set_text(const Glib::ustring &text)
{
	m_text_ = text;
}

time_t 
GrowlogEntry::get_created_on() const
{
	return m_created_on_;
}

Glib::ustring
GrowlogEntry::get_created_on_format(const Glib::ustring &format) const
{
#ifdef NATIVE_WINDOWS
	tm *datetime = localtime(&m_created_on_);
	if (!datetime)
		return Glib::ustring;

	const size_t size = 100;
	char buf[size];
	buf[0] = '\1';
	buf[size-1] = '\0';
	size_t len = strftime(buf,size,format.c_str(),datetime);

	if (len == 0 && buf[0] != '\0') {
		//strftime went wrong
		return Glib::ustring();
	}
	Glib::ustring ret = buf;
	return ret;
#else // !NATIVE_WINDOWS

	tm datetime;
	if (localtime_r(&m_created_on_,&datetime) != &datetime) {
		//datetime went wrong
		return Glib::ustring();
	}
	const size_t size = 100;
	char buf[size];
	buf[0] = '\1';
	buf[size-1] = '\0';
	size_t len = strftime(buf,size,format.c_str(),&datetime);
	if (len == 0 && buf[0] != '\0') {
		//strftime went wrong
		return Glib::ustring();
	}
	Glib::ustring ret = buf;
	return ret;
#endif // !NATIVE_WINDOWS
}
