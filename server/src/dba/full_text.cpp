//
//  full_text.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 30.04.13.
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

#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <time.h>
#include <math.h>

#include <mongo/bson/bson.h>

#include "../config/config.hpp"

#include "../connection/connection.hpp"

#include "../extras/utils.hpp"

#include "collections.hpp"
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
        for (auto it = data.begin(); it.more(); ) {
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
            for (auto itt = e.Obj().begin(); itt.more(); ) {
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
          auto cursor = c->query(helpers::collection_name(Collections::TEXT_SEARCH()),
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

      bool get_related_terms(const std::string& name, const std::string& norm_name,
                             const std::string& key_name, const std::string& type,
                             std::vector<std::string>& related_terms,
                             std::string& msg)
      {
        Connection c;
        mongo::BSONObj term_bson = c->findOne(helpers::collection_name(Collections::TEXT_SEARCH()),
                                              BSON(key_name << norm_name << "type" << type));
        c.done();

        if (term_bson.isEmpty()) {
          msg = "Internal error: '" + type + "' '" + name + "' not found.";
          return false;
        }

        if (!term_bson.hasElement("related_terms")) {
          return true;
        }

        std::vector<mongo::BSONElement> e = term_bson["related_terms"].Array();
        for (const mongo::BSONElement & be : e) {
          related_terms.push_back(be.str());
        }

        return true;
      }

      bool insert_related_term(const utils::IdName &id_name, const std::vector<std::string> &related_terms,
                               std::string &msg)
      {
        mongo::BSONObj query = BSON("epidb_id" << id_name.id);
        mongo::BSONArray related_terms_arr = utils::build_array(related_terms);
        mongo::BSONObj append_value = BSON("$addToSet" << BSON("related_terms" << BSON("$each" << related_terms_arr)));

        // Update the biosource term (that were informed in the id_names.name)
        Connection c;
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

      bool insert_related_gene_ontology_term(const std::string& go_term_id, const std::vector<std::string>& terms_to_include,
                                             std::string& msg)
      {
        Connection c;
        {
          mongo::BSONObjBuilder index_name;
          index_name.append("type", 1);
          index_name.append("go_id", 1);
          c->createIndex(helpers::collection_name(Collections::TEXT_SEARCH()), index_name.obj());
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
        }

        mongo::BSONArray terms_to_include_arr = utils::build_array(terms_to_include);
        mongo::BSONObj append_value = BSON("$addToSet" << BSON("related_terms" << BSON("$each" << terms_to_include_arr)));

        c->update(helpers::collection_name(Collections::TEXT_SEARCH()), BSON("type" << "gene_ontology" << "go_id" << go_term_id), append_value);
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
