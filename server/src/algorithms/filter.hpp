//
//  filter.cpp
//  epidb
//
//  Created by Felipe Albrecht on 22.09.13.
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

#ifndef EPIDB_DBA_FILTER_HPP
#define EPIDB_DBA_FILTER_HPP

#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include "../extras/utils.hpp"

namespace epidb {
  namespace algorithms {

    class Filter {
    public:
      enum Type {
        STRING,
        NUMBER
      };


    protected:
      Type type;

      std::string s_value;
      Score n_value;

      bool check(Type t)
      {
        return type == t;
      }


    public:
      Filter(const std::string &value) :
        type(STRING),
        s_value(value),
        n_value(0.0)
      { }

      Filter(const Score value) :
        type(NUMBER),
        s_value(),
        n_value(value)
      { }

      virtual bool is(const std::string &value) = 0;
      virtual bool is(const Score value) = 0;

      virtual ~Filter() {}
    };

    class EqualsFilter: public Filter {
    public:
      EqualsFilter(const std::string &value): Filter(value) { }
      EqualsFilter(const Score value): Filter(value) { }

      bool is(const std::string &value)
      {
        if (check(STRING) && value == s_value) {
          return true;
        } else if (check(NUMBER)) {
          Score s;
          if (!utils::string_to_score(value, s)) {
            return false;
          }
          return std::fabs(s - n_value) < std::numeric_limits<Score>::epsilon();
        }
        return false;
      }

      bool is(const Score value)
      {
        if (!check(NUMBER)) {
          return false;
        }
        return std::fabs(value - n_value) < std::numeric_limits<Score>::epsilon();
      }
    };

    class NotEqualsFilter: public Filter {
    public:
      NotEqualsFilter(const std::string &value): Filter(value) { }
      NotEqualsFilter(const Score value): Filter(value) { }

      bool is(const std::string &value)
      {
        if (check(STRING) && value != s_value) {
          return true;
        } else if (check(NUMBER)) {
          Score s;
          if (!utils::string_to_score(value, s)) {
            return false;
          }
          return std::fabs(s - n_value) > std::numeric_limits<Score>::epsilon();
        }
        return false;
      }

      bool is(const Score value)
      {
        if (!check(NUMBER)) {
          return false;
        }
        return std::fabs(value - n_value) > std::numeric_limits<Score>::epsilon();
      }
    };

    class GreaterFilter: public Filter {
    public:
      GreaterFilter(const std::string &value): Filter(value) { }
      GreaterFilter(const Score value): Filter(value) { }

      bool is(const std::string &value)
      {
        if (!check(NUMBER)) {
          return false;
        }
        Score s;
        if (!utils::string_to_score(value, s)) {
          return false;
        }

        return s > n_value + std::numeric_limits<Score>::epsilon();
      }

      bool is(const Score value)
      {
        if (!check(NUMBER)) {
          return false;
        }

        return value > n_value;
      }
    };

    class GreaterEqualsFilter: public Filter {
    public:
      GreaterEqualsFilter(const std::string &value): Filter(value) { }
      GreaterEqualsFilter(const Score value): Filter(value) { }

      bool is(const std::string &value)
      {
        if (!check(NUMBER)) {
          return false;
        }
        Score s;
        if (!utils::string_to_score(value, s)) {
          return false;
        }
        return s >= n_value + std::numeric_limits<Score>::epsilon();
      }

      bool is(const Score value)
      {
        if (!check(NUMBER)) {
          return false;
        }
        return value >= n_value;
      }
    };

    class LessFilter: public Filter {
    public:
      LessFilter(const std::string &value): Filter(value) { }
      LessFilter(const Score value): Filter(value) { }

      bool is(const std::string &value)
      {
        if (!check(NUMBER)) {
          return false;
        }
        Score s;
        if (!utils::string_to_score(value, s)) {
          return false;
        }
        return s < n_value ;
      }

      bool is(const Score value)
      {
        if (!check(NUMBER)) {
          return false;
        }
        return value < n_value;
      }
    };

    class LessEqualsFilter: public Filter {
    public:
      LessEqualsFilter(const std::string &value): Filter(value) { }
      LessEqualsFilter(const Score value): Filter(value) { }

