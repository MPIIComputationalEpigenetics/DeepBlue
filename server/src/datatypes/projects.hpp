//
//  projects.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.05.14.
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

#ifndef EPIDB_DBA_PROJECTS_HPP
#define EPIDB_DBA_PROJECTS_HPP

#include <string>

#include "../extras/utils.hpp"

namespace epidb {
  namespace datatypes {
    namespace projects {

      bool add_project(const std::string &name, const std::string &norm_name,
                       const std::string &description, const std::string &norm_description,
                       const utils::IdName &user,
                       std::string &project_id, std::string &msg);

      bool get_id(const std::string &name, std::string& id, std::string &msg);

      /*
       * \brief Set the project as public. Only project owner or admin may execute this operation
       */
      bool set_public(const std::string &project_id, const bool set, std::string &msg);

      bool add_user_to_project(const std::string &user_id, const std::string &project_id, const bool include, std::string &msg);
    }
  }
}

#endif
