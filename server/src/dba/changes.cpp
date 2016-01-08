//
//  changes.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.09.14.
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

#include "../connection/connection.hpp"

#include "config.hpp"
#include "collections.hpp"
#include "helpers.hpp"
#include "full_text.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace dba {
    namespace changes {

      bool change_extra_metadata(const std::string &id, const std::string &key, const std::string &value, std::string &msg)
      {
        bool is_sample = false;
        std::string collection;
        if (id.compare(0, 1, "a") == 0) {
          collection = Collections::ANNOTATIONS();
        } else if (id.compare(0, 2, "bs") == 0) {
          collection = Collections::BIOSOURCES();
        } else if (id.compare(0, 1, "em") == 0) {
          collection = Collections::EPIGENETIC_MARKS();
        } else if (id.compare(0, 1, "s") == 0) {
          collection = Collections::SAMPLES();
          is_sample = true;
        } else if (id.compare(0, 1, "e") == 0) {
          collection = Collections::EXPERIMENTS();
        } else {
          msg = Error::m(ERR_INVALID_IDENTIFIER, id);
          msg += ". It is accepted only data ID from experiments, annotations, biosources, and samples";
          return false;
        }

        Connection c;

        std::string norm_value = utils::normalize_name(value);

        mongo::BSONObj query = BSON("_id" << id);
        mongo::BSONObj change_value;

        std::string db_key;

        if (is_sample) {
          if ((key == "_id") || (key == "epidb_id") || (key == "type") || (key == "related_terms") || (key == "biosource_name")) {
            msg = "The sample key " + key + " is immutable";
            return false;
          }
          db_key = key;
        } else {
          db_key = "extra_metadata." + key;
        }

        // we do not normalize the extra metadata content
        if (value.empty()) {
          if (is_sample) {
            change_value = BSON("$unset" << BSON(db_key << "" << "norm_" + key << ""));
          } else {
            change_value = BSON("$unset" << BSON(db_key << ""));
          }
        } else {
          if (is_sample) {
            change_value = BSON("$set" << BSON(db_key << value << "norm_" + key << norm_value));
          } else {
            change_value = BSON("$set" << BSON(db_key << value));
          }
        }

        c->update(helpers::collection_name(collection), query, change_value);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!search::change_extra_metadata_full_text(id, key, value, norm_value, is_sample, msg)) {
          c.done();
          return false;
        }

        if (!helpers::notify_change_occurred(collection, msg)) {
          return false;
        }

        c.done();
        return true;
      }
    }
  }
}
