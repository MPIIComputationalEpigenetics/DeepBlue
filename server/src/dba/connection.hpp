//
//  connection.cpp
//  epidb
//
//  Created by Felipe Albrecht on 02.02.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights reserved.
//

#include <memory>
#include <string>

#include <mongo/client/dbclient.h>


namespace epidb {
  namespace dba {

    class Connection {
    public:
      Connection();
      explicit Connection(const std::string &host);
      explicit Connection(const mongo::ConnectionString &cs);
      //virtual ~Connection();

      mongo::DBClientBase *operator->();
      mongo::DBClientBase &conn();
      mongo::DBClientBase *get();

      mongo::ConnectionString getConnectionString();

      void done();
      bool ok();
    private:
      mongo::ConnectionString m_ConnString;
      std::unique_ptr<mongo::DBClientBase> m_Connection;
    };

  }
}