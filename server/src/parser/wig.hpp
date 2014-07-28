//
//  wig.hpp
//  epidb
//
//  Created by Felipe Albrecht on 20.07.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef WIG_HPP
#define WIG_HPP


#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <boost/icl/split_interval_map.hpp>

#include "../extras/utils.hpp"

namespace epidb {
  namespace parser {

    typedef enum {
      FIXED_STEP,
      VARIABLE_STEP
    } WigTrackType;

    class Track {
    private:
      WigTrackType _type;
      std::string _chromosome;
      boost::icl::interval_set<int> overlap_counter;
      size_t _start;
      size_t _end;
      size_t _step;
      size_t _span;

      std::vector<float>  _data_fixed;
      std::vector<std::pair<size_t, float> > _data_variable;

      bool check_feature(const size_t start, const size_t &line, std::string &msg)
      {
        boost::icl::discrete_interval<int> inter_val =  boost::icl::discrete_interval<int>::right_open(start, start + _span);

        if (boost::icl::intersects(overlap_counter, inter_val)) {
          msg = "The region " + _chromosome + " " + utils::integer_to_string(start) + " " +
                utils::integer_to_string(start + _span) + " is overlaping with some other region. Line: " +
                utils::integer_to_string(line);
          return false;
        }
        overlap_counter += inter_val;
        return true;
      }

    public:
      Track(std::string &chr, size_t start, size_t step, size_t span = 1); // FixedStep
      Track(std::string &chr, size_t span = 1); // VariableStep
      WigTrackType type();
      std::string chromosome() { return _chromosome; }
      size_t start() { return _start; }
      size_t end() { return _end; }
      size_t step() { return _step; }
      size_t span() { return _span; }
      size_t size();
      void* data();
      size_t data_size();
      void add_feature(float _score);
      void add_feature(size_t position, float _score);
    };

    typedef boost::shared_ptr<Track> TrackPtr;

    TrackPtr build_fixed_track(std::string &chr, size_t start, size_t step, size_t span);
    TrackPtr build_variable_track(std::string &chr, size_t span);

    typedef std::vector<TrackPtr> WigContent;
    class WigFile : boost::noncopyable {
    private:
      WigContent content;

    public:
      WigContent::const_iterator tracks_iterator();
      WigContent::const_iterator tracks_iterator_end();
      size_t size();
      void add_track(TrackPtr track);
    };



    typedef boost::shared_ptr<WigFile> WigPtr;

  }
}

#endif /* defined(WIG_HPP) */