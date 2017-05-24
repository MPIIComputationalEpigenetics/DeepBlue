#include <boost/asio.hpp>

namespace epidb {
  namespace mdbq {
    class Janitor {
    public:
      Janitor(float interval) : m_interval(interval) {}
      void run();

    private:
      float m_interval;
      std::auto_ptr<boost::asio::deadline_timer> m_timer;
      boost::asio::io_service ios;
      bool clean_oldest(const boost::system::error_code &error);
      void reg(boost::asio::io_service &io_service, float interval);
    };
  }
}