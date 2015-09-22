//
//  full_text.cpp
//  epidb
//
//  Created by Felipe Albrecht on 30.04.13.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <time.h>
#include <math.h>

#include <boost/foreach.hpp>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "../extras/utils.hpp"

#include "collections.hpp"
#include "config.hpp"
#include "helpers.hpp"
#include "full_text.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace dba {
    namespace search {

      /**
       * Full Text Search
       */

      bool insert_full_text(const std::string &type, const std::string &id,
                            const mongo::BSONObj &data, std::string &msg)
      {
        Connection c;

        // XXX
        // It was a bug in older C++ driver that came with mongodb version 2.4
        // Delete these lines if the bug does not happen anymore
        // if the collection is dropped MongoDB doesn't realize the index is deleted
        // and will not recreate it even though it's missing. So we have to reset the
        // index cache manually.
        // c->resetIndexCache();

        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c->createIndex(helpers::collection_name(Collections::TEXT_SEARCH()),
                       mongo::fromjson("{\"epidb_id\": 1}"));
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c->createIndex(helpers::collection_name(Collections::TEXT_SEARCH()),
                       mongo::fromjson("{\"$**\": \"text\", \"name\":\"text\", \"description\":\"text\"}"));

        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        mongo::BSONObjBuilder create_text_search_builder;
        for (mongo::BSONObj::iterator it = data.begin(); it.more(); ) {
          mongo::BSONElement e = it.next();
          // don't write the _id field
          std::string field_name = std::string(e.fieldName());
          if (field_name == "_id") {
            continue;
          }

          if (field_name == "D") {
            continue;
          }

          if (e.isSimpleType()) {
            create_text_search_builder.append(e.fieldName(), e.str());
            continue;
          }

          if (field_name == "extra_metadata" || field_name == "sample_info") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement em = itt.next();
              if (em.isSimpleType()) {
                create_text_search_builder.append(field_name + "_" + em.fieldName(), em.str());
              }
            }
          }
        }

        if (type == "experiments" || type == "samples") {
          std::string norm_biosource_name;
          if (type == "experiments") {
            norm_biosource_name = data["sample_info"]["norm_biosource_name"].str();
          } else {
            norm_biosource_name = data["norm_biosource_name"].str();
          }
          std::auto_ptr<mongo::DBClientCursor> cursor = c->query(helpers::collection_name(Collections::TEXT_SEARCH()),
              mongo::fromjson("{\"norm_name\": \"" + norm_biosource_name + "\", \"type\": \"biosources\"}"));

          if (!cursor->more()) {
            std::string s = Error::m(ERR_DATABASE_INVALID_BIOSOURCE, norm_biosource_name);
            EPIDB_LOG_TRACE(s);
            msg = s;
            c.done();
            return false;
          }

          mongo::BSONObj biosource = cursor->next().getOwned();
          if (biosource.hasField("related_terms")) {
            create_text_search_builder.append(biosource["related_terms"]);
          }
        }

        create_text_search_builder.append("epidb_id", id);
        create_text_search_builder.append("type", type);
        mongo::BSONObj q = create_text_search_builder.obj();

        c->insert(helpers::collection_name(Collections::TEXT_SEARCH()), q);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();
        return true;
      }

      bool insert_related_term(const utils::IdName &id_name, const std::vector<std::string> &related_terms,
                               std::string &msg)
      {
        Connection c;

        mongo::BSONObjBuilder query_builder;
        query_builder.append("epidb_id", id_name.id);
        mongo::BSONObj query = query_builder.obj();

        mongo::BSONArray related_terms_arr = helpers::build_array(related_terms);
        mongo::BSONObj append_value = BSON("$addToSet" << BSON("related_terms" << BSON("$each" << related_terms_arr)));

        // Update the biosource term (that were informed in the id_names.name)
        c->update(helpers::collection_name(Collections::TEXT_SEARCH()), query, append_value, false, true);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        // Update the experiments (that were informed in the id_names.id)
        mongo::BSONObjBuilder update_related_query_builder;
        update_related_query_builder.append("norm_biosource_name",  id_name.name);
        mongo::BSONObj update_related_query = update_related_query_builder.obj();
        c->update(helpers::collection_name(Collections::TEXT_SEARCH()), update_related_query, append_value, false, true);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }


        // Update the samples
        mongo::BSONObjBuilder update_related_query_builder_for_experiments;
        update_related_query_builder_for_experiments.append("sample_info_norm_biosource_name", id_name.name);
        mongo::BSONObj update_related_query_for_experiments = update_related_query_builder_for_experiments.obj();
        c->update(helpers::collection_name(Collections::TEXT_SEARCH()), update_related_query_for_experiments, append_value, false, true);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();
        return true;
      }

      bool change_extra_metadata_full_text(const std::string &id, const std::string &key, const std::string &value,
                                           const std::string &norm_value, const bool is_sample,
                                           std::string &msg)
      {
        Connection c;

        mongo::BSONObj query = BSON("epidb_id" << id);
        mongo::BSONObj change_value;

        std::string db_key;
        std::string norm_db_key;

        if (is_sample) {
          db_key = key;
          norm_db_key = "norm_" + key;
        } else {
          db_key = "extra_metadata_" + key;
          norm_db_key = "extra_metadata_norm_" + key;
        }

        if (value.empty()) {
          if (is_sample) {
            change_value = BSON("$unset" << BSON(db_key << "" << norm_db_key << ""));
          } else {
            change_value = BSON("$unset" << BSON(db_key << ""));
          }

        } else {
          if (is_sample) {
            change_value = BSON("$set" << BSON(db_key << value << norm_db_key << norm_value));
          } else {
            change_value = BSON("$set" << BSON(db_key << value));
          }
        }

        c->update(helpers::collection_name(Collections::TEXT_SEARCH()), query, change_value);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();
        return true;
      }

      bool search_full_text(const std::string &text, const std::vector<std::string> &types,
                            const std::vector<std::string>& private_projects,
                            std::vector<TextSearchResult> &results, std::string &msg)
      {
        Connection c;

        mongo::BSONObjBuilder cmd_builder;
        if (!types.empty()) {
          cmd_builder.append("type", helpers::build_condition_array<std::string>(types, "$in"));
        }

        if (!private_projects.empty()) {
          cmd_builder.append("norm_project", helpers::build_condition_array<std::string>(private_projects, "$nin"));
          cmd_builder.append("norm_name", helpers::build_condition_array<std::string>(private_projects, "$nin"));
        }

        mongo::BSONObjBuilder text_builder;
        text_builder.append("$search", text);
        cmd_builder.append("$text", text_builder.obj());
        mongo::BSONObj search = cmd_builder.obj();

        mongo::BSONObjBuilder view_builder;
        view_builder.append("epidb_id", 1);
        view_builder.append("name", 1);
        view_builder.append("type", 1);
        view_builder.append("score", BSON("$meta" << "textScore"));

        mongo::BSONObj projection = view_builder.obj();

        mongo::BSONObj SORT = BSON("score" << BSON("$meta" << "textScore"));

        const size_t MAX_RESULTS = 50;


        auto cursor = c->query(helpers::collection_name(Collections::TEXT_SEARCH()), mongo::Query(search).sort(SORT), MAX_RESULTS, 0, &projection);

        while (cursor->more()) {
          TextSearchResult res;
          mongo::BSONObj o = cursor->next();
          res.id = o["epidb_id"].str();
          res.name = o["name"].str();
          res.type = o["type"].str();
          res.score = o["score"].number();
          results.push_back(res);
        }
        c.done();

        return true;
      }

      bool remove(const std::string &id, std::string &msg)
      {
        const std::string collection = helpers::collection_name(Collections::TEXT_SEARCH());
        return helpers::remove_all(collection, BSON("epidb_id" << id), msg);
      }
    }
  }
}
