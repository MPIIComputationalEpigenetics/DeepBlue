//
//  wig.cpp
//  epidb
//
//  Created by Felipe Albrecht on 20.07.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <limits>

#include "wig.hpp"

namespace epidb {
  namespace parser {


    TrackPtr build_fixed_track(std::string &chr, size_t start, size_t step, size_t span)
    {
      return boost::shared_ptr<Track>(new Track(chr, start, step, span));
    }

    TrackPtr build_variable_track(std::string &chr, size_t span)
    {
      return boost::shared_ptr<Track>(new Track(chr, span));
    }

    Track::Track(std::string &chr, size_t start, size_t step, size_t span) :
      _type(FIXED_STEP),
      _chromosome(chr),
      _start(start),
      _end(0),
      _step(step),
      _span(span)
    { }

    Track::Track(std::string &chr, size_t span) :
      _type(VARIABLE_STEP),
      _chromosome(chr),
      _start(std::numeric_limits<size_t>::max()),
      _end(0),
      _step(-1),
      _span(span)
    { }

    WigTrackType Track::type()
    {
      return _type;
    }

    size_t Track::size()
    {
      if (_type == FIXED_STEP) {
        std::cerr << "FIXED_STEP size : " << _data_fixed.size() << std::endl;
        return _data_fixed.size();
      } else {
        return _data_variable.size();
        std::cerr << "VARIABLE_STEP size : " << _data_variable.size() << std::endl;
      }
      return _type;
    }

    void Track::add_feature(float score)
    {
      if (_type == FIXED_STEP) {
        _data_fixed.push_back(score);
        _end = _start + (_data_fixed.size() * _span);
      } else {
        std::cerr << "ERROR WITH ADD FEATURE 1" << std::endl;
      }
    }

    void Track::add_feature(size_t position, float score)
    {
      if (_type == VARIABLE_STEP) {
        std::pair<size_t, float> p(position, score);
        _data_variable.push_back(p);
        if (position + _span > _end) {
          _end = position + _span;
        }
        if (position < _start) {
          _start = position;
        }
      } else {
        std::cerr << "ERROR WITH ADD FEATURE 1" << std::endl;
      }
    }

    void *Track::data()
    {
      if (_type == VARIABLE_STEP) {
        return (void *) _data_variable.data();
      }
      else  {
        return (void *) _data_fixed.data();
      }
    }

    size_t Track::data_size()
    {
      if (_type == VARIABLE_STEP) {
        return _data_variable.size() * sizeof(std::pair<size_t, float>);
      }

      else  {
        return _data_fixed.size() * sizeof(float);
      }
    }


    void WigFile::add_track(boost::shared_ptr<Track> track)
    {
      content.push_back(track);
    }

    WigContent::const_iterator WigFile::tracks_iterator()
    {
      return content.begin();
    }

    WigContent::const_iterator WigFile::tracks_iterator_end()
    {
      return content.end();
    }

    size_t WigFile::size()
    {
      size_t size(0);
      for (WigContent::iterator it = content.begin(); it != content.end(); it++) {
        size += (*it)->size();
      }
      return size;
    }

  }
}