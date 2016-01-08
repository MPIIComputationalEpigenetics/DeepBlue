//
//  delete.hpp
//  epidb
//
//  Created by Felipe Albrecht on 06.11.14.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
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
