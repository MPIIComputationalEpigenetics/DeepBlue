//
//  connection.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//
//
// based on:
//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "connection.hpp"

#include "request_handler.hpp"

#include "../log.hpp"

namespace epidb {
  namespace httpd {

    Connection::Connection(boost::asio::io_service &io_service,
                           request_handler &handler)
      :
      m_read_length(0),
      m_expected_length(0),
      m_content_(new std::vector<char>()),
      strand_(io_service),
      socket_(io_service),
      request_handler_(handler),
      id_(CONNECTION_COUNT++),
      request_(id_)
    {
      EPIDB_LOG("Starting connection " << id_);
    }

    boost::asio::ip::tcp::socket &Connection::socket()
    {
      return socket_;
    }

    void Connection::start()
    {
      //PROFINY_SCOPE
      boost::asio::async_read_until(socket_, streambuf_, "\r\n\r\n",
                                    strand_.wrap(boost::bind(&Connection::handle_read,
                                        shared_from_this(), boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred)));

    }

    void Connection::handle_read(const boost::system::error_code &error,
                                 std::size_t bytes_transferred)
    {
      if (!error) {
        bool result = request_parser_.parse(request_, &streambuf_);
        m_expected_length = request_.content_length;
        request_.ip =  socket_.remote_endpoint().address().to_string();
        if (result) {
          read_data();
        } else if (error == boost::asio::error::eof) {
          EPIDB_LOG_WARN("EOF: Connection :"  << id_);
        } else {
          Reply reply = Reply::stock_reply(Reply::bad_request, "");
          boost::asio::write(socket_, reply.to_buffers());
        }
      }
    }

    void Connection::handle_content(const boost::system::error_code &error, std::size_t bytes_transferred)
    {
      if (!error) {
        read_data();
      } else if (error == boost::asio::error::eof) {
        EPIDB_LOG_WARN("EOF: Connection :"  << id_)
      } else {
        EPIDB_LOG_ERR(error.message());
      }
    }

    void Connection::read_data()
    {
      std::istream stream(&streambuf_);

      while (!stream.eof()) {
        char buf[4096];
        stream.read(buf, 4096);
        std::streamsize tocopy = stream.gcount();
        if ((m_read_length + tocopy > m_expected_length) && m_expected_length) {
          tocopy = m_expected_length - m_read_length;
        }
        std::copy(buf, buf + tocopy, std::back_inserter(*m_content_.get()));

        m_read_length += tocopy;
      }

      if (m_read_length == m_expected_length) {
        content_ = std::string(m_content_.get()->begin(), m_content_.get()->end());
        m_content_.reset();
        reply_ = request_handler_.handle_request(request_, content_);
        buffers_ = reply_.to_buffers();
        boost::asio::async_write(socket_, reply_.to_buffers(),
                                 strand_.wrap(
                                   boost::bind(&Connection::handle_write, shared_from_this(),
                                               boost::asio::placeholders::error)));
      } else {

        boost::asio::async_read(
          socket_, streambuf_,
          boost::asio::transfer_at_least(1),
          strand_.wrap(boost::bind(&Connection::handle_content,
                                   shared_from_this(), boost::asio::placeholders::error,
                                   boost::asio::placeholders::bytes_transferred)));
      }
    }

    void Connection::handle_write(const boost::system::error_code &e)
    {
      //PROFINY_SCOPE
      if (!e) {
        // Initiate graceful connection closure.
        boost::system::error_code ignored_ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
      }
    }

  } // namespace httpd
} // namespace epidb
