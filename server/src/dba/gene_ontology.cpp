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

#include "../dba/genes.hpp"

#include "../cache/connected_cache.hpp"

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


        // Building the data necessary for searching in full text
        mongo::BSONObjBuilder search_data_builder;
        search_data_builder.append("_id", gene_ontology_term_id);
        search_data_builder.append("go_id", go_id);
        search_data_builder.append("go_label", go_label);
        search_data_builder.append("description", description);
        search_data_builder.append("norm_description", norm_description);
        search_data_builder.append("go_namespace", go_namespace);
        mongo::BSONObj search_data = search_data_builder.obj();

        // Building the data of the GO Terms in the regular database
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

        mongo::BSONObj go_term;
        if (!helpers::get_one(Collections::GENE_ONTOLOGY(), BSON("go_id" << go_id), go_term)) {
          return false;
        }

        mongo::BSONObjBuilder gene_ontology_term_bob;
        gene_ontology_term_bob.append(go_term["go_id"]);
        gene_ontology_term_bob.append(go_term["go_label"]);
        gene_ontology_term_bob.append(go_term["go_namespace"]);
        mongo::BSONObj gene_ontology_term = gene_ontology_term_bob.obj();
        mongo::BSONObj change_value = BSON("$addToSet" << BSON("go_annotation" << gene_ontology_term));

        mongo::BSONObjBuilder gene_query_bob;
        gene_query_bob.appendRegex(KeyMapper::ATTRIBUTES() + ".gene_id", "^"+gene_ensg_id);
        mongo::BSONObj gene_query = gene_query_bob.obj();
        c->update(helpers::collection_name(Collections::GENES()), gene_query, change_value, false, true);

        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        {
          mongo::BSONObjBuilder index_name;
          index_name.append("go_annotation.go_id", 1);
          c->createIndex(helpers::collection_name(Collections::GENES()), index_name.obj());
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
        }

        {
          mongo::BSONObjBuilder index_name;
          index_name.append("go_annotation.go_namespace", 1);
          c->createIndex(helpers::collection_name(Collections::GENES()), index_name.obj());
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
        }

        c.done();

        return true;
      }


      bool get_annotated_gene(const std::string& go_id,
                              std::vector<mongo::BSONObj>& genes,  std::string& msg)
      {
        {
          mongo::BSONObjBuilder index_name;
          index_name.append("go_annotation.go_id", 1);
          Connection c;
          c->createIndex(helpers::collection_name(Collections::GENES()), index_name.obj());
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
          c.done();
        }

        mongo::BSONObj query = BSON("go_annotation.go_id" << go_id);
        return helpers::get(Collections::GENES(), query, genes, msg);
      }

      ConnectedCache go_cache;

      bool __is_connected(const std::string &term_1, const std::string &term_2,
                          const bool recursive,
                          bool &r, std::string &msg)
      {
        if (go_cache.is_connected(term_1, term_2)) {
          r = true;
          return true;
        }

        mongo::BSONObj query_obj = BSON("go_id" << term_1);
        mongo::BSONObj term_bson;

        Connection c;
        term_bson = c->findOne(helpers::collection_name(Collections::GENE_ONTOLOGY()), query_obj);
        c.done();

        if (term_bson.isEmpty()) {
          r = false;
          return true;
        }

        if (!term_bson.hasField("subs")) {
          r = false;
          return true;
        }
        std::vector<mongo::BSONElement> e = term_bson["subs"].Array();

        std::list<std::string> subs;

        for (const mongo::BSONElement & be : e) {
          std::string sub = be.str();
          if (sub == term_2) {
            r = true;
            go_cache.set_connection(term_1, term_2);
            go_cache.set_connection(sub, term_1);
            return true;
          }
          subs.push_back(sub);
        }

        if (recursive) {
          for (const std::string & sub : subs) {
            bool rr;
            if (!__is_connected(sub, term_2, recursive, rr, msg)) {
              return false;
            }
            if (rr) {
              r = true;
              go_cache.set_connection(term_1, term_2);
              go_cache.set_connection(sub, term_2);
              return true;
            }
          }
        }

        r = false;
        return true;
      }

      bool __get_down_connected(const std::string &go_id, std::vector<std::string> &go_terms_id, std::string &msg)
      {
        go_terms_id.push_back(go_id);

        Connection c;
        mongo::BSONObj term_bson = c->findOne(helpers::collection_name(Collections::GENE_ONTOLOGY()), BSON("go_id" << go_id));
        c.done();

        if (!term_bson.hasField("subs")) {
          return true;
        }

        std::vector<mongo::BSONElement> e = term_bson["subs"].Array();
        for (const mongo::BSONElement & be : e) {
          std::string sub = be.str();
          if (!__get_down_connected(sub, go_terms_id, msg)) {
            return false;
          }
        }
        return true;
      }


      bool __get_up_connected(const std::string &go_id, std::vector<std::string> &go_terms_id, std::string &msg)
      {
        go_terms_id.push_back(go_id);

        Connection c;
        mongo::BSONObj term_bson = c->findOne(helpers::collection_name(Collections::GENE_ONTOLOGY()), BSON("go_id" << go_id));
        c.done();

        if (!term_bson.hasField("uppers")) {
          return true;
        }

        std::vector<mongo::BSONElement> e = term_bson["uppers"].Array();
        for (const mongo::BSONElement & be : e) {
          std::string up = be.str();
          if (!__get_up_connected(up, go_terms_id, msg)) {
            return false;
          }
        }
        return true;
      }

      bool __full_text_relation(const std::string &go_term_id, const std::string &smaller_scope, std::string &msg)
      {
        // Get the sub terms
        std::vector<std::string> related_terms;

        mongo::BSONObj go_term;
        if (!helpers::get_one(Collections::GENE_ONTOLOGY(), BSON("go_id" <<go_term_id), go_term)) {
          return false;
        }

        // Load the related terms
        related_terms.push_back(go_term["go_label"].str());
        related_terms.push_back(go_term["go_namespace"].str());

        if (!search::get_related_terms(go_term_id, go_term_id,
                                       "go_id", "gene_ontology",
                                       related_terms, msg)) {
          return false;
        }

        // Get the sub terms
        std::vector<std::string> go_terms_id;
        if (!__get_down_connected(go_term_id, go_terms_id, msg)) {
          return false;
        }

        // Include the related terms in sub terms
        for (auto &go_term_id : go_terms_id) {
          if (!search::insert_related_gene_ontology_term(go_term_id, related_terms, msg)) {
            return false;
          }
        }

        return true;
      }

      // A term will store ONLY the direct sub elements but
      // a term will store ALL the upper elements
      bool set_go_parent(const std::string& bigger_scope, const std::string& smaller_scope, std::string& msg)
      {
        bool is_connected(false);
        if (!__is_connected(bigger_scope, smaller_scope, false, is_connected, msg)) {
          return false;
        }
        if (is_connected) {
          std::string e = Error::m(ERR_ALREADY_CONECTED_GENE_ONTOLOGY_TERM, bigger_scope, smaller_scope);
          msg = e;
          return false;
        }

        if (!__is_connected(smaller_scope, bigger_scope, true, is_connected, msg)) {
          return false;
        }

        if (is_connected) {
          std::string e = Error::m(ERR_ALREADY_CONECTED_GENE_ONTOLOGY_TERM, smaller_scope, bigger_scope);
          msg = e;
          return false;
        }

        // Include the sub element in the subs of the upper element
        mongo::BSONObj query = BSON("go_id" << bigger_scope);
        mongo::BSONObj append_value = BSON("$addToSet" << BSON("subs" << smaller_scope));

        Connection c;
        c->update(helpers::collection_name(Collections::GENE_ONTOLOGY()), query, append_value);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        // Get the upper elements from upper and include in the sub
        mongo::BSONObj upper_query = BSON("go_id" << smaller_scope);
        mongo::BSONObj upper_append_value = BSON("$addToSet" << BSON("uppers" << bigger_scope));

        c->update(helpers::collection_name(Collections::GENE_ONTOLOGY()), upper_query, upper_append_value);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        // Set the relation in the full text data
        c.done();
        if (!__full_text_relation(bigger_scope, smaller_scope, msg)) {
          c.done();
          return false;
        }
        go_cache.set_connection(bigger_scope, smaller_scope);

        // Now, update the GENES annotated with the smaller scope.

        std::vector<mongo::BSONObj> annotated_genes;
        if (!get_annotated_gene(smaller_scope, annotated_genes, msg)) {
          return false;
        }

        if (!annotated_genes.empty()) {
          std::vector<std::string> up_terms_id;
          if (! __get_up_connected(bigger_scope, up_terms_id, msg)) {
            return false;
          }

          for (auto const& gene_obj: annotated_genes) {
            for (auto const& up_go_id: up_terms_id) {
              const std::string& gene_ensg_id = gene_obj[KeyMapper::ATTRIBUTES()]["gene_id"].str();
              if (!annotate_gene(gene_ensg_id, up_go_id, msg)) {
                return false;
              }
            }
          }
        }

        return true;
      }

      bool count_go_terms_in_genes(const std::vector<std::string> &chromosomes, const Position start, const Position end,
                                   const std::string& strand,
                                   const std::vector<std::string>& genes, const std::vector<std::string>& go_terms,
                                   const std::string& gene_model, const std::string& norm_gene_model,
                                   std::vector<utils::IdNameCount>& counts, size_t &total_go_terms,
                                   std::string& msg)
      {
        total_go_terms = 0;

        mongo::BSONObj query;
        if (!genes::build_genes_database_query(chromosomes, start, end, strand,
                                               genes, go_terms,
                                               norm_gene_model, false, query, msg)) {
          return false;
        }

        mongo::BSONObj match = BSON("$match" << query);
        mongo::BSONObj unwind = BSON("$unwind" << "$go_annotation");
        mongo::BSONObj group = BSON( "$group" << BSON( "_id" << BSON("go_id" << "$go_annotation.go_id" << "go_label" << "$go_annotation.go_label") << "total" << BSON( "$sum" << 1 ) ) );
        mongo::BSONObj sort = BSON( "$sort" << BSON( "total" << 1));


        mongo::BSONArray pipeline = BSON_ARRAY( match << unwind << group << sort);

        Connection c;
        auto cursor = c->aggregate(helpers::collection_name(Collections::GENES()), pipeline);

        while  (cursor->more()) {
          mongo::BSONObj be = cursor->next();
          const mongo::BSONElement& _id = be["_id"];

          long count = be["total"].safeNumberLong();
          utils::IdNameCount inc;
          inc = utils::IdNameCount(_id["go_id"].String(), _id["go_label"].String(), count);
          counts.push_back(inc);
          total_go_terms += count;
        }

        c.done();

        return true;
      }
    }
  }
}
