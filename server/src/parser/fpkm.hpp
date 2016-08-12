//
//  fpkm.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 13.07.16.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights
//  reserved.
//

#ifndef FPKM_HPP
#define FPKM_HPP

#include <memory>
#include <string>
#include <vector>

#include <boost/utility.hpp>

#include "../interfaces/serializable.hpp"

#include "../types.hpp"

namespace epidb {
  namespace parser {

    class FPKMRow : public ISerializable {

    private:
      std::string _tracking_id;
      std::string _gene_id;
      std::string _gene_short_name;
      Score _fpkm;
      Score _fpkm_lo;
      Score _fpkm_hi;
      std::string _fpkm_status;

    public:
      FPKMRow(const std::string &tracking_id, const std::string &gene_id, const std::string &gene_short_name,
              Score fpkm, Score fpkm_lo, Score fpkm_hi,
              const std::string& fpkm_status);

      const std::string& tracking_id() const { return _tracking_id; }
      const std::string& gene_id() const { return _gene_id; }
      const std::string& gene_short_name() const { return _gene_short_name; }
      const Score fpkm() const { return _fpkm; }
      const Score fpkm_lo() const { return _fpkm_lo; }
      const Score fpkm_hi() const { return _fpkm_hi; }
      const std::string& fpkm_status() const { return _fpkm_status; }

      virtual const mongo::BSONObj to_BSON();
    };

    class FPKMFile : ISerializableFile {
    public:
      void add_row(const std::string &tracking_id, const std::string &gene_id, const std::string &gene_short_name,
                   Score fpkm, Score fpkm_lo, Score fpkm_hi, const std::string& fpkm_status);
    };
  }
}

#endif /* defined(FPKM_HPP) */