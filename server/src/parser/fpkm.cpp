//
//  fpkm.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 16.07.16.
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

#include "fpkm.hpp"
#include "../types.hpp"

namespace epidb {
  namespace parser {

    FPKMRow::FPKMRow(const std::string &tracking_id, const std::string &gene_id, const std::string &gene_short_name,
                     Score fpkm, Score fpkm_lo, Score fpkm_hi,
                     const std::string& fpkm_status):
      _tracking_id(tracking_id),
      _gene_id(gene_id),
      _gene_short_name(gene_short_name),
      _fpkm(fpkm),
      _fpkm_lo(fpkm_lo),
      _fpkm_hi(fpkm_hi),
      _fpkm_status(fpkm_status)
    { }

    void FPKMFile::add_row(const std::string &tracking_id, const std::string &gene_id, const std::string &gene_short_name,
                          Score fpkm, Score fpkm_lo, Score fpkm_hi, const std::string& fpkm_status)
    {
      _content.emplace_back(tracking_id, gene_id, gene_short_name, fpkm, fpkm_lo, fpkm_hi, fpkm_status);
    }

    const FPKMContent & FPKMFile::rows() const
    {
      return _content;
    }

    size_t FPKMFile::size() const
    {
      return _content.size();
    }

  }
}