      bool is(const std::string &value)
      {
        if (!check(NUMBER)) {
          return false;
        }
        Score s;
        if (!utils::string_to_score(value, s)) {
          return false;
        }
        return s <= n_value;
      }

      bool is(const Score value)
      {
        if (!check(NUMBER)) {
          return false;
        }
        return value <= n_value;
      }
    };

    class FilterBuilder {

    public:

      class {
        // TODO: move to utils directory
      public:
        template<typename T>
        operator std::shared_ptr<T>()
        {
          return std::shared_ptr<T>();
        }
      } nullPtr;


      static FilterBuilder &getInstance()
      {
        static FilterBuilder instance;
        return instance;
      }

      typedef std::shared_ptr<Filter> FilterPtr;

    private:

      static std::vector<std::string> build_operations()
      {
        std::vector<std::string> operations;

        operations.push_back("==");
        operations.push_back("!=");
        operations.push_back(">");
        operations.push_back(">=");
        operations.push_back("<");
        operations.push_back("<=");

        return operations;
      }

      FilterBuilder( )
      {

      }
      FilterBuilder(const FilterBuilder &);

      bool check_name(const std::string &s, const std::string &type, std::string &msg)
      {
        if (s.length() == 0) {
          msg = "The given value for " + type + " is empty.";
          return false;
        }
        static const std::string invalid_start("$");
        if (s.find(invalid_start) != std::string::npos) {
          msg = "The given value(" + s + ") for " + type + " is invalid. Please, do not use '$'";
          return false;
        }

        return true;
      }

      bool check_operation(const std::string &operation, const std::string &type, std::string &msg)
      {
        if (type == "string") {
          if ((operation == "==") || (operation == "!=")) {
            return true;
          } else {
            msg = "Only equals (==) or not equals (!=) are available for type 'string'";
            return false;
          }
        } else if (type == "number") {
          for(const std::string & op: operations()) {
            if (operation == op) {
              return true;
            }
          }
        }
        msg = "Operation (" + operation + ") is not supported.";
        return false;
      }

    public:
      static std::vector<std::string> &operations()
      {
        static std::vector<std::string> operations = build_operations();
        return operations;
      }

      static bool is_valid_operations(const std::string &in_op)
      {
        for (std::string op : operations()) {
          if (in_op == op) {
            return true;
          }
        }
        return false;
      }

      FilterPtr build(const std::string &field, const std::string &op, const std::string &value,
                      bool &error, std::string &msg)
      {
        if (!check_name(field, "field", msg)) {
          error = true;
          return nullPtr;
        }

        if (!check_name(value, "value", msg)) {
          error = true;
          return nullPtr;
        }

        if (!check_operation(op, "string", msg)) {
          error = true;
          return nullPtr;
        }

        if (op == "==") {
          error = false;
          return std::shared_ptr<Filter>(new EqualsFilter(value));
        }

        if (op == "!=") {
          error = false;
          return std::shared_ptr<Filter>(new NotEqualsFilter(value));
        }

        error = true;
        msg = "Invalid command";
        return nullPtr;
      }

      FilterPtr build(const std::string &field, const std::string &op, const Score value,
                      bool &error, std::string &msg)
      {
        if (!check_name(field, "field", msg)) {
          error = true;
          return nullPtr;
        }

        if (op == "==") {
          error = false;
          return std::shared_ptr<Filter>(new  EqualsFilter(value));
        }

        if (op == "!=") {
          error = false;
          return std::shared_ptr<Filter>(new  NotEqualsFilter(value));
        }

        if (op == ">") {
          error = false;
          return std::shared_ptr<Filter>(new  GreaterFilter(value));
        }

        if (op == ">=") {
          error = false;
          return std::shared_ptr<Filter>(new  GreaterEqualsFilter(value));
        }

        if (op == "<") {
          error = false;
          return std::shared_ptr<Filter>(new  LessFilter(value));
        }

        if (op == "<=") {
          error = false;
          return std::shared_ptr<Filter>(new  LessEqualsFilter(value));
        }

        error = true;
        msg = "Operation (" + op + ") is not supported.";
        return nullPtr;
      }

    };
  }
}

#endif