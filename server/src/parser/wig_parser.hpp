//
//  wig_parser.hpp
//  epidb
//
//  Created by Felipe Albrecht on 20.01.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef WIG_PARSER_HPP
#define WIG_PARSER_HPP

#include <sstream>
#include <list>

#include "wig.hpp"

namespace epidb {
  namespace parser {

    /*
        class Feature {
        public:
          std::string _chrom;
          size_t _start;
          size_t _end;
          double _value;
          Feature() :
            _chrom(),
            _start(-1),
            _end(-1),
            _value(0.0) {}

          void set(const std::string chrom,
                  const size_t start, const size_t end, const double value) {
            _chrom = chrom;
            _start = start;
            _end = end;
            _value = value;
          }

          bool empty() {
            return _start == (size_t) -1 && _end == (size_t) -1 && _value == 0.0;
          }

          void clear()  {
            _chrom = "";
            _start = -1;
            _end = -1;
            _value = 0.0;
          }

          friend std::ostream& operator<< (std::ostream& stream, const Feature& feature) {
            stream << feature._chrom << "\t" << feature._start << "\t" << feature._end << "\t" << feature._value;
            return stream;
          }
        };
        */

    class WIGParser {
    private:
      size_t actual_line_;
      std::stringstream input_;
      std::map<std::string, std::string> info_;
      bool declare_track_;
      TrackPtr actual_track;
      bool read_track(const std::string &line, std::map<std::string, std::string> &info, std::string &msg);
      bool read_parameters(const std::vector<std::string> &strs, std::map<std::string, std::string> &params, std::string &msg);
      bool read_format(const std::vector<std::string> &strs, TrackPtr& track, std::string &msg);
      bool get_line(std::string line, std::string &msg);

      const std::string line_str()
      {
        return utils::integer_to_string(actual_line_);
      }

    public:
      WIGParser(const std::string &content);
      bool get(WigPtr &wig, std::string &msg);
      size_t actual_line();
      bool eof();
    };
  }
}

#endif /* defined(WIG_PARSER_HPP) */