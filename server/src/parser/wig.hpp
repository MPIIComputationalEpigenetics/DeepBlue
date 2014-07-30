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

    typedef std::vector<float>  DataFixed;
    typedef std::vector<std::pair<size_t, float> > DataVariable;

    typedef enum {
      FIXED_STEP,
      VARIABLE_STEP,
      ENCODE_BEDGRAPH,
      MISC_BEDGRAPH
    } WigTrackType;

    typedef struct {
      size_t start;
      size_t end;
      float score;
    } BedGraphRegion;

    typedef std::vector<float>  DataFixed;
    typedef std::vector<std::pair<size_t, float> > DataVariable;
    typedef std::vector<BedGraphRegion> DataBedgraph;

    class Track {
    private:
      WigTrackType _type;
      std::string _chromosome;
      size_t _start;
      size_t _end;
      size_t _step;
      size_t _span;
      DataFixed  _data_fixed;
      DataVariable _data_variable;
      DataBedgraph _data_bedgraph;

    public:
      Track(std::string &chr, size_t start, size_t step, size_t span); // FixedStep
      Track(std::string &chr, size_t span); // VariableStep
      Track(std::string &chr, size_t start, size_t end); // EncodeBedgraph
      Track(std::string &chr); // MiscBedgraph

      WigTrackType type();
      std::string chromosome() { return _chromosome; }
      size_t start() { return _start; }
      size_t end() { return _end; }
      size_t step() { return _step; }
      size_t span() { return _span; }
      size_t features();
      void* data();
      size_t data_size();
      void add_feature(float _score);
      void add_feature(size_t position, float _score);
      void add_feature(size_t start, size_t end, float _score);
    };

    typedef boost::shared_ptr<Track> TrackPtr;

    TrackPtr build_fixed_track(std::string &chr, size_t start, size_t step, size_t span);
    TrackPtr build_variable_track(std::string &chr, size_t span);
    TrackPtr build_bedgraph_track(std::string &chr);
    TrackPtr build_bedgraph_track(std::string &chr, size_t start, size_t end);

    typedef std::vector<TrackPtr> WigContent;
    class WigFile : boost::noncopyable {
    private:
      WigContent content;

    public:
      bool check_feature(const std::string& chrm, const size_t start, const size_t &line, std::string &msg);
      WigContent::const_iterator tracks_iterator();
      WigContent::const_iterator tracks_iterator_end();
      size_t size();
      void add_track(TrackPtr track);
    };

    typedef boost::shared_ptr<WigFile> WigPtr;
  }
}

#endif /* defined(WIG_HPP) */