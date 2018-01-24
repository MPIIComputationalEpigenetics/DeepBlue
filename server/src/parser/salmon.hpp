//
//  salmon.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.01.17.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights
//  reserved.
//

#ifndef SALMON_HPP
#define SALMON_HPP

#include <memory>
#include <string>
#include <vector>

#include <boost/utility.hpp>

#include "../interfaces/serializable.hpp"

#include "../types.hpp"

namespace epidb {
  namespace parser {

    class TPMRow : public ISerializable {

    private:
      std::string _tracking_id;
      Length _length;
      double _effective_length;
      double _tpm;
      double _num_reads;

    public:
      TPMRow(const std::string &tracking_id, const double length, const double effective_length,
              double tpm, double num_reads);

      const std::string& tracking_id() const { return _tracking_id; }
      const Length length() const { return _length; }
      const double effective_length() const { return _effective_length; }
      const double tpm() const { return _tpm; }
      const double num_reads() const { return _num_reads; }

      virtual const mongo::BSONObj to_BSON();
    };

    class TPMFile : public ISerializableFile {
    public:
      void add_row(const std::string &tracking_id, const double length, const double effective_length,
                    double tpm, double num_reads);
    };
  }
}

#endif /* defined(SALMON_HPP) */