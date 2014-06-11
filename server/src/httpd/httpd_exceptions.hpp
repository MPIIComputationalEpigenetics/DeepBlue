//
//  httpd_exceptions.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_HTTPD_EXCEPTIONS_HPP
#define EPIDB_HTTPD_EXCEPTIONS_HPP

#include <exception>

#include "boost/format.hpp"

namespace epidb {
  namespace httpd {

    class illegal_number_of_parameters_exception: public std::exception
    {
    public:
      illegal_number_of_parameters_exception(const size_t expected_parameters, const size_t parameters_count) :
      expected_parameters_(expected_parameters),
      parameters_count_(parameters_count)
      {
      }
    private:
      size_t expected_parameters_;
      size_t parameters_count_;

      virtual const char* what() const throw()
      {
        boost::format format("Invalid number of parameters. Expecting %1% but received %1%");
        format % expected_parameters_;
        format %  parameters_count_;

        return format.str().c_str();
      }
    };

    class illegal_argument_type_exception: public std::exception
    {
    public:
      illegal_argument_type_exception(const std::string expected_type, const std::string received_type) :
      expected_type_(expected_type),
      received_type_(received_type)
      {
      }

      virtual ~illegal_argument_type_exception() throw() { }
    private:
      std::string expected_type_;
      std::string received_type_;

      virtual const char* what() const throw()
      {
        return "Invalid number of parameters";
      }
    };


  }
}

#endif
