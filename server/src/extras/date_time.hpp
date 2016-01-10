//
//  data_time.hpp
//  DeepBlue Epigenomic Data Server

//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Code from https://github.com/temporaer/MDBQ/
// adapted to DeepBlue by Felipe Albrecht on 22.01.2015

#ifndef EPIDB_EXTRAS_DATE_TIME_HPP
#define EPIDB_EXTRAS_DATE_TIME_HPP

#include <cassert>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

namespace epidb {
  namespace extras {
    inline boost::posix_time::ptime universal_date_time()
    {
      boost::posix_time::time_duration dur(boost::posix_time::microsec_clock::universal_time().time_of_day());
      boost::gregorian::date date(boost::gregorian::day_clock::universal_day());
      return boost::posix_time::ptime(date, dur);
    }

    inline std::string dt_format(const boost::posix_time::ptime& t)
    {
      std::stringstream ss;
      ss << t.date().month()
         << "-"             << t.date().day()
         << " "             << t.time_of_day().hours()
         << ":"             << t.time_of_day().minutes()
         << ":"             << t.time_of_day().seconds();
      return ss.str();
    }

    inline mongo::Date_t to_mongo_date(const boost::posix_time::ptime& pt)
    {
      using boost::gregorian::date;
      using boost::posix_time::ptime;
      using boost::posix_time::microsec_clock;
      static ptime const epoch(date(1970, 1, 1));

      boost::posix_time::time_duration::tick_type tt = (pt - epoch).total_milliseconds();

      mongo::Date_t d(tt);
      return d;
    }

    inline boost::posix_time::ptime to_ptime(const mongo::Date_t& md)
    {
      using boost::gregorian::date;
      using boost::posix_time::ptime;
      using boost::posix_time::microsec_clock;

      static ptime const epoch(date(1970, 1, 1));
      return epoch + boost::posix_time::milliseconds(md);
    }
  }
}

#endif /* EPIDB_EXTRAS_DATE_TIME_HPP */
