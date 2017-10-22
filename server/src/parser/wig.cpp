//
//  wig.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 20.07.14.
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

#include <limits>

#include "wig.hpp"
#include "../types.hpp"

namespace epidb {
  namespace parser {


    TrackPtr build_fixed_track(std::string &chr, Position start, Length step, Length span)
    {
      return std::make_shared<Track>(chr, start, step, span);
    }

    TrackPtr build_variable_track(std::string &chr, Length span)
    {
      return std::make_shared<Track>(chr, span);
    }

    TrackPtr build_bedgraph_track(std::string &chr, Length start, Length end)
    {
      return std::make_shared<Track>(chr, start, end);
    }

    TrackPtr build_bedgraph_track(std::string &chr)
    {
      return std::make_shared<Track>(chr);
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
      _start(std::numeric_limits<Position>::max()),
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

    WigTrackType Track::type() const
    {
      return _type;
    }

    size_t Track::features() const
    {
      return _scores.size();
    }

    void Track::add_feature(float score)
    {
      _scores.push_back(score);
      _end = _start + (_scores.size() * _span);
    }

    void Track::add_feature(Position position, Score score)
    {
      if (position + _span > _end) {
        _end = position + _span;
      }
      if (position < _start) {
        _start = position;
      }

      _starts.push_back(position);
      _scores.push_back(score);
    }

    void Track::add_feature(Position start, Position end, Score score)
    {
      if (_scores.empty() && _start == 0) {
        _start = start;
      }

      if (_type == MISC_BEDGRAPH && _end < end) {
        _end = end;
      }

      _starts.push_back(start);
      _ends.push_back(end);
      _scores.push_back(score);
    }

    std::shared_ptr<char> Track::data() const
    {
      size_t _starts_size = _starts.size() * sizeof(Position);
      size_t _ends_size = _ends.size() * sizeof(Position);
      size_t _scores_size = _scores.size() * sizeof(Score);

      if (_type == FIXED_STEP) {
        char *data = (char *) malloc(_scores_size);
        memcpy(data, _scores.data(), _scores_size);
        return std::shared_ptr<char>(data);

      } else if (_type == VARIABLE_STEP) {
        char *data = (char *) malloc(_starts_size + _scores_size);
        memcpy(data, _starts.data(), _starts_size);
        memcpy(data + _starts_size, _scores.data(), _scores_size);
        return std::shared_ptr<char>(data);


      } else { /* ENCODE_BEDGRAPH or MISC_BEDGRAPH */
        char *data = (char *) malloc(_starts_size + _ends_size + _scores_size);
        memcpy(data, _starts.data(), _starts_size);
        memcpy(data + _starts_size, _ends.data(), _ends_size);
        memcpy(data + _starts_size + _ends_size, _scores.data(), _scores_size);
        return std::shared_ptr<char>(data);
      }
    }

    size_t Track::data_size() const
    {
      size_t _starts_size = _starts.size() * sizeof(Position);
      size_t _ends_size = _ends.size() * sizeof(Position);
      size_t _scores_size = _scores.size() * sizeof(Score);

      if (_type == FIXED_STEP) {
        return _scores_size;
      }

      else if (_type == VARIABLE_STEP) {
        return _starts_size + _scores_size;
      }

      else { /* ENCODE_BEDGRAPH or MISC_BEDGRAPH */
        return _starts_size + _ends_size + _scores_size;
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
        size_t _last_position = *_ends.rbegin();
        TrackPtr track = build_bedgraph_track(_chromosome, _last_position, _end);
        _end = _last_position;
        return track;
      }

      else { /* (type == MISC_BEDGRAPH) */
        return build_bedgraph_track(_chromosome);
      }
    }

    void WigFile::add_track(std::shared_ptr<Track> track)
    {
      content.push_back(track);
    }

    WigContent::const_iterator WigFile::tracks_iterator() const
    {
      return content.begin();
    }

    WigContent::const_iterator WigFile::tracks_iterator_end() const
    {
      return content.end();
    }

    size_t WigFile::size() const
    {
      size_t size(0);
      for (WigContent::const_iterator it = content.begin(); it != content.end(); it++) {
        size += (*it)->features();
      }
      return size;
    }

  }
}