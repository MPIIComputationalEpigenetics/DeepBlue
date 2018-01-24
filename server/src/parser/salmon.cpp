//
//  salmon.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.01.17.
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
#include <memory>

#include <mongo/bson/bson.h>

#include "salmon.hpp"

#include "../dba/key_mapper.hpp"

#include "../interfaces/serializable.hpp"

#include "../types.hpp"

namespace epidb {
  namespace parser {

    TPMRow::TPMRow(const std::string &tracking_id, const double length, const double effective_length,
                   double tpm, double num_reads):
      _tracking_id(tracking_id),
      _length(length),
      _effective_length(effective_length),
      _tpm(tpm),
      _num_reads(num_reads)
    { }

    const mongo::BSONObj TPMRow::to_BSON()
    {
      mongo::BSONObjBuilder bob;

      bob.append(dba::KeyMapper::TRACKING_ID(), _tracking_id);
      bob.append(dba::KeyMapper::LENGTH(), _length);
      bob.append(dba::KeyMapper::EFFECTIVE_LENGTH(), _effective_length);
      bob.append(dba::KeyMapper::TPM(), _tpm);
      bob.append(dba::KeyMapper::NUM_READS(), _num_reads);

      return bob.obj();
    }

    void TPMFile::add_row(const std::string &tracking_id, const double length, const double effective_length,
                          double tpm, double num_reads)
    {
      _data.emplace_back(
        std::unique_ptr<TPMRow>(
          new TPMRow(tracking_id, length, effective_length, tpm, num_reads)
        )
      );
    }
  }
}