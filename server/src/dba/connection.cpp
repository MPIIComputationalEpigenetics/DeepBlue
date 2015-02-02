/*
 * Connection.cpp
 *
 *  Created on: 01 Dec 2014
 *      Author: pieter
 */

#include <iostream>

#include "config.hpp"
#include "connection.hpp"

#include "../log.hpp"

namespace epidb {
  namespace dba {
    Connection::Connection() :
      Connection(config::get_mongodb_server_connection())
    {
    }

    Connection::Connection(const std::string &host)
    {
      std::string errMessage;
      m_ConnString = mongo::ConnectionString::parse(host, errMessage);
      if (!errMessage.empty()) {
        EPIDB_LOG_ERR(errMessage);
      }
      Connection(m_ConnString);
    }

    Connection::Connection(const mongo::ConnectionString &cs) : m_ConnString(cs)
    {
      std::string errMessage;
      m_Connection.reset(m_ConnString.connect(errMessage));
      if (!m_Connection) {
        EPIDB_LOG_ERR("Could not connect to " << m_ConnString.getServers()[0].toString() << ": " << errMessage);
      }
    }

    mongo::DBClientBase *Connection::operator ->()
    {
      return m_Connection.get();
    }

    mongo::DBClientBase &Connection::conn()
    {
      return *m_Connection;
    }

    mongo::DBClientBase *Connection::get()
    {
      return m_Connection.get();
    }

    void Connection::done()
    {
      /* nothing */
    }

    bool Connection::ok()
    {
      return m_Connection != 0;
    }
  }

}
