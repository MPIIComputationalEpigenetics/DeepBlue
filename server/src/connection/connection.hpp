//
//  connection.cpp
//  epidb
//
//  Created by Felipe Albrecht on 02.02.15.
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
//

#ifndef EPIDB_CONNECTION_CONNECTION_HPP
#define EPIDB_CONNECTION_CONNECTION_HPP

#include <string>

#include <mongo/client/dbclient.h>

#include "connection_pool.hpp"


namespace epidb {
  class Connection {
  public:
    Connection();
    explicit Connection(const std::string &host);
    explicit Connection(const mongo::ConnectionString &cs);
    virtual ~Connection();

    mongo::DBClientBase *operator->();
    mongo::DBClientBase &conn();
    mongo::DBClientBase *get();

    mongo::ConnectionString getConnectionString();

    void done();
    bool ok();
  private:
    mongo::ConnectionString m_ConnString;
    mongo::DBClientBase *m_Connection;
  };

  extern MongoConnectionPool g_Pool;
}




#endif