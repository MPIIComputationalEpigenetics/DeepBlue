//
//  changes.cpp
//  epidb
//
//  Created by Felipe Albrecht on 29.09.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "config.hpp"
#include "collections.hpp"
#include "helpers.hpp"
#include "full_text.hpp"

namespace epidb {
  namespace dba {
    namespace changes {

      bool change_extra_metadata(const std::string &id, const std::string &key, const std::string &value, std::string &msg)
      {
        std::string collection;
        if (id.compare(0, 1, "a") == 0) {
          collection = Collections::ANNOTATIONS();
        } else if (id.compare(0, 2, "bs") == 0) {
          collection = Collections::BIOSOURCES();
        } else if (id.compare(0, 1, "s") == 0) {
          collection = Collections::SAMPLES();
        } else if (id.compare(0, 1, "e") == 0) {
          collection = Collections::EXPERIMENTS();
        } else {
          msg = "Invalid identifier: " + id + ". It is accepted only data ID from experiments, annotations, biosources, and samples";
          return false;
        }

        Connection c;

        mongo::BSONObj query = BSON("_id" << id);
        mongo::BSONObj change_value;

        if (value.empty()) {
        	change_value = BSON("$unset" << BSON("extra_metadata." + key << value));
        } else {
        	change_value = BSON("$set" << BSON("extra_metadata." + key << value));
        }

        c->update(helpers::collection_name(collection), query, change_value);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!search::change_extra_metadata_full_text(id, key, value, msg)) {
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
