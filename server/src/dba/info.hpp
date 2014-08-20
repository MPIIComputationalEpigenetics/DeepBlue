//
//  info.hpp
//  epidb
//
//  Created by Felipe Albrecht on 04.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_INFO_HPP
#define EPIDB_DBA_INFO_HPP

#include <string>
#include <map>


namespace epidb {
  namespace dba {
    namespace info {

      bool get_genome(const std::string &id, std::map<std::string, std::string> &, std::string &msg, bool full = false);

      bool get_project(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false);

      bool get_bio_source(const std::string &id, std::map<std::string, std::string> &res,
                          std::map<std::string, std::string> &metadata, std::string &msg, bool full = false);

      bool get_technique(const std::string &id, std::map<std::string, std::string> &res,
                         std::map<std::string, std::string> &metadata, std::string &msg, bool full = false);

      bool get_sample_by_id(const std::string &id, std::map<std::string, std::string> &, std::string &msg, bool full = false);

      bool get_epigenetic_mark(const std::string &id, std::map<std::string, std::string> &, std::string &msg, bool full = false);

      bool get_annotation(const std::string &id, std::map<std::string, std::string> &res,
                          std::map<std::string, std::string> &metadata, std::string &msg, bool full = false);

      bool get_experiment(const std::string &id, std::map<std::string, std::string> &res,
                          std::map<std::string, std::string> &metadata, std::string &msg, bool full = false);

      bool get_query(const std::string &id, std::map<std::string, std::string> &, std::string &msg);

      bool get_sample_field(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full = false);
    }
  }
}

#endif