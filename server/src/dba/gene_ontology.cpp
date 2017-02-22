//
//  gene_ontogy.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 21.02.2017
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

#include "../datatypes/user.hpp"

#include "collections.hpp"
#include "full_text.hpp"
#include "helpers.hpp"
#include "key_mapper.hpp"
#include "users.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace dba {
    namespace gene_ontology {

      const std::string GO_NAMESPACE_CELLULAR_COMPONENT = "cellular_component";
      const std::string GO_NAMESPACE_BIOLOGICAL_PROCESS = "biological_process";
      const std::string GO_NAMESPACE_MOLECULAR_FUNCTION = "molecular_function";

      bool exists_gene_ontology_term(const std::string &go_id)
      {
        return helpers::check_exist(Collections::GENE_ONTOLOGY(), "go_id", go_id);
      }

      bool is_valid_gene_ontology(const std::string &go_id,  const std::string &go_label,
                                  const std::string &go_namespace, std::string &msg)
      {
        if (exists_gene_ontology_term(go_id)) {
          msg = Error::m(ERR_DUPLICATED_GENE_ONTOLOGY_TERM_ID, go_id);
          return false;
        }

        if (helpers::check_exist(Collections::GENE_ONTOLOGY(), "go_label", go_label)) {
          msg = Error::m(ERR_DUPLICATED_GENE_ONTOLOGY_TERM_LABEL, go_label);
          return false;
        }

        if ((go_namespace != GO_NAMESPACE_CELLULAR_COMPONENT) &&
            (go_namespace != GO_NAMESPACE_BIOLOGICAL_PROCESS) &&
            (go_namespace != GO_NAMESPACE_MOLECULAR_FUNCTION)) {

          msg = Error::m(ERR_INVALID_GENE_ONTOLOGY_NAMESPACE, go_namespace);
          return false;
        }

        return true;
      }


      bool add_gene_ontology_term(const std::string &go_id,
                                  const std::string &go_label,
                                  const std::string &description, const std::string &norm_description,
                                  const std::string &go_namespace,
                                  const std::string &user_key,
                                  std::string &gene_ontology_term_id, std::string &msg)
      {
        {
          int id;
          if (!helpers::get_increment_counter("gene_ontology_term", id, msg) ||
              !helpers::notify_change_occurred(Collections::GENE_ONTOLOGY(), msg)) {
            return false;
          }
          gene_ontology_term_id = "go" + utils::integer_to_string(id);
        }

        mongo::BSONObjBuilder search_data_builder;
        search_data_builder.append("_id", gene_ontology_term_id);
        search_data_builder.append("go_id", go_id);
        search_data_builder.append("go_label", go_label);
        search_data_builder.append("description", description);
        search_data_builder.append("norm_description", norm_description);
        search_data_builder.append("go_namespace", go_namespace);

        mongo::BSONObj search_data = search_data_builder.obj();
        mongo::BSONObjBuilder create_gene_ontology_term_builder;
        create_gene_ontology_term_builder.appendElements(search_data);

        utils::IdName user;
        if (!users::get_user(user_key, user, msg)) {
          return false;
        }
        create_gene_ontology_term_builder.append("user", user.id);
        mongo::BSONObj cem = create_gene_ontology_term_builder.obj();

        Connection c;
        c->insert(helpers::collection_name(Collections::GENE_ONTOLOGY()), cem);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!search::insert_full_text(Collections::GENE_ONTOLOGY(), gene_ontology_term_id, search_data, msg)) {
          c.done();
          return false;
        }

        {
          mongo::BSONObjBuilder index_name;
          index_name.append("go_id", "hashed");
          c->createIndex(helpers::collection_name(Collections::GENE_ONTOLOGY()), index_name.obj());
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
        }

        {
          mongo::BSONObjBuilder index_name;
          index_name.append("go_label", "hashed");
          c->createIndex(helpers::collection_name(Collections::GENE_ONTOLOGY()), index_name.obj());
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
        }

        {
          mongo::BSONObjBuilder index_name;
          index_name.append("go_namespace", "hashed");
          c->createIndex(helpers::collection_name(Collections::GENE_ONTOLOGY()), index_name.obj());
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
        }

        c.done();
        return true;
      }

      bool annotate_gene(const std::string& gene_ensg_id, const std::string& go_id, std::string& msg)
      {
        Connection c;

        mongo::BSONObjBuilder gene_query_bob;
        gene_query_bob.appendRegex(KeyMapper::ATTRIBUTES() + ".gene_id", "^"+gene_ensg_id);
        mongo::BSONObj gene_query = gene_query_bob.obj();


        mongo::BSONObjBuilder gene_ontology_term_query_bob;
        gene_ontology_term_query_bob.append("go_id", go_id);
        mongo::BSONObj gene_ontology_term_query = gene_ontology_term_query_bob.obj();

        mongo::BSONObj go_term;

        if (!helpers::get_one(Collections::GENE_ONTOLOGY(), gene_ontology_term_query, go_term)) {
          return false;
        }

        mongo::BSONObjBuilder gene_ontology_term_bob;
        gene_ontology_term_bob.append(go_term["go_id"]);
        gene_ontology_term_bob.append(go_term["go_label"]);
        gene_ontology_term_bob.append(go_term["go_namespace"]);
        mongo::BSONObj gene_ontology_term = gene_ontology_term_bob.obj();

        mongo::BSONObj change_value = BSON("$push" << BSON("go_annotation" << gene_ontology_term));

        c->update(helpers::collection_name(Collections::GENES()), gene_query, change_value, false, true);

        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();

        return true;

      }

    }
  }
}
