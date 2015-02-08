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
    typedef boost::shared_ptr<Track> TrackPtr;

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
      boost::shared_ptr<char> data() const;
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

    typedef boost::shared_ptr<WigFile> WigPtr;
  }
}

#endif /* defined(WIG_HPP) */