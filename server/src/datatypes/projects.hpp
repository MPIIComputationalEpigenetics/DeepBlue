//
//  projects.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.14.
//  Copyright (c) 2013,2014,2015 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_PROJECTS_HPP
#define EPIDB_DBA_PROJECTS_HPP

#include <string>

#include "../extras/utils.hpp"

namespace epidb {
  namespace datatypes {
    namespace projects {


      bool get_id(const std::string &name, std::string& id, std::string &msg);


      bool is_public(const std::string &project_id, bool& ret, std::string &msg);

      /*
       * \brief Set the project as public. Only project owner or admin may execute this operation
       */
      bool set_public(const std::string &project_id, const bool set, std::string &msg);

      bool add_user_to_project(const std::string &user_id, const std::string &project_id, std::string &msg);

      /*
       * \brief List all projects that are available for the user
       */
      bool list_user_projects(const std::string &user_id, const std::string &project_id, std::string &msg);
    }
  }
}

#endif
