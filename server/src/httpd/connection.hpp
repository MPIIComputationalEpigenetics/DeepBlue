//
//  connection.hpp
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
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EPIDB_HTTPD_CONNECTION_HPP
#define EPIDB_HTTPD_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <memory>

#include "network.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

namespace epidb {
  namespace httpd {

    static unsigned long long CONNECTION_COUNT = 0;


    /// Represents a single connection from a client.
    class Connection
      : public std::enable_shared_from_this<Connection>,
        private boost::noncopyable {
    public:
      /// Construct a connection with the given io_service.
      explicit Connection(boost::asio::io_service &io_service,
                          request_handler &handler);

      /// Get the socket associated with the connection.
      boost::asio::ip::tcp::socket &socket();

      /// Start the first asynchronous operation for the connection.
      void start();

    private:
      std::streamsize m_read_length;
      std::streamsize m_expected_length;
      vector_ptr m_content_;

      /// Handle completion of a read operation.
      void handle_read(const boost::system::error_code &e, std::size_t bytes_transferred);

      /// Handle completion of a write operation.
      void handle_write(const boost::system::error_code &e, std::size_t bytes_transferred);

      void handle_content(const boost::system::error_code &e, std::size_t bytes_transferred);

      void handle_download(const std::string& uri);

      void read_data();

      /// Strand to ensure the connection's handlers are not called concurrently.
      boost::asio::io_service::strand strand_;

      /// Socket for the connection.
      boost::asio::ip::tcp::socket socket_;

      /// The handler used to process the incoming request.
      request_handler &request_handler_;

      /// Buffer for incoming data.
      boost::asio::streambuf streambuf_;

      /// The reply to be sent back to the client.
      Reply reply_;

      std::string content_;

      std::vector<boost::asio::const_buffer> buffers_;

      /// The parser for the incoming request.
      request_parser request_parser_;

      // Connection id
      unsigned long long id_;

      /// The incoming request.
      Request request_;

    };

    typedef std::shared_ptr<Connection> connection_ptr;

  } // namespace httpd
} // namespace epidb
#endif // EPIDB_HTTPD_CONNECTION_HPP
