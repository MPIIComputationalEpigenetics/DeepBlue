//
//  connection.cpp
//  epidb
//
//  Created by Felipe Albrecht on 02.02.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
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