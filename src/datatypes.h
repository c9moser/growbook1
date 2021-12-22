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
#include <time.h>

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
}; // Breeder class


class Strain:
	public RefClass
{
	private:
		uint64_t m_id_;
		uint64_t m_breeder_id_;
		Glib::ustring m_breeder_name_;
		Glib::ustring m_name_;
		Glib::ustring m_info_;
		Glib::ustring m_description_;
		std::string m_homepage_;
		std::string m_seedfinder_;
		
	private:
		Strain(const Strain &src) = delete;
		Strain& operator = (const Strain &src) = delete;

	protected:
		Strain(uint64_t breeder_id,
		       const Glib::ustring &breeder_name,
		       const Glib::ustring &name,
		       const Glib::ustring &info,
		       const Glib::ustring &description,
		       const std::string &homepage,
		       const std::string &seedfinder);
		Strain(uint64_t id,
		       uint64_t breeder_id,
		       const Glib::ustring &breeder_name,
		       const Glib::ustring &name,
		       const Glib::ustring &info,
		       const Glib::ustring &description,
		       const std::string &homepage,
		       const std::string &seedfinder);

	public:
		virtual ~Strain();

	public:
		static Glib::RefPtr<Strain> create(uint64_t breeder_id,
		                                   const Glib::ustring &breeder_name,
		                                   const Glib::ustring &name,
		                                   const Glib::ustring &info,
		                                   const Glib::ustring &description,
		                                   const std::string &homepage,
		                                   const std::string &seedfinder);
		static Glib::RefPtr<Strain> create(uint64_t id,
		                                   uint64_t breeder_id,
		                                   const Glib::ustring &breeder_name,
		                                   const Glib::ustring &name,
		                                   const Glib::ustring &info,
		                                   const Glib::ustring &descritpion,
		                                   const std::string &homepage,
		                                   const std::string &seedfinder);

	public:
		uint64_t get_id() const;
		uint64_t get_breeder_id() const;

		Glib::ustring get_breeder_name() const;
		void set_breeder_name(const Glib::ustring &breeder_name);

		Glib::ustring get_name() const;
		void set_name(const Glib::ustring &name);

		Glib::ustring get_info() const;
		void set_info(const Glib::ustring &info);

		Glib::ustring get_description() const;
		void set_description(const Glib::ustring &description);

		std::string get_homepage() const;
		void set_homepage(const std::string &homepage);

		std::string get_seedfinder() const;
		void set_seedfinder(const std::string &seedfinder);
}; // Strain class


#define DATE_ISO_FORMAT "%Y-%m-%d"
#define DATETIME_ISO_FORMAT "%Y-%m-%d %H:%M:%S" 

class Growlog:
	public RefClass
{
	private:
		uint64_t m_id_;
		Glib::ustring m_title_;
		Glib::ustring m_description_;
		time_t m_created_on_;
		time_t m_flower_on_;
		time_t m_finished_on_;
		
	private:
		Growlog(const Growlog &src) = delete;
		Growlog& operator = (const Growlog &src) = delete;
		
	protected:
		Growlog(const Glib::ustring &title,
		        const Glib::ustring &description,
		        time_t created_on=0,
		        time_t flower_on=0,
		        time_t finished_on=0);
		Growlog(uint64_t id,
		        const Glib::ustring &title,
		        const Glib::ustring &description,
		        time_t created_on,
		        time_t flower_on,
		        time_t finished_on);
	public:
		virtual ~Growlog();

	public:
		static Glib::RefPtr<Growlog> create(const Glib::ustring &title,
		                                    const Glib::ustring &description = Glib::ustring(),
		                                    time_t created_on=0,
		                                    time_t flower_on=0,
		                                    time_t finished_on=0);
		static Glib::RefPtr<Growlog> create(uint64_t id,
		                                    const Glib::ustring &title,
		                                    const Glib::ustring &description,
		                                    time_t created_on,
		                                    time_t flower_on,
		                                    time_t finished_on);
	public:
		uint64_t get_id() const;

		Glib::ustring get_title() const;
		void set_title(const Glib::ustring &title);

		Glib::ustring get_description() const;
		void set_description (const Glib::ustring &description);

		time_t get_created_on() const;
		Glib::ustring get_created_on_format(const Glib::ustring &format=DATETIME_ISO_FORMAT) const;
		void set_created_on(time_t time);
		
		time_t get_flower_on() const;
		Glib::ustring get_flower_on_format(const Glib::ustring &format=DATE_ISO_FORMAT) const;
		void set_flower_on(time_t time=-1);
		
		time_t get_finished_on() const;
		Glib::ustring get_finished_on_format(const Glib::ustring &format=DATETIME_ISO_FORMAT) const;
		void set_finished_on(time_t time=-1);
};

class GrowlogEntry:
	public RefClass
{
	private:
		uint64_t m_id_;
		uint64_t m_growlog_id_;
		Glib::ustring m_text_;
		time_t m_created_on_;

	private:
		GrowlogEntry(const GrowlogEntry &src) = delete;
		GrowlogEntry& operator = (const GrowlogEntry &src) = delete;

	protected:
		GrowlogEntry(uint64_t growlog_id,
		             const Glib::ustring &text,
		             time_t created_on = 0);
		GrowlogEntry(uint64_t id,
		             uint64_t growlog_id,
		             const Glib::ustring &text,
		             time_t created_on);
	public:
		virtual ~GrowlogEntry();

	public:
		static Glib::RefPtr<GrowlogEntry> create(uint64_t growlog_id,
		                                         const Glib::ustring &text,
		                                         time_t created_on = 0);
		static Glib::RefPtr<GrowlogEntry> create(uint64_t id,
		                                         uint64_t growlog_id,
		                                         const Glib::ustring &text,
		                                         time_t created_on);

	public:
		uint64_t get_id() const;

		uint64_t get_growlog_id() const;

		Glib::ustring get_text() const;
		void set_text(const Glib::ustring &text);

		time_t get_created_on() const;
		Glib::ustring get_created_on_format(const Glib::ustring &format = DATETIME_ISO_FORMAT) const;
};

#endif /* __DATATYPES_H__ */
