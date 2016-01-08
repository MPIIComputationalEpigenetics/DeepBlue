//
//  parser_factory.h
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

#ifndef EPIDB_PARSER_PARSER_FACTORY_HPP
#define EPIDB_PARSER_PARSER_FACTORY_HPP


#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "../dba/column_types.hpp"

namespace epidb {
  namespace parser {

    class BedLine {
    public:
      std::string chromosome;
      unsigned int start;
      unsigned int end;
      std::vector<std::string> tokens;
      size_t size() const;
    };

    bool operator<(const BedLine &a, const BedLine &b);

    typedef std::vector<parser::BedLine> BedLines;
    typedef std::pair<std::string, BedLines> ChromosomeBedLines;

    typedef struct ChromosomeRegionsMap {
      typedef std::map<std::string, BedLines>::iterator iterator;
      typedef std::map<std::string, BedLines>::const_iterator const_iterator;
      std::map<std::string, BedLines > data_;

      iterator begin()
      {
        return data_.begin();
      }

      const_iterator begin() const
      {
        return data_.begin();
      }

      iterator end()
      {
        return data_.end();
      }

      const_iterator end() const
      {
        return data_.end();
      }

      void insert(BedLine  &&region)
      {
        const std::string &chromosome = region.chromosome;
        if (data_.find(chromosome) == data_.end()) {
          data_[chromosome] = std::vector<parser::BedLine>();
        }
        data_[chromosome].push_back(std::move(region));
      }

      void finish()
      {
        for (auto &chromosome : data_) {
          std::vector<parser::BedLine> &regions = chromosome.second;
          std::sort(regions.begin(), regions.end());
        }
      }

      std::vector<std::string> chromosomes()
      {
        std::vector<std::string> chromosomes;
        for (auto kv : data_) {
          chromosomes.push_back(kv.first);
        }
        return chromosomes;
      }

    } ChromosomeRegionsMap;

    typedef std::map<std::string, std::string> ParsedLine;
    //typedef std::vector<bed_line> Tokens;

    // TODO: move to bed_format.hpp
    class FileFormat {
    private:
      std::string format_;
      std::vector<dba::columns::ColumnTypePtr> fields_;

      static const FileFormat default_format_builder();
      static const FileFormat wig_format_builder();
      static const FileFormat gtf_format_builder();

    public:
      typedef std::vector<dba::columns::ColumnTypePtr>::iterator iterator;
      typedef std::vector<dba::columns::ColumnTypePtr>::const_iterator const_iterator;

      mongo::BSONArray to_bson() const;

      static const FileFormat default_format();
      static const FileFormat wig_format();
      static const FileFormat gtf_format();

      bool operator==(const FileFormat &other) const
      {
        if (size() != other.size()) {
          return false;
        }

        for (auto& this_field : fields_) {
          bool found = false;
          for (auto& other_field : other) {
            if (this_field->name() == other_field->name()) {
              found = true;
              break;
            }
          }
          if (!found) {
            return false;
          }
        }

        return true;
      }

      bool operator !=(const FileFormat &other) const
      {
        return !(*this == other);
      }

      void set_format(const std::string& format)
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
      static bool build_for_outout(const std::string &format, const std::vector<mongo::BSONObj> &experiment_columns, processing::StatusPtr status, FileFormat &file_format, std::string &msg);
    private:
      static bool build_metafield_column(const std::string &name, epidb::dba::columns::ColumnTypePtr &column_type, std::string &msg);
    };

    class Parser {
    private:
      Parser();
      std::string actual_line_content_;
      size_t actual_line_;
      bool first_column_useless;
      std::unique_ptr<std::istream> input_;
      bool first_;
      FileFormat format_;
      size_t chromosome_pos;
      size_t start_pos;
      size_t end_pos;

    public:
      Parser(std::unique_ptr<std::istream> &&input, FileFormat &format);
      bool check_format(std::string &msg);
      bool parse_line(BedLine &bed_line, std::string &msg);
      bool check_length(const BedLine &bed_line);
      size_t actual_line();
      const std::string actual_line_content();
      size_t count_fields();
      bool eof();
    };
  }
}

#endif
