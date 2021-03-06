//
//  controlled_vocabulary.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 01.08.13.
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

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "../cache/connected_cache.hpp"

#include "../extras/utils.hpp"

#include "controlled_vocabulary.hpp"
#include "collections.hpp"
#include "exists.hpp"
#include "helpers.hpp"
#include "full_text.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace dba {
    namespace cv {

      ConnectedCache biosources_cache;

      bool __get_synonyms_from_biosource(const std::string &id,
                                         const std::string &biosource_name, const std::string &norm_biosource_name,
                                         std::vector<utils::IdName> &syns, std::string &msg)
      {
        utils::IdName id_name_biosource;
        if (id.empty()) {
          if (!helpers::get_name(Collections::BIOSOURCES(), norm_biosource_name, id_name_biosource, msg)) {
            return false;
          }
        } else {
          id_name_biosource = utils::IdName(id, biosource_name);
        }

        syns.push_back(id_name_biosource);

        Connection c;
        auto syns_cursor = c->query(helpers::collection_name(
                                      Collections::BIOSOURCE_SYNONYMS()),
                                    BSON("norm_name" << norm_biosource_name)
                                   );

        if (!syns_cursor->more()) {
          c.done();
          return true;
        }

        mongo::BSONObj syn_bson = syns_cursor->next();
        std::vector<mongo::BSONElement> e = syn_bson["synonyms"].Array();

        for (const mongo::BSONElement & be : e) {
          std::string norm_synonym = be.str();
          mongo::BSONObj query = BSON("norm_synonym" << norm_synonym);

          auto syns_names_cursor = c->query(helpers::collection_name(Collections::BIOSOURCE_SYNONYM_NAMES()), query);

          if (!syns_names_cursor->more()) {
            msg = "It was not possible to find the name of " + norm_synonym + " .";
            c.done();
            return false;
          }
          mongo::BSONObj e_syn = syns_names_cursor->next();
          mongo::BSONElement syn = e_syn["synonym"];
          std::string syn_name = syn.str();
          utils::IdName id_syn_name(id_name_biosource.id, syn_name);
          syns.push_back(id_syn_name);
        }

        c.done();
        return true;
      }

      bool __get_synonyms_from_synonym(const std::string &synonym, const std::string &norm_synonym,
                                       std::vector<utils::IdName> &syns, std::string &msg)
      {
        Connection c;

        mongo::BSONObjBuilder syn_query_builder;
        syn_query_builder.append("norm_synonym", norm_synonym);

        mongo::BSONObj query_obj = syn_query_builder.obj();
        mongo::Query query = mongo::Query(query_obj).sort("synonym");
        auto syns_cursor = c->query(helpers::collection_name(Collections::BIOSOURCE_SYNONYM_NAMES()), query);

        if (!syns_cursor->more()) {
          msg = "It was not possible to find the biosources synonyms for " + synonym + " .";
          c.done();
          return false;
        }

        mongo::BSONObj syn_bson = syns_cursor->next();
        std::string biosource_name = syn_bson["biosource_name"].str();
        std::string norm_biosource_name = syn_bson["norm_biosource_name"].str();

        c.done();

        if (!__get_synonyms_from_biosource("", biosource_name, norm_biosource_name, syns, msg)) {
          return false;
        }

        return true;
      }

      bool __full_text_relation(const std::string &term, const std::string &norm_term,
                                const std::string &biosource, const std::string &norm_biosource,
                                std::string &msg)
      {
        std::vector<std::string> related_terms;

        // Get the synonyms from this term
        std::vector<utils::IdName> syns;
        if (!__get_synonyms_from_biosource("", term, norm_term, syns, msg)) {
          return false;
        }

        for (const utils::IdName & syn : syns) {
          related_terms.push_back(syn.name);
        }

        if (!search::get_related_terms(term, norm_term,
                                       "norm_name", "biosources",
                                       related_terms, msg)) {
          return false;
        }

        // Get the sub terms
        std::vector<std::string> norm_subs;
        if (!get_biosource_children(biosource, norm_biosource, true, norm_subs, msg)) {
          return false;
        }

        std::vector<utils::IdName> id_names;
        for (const std::string & norm_sub : norm_subs) {
          std::string biosource_id;
          if (!helpers::get_id(Collections::BIOSOURCES(), norm_sub, biosource_id, msg)) {
            return false;
          }
          id_names.push_back(utils::IdName(biosource_id, norm_sub));
        }

        // Update
        for (auto &id_name : id_names) {
          if (!search::insert_related_term(id_name, related_terms, msg)) {
            return false;
          }
        }

        return true;
      }

      bool get_synonym_root(const std::string &synonym, const std::string &norm_synonym,
                            std::string &biosource_name, std::string &norm_biosource_name, std::string &msg)
      {
        Connection c;

        mongo::BSONObjBuilder syn_query_builder;
        syn_query_builder.append("norm_synonym", norm_synonym);

        mongo::BSONObj query_obj = syn_query_builder.obj();
        mongo::Query query = mongo::Query(query_obj).sort("synonym");
        auto syns_cursor = c->query(helpers::collection_name(Collections::BIOSOURCE_SYNONYM_NAMES()), query);

        if (!syns_cursor->more()) {
          msg = "It was not possible to find the biosource root for " + synonym + " .";
          c.done();
          return false;
        }

        mongo::BSONObj syn_bson = syns_cursor->next();
        biosource_name = syn_bson["biosource_name"].str();
        norm_biosource_name = syn_bson["norm_biosource_name"].str();

        c.done();

        return true;
      }


      bool __set_biosource_synonym(const datatypes::User& user,
                                   const std::string &input_biosource_name, const std::string &synonym,
                                   bool is_biosource, const bool is_syn, std::string &msg)
      {
        std::string biosource_name;
        std::string norm_biosource_name;

        if (is_syn) {
          std::string norm_input_biosource_name = utils::normalize_name(input_biosource_name);
          if (!get_synonym_root(input_biosource_name, norm_input_biosource_name,
                                biosource_name, norm_biosource_name, msg)) {
            return false;
          }
        } else {
          biosource_name = input_biosource_name;
          norm_biosource_name = utils::normalize_name(input_biosource_name);
        }

        std::string norm_synonym = utils::normalize_name(synonym);

        mongo::BSONObjBuilder query_builder;
        query_builder.append("norm_name", norm_biosource_name);
        mongo::BSONObj query = query_builder.obj();

        mongo::BSONObj value = BSON("synonyms" << norm_synonym);

        mongo::BSONObj append_value;
        if (is_biosource) {
          mongo::BSONObjBuilder syn_insert_builder;
          syn_insert_builder.append("name", input_biosource_name);
          syn_insert_builder.append("norm_name", norm_biosource_name);
          append_value = BSON("$set" << syn_insert_builder.obj() << "$addToSet" << value);
        } else {
          append_value = BSON("$addToSet" << value);
        }

        Connection c;

        c->update(helpers::collection_name(Collections::BIOSOURCE_SYNONYMS()), query, append_value, true, false);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder syn_builder;
        syn_builder.append("synonym", synonym);
        syn_builder.append("norm_synonym", norm_synonym);
        syn_builder.append("biosource_name", biosource_name);
        syn_builder.append("norm_biosource_name", norm_biosource_name);
        syn_builder.append("user", user.id());
        syn_builder.append("public", true);
        c->insert(helpers::collection_name(Collections::BIOSOURCE_SYNONYM_NAMES()), syn_builder.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();

        if (!__full_text_relation(biosource_name, norm_biosource_name,
                                  biosource_name, norm_biosource_name,
                                  msg)) {
          return false;
        }

        return true;
      }

      bool set_biosource_synonym_complete(const datatypes::User& user,
                                          const std::string &biosource_name, const std::string &synonym_name,
                                          std::string& msg)
      {

        const std::string norm_biosource_name = utils::normalize_name(biosource_name);

        // TODO Move to a helper function: get_biosource_root
        // Check if the actual biosource exists
        bool is_biosource = exists::biosource(norm_biosource_name);
        bool is_syn = exists::biosource_synonym(norm_biosource_name);

        if (!(is_biosource || is_syn)) {
          msg = Error::m(ERR_INVALID_BIOSOURCE_NAME, biosource_name);
          return false;
        }

        // TODO Move to a helper function: get_biosource_root
        // Check if synonym name is already being user
        std::string norm_synoynm_name = utils::normalize_name(synonym_name);
        bool syn_is_biosource = exists::biosource(norm_synoynm_name);
        bool syn_is_syn = exists::biosource_synonym(norm_synoynm_name);

        if (syn_is_biosource || syn_is_syn) {
          msg = Error::m(ERR_INVALID_BIOSOURCE_SYNONYM, synonym_name);
          return false;
        }

        return __set_biosource_synonym(user, biosource_name, synonym_name, is_biosource, is_syn, msg);
      }

      bool get_biosource_synonyms(const std::string &id, const std::string &biosource_name,
                                  const std::string &norm_biosource_name,
                                  bool is_biosource,
                                  std::vector<utils::IdName> &syns, std::string &msg)
      {
        if (is_biosource) {
          return __get_synonyms_from_biosource(id, biosource_name, norm_biosource_name, syns, msg);
        } else {
          return __get_synonyms_from_synonym(biosource_name, norm_biosource_name, syns, msg);
        }
      }

      bool __is_connected(const std::string &norm_s1, const std::string &norm_s2,
                          const bool recursive,
                          bool &r, std::string &msg)
      {
        if (biosources_cache.is_connected(norm_s1, norm_s2)) {
          r = true;
          return true;
        }

        Connection c;

        mongo::BSONObjBuilder syn_query_builder;
        syn_query_builder.append("norm_biosource_name", norm_s1);

        mongo::BSONObj query_obj = syn_query_builder.obj();
        mongo::Query query = mongo::Query(query_obj);
        auto syns_cursor = c->query(helpers::collection_name(Collections::BIOSOURCE_EMBRACING()), query);

        if (!syns_cursor->more()) {
          c.done();
          r = false;
          return true;
        }

        mongo::BSONObj syn_bson = syns_cursor->next();
        std::vector<mongo::BSONElement> e = syn_bson["subs"].Array();

        c.done();

        std::list<std::string> subs;

        for (const mongo::BSONElement & be : e) {
          std::string sub = be.str();
          if (sub == norm_s2) {
            r = true;
            biosources_cache.set_connection(norm_s1, norm_s2);
            biosources_cache.set_connection(sub, norm_s2);
            return true;
          }
          subs.push_back(sub);
        }

        if (recursive) {
          for (const std::string & norm_sub : subs) {
            bool rr;
            if (!__is_connected(norm_sub, norm_s2, recursive, rr, msg)) {
              return false;
            }
            if (rr) {
              r = true;
              biosources_cache.set_connection(norm_s1, norm_s2);
              biosources_cache.set_connection(norm_sub, norm_s2);
              return true;
            }
          }
        }

        r = false;
        return true;
      }

      bool set_biosource_parent(const datatypes::User& user,
                                const std::string &biosource_more_embracing, const std::string &norm_biosource_more_embracing,
                                const std::string &biosource_less_embracing, const std::string &norm_biosource_less_embracing,
                                bool more_embracing_is_syn, const bool less_embracing_is_syn,
                                std::string &msg)
      {
        std::string more_embracing_root;
        std::string norm_more_embracing_root;
        std::string less_embracing_root;
        std::string norm_less_embracing_root;

        if (more_embracing_is_syn) {
          if (!get_synonym_root(biosource_more_embracing, norm_biosource_more_embracing,
                                more_embracing_root, norm_more_embracing_root, msg)) {
            return false;
          }
        } else {
          more_embracing_root = biosource_more_embracing;
          norm_more_embracing_root = norm_biosource_more_embracing;
        }

        if (less_embracing_is_syn) {
          if (!get_synonym_root(biosource_less_embracing, norm_biosource_less_embracing,
                                less_embracing_root, norm_less_embracing_root, msg)) {
            return false;
          }
        } else {
          less_embracing_root = biosource_less_embracing;
          norm_less_embracing_root = norm_biosource_less_embracing;
        }

        if (norm_more_embracing_root == norm_less_embracing_root) {
          msg = biosource_more_embracing + " and " + biosource_less_embracing + " are synonyms.";
          return false;
        }

        bool is_connected(false);
        if (!__is_connected(norm_more_embracing_root, norm_less_embracing_root, false, is_connected, msg)) {
          return false;
        }

        if (is_connected) {
          std::string e = Error::m(ERR_ALREADY_CONECTED_BIOSOURCE_NAME,
                                   biosource_more_embracing, biosource_less_embracing);
          EPIDB_LOG_TRACE(e);
          msg = e;
          return false;
        }

        if (!__is_connected(norm_less_embracing_root, norm_more_embracing_root, true, is_connected, msg)) {
          return false;
        }

        if (is_connected) {
          std::string e = Error::m(ERR_ALREADY_CONECTED_BIOSOURCE_NAME,
                                   biosource_more_embracing, biosource_less_embracing);
          EPIDB_LOG_TRACE(e);
          msg = e;
          return false;
        }

        Connection c;

        mongo::BSONObjBuilder query_builder;
        query_builder.append("norm_biosource_name", norm_more_embracing_root);
        mongo::BSONObj query = query_builder.obj();

        mongo::BSONObj value = BSON("subs" << norm_less_embracing_root);
        mongo::BSONObj append_value = BSON("$addToSet" << value);

        c->update(helpers::collection_name(Collections::BIOSOURCE_EMBRACING()), query, append_value, true, false);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();

        if (!__full_text_relation(more_embracing_root, norm_more_embracing_root,
                                  less_embracing_root, norm_less_embracing_root, msg)) {
          return false;
        }

        biosources_cache.set_connection(norm_biosource_more_embracing, norm_biosource_less_embracing);

        return true;
      }

      bool __get_down_connected(const std::string &norm_s1,
                                std::vector<std::string> &norm_names, std::string &msg)
      {
        norm_names.push_back(norm_s1);

        Connection c;

        mongo::BSONObjBuilder query_builder;
        query_builder.append("norm_biosource_name", norm_s1);

        mongo::BSONObj query_obj = query_builder.obj();
        mongo::Query query = mongo::Query(query_obj);
        auto syns_cursor = c->query(helpers::collection_name(Collections::BIOSOURCE_EMBRACING()), query);

        if (!syns_cursor->more()) {
          c.done();
          return true;
        }

        mongo::BSONObj syn_bson = syns_cursor->next().getOwned();
        std::vector<mongo::BSONElement> e = syn_bson["subs"].Array();

        c.done();

        for (const mongo::BSONElement & be : e) {
          std::string sub = be.str();
          if (!__get_down_connected(sub, norm_names, msg)) {
            return false;
          }
        }

        return true;
      }

      bool get_biosource_children(const std::string &biosource_name, const std::string &norm_biosource_name,
                                  bool is_biosource,
                                  std::vector<std::string> &norm_subs, std::string &msg)
      {
        std::string more_embracing_root;
        std::string norm_more_embracing_root;

        if (!is_biosource) {
          if (!get_synonym_root(biosource_name, norm_biosource_name,
                                more_embracing_root, norm_more_embracing_root, msg)) {
            return false;
          }
        } else {
          norm_more_embracing_root = norm_biosource_name;
        }

        if (!__get_down_connected(norm_more_embracing_root, norm_subs, msg)) {
          return false;
        }
        return true;
      }

      bool __get_upper_connected(const std::string &norm_s1,
                                 std::vector<std::string> &norm_uppers, std::string &msg)
      {
        Connection c;

        mongo::BSONObjBuilder query_builder;
        query_builder.append("subs", norm_s1);
        mongo::BSONObj query_obj = query_builder.obj();
        mongo::Query query = mongo::Query(query_obj);
        auto upper_cursor = c->query(helpers::collection_name(Collections::BIOSOURCE_EMBRACING()), query);

        while (upper_cursor->more()) {
          mongo::BSONObj upper_bson = upper_cursor->next();
          std::string e = upper_bson["norm_biosource_name"].String();
          norm_uppers.push_back(e);
        }

        c.done();
        return true;
      }

      bool get_biosource_parents(const std::string &biosource_name, const std::string &norm_biosource_name,
                                 bool is_biosource,
                                 std::vector<std::string> &norm_uppers, std::string &msg)
      {
        std::string more_embracing_root;
        std::string norm_more_embracing_root;

        if (!is_biosource) {
          if (!get_synonym_root(biosource_name, norm_biosource_name,
                                more_embracing_root, norm_more_embracing_root, msg)) {
            return false;
          }
        } else {
          norm_more_embracing_root = norm_biosource_name;
        }

        if (!__get_upper_connected(norm_more_embracing_root, norm_uppers, msg)) {
          return false;
        }
        return true;
      }

      bool remove_biosouce(const std::string &id, const std::string &biosource_name, const std::string &norm_biosource_name, std::string &msg)
      {
        Connection c;

        // Start deleting the data
        // delete from full text search
        if (!search::remove(id, msg)) {
          c.done();
          return false;
        }

        c->update(helpers::collection_name(Collections::TEXT_SEARCH()), BSON("related_terms" << norm_biosource_name), BSON("$pull" << BSON("related_terms" << norm_biosource_name)), false, true);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        // Remove from embracing collection // useless. just for sake
        if (!helpers::remove_all(helpers::collection_name(Collections::BIOSOURCE_EMBRACING()), BSON("norm_biosource_name" << norm_biosource_name), msg)) {
          c.done();
          return false;
        }

        // Remove from the others biosources scope
        c->update(helpers::collection_name(Collections::BIOSOURCE_EMBRACING()), BSON("subs" << norm_biosource_name), BSON("$pull" << BSON("subs" << norm_biosource_name)), false, true);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        // remove from synonyms collection
        if (!helpers::remove_all(helpers::collection_name(Collections::BIOSOURCE_SYNONYMS()), BSON("norm_name" << norm_biosource_name), msg)) {
          return false;
        }

        // remove from synonyms names collection
        if (!helpers::remove_all(helpers::collection_name(Collections::BIOSOURCE_SYNONYM_NAMES()), BSON("norm_biosource_name" << norm_biosource_name), msg)) {
          c.done();
          return false;
        }


        // remove itself
        if (!helpers::remove_one(helpers::collection_name(Collections::BIOSOURCES()), id, msg)) {
          c.done();
          return false;
        }

        c.done();
        return true;
      }
    }
  }
}
