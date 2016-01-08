//
//  data.cpp
//  DeepBlue Epigenomic Data Server

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
#include <string>

#include "collections.hpp"
#include "column_types.hpp"
#include "helpers.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace dba {
    namespace data {

      bool sample(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::SAMPLES(), mongo::Query(BSON("_id" << id)), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_SAMPLE_ID, id);
          return false;
        }
      }

      bool genome(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::GENOMES(), mongo::Query(BSON("_id" << id)), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_GENOME_ID, id);
          return false;
        }
      }

      bool project(const std::string &id, const std::vector<std::string>& user_projects,
                   mongo::BSONObj &result, std::string &msg)
      {
        mongo::BSONObj query = BSON("$and" <<
                                    BSON_ARRAY(
                                      BSON("_id" << id) <<
                                      BSON("norm_name" <<
                                           BSON("$in" << utils::build_array(user_projects))
                                          )
                                    )
                                   );

        if (helpers::get_one(Collections::PROJECTS(), mongo::Query(query), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_PROJECT_ID, id);
          return false;
        }
      }

      bool technique(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::TECHNIQUES(), mongo::Query(BSON("_id" << id)), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_TECHNIQUE_ID, id);
          return false;
        }
      }

      bool biosource(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::BIOSOURCES(), mongo::Query(BSON("_id" << id)), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_BIOSOURCE_ID, id);
          return false;
        }
      }

      bool epigenetic_mark(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::EPIGENETIC_MARKS(), mongo::Query(BSON("_id" << id)), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_EPIGENETIC_MARK_ID, id);
          return false;
        }
      }

      bool annotation(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::ANNOTATIONS(), mongo::Query(BSON("_id" << id)), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_ANNOTATION_ID, id);
          return false;
        }
      }

      bool gene_set(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::GENE_SETS(), mongo::Query(BSON("_id" << id)), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_GENE_SET_ID, id);
          return false;
        }
      }

      bool experiment(const std::string &id, const std::vector<std::string>& user_projects,
                      mongo::BSONObj &result, std::string &msg)
      {
        mongo::BSONObj query = BSON("$and" <<
                                    BSON_ARRAY(
                                      BSON("_id" << id) <<
                                      BSON("norm_project" <<
                                           BSON("$in" << utils::build_array(user_projects))
                                          )
                                    )
                                   );
        if (helpers::get_one(Collections::EXPERIMENTS(), mongo::Query(query), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_EXPERIMENT_ID, id);
          return false;
        }
      }

      bool query(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::QUERIES(), mongo::Query(BSON("_id" << id)), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_QUERY_ID, id);
          return false;
        }
      }

      bool tiling_region(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::TILINGS(), mongo::Query(BSON("_id" << id)), result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_TILING_REGIONS_ID, id);
          return false;
        }
      }

      bool column_type(const std::string &id, mongo::BSONObj &result, std::string &msg)
      {
        if (helpers::get_one(Collections::COLUMN_TYPES(), mongo::Query(BSON("_id" << id)),  result)) {
          return true;
        } else {
          msg = Error::m(ERR_INVALID_COLUMN_TYPE_ID, id);
          return false;
        }
      }

    }
  }
}