//
//  parser_factory.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 05.06.13.
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

#include <sstream>
#include <string>
#include <limits>

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

    Parser::Parser(std::unique_ptr<std::istream> &&input, FileFormat &format) :
      actual_line_content_(""),
      actual_line_(0),
      first_column_useless(false),
      input_(std::move(input)),
      first_(true),
      format_(format),
      chromosome_pos(std::numeric_limits<size_t>::max()),
      start_pos(std::numeric_limits<size_t>::max()),
      end_pos(std::numeric_limits<size_t>::max())
    {
      // TODO: check duplicated fields
      // TODO: check missing fields
      size_t count = 0;
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
      if (chromosome_pos == std::numeric_limits<size_t>::max()) {
        msg = Error::m(ERR_FORMAT_CHROMOSOME_MISSING);
        return false;
      }
      if (start_pos == std::numeric_limits<size_t>::max()) {
        msg = Error::m(ERR_FORMAT_START_MISSING);
        return false;
      }
      if (end_pos == std::numeric_limits<size_t>::max()) {
        msg = Error::m(ERR_FORMAT_END_MISSING);
        return false;
      }

      return true;
    }

    bool Parser::parse_line(BedLine &bed_line, std::string &msg)
    {
      std::string line;

      do {
        if (input_->eof()) {
          msg = "EOF";
          return true;
        }
        std::getline(*input_, line);
        actual_line_content_ = line;
        boost::trim(line);

        if (line.empty()) {
          msg = "Empty line";
          return false;
        }

      } while ((line[0] == '#') || (line.substr(0, 5) == "track") || (line.substr(0, 7) == "browser") || (line.compare(0, 9, "chr\tstart") == 0));

      if (first_) {
        std::stringstream ss(line);
        std::string first;
        ss >> first;
        std::string second;
        ss >> second;


        std::vector<std::string> strs;
        boost::split(strs, line, boost::is_any_of("\t"));

        if (strs.size() == format_.size()) {
          first_column_useless = false;
        } else if (strs.size() - 1 == format_.size()) {
          first_column_useless = true;
        } else {
          std::stringstream m;
          m << "Error while reading the BED file. Line: 1 - '";
          m << line;
          m << "'. The number of tokens (" ;
          m << strs.size() ;
          m << ") is different from the format size (" ;
          m << format_.size();
          m << ") - ";
          m << format_.format();
          msg = m.str();
          return false;
        }

        first_ = false;
      }

      std::vector<std::string> strs;
      boost::split(strs, line, boost::is_any_of("\t"));

      std::vector<std::string>::iterator it = strs.begin();
      if (first_column_useless) {
        it++;
      }

      for (size_t pos = 0; it < strs.end(); pos++, it++) {
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
      return input_->eof();
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

    const FileFormat FileFormat::gtf_format()
    {
      static FileFormat GTF_FORMAT = FileFormat::gtf_format_builder();
      return GTF_FORMAT;
    }

    const FileFormat FileFormat::default_format_builder()
    {
      FileFormat format;

      dba::columns::ColumnTypePtr chromosome;
      dba::columns::ColumnTypePtr start;
      dba::columns::ColumnTypePtr end;

      processing::StatusPtr status = processing::build_dummy_status();

      std::string msg;

      format.set_format("CHROMOSOME,START,END");

      if (!dba::columns::load_column_type("CHROMOSOME", status, chromosome, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("START", status, start, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("END", status, end, msg)) {
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

      processing::StatusPtr status = processing::build_dummy_status();

      if (!dba::columns::load_column_type("CHROMOSOME", status, chromosome, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("START", status, start, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("END", status, end, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("VALUE", status, value, msg)) {
        EPIDB_LOG_ERR(msg);
      }

      format.add(chromosome);
      format.add(start);
      format.add(end);
      format.add(value);

      return format;
    }

    const FileFormat FileFormat::gtf_format_builder()
    {
      FileFormat format;

      dba::columns::ColumnTypePtr chromosome;
      dba::columns::ColumnTypePtr source;
      dba::columns::ColumnTypePtr feature;
      dba::columns::ColumnTypePtr start;
      dba::columns::ColumnTypePtr end;
      dba::columns::ColumnTypePtr score;
      dba::columns::ColumnTypePtr strand;
      dba::columns::ColumnTypePtr frame;
      dba::columns::ColumnTypePtr attributes;

      std::string msg;

      format.set_format("CHROMOSOME,SOURCE,FEATURE,START,END,GTF_SCORE,STRAND,FRAME,GTF_ATTRIBUTES");

      processing::StatusPtr status = processing::build_dummy_status();

      if (!dba::columns::load_column_type("CHROMOSOME", status, chromosome, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("SOURCE", status, source, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("FEATURE", status, feature, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("START", status, start, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("END", status, end, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("GTF_SCORE", status, score, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("STRAND", status, strand, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("FRAME", status, frame, msg)) {
        EPIDB_LOG_ERR(msg);
      }
      if (!dba::columns::load_column_type("GTF_ATTRIBUTES", status, attributes, msg)) {
        EPIDB_LOG_ERR(msg);
      }

      format.add(chromosome);
      format.add(source);
      format.add(feature);
      format.add(start);
      format.add(end);
      format.add(score);
      format.add(strand);
      format.add(frame);
      format.add(attributes);
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

    bool FileFormatBuilder::check_outout(const std::string &format, std::string &msg )
    {
      if (format.empty()) {
        return true;
      }

      std::vector<std::string> fields_string;
      boost::split(fields_string, format, boost::is_any_of(","));

      for(std::string & field_string: fields_string) {
        boost::trim(field_string);

        if (field_string.empty()) {
          msg = Error::m(ERR_FORMAT_COLUMN_NAME_MISSING);
          return false;
        }

        dba::columns::ColumnTypePtr column_type;
        bool found = false;

        // Check if it is metafield
        if (dba::Metafield::is_meta(field_string)) {
          if (!build_metafield_column(field_string, column_type, msg)) {
            return false;
          }
          found = true;
        }

        // Load from database
        if (!found && dba::columns::exists_column_type(field_string, msg)) {
          found = true;
        }

        if (!found) {
          msg = Error::m(ERR_INVALID_COLUMN_NAME, field_string);
          return false;
        }
      }

      return true;
    }


    bool FileFormatBuilder::build_for_outout(const std::string &format, const std::vector<mongo::BSONObj> &experiment_columns, processing::StatusPtr status, FileFormat &file_format, std::string &msg )
    {
      if (format.empty()) {
        file_format = FileFormat::default_format();
        file_format.set_format("CHROMOSOME,START,END");
        return true;
      }

      file_format.set_format(format);
      std::vector<std::string> fields_string;
      boost::split(fields_string, format, boost::is_any_of(","));

      for(std::string & field_string: fields_string) {
        boost::trim(field_string);

        if (field_string.empty()) {
          msg = Error::m(ERR_FORMAT_COLUMN_NAME_MISSING);
          return false;
        }

        dba::columns::ColumnTypePtr column_type;
        bool found = false;

        // Look into experiment columns
        for(const mongo::BSONObj & column: experiment_columns) {
          if (found) {
            break;
          }
          if (column["name"].str() == field_string) {
            if (!dba::columns::column_type_bsonobj_to_class(column, status, column_type, msg)) {
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
        if (!found && dba::columns::load_column_type(field_string, status, column_type, msg)) {
          found = true;
        }

        if (found) {
          file_format.add(column_type);
        } else {
          msg = Error::m(ERR_INVALID_COLUMN_NAME, field_string);
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

      for(const std::string & field_string: fields_string) {
        processing::StatusPtr status = processing::build_dummy_status();

        dba::columns::ColumnTypePtr column_type;
        // Load from database
        if (!dba::columns::load_column_type(field_string, status, column_type, msg)) {
          msg = Error::m(ERR_INVALID_COLUMN_NAME, field_string);
          return false;
        }
        file_format.add(column_type);
      }
      return true;
    }

    bool FileFormatBuilder::build_metafield_column(const std::string &op,
        dba::columns::ColumnTypePtr &column_type, std::string &msg)
    {
      static const std::string open_parenthesis("(");
      std::string command = op.substr(0, op.find(open_parenthesis));
      std::string type = dba::Metafield::command_type(command);
      if (type.empty()) {
        msg = Error::m(ERR_INVALID_META_COLUMN_NAME, command);
        return false;
      }
      return dba::columns::column_type_simple(op, type, column_type, msg);
    }
  }
}
