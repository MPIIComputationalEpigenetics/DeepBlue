//
//  parser_factory.cpp
//  epidb
//
//  Created by Felipe Albrecht on 05.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <string>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include "../dba/metafield.hpp"

#include "parser_factory.hpp"

#include "../log.hpp"


namespace epidb {
  namespace parser {

    Parser::Parser(const std::string &content, FileFormat &format) :
      actual_line_content_(""),
      actual_line_(0),
      first_column_useless(false),
      input_(content),
      first_(true),
      format_(format)
    {
    }

    bool Parser::parse_line(Tokens &tokens)
    {
      if (input_.eof()) {
        return false;
      }

      std::string line;
      std::getline(input_, line);
      actual_line_content_ = line;

      boost::trim(line);

      if (line.empty()) {
        return false;
      }

      if (first_) {
        std::string buf;
        std::stringstream ss(line);
        std::string s;
        ss >> s;
        first_column_useless = s.find_first_not_of("0123456789") == std::string::npos;
        first_ = false;
      }

      std::string buf;
      std::stringstream ss(line);

      std::vector<std::string> strs;
      boost::split(strs, line, boost::is_any_of("\t"));

      std::vector<std::string>::iterator it = strs.begin();
      if (first_column_useless) {
        it++;
      }

      for (; it < strs.end(); it++) {
        std::string s = *it;
        boost::trim(s);
        tokens.push_back(s);
      }

      actual_line_++;
      return true;
    }

    bool Parser::check_length(const Tokens &tokens)
    {
      if (tokens.size() != format_.size() ) {
        return false;
      }

      return true;
    }

    size_t Parser::count_fields()
    {
      return format_.size();
    }

    bool Parser::eof()
    {
      return input_.eof();
    }

    size_t Parser::actual_line()
    {
      return actual_line_;
    }

    const std::string Parser::actual_line_content()
    {
      return actual_line_content_;
    }

    const FileFormat FileFormat::default_format()
    {
      static FileFormat DEFAULT_FORMAT = FileFormat::default_format_builder();
      return DEFAULT_FORMAT;
    }

    const FileFormat FileFormat::default_format_builder()
    {
      FileFormat format;

      dba::columns::ColumnTypePtr chromosome;
      dba::columns::ColumnTypePtr start;
      dba::columns::ColumnTypePtr end;

      std::string msg;

      format.set_format("CHROMOSOME,START,END");

      if (!dba::columns::load_column_type("CHROMOSOME", chromosome, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("START", start, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("END", end, msg)) {
        EPIDB_LOG_ERR(msg);
      }

      format.add(chromosome);
      format.add(start);
      format.add(end);

      return format;
    }

    bool FileFormatBuilder::build_for_outout(const std::string &format, FileFormat &file_format,
        std::vector<mongo::BSONObj> experiment_columns, std::string &msg )

    {
      if (format.empty()) {
        file_format = FileFormat::default_format();
        file_format.set_format("CHROMOSOME,START,END");
        return true;
      }

      file_format.set_format(format);
      std::vector<std::string> fields_string;
      boost::split(fields_string, format, boost::is_any_of(","));

      BOOST_FOREACH(const std::string & field_string, fields_string) {
        if (field_string.empty()) {
          msg = "Column name is empty";
          return false;
        }
        std::vector<std::string> field_info;
        boost::split(field_info, field_string, boost::is_any_of(":"));

        size_t s = field_info.size();
        if (s == 1) {
          dba::columns::ColumnTypePtr column_type;
          bool found = false;

          // Look into experiment columns
          BOOST_FOREACH(const mongo::BSONObj& column, experiment_columns) {
            if (found) {
              break;
            }
            if (column["name"].str() == field_info[0]) {
              if (!dba::columns::column_type_bsonobj_to_class(column, column_type, msg)) {
                return false;
              } else {
                found = true;
              }
            }
          }

          // Check if it is metafield
          if (dba::Metafield::is_meta(field_string)) {
            if (!dba::Metafield::build_column(field_info[0], column_type, msg)) {
              return false;
            }
            found = true;
          }
          // Load from database
          if (!found && dba::columns::load_column_type(field_info[0], column_type, msg)) {
            found = true;
          }

          // Create own column
          if (!found && dba::columns::column_type_simple(field_info[0], "string", "", column_type, msg)) {
            found = true;
          }

          if (found) {
            file_format.add(column_type);
          } else {
            msg = "Unable to build column " + field_info[0];
            return false;
          }

        } else if (s == 2) {
          dba::columns::ColumnTypePtr column_type;

          // Check if it is metafield and has default value
          if (dba::Metafield::is_meta(field_string)) {
            std::cerr << field_info[1] << std::endl;
            if (!dba::Metafield::build_column(field_info[0], field_info[1], column_type, msg)) {
              return false;
            }
          } else {
            msg = "Invalid column " + field_info[0] + ". Please inform the column type inside the column definition: '" + field_info[0] + ":(integer|double|string):" + field_info[1] +"'";
            return false;
          }
          file_format.add(column_type);
        } else if (s >= 3) {
          dba::columns::ColumnTypePtr column_type;
          if (!dba::columns::column_type_simple(field_info[0], field_info[1], field_info[2], column_type, msg)) {
            return false;
          }
          file_format.add(column_type);
        } else {
          msg = "File Format Error: Invalid field '" + field_string + "'";
          return false;
        }
      }

      return true;
    }

    bool FileFormatBuilder::build(const std::string &format, FileFormat &file_format, std::string &msg )
    {
      if (format.empty()) {
        file_format = FileFormat::default_format();
        file_format.set_format("CHROMOSOME,START,END");
        return true;
      }

      file_format.set_format(format);
      std::vector<std::string> fields_string;
      boost::split(fields_string, format, boost::is_any_of(","));

      BOOST_FOREACH(const std::string & field_string, fields_string) {
        std::vector<std::string> field_info;
        boost::split(field_info, field_string, boost::is_any_of(":"));

        size_t s = field_info.size();
        if (s == 1) {
          dba::columns::ColumnTypePtr column_type;
          // Load from database
          if (!dba::columns::load_column_type(field_info[0], column_type, msg)) {
            msg = "Error loading column type: '" + field_info[0] + "'";
            return false;
          }
          file_format.add(column_type);
        } else if (s == 2) {
          dba::columns::ColumnTypePtr column_type;
          if (!dba::columns::column_type_simple(field_info[0], field_info[1], "", column_type, msg)) {
            return false;
          }
          file_format.add(column_type);
        } else if (s >= 3) {
          dba::columns::ColumnTypePtr column_type;
          if (!dba::columns::column_type_simple(field_info[0], field_info[1], field_info[2], column_type, msg)) {
            return false;
          }
          file_format.add(column_type);
        } else {
          msg = "File Format Error: Invalid field '" + field_string + "'";
          return false;
        }
      }

      return true;
    }
  }
}
