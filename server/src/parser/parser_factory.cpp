//
//  parser_factory.cpp
//  epidb
//
//  Created by Felipe Albrecht on 05.06.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>

#include "../dba/metafield.hpp"
#include "../extras/utils.hpp"

#include "parser_factory.hpp"

#include "../errors.hpp"
#include "../log.hpp"


namespace epidb {
  namespace parser {

    size_t BedLine::size() const
    {
      return tokens.size() + 3;
    }

    bool operator<(const BedLine &a, const BedLine &b)
    {
      return a.start < b.start;
    }

    Parser::Parser(const std::string &content, FileFormat &format) :
      actual_line_content_(""),
      actual_line_(0),
      first_column_useless(false),
      input_(content),
      first_(true),
      format_(format),
      chromosome_pos(-1),
      start_pos(-1),
      end_pos(-1)
    {
      // TODO: check duplicated fields
      // TODO: check missing fields
      int count = 0;
      for (auto &column : format_) {
        if (column->name() == "CHROMOSOME") {
          chromosome_pos = count;
        }

        if (column->name() == "START") {
          start_pos = count;
        }

        if (column->name() == "END") {
          end_pos = count;
        }
        count++;
      }
    }

    bool Parser::check_format(std::string &msg)
    {
      if (chromosome_pos == -1) {
        msg = Error::m(ERR_FORMAT_CHROMOSOME_MISSING);
        return false;
      }
      if (start_pos == -1) {
        msg = Error::m(ERR_FORMAT_START_MISSING);
        return false;
      }
      if (end_pos == -1) {
        msg = Error::m(ERR_FORMAT_END_MISSING);
        return false;
      }

      return true;
    }

    bool Parser::parse_line(BedLine &bed_line, std::string &msg)
    {
      if (input_.eof()) {
        msg = "Unexpected EOF";
        return false;
      }
      std::string line;
      std::getline(input_, line);
      actual_line_content_ = line;

      boost::trim(line);

      if (line.empty()) {
        msg = "Empty line";
        return false;
      }

      if (first_) {
        std::string buf;
        std::stringstream ss(line);
        std::string first;
        ss >> first;
        std::string second;
        ss >> second;

        first_column_useless = (chromosome_pos == 0 &&
          (first.find_first_not_of("0123456789") == std::string::npos) &&
          (second.compare(0, 3, std::string("chr")) == 0));

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

      for (int pos = 0; it < strs.end(); pos++, it++) {
        std::string s = *it;
        boost::trim(s);

        if (pos == chromosome_pos) {
          bed_line.chromosome = s;
        } else if (pos == start_pos) {
          Position start;
          if (!utils::string_to_position(s, start)) {
            msg = s + " is not a valid start position";
            return false;
          }
          bed_line.start = start;
        } else if (pos == end_pos) {
          Position end;
          if (!utils::string_to_position(s, end)) {
            msg = s + " is not a valid end position";
            return false;
          }
          bed_line.end = end;
        } else {
          bed_line.tokens.push_back(s);
        }
      }

      actual_line_++;
      return true;
    }

    bool Parser::check_length(const BedLine &bed_line)
    {
      // +3 for the mandatory fields // chrom, start, end
      if (bed_line.size() != format_.size() ) {
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

    const FileFormat FileFormat::wig_format()
    {
      static FileFormat WIG_FORMAT = FileFormat::wig_format_builder();
      return WIG_FORMAT;
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

    const FileFormat FileFormat::wig_format_builder()
    {
      FileFormat format;

      dba::columns::ColumnTypePtr chromosome;
      dba::columns::ColumnTypePtr start;
      dba::columns::ColumnTypePtr end;
      dba::columns::ColumnTypePtr value;

      std::string msg;

      format.set_format("CHROMOSOME,START,END,VALUE");

      if (!dba::columns::load_column_type("CHROMOSOME", chromosome, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("START", start, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("END", end, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("VALUE", value, msg)) {
        EPIDB_LOG_ERR(msg);
      }

      format.add(chromosome);
      format.add(start);
      format.add(end);
      format.add(value);

      return format;
    }

    mongo::BSONArray FileFormat::to_bson() const
    {
      mongo::BSONArrayBuilder ab;

      for (parser::FileFormat::const_iterator it =  begin(); it != end(); it++) {
        mongo::BSONObj o = (*it)->BSONObj();
        ab.append(o);
      }
      return ab.arr();
    }

    bool FileFormatBuilder::build_for_outout(const std::string &format, const std::vector<mongo::BSONObj> &experiment_columns, FileFormat &file_format, std::string &msg )
    {
      if (format.empty()) {
        file_format = FileFormat::default_format();
        file_format.set_format("CHROMOSOME,START,END");
        return true;
      }

      file_format.set_format(format);
      std::vector<std::string> fields_string;
      boost::split(fields_string, format, boost::is_any_of(","));

      BOOST_FOREACH(std::string & field_string, fields_string) {

        boost::trim(field_string);

        if (field_string.empty()) {
          msg = Error::m(ERR_FORMAT_COLUMN_NAME_MISSING);
          return false;
        }

        dba::columns::ColumnTypePtr column_type;
        bool found = false;

        // Look into experiment columns
        BOOST_FOREACH(const mongo::BSONObj & column, experiment_columns) {
          if (found) {
            break;
          }
          if (column["name"].str() == field_string) {
            if (!dba::columns::column_type_bsonobj_to_class(column, column_type, msg)) {
              return false;
            } else {
              found = true;
            }
          }
        }

        // Check if it is metafield
        if (dba::Metafield::is_meta(field_string)) {
          if (!build_metafield_column(field_string, column_type, msg)) {
            return false;
          }
          found = true;
        }
        // Load from database
        if (!found && dba::columns::load_column_type(field_string, column_type, msg)) {
          found = true;
        }

        if (found) {
          file_format.add(column_type);
        } else {
          msg = Error::m(ERR_INVALID_COLUMN_NAME, field_string.c_str());
          return false;
        }
      }

      return true;
    }

    bool FileFormatBuilder::build(const std::string &format, FileFormat &file_format, std::string &msg)
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
          if (!dba::columns::column_type_simple(field_info[0], field_info[1], column_type, msg)) {
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

    bool FileFormatBuilder::build_metafield_column(const std::string &op,
        dba::columns::ColumnTypePtr &column_type, std::string &msg)
    {
      static const std::string open_parenthesis("(");
      std::string command = op.substr(0, op.find(open_parenthesis));
      std::string type = dba::Metafield::command_type(command);
      return dba::columns::column_type_simple(op, type, column_type, msg);
    }
  }
}
