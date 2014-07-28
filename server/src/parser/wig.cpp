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

    TrackPtr build_bedgraph_track(std::string &chr, size_t start, size_t end)
    {
      return boost::shared_ptr<Track>(new Track(chr, start, end));
    }

    TrackPtr build_bedgraph_track(std::string &chr)
    {
      return boost::shared_ptr<Track>(new Track(chr));
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
      _step(0),
      _span(span)
    { }

    Track::Track(std::string &chr, size_t start, size_t end) :
      _type(ENCODE_BEDGRAPH),
      _chromosome(chr),
      _start(start),
      _end(end),
      _step(0),
      _span(0)
    { }

    Track::Track(std::string &chr) :
      _type(MISC_BEDGRAPH),
      _chromosome(chr),
      _start(0),
      _end(0),
      _step(0),
      _span(0)
    { }

    WigTrackType Track::type()
    {
      return _type;
    }

    size_t Track::features()
    {
      if (_type == FIXED_STEP) {
        return _data_fixed.size();
      } else {
        return _data_variable.size();
      }
      return _type;
    }

    void Track::add_feature(float score)
    {
      _end = _start + (_data_fixed.size() * _span);

      _data_fixed.push_back(score);
    }

    void Track::add_feature(size_t position, float score)
    {

      if (position + _span > _end) {
        _end = position + _span;
      }
      if (position < _start) {
        _start = position;
      }

      std::pair<size_t, float> p(position, score);
      _data_variable.push_back(p);
    }

    void Track::add_feature(size_t start, size_t end, float score)
    {
      BedGraphRegion region;
      region.start = start;
      region.end = end;
      region.score = score;

      if (_data_bedgraph.empty() && _start == 0) {
        _start = start;
      }

      if (_type == MISC_BEDGRAPH && _end < end) {
        _end = end;
      }

      _data_bedgraph.push_back(region);
    }

    void *Track::data()
    {
      if (_type == VARIABLE_STEP) {
        return (void *) _data_variable.data();
      } else  {
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
      std::cerr << "iteratorrr" << std::endl;
      std::cerr << content.size() << std::endl;
      std::cerr << content[0]->features() << std::endl;
      return content.end();
    }

    size_t WigFile::size()
    {
      size_t size(0);
      for (WigContent::iterator it = content.begin(); it != content.end(); it++) {
        size += (*it)->features();
      }
      return size;
    }

  }
}