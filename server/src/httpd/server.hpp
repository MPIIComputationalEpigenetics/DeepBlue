//
//  server.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
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
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EPIDB_HTTPD_SERVER_HPP
#define EPIDB_HTTPD_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <memory>
#include "connection.hpp"
#include "request_handler.hpp"

namespace epidb {
  namespace httpd {

    /// The top-level class of the HTTP server.
    class server
    : private boost::noncopyable
    {
    public:
      /// Construct the server to listen on the specified TCP address and port, and
      /// serve up files from the given directory.
      explicit server(const std::string& address, const std::string& port, std::size_t thread_pool_size);

      /// Run the server's io_service loop.
      void run();

    private:
      /// Initiate an asynchronous accept operation.
      void start_accept();

      /// Handle completion of an asynchronous accept operation.
      void handle_accept(const boost::system::error_code& e);

      /// Handle a request to stop the server.
      void handle_stop();

      /// The number of threads that will call io_service::run().
      std::size_t thread_pool_size_;

      /// The io_service used to perform asynchronous operations.
      boost::asio::io_service io_service_;

      /// The signal_set is used to register for process termination notifications.
      boost::asio::signal_set signals_;

      /// Acceptor used to listen for incoming connections.
      boost::asio::ip::tcp::acceptor acceptor_;

      /// The next connection to be accepted.
      connection_ptr new_connection_;

      /// The handler for all incoming requests.
      request_handler request_handler_;
    };

  } // namespace httpd
} // namespace epidb

#endif // EPIDB_HTTPD_SERVER_HPP
