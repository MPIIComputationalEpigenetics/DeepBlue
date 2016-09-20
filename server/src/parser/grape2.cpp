//
//  grape2.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 09.08.16.
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

#include <string>

#include <mongo/bson/bson.h>

#include "grape2.hpp"

#include "../dba/key_mapper.hpp"

#include "../types.hpp"

namespace epidb {
  namespace parser {

    Grape2Row::Grape2Row(const std::string gene_id, const std::string transcript_ids, const double length,
                         const double effective_length, const double expected_count, const double TPM, const double FPKM,
                         const double posterior_mean_count, const double posterior_standard_deviation_of_count, const double pme_TPM,
                         const double pme_FPKM, const double TPM_ci_lower_bound, const double TPM_ci_upper_bound,
                         const double FPKM_ci_lower_bound, const double FPKM_ci_upper_bound):
      ISerializable(),
      _gene_id(gene_id),
      _transcript_ids(transcript_ids),
      _length(length),
      _effective_length(effective_length),
      _expected_count(expected_count),
      _TPM(TPM),
      _FPKM(FPKM),
      _posterior_mean_count(posterior_mean_count),
      _posterior_standard_deviation_of_count(posterior_standard_deviation_of_count),
      _pme_TPM(pme_TPM),
      _pme_FPKM(pme_FPKM),
      _TPM_ci_lower_bound(TPM_ci_lower_bound),
      _TPM_ci_upper_bound(TPM_ci_upper_bound),
      _FPKM_ci_lower_bound(FPKM_ci_lower_bound),
      _FPKM_ci_upper_bound(FPKM_ci_upper_bound)
    { }

    const mongo::BSONObj Grape2Row::to_BSON()
    {
      mongo::BSONObjBuilder bob;

      bob.append(dba::KeyMapper::TRACKING_ID(), _gene_id);
      bob.append(dba::KeyMapper::GENE_ID(), _gene_id);
      bob.append(dba::KeyMapper::TRANSCRIPT_IDS(), _transcript_ids);
      bob.append(dba::KeyMapper::LENGTH(), _length);
      bob.append(dba::KeyMapper::EFFECTIVE_LENGTH(), _effective_length);
      bob.append(dba::KeyMapper::EXPECTED_COUNT(), _expected_count);
      bob.append(dba::KeyMapper::TPM(), _TPM);
      bob.append(dba::KeyMapper::FPKM(), _FPKM);
      bob.append(dba::KeyMapper::POSTERIOR_MEAN_COUNT(), _posterior_mean_count);
      bob.append(dba::KeyMapper::POSTERIOR_STANDARD_DEVIATION_OF_COUNT(), _posterior_standard_deviation_of_count);
      bob.append(dba::KeyMapper::PME_TPM(), _pme_TPM);
      bob.append(dba::KeyMapper::PME_FPKM(), _pme_FPKM);
      bob.append(dba::KeyMapper::TPM_CI_LOWER_BOUND(), _TPM_ci_lower_bound);
      bob.append(dba::KeyMapper::TPM_CI_UPPER_BOUND(), _TPM_ci_upper_bound);
      bob.append(dba::KeyMapper::FPKM_CI_LOWER_BOUND(), _FPKM_ci_lower_bound);
      bob.append(dba::KeyMapper::FPKM_CI_UPPER_BOUND(), _FPKM_ci_upper_bound);

      return bob.obj();
    }

    void Grape2File::add_row(const std::string gene_id, const std::string transcript_ids, const double length,
                             const double effective_length, const double expected_count, const double TPM, const double FPKM,
                             const double posterior_mean_count, const double posterior_standard_deviation_of_count, const double pme_TPM,
                             const double pme_FPKM, const double TPM_ci_lower_bound, const double TPM_ci_upper_bound,
                             const double FPKM_ci_lower_bound, const double FPKM_ci_upper_bound)
    {
      _data.emplace_back(
        std::unique_ptr<Grape2Row>(
          new Grape2Row(gene_id, transcript_ids, length, effective_length, expected_count, TPM, FPKM,
                        posterior_mean_count, posterior_standard_deviation_of_count,
                        pme_TPM, pme_FPKM,
                        TPM_ci_lower_bound, TPM_ci_upper_bound,
                        FPKM_ci_lower_bound, FPKM_ci_upper_bound)
        )
      );
    }
  }
}