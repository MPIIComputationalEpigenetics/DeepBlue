//
//  delete.hpp
//  epidb
//
//  Created by Felipe Albrecht on 06.11.14.
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

#ifndef EPIDB_DBA_DELETE_HPP
#define EPIDB_DBA_DELETE_HPP

#include <string>

namespace epidb {
  namespace dba {
    namespace remove {
      bool dataset(const int dataset_id, const std::string& genome, std::string& msg);

      bool annotation(const std::string &id, const std::string &user_id, std::string &msg);

      bool gene_set(const std::string &id, const std::string &user_id, std::string &msg);

      bool genome(const std::string &id, const std::string &user_id, std::string &msg);

      bool project(const std::string &id, const std::string &user_id, std::string &msg);

      bool biosource(const std::string &id, const std::string &user_id, std::string &msg);

      bool sample(const std::string &id, const std::string &user_id, std::string &msg);

      bool epigenetic_mark(const std::string &id, const std::string &user_id, std::string &msg);

      bool experiment(const std::string &id, const std::string &user_id, std::string &msg);

      bool technique(const std::string &id, const std::string &user_id, std::string &msg);

      bool column_type(const std::string &id, const std::string &user_id, std::string &msg);
    }
  }
}

#endif
