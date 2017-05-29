//
//  janitor.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.05.17.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.

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

#include <boost/asio.hpp>

namespace epidb {
  namespace mdbq {
    class Janitor {
    public:
      Janitor(float interval) : m_interval(interval) {}
      void run();

    private:
      float m_interval;
      std::auto_ptr<boost::asio::deadline_timer> m_timer;
      boost::asio::io_service ios;
      bool clean_oldest(const boost::system::error_code &error);
      void reg(boost::asio::io_service &io_service, float interval);
    };
  }
}