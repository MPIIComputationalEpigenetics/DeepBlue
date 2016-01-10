//
//  wig.hpp
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

#ifndef WIG_HPP
#define WIG_HPP


#include <string>
#include <vector>

#include <memory>
#include <boost/utility.hpp>

#include <boost/icl/split_interval_map.hpp>

#include "../extras/utils.hpp"
#include "../types.hpp"

namespace epidb {
  namespace parser {

    typedef enum {
      FIXED_STEP,
      VARIABLE_STEP,
      ENCODE_BEDGRAPH,
      MISC_BEDGRAPH
    } WigTrackType;

    typedef std::vector<Position> PositionStart;
    typedef std::vector<Position> PositionEnd;
    typedef std::vector<Score>  DataFixed;

    class Track;
    typedef std::shared_ptr<Track> TrackPtr;

    class Track {
    private:
      WigTrackType _type;
      std::string _chromosome;
      Position _start;
      Position _end;
      Length _step;
      Length _span;

      PositionStart _starts;
      PositionEnd _ends;
      DataFixed  _scores;

    public:
      Track(std::string &chr, Position start, Length step, Length span); // FixedStep
      Track(std::string &chr, Position span); // VariableStep
      Track(std::string &chr, Position start, Position end); // EncodeBedgraph
      Track(std::string &chr); // MiscBedgraph

      WigTrackType type() const;
      std::string chromosome() const
      {
        return _chromosome;
      }
      Position start() const
      {
        return _start;
      }
      Position end() const
      {
        return _end;
      }
      Length step() const
      {
        return _step;
      }
      Length span() const
      {
        return _span;
      }
      size_t features() const;
      std::shared_ptr<char> data() const;
      size_t data_size() const;
      TrackPtr split();
      void add_feature(Score _score);
      void add_feature(Position position, Score _score);
      void add_feature(Position start, Position end, Score _score);
    };

    TrackPtr build_fixed_track(std::string &chr, Position start, Length step, Length span);
    TrackPtr build_variable_track(std::string &chr, Length span);
    TrackPtr build_bedgraph_track(std::string &chr);
    TrackPtr build_bedgraph_track(std::string &chr, Position start, Position end);

    typedef std::vector<TrackPtr> WigContent;
    class WigFile : boost::noncopyable {
    private:
      WigContent content;

    public:
      bool check_feature(const std::string &chrm, const Position start, const size_t &line, std::string &msg) const;
      WigContent::const_iterator tracks_iterator() const;
      WigContent::const_iterator tracks_iterator_end() const;
      size_t size() const;
      void add_track(TrackPtr track);
    };

    typedef std::shared_ptr<WigFile> WigPtr;
  }
}

#endif /* defined(WIG_HPP) */