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

#include <boost/shared_ptr.hpp>

#include "wig.hpp"

namespace epidb {
  namespace parser {

    static const size_t BLOCK_SIZE = 40;

    class WIGParser {
    protected:
      size_t actual_line_;
      std::stringstream input_;
      std::map<std::string, std::string> info_;
      bool declare_track_;
      TrackPtr actual_track;
      std::map<std::string, boost::shared_ptr<boost::icl::interval_set<int> > > map_overlap_counter;
      bool read_track(const std::string &line, std::map<std::string, std::string> &info, std::string &msg);
      bool read_parameters(const std::vector<std::string> &strs, std::map<std::string, std::string> &params, std::string &msg);
      bool read_format(const std::vector<std::string> &strs, TrackPtr &track, std::string &msg);
      bool check_feature(const std::string &chromosome, const size_t start, const size_t span, const size_t &line, std::string &msg);
      void check_block_size(WigPtr &wig);
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