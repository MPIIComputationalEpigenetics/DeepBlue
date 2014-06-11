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

#include <boost/icl/split_interval_map.hpp>

namespace epidb {
  namespace parser {

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

    class WIGParser {
    private:
      size_t actual_line_;
      std::stringstream input_;
      std::map<std::string, std::string> info_;
      bool declare_track_;
      Feature last_feature_;
      std::map<std::string, std::string> params;
      size_t start;
      size_t step;
      size_t span;
      std::map<std::string, boost::icl::interval_set<int> > overlap_counters;
      bool get_line(std::string line, std::string& msg);

    private:

      const std::string line_str()
      {
        return boost::lexical_cast<std::string>(actual_line_);
      }

      bool check_feature(const Feature& feature, std::string& msg)
      {
        boost::icl::interval_set<int> &overlap_counter = overlap_counters[feature._chrom];
        boost::icl::discrete_interval<int> inter_val =  boost::icl::discrete_interval<int>::right_open(feature._start, feature._end);

        if (boost::icl::intersects(overlap_counter, inter_val)) {
            msg = "The region " + feature._chrom + " " + boost::lexical_cast<std::string>(feature._start) + " " +
            boost::lexical_cast<std::string>(feature._end) + " is overlaping with some other region. Line: " +
            line_str();
          return false;
        }
        overlap_counter += inter_val;
        return true;
      }

    public:
      WIGParser(const std::string& content);
      bool get_features(std::vector<Feature>& features, std::string& msg);
      size_t actual_line();
      bool eof();
    };
  }
}

#endif /* defined(WIG_PARSER_HPP) */