/*
 * Connection.cpp
 *
 *  Created on: 01 Dec 2014
 *      Author: pieter
 */

#include <iostream>

#include "../connection/connection.hpp"

#include "../config/config.hpp"
#include "../log.hpp"

namespace epidb {
  Connection::Connection() :
    Connection(config::get_mongodb_server_connection())
  {
  }

  Connection::Connection(const std::string &host) : m_Connection(g_Pool.get(host))
  {
    std::string errMessage;
    m_ConnString = mongo::ConnectionString::parse(host, errMessage);
    if (!errMessage.empty()) {
      EPIDB_LOG_ERR(errMessage);
    }
  }

  Connection::Connection(const mongo::ConnectionString &cs) : m_ConnString(cs), m_Connection(g_Pool.get(cs)) { }


  Connection::~Connection()
  {
    if (m_Connection) {
      if (m_Connection->isFailed()) {
        if (m_Connection->getSockCreationMicroSec() == mongo::DBClientBase::INVALID_SOCK_CREATION_TIME) {
          //delete m_Connection;
        } else {
          done();
        }
      } else {
        EPIDB_LOG_WARN("scoped connection to " << m_Connection->getServerAddress() << " not being returned to the pool");
        //delete m_Connection;
      }
    }
  }

  mongo::DBClientBase *Connection::operator ->()
  {
    return m_Connection;
  }

  mongo::DBClientBase &Connection::conn()
  {
    return *m_Connection;
  }

  mongo::DBClientBase *Connection::get()
  {
    return m_Connection;
  }

  void Connection::done()
  {
    if (!m_Connection) {
      return;
    }
    g_Pool.release(m_ConnString, m_Connection);
    m_Connection = 0;
  }

  bool Connection::ok()
  {
    return m_Connection != 0;
  }
}
