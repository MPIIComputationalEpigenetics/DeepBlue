//
//  grape2.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 04.09.15.
//  Copyright (c) 2015 Max Planck Institute for Computer Science. All rights
//  reserved.
//
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

#ifndef GRAPE2_HPP
#define GRAPE2_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../interfaces/serializable.hpp"

#include "../types.hpp"

namespace epidb {
  namespace parser {

    class Grape2Row : public ISerializable {

    private:
      std::string _gene_id;
      std::string _transcript_ids;
      double _length;
      double _effective_length;
      double _expected_count;
      double _TPM;
      double _FPKM;
      double _posterior_mean_count;
      double _posterior_standard_deviation_of_count;
      double _pme_TPM;
      double _pme_FPKM;
      double _TPM_ci_lower_bound;
      double _TPM_ci_upper_bound;
      double _FPKM_ci_lower_bound;
      double _FPKM_ci_upper_bound;

    public:
      Grape2Row(const std::string gene_id, const std::string transcript_ids, const double length,
                const double effective_length, const double expected_count, const double TPM, const double FPKM,
                const double posterior_mean_count, const double posterior_standard_deviation_of_count, const double pme_TPM,
                const double pme_FPKM, const double TPM_ci_lower_bound, const double TPM_ci_upper_bound,
                const double FPKM_ci_lower_bound, const double FPKM_ci_upper_bound);

      virtual const mongo::BSONObj to_BSON();

    };

    class Grape2File: public ISerializableFile {
    public:
      void add_row(const std::string gene_id, const std::string transcript_ids, const double length,
                   const double effective_length, const double expected_count, const double TPM, const double FPKM,
                   const double posterior_mean_count, const double posterior_standard_deviation_of_count, const double pme_TPM,
                   const double pme_FPKM, const double TPM_ci_lower_bound, const double TPM_ci_upper_bound,
                   const double FPKM_ci_lower_bound, const double FPKM_ci_upper_bound);
    };
  }
}

#endif /* defined(GRAPE2_HPP) */
