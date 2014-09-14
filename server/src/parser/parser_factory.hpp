//
//  parser_factory.h
//  epidb
//
//  Created by Felipe Albrecht on 05.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_PARSER_PARSER_FACTORY_HPP
#define EPIDB_PARSER_PARSER_FACTORY_HPP

#include <iostream>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include "field_type.hpp"

#include "../dba/column_types.hpp"

namespace epidb {
  namespace parser {

    typedef std::map<std::string, std::string> ParsedLine;
    typedef std::vector<Token> Tokens;

    // TODO: move to bed_format.hpp
    class FileFormat {
    private:
      std::string format_;
      std::vector<dba::columns::ColumnTypePtr> fields_;

      static const FileFormat default_format_builder();

    public:
      typedef std::vector<dba::columns::ColumnTypePtr>::iterator iterator;
      typedef std::vector<dba::columns::ColumnTypePtr>::const_iterator const_iterator;

      static const FileFormat default_format();

      void set_format(const std::string format)
      {
        format_ = format;
      }

      const std::string format() const
      {
        return format_;
      }

      size_t size() const
      {
        return fields_.size();
      }

      iterator begin()
      {
        return fields_.begin();
      }

      const_iterator begin() const
      {
        return fields_.begin();
      }

      iterator end()
      {
        return fields_.end();
      }

      const_iterator end() const
      {
        return fields_.end();
      }

      void add(dba::columns::ColumnTypePtr column)
      {
        fields_.push_back(column);
      }
    };

    class FileFormatBuilder {
    public:
      static bool build(const std::string &format, FileFormat &file_format, std::string &msg);
      static bool build_for_outout(const std::string &format, FileFormat &file_format,
                                   std::vector<mongo::BSONElement> experiment_columns, std::string &msg );
    };

    class Parser {
    private:
      Parser();
      std::string actual_line_content_;
      size_t actual_line_;
      bool first_column_useless;
      std::stringstream input_;
      bool first_;
      FileFormat format_;

    public:
      Parser(const std::string &content, FileFormat &format);
      bool parse_line(Tokens &tokens);
      bool check_length(const Tokens &tokens);
      size_t actual_line();
      const std::string actual_line_content();
      size_t count_fields();
      bool eof();
    };
  }
}

#endif
