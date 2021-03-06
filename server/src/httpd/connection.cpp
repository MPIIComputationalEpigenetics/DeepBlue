//
//  connection.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.05.13.
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

#include <regex>
#include <string>
#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "connection.hpp"
#include "download.hpp"
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

        static std::string DOWNLOAD_STRING("/download");
        if (request_.path.substr(0, DOWNLOAD_STRING.length()) == DOWNLOAD_STRING) {
          reply_ = get_download_data(request_.path);
          buffers_ = reply_.to_buffers();
          boost::asio::async_write(socket_, reply_.to_buffers(),
                                   strand_.wrap(
                                      boost::bind(&Connection::handle_write, shared_from_this(),
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred)));
        } else {
          m_content_->reserve(m_expected_length);
          request_.ip =  socket_.remote_endpoint().address().to_string();
          if (result) {
            read_data();
          } else if (error == boost::asio::error::eof) {
            EPIDB_LOG_WARN("EOF: Connection :"  << id_);
          } else {
            reply_ = Reply::stock_reply(Reply::bad_request, "");
            buffers_ = reply_.to_buffers();
            boost::asio::async_write(socket_, reply_.to_buffers(),
                                     strand_.wrap(
                                       boost::bind(&Connection::handle_write, shared_from_this(),
                                                   boost::asio::placeholders::error,
                                                   boost::asio::placeholders::bytes_transferred)));
          }
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
        char buf[4096] = {};
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
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred)));

      } else {

        boost::asio::async_read(
          socket_, streambuf_,
          boost::asio::transfer_at_least(1),
          strand_.wrap(boost::bind(&Connection::handle_content,
                                   shared_from_this(), boost::asio::placeholders::error,
                                   boost::asio::placeholders::bytes_transferred)));
      }
    }

    void Connection::handle_write(const boost::system::error_code &e, size_t bytes_transferred)
    {
      if (e) {
        EPIDB_LOG(e.category().name() << ":" << e.message() << " - Sent " << bytes_transferred << " bytes.");
      } else {
        boost::system::error_code ignored_ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
      }
    }

  } // namespace httpd
} // namespace epidb
