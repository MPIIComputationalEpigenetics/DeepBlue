#include <string>

#include <boost/utility.hpp>

namespace epidb {

  class DataType : boost::noncopyable {

  public:
    virtual bool insert(void *data, std::string &msg) const =0;
    virtual bool select(void *data, std::string &msg) const =0;
  };


  class BEDType : public DataType {

  public:
    bool insert(void *data, std::string &msg) const;
    bool select(void *data, std::string &msg) const;
  };


  class WIGType : public DataType {

  public:
    bool insert(void *data, std::string &msg) const;
    bool select(void *data, std::string &msg) const;
  };


  class Methylation450Type : public DataType {

  public:
    bool insert(void *data, std::string &msg) const;
    bool select(void *data, std::string &msg) const;
  };

}


