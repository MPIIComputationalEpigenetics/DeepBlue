
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

namespace epidb {
  namespace parser {

    typedef std::vector<double> ValuesPtr;
    typedef std::vector<std::pair<size_t, double> > PositionValuesPtr;

    typedef enum {
      VARIABLE_STEP,
      FIXED_STEP
    } WigTrackType;

    class Track {
    private:
      WigTrackType _type;
      std::string _chromosome;
      size_t _start;
      size_t _span;

    public:
      WigTrackType type();
      size_t start();
      ValuesPtr values();
      PositionValuesPtr position_values();
    };

    class VariableTrack {
    private:
      std::vector<std::pair<size_t, float> > _data;
    public:
    	WigTrackType type() {
    		return VARIABLE_STEP;
    	}
      void add_score(float score);
    };

    class FixedTrack {
    private:
      size_t _step;
      std::vector<float> _data;
    public:
    	WigTrackType type() {
    		return FIXED_STEP;
    	}
      void add_score(size_t position, float _score);

    };

    typedef std::vector<Track> WigContent;


    class WigFile : boost::noncopyable {
    private:
      std::string name;
      WigContent content;

    public:
      size_t size();

    };

    typedef boost::shared_ptr<WigFile> WigPtr;
  }
}