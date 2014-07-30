//
//  wig.cpp
//  epidb
//
//  Created by Felipe Albrecht on 20.07.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <limits>

#include "wig.hpp"
#include "../types.hpp"

namespace epidb {
  namespace parser {


    TrackPtr build_fixed_track(std::string &chr, Position start, Length step, Length span)
    {
      return boost::shared_ptr<Track>(new Track(chr, start, step, span));
    }

    TrackPtr build_variable_track(std::string &chr, Length span)
    {
      return boost::shared_ptr<Track>(new Track(chr, span));
    }

    TrackPtr build_bedgraph_track(std::string &chr, Length start, Length end)
    {
      return boost::shared_ptr<Track>(new Track(chr, start, end));
    }

    TrackPtr build_bedgraph_track(std::string &chr)
    {
      return boost::shared_ptr<Track>(new Track(chr));
    }


    Track::Track(std::string &chr, Position start, Length step, Length span) :
      _type(FIXED_STEP),
      _chromosome(chr),
      _start(start),
      _end(0),
      _step(step),
      _span(span)
    { }

    Track::Track(std::string &chr, Length span) :
      _type(VARIABLE_STEP),
      _chromosome(chr),
      _start(std::numeric_limits<size_t>::max()),
      _end(0),
      _step(0),
      _span(span)
    { }

    Track::Track(std::string &chr, Position start, Position end) :
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
      } else if (_type == VARIABLE_STEP) {
        return _data_variable.size();
      } else {
        return _data_bedgraph.size();
      }
      return _type;
    }

    void Track::add_feature(float score)
    {
      _data_fixed.push_back(score);
      _end = _start + (_data_fixed.size() * _span);
    }

    void Track::add_feature(Position position, Score score)
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

    void Track::add_feature(Position start, Position end, Score score)
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
      } else if (_type == FIXED_STEP) {
        return (void *) _data_fixed.data();
      } else { /* ENCODE_BEDGRAPH or MISC_BEDGRAPH */
        return (void *) _data_bedgraph.data();
      }
    }

    size_t Track::data_size()
    {
      if (_type == VARIABLE_STEP) {
        return _data_variable.size() * sizeof(PositionScorePair);
      }

      else if (_type == FIXED_STEP) {
        return _data_fixed.size() * sizeof(Score);
      }

      else { /* ENCODE_BEDGRAPH or MISC_BEDGRAPH */
        return _data_bedgraph.size() * (sizeof(BedGraphRegion));
      }
    }

    TrackPtr Track::split()
    {
      if (_type == VARIABLE_STEP) {
        return build_variable_track(_chromosome, _span);
      }

      else if (_type == FIXED_STEP) {
        return build_fixed_track(_chromosome, _end, _step, _span);
      }

      else if (_type == ENCODE_BEDGRAPH) {
        size_t _last_position = _data_bedgraph.rbegin()->end;
        TrackPtr track = build_bedgraph_track(_chromosome, _last_position, _end);
        _end = _last_position;
        return track;
      }

      else { /* (type == MISC_BEDGRAPH) */
        return build_bedgraph_track(_chromosome);
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
        size += (*it)->features();
      }
      return size;
    }

  }
}