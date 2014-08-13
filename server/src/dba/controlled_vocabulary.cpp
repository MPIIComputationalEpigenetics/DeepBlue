//
//  controlled_vocabulary.cpp
//  epidb
//
//  Created by Felipe Albrecht on 01.08.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <boost/foreach.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "../extras/utils.hpp"

#include "controlled_vocabulary.hpp"
#include "config.hpp"
#include "collections.hpp"
#include "helpers.hpp"
#include "full_text.hpp"

#include "../errors.hpp"
#include "../log.hpp"

namespace epidb {
  namespace dba {
    namespace cv {

      bool __get_synonyms_from_bio_source(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                                          const std::string &user_key,
                                          std::vector<utils::IdName> &syns, std::string &msg)
      {
        utils::IdName id_name_bio_source;
        if (!helpers::get_name(Collections::BIO_SOURCES(), norm_bio_source_name, id_name_bio_source, msg)) {
          return false;
        }

        syns.push_back(id_name_bio_source);

        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder syn_query_builder;
        syn_query_builder.append("norm_name", norm_bio_source_name);
        mongo::BSONObj query_obj = syn_query_builder.obj();
        mongo::Query query = mongo::Query(query_obj);
        std::auto_ptr<mongo::DBClientCursor> syns_cursor = c->query(helpers::collection_name(Collections::BIO_SOURCE_SYNONYMS()), query);

        if (!syns_cursor->more()) {
          c.done();
          return true;
        }

        mongo::BSONObj syn_bson = syns_cursor->next();
        std::vector<mongo::BSONElement> e = syn_bson["synonyms"].Array();

        BOOST_FOREACH(mongo::BSONElement be, e) {
          std::string norm_synonym = be.str();
          mongo::BSONObjBuilder syn_query;
          syn_query.append("norm_synonym", norm_synonym);
          mongo::Query query = mongo::Query(syn_query.obj());

          std::auto_ptr<mongo::DBClientCursor> syns_names_cursor = c->query(helpers::collection_name(Collections::BIO_SOURCE_SYNONYM_NAMES()), query);

          if (!syns_names_cursor->more()) {
            msg = "It was not possible to find the name of " + norm_synonym + " .";
            c.done();
            return false;
          }
          mongo::BSONObj e_syn = syns_names_cursor->next();
          mongo::BSONElement syn = e_syn["synonym"];
          std::string syn_name = syn.str();
          utils::IdName id_syn_name(id_name_bio_source.id , syn_name);
          syns.push_back(id_syn_name);
        }

        c.done();
        return true;
      }

      bool __get_synonyms_from_synonym(const std::string &synonym, const std::string &norm_synonym,
                                       const std::string &user_key,
                                       std::vector<utils::IdName> &syns, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder syn_query_builder;
        syn_query_builder.append("norm_synonym", norm_synonym);

        mongo::BSONObj query_obj = syn_query_builder.obj();
        mongo::Query query = mongo::Query(query_obj).sort("synonym");
        std::auto_ptr<mongo::DBClientCursor> syns_cursor = c->query(helpers::collection_name(Collections::BIO_SOURCE_SYNONYM_NAMES()), query);

        if (!syns_cursor->more()) {
          msg = "It was not possible to find the bio_sources synonyms for " + synonym + " .";
          c.done();
          return false;
        }

        mongo::BSONObj syn_bson = syns_cursor->next();
        std::string bio_source_name = syn_bson["bio_source_name"].str();
        std::string norm_bio_source_name = syn_bson["norm_bio_source_name"].str();

        c.done();

        if (!__get_synonyms_from_bio_source(bio_source_name, norm_bio_source_name, user_key, syns, msg)) {
          return false;
        }

        return true;
      }

      bool __full_text_relation(const std::string &term, const std::string &norm_term,
                                const std::string &bio_source, const std::string &norm_bio_source,
                                std::string &msg)
      {
        std::vector<utils::IdName> syns;
        if (!__get_synonyms_from_bio_source(term, norm_term, "", syns, msg)) {
          return false;
        }

        std::vector<std::string> terms;
        BOOST_FOREACH(utils::IdName syn, syns) {
          terms.push_back(syn.name);
          terms.push_back(utils::normalize_name(syn.name));
        }

        std::vector<std::string> norm_subs;
        if (!get_bio_source_embracing(bio_source, norm_bio_source,
                                      true, "", norm_subs, msg)) {
          return false;
        }

        BOOST_FOREACH(std::string norm_sub, norm_subs) {
          std::string bio_source_id;
          if (!helpers::get_bio_source_id(norm_sub, bio_source_id, msg)) {
            return false;
          }

          BOOST_FOREACH(std::string term, terms) {
            if (norm_sub != utils::normalize_name(term)) {
              if (!search::insert_related_term(bio_source_id, term, msg)) {
                return false;
              }
            }
          }
        }
        return true;
      }

      bool get_synonym_root(const std::string &synonym, const std::string &norm_synonym,
                              std::string &bio_source_name, std::string &norm_bio_source_name, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder syn_query_builder;
        syn_query_builder.append("norm_synonym", norm_synonym);

        mongo::BSONObj query_obj = syn_query_builder.obj();
        mongo::Query query = mongo::Query(query_obj).sort("synonym");
        std::auto_ptr<mongo::DBClientCursor> syns_cursor = c->query(helpers::collection_name(Collections::BIO_SOURCE_SYNONYM_NAMES()), query);

        if (!syns_cursor->more()) {
          msg = "It was not possible to find the bio_source root for " + synonym + " .";
          c.done();
          return false;
        }

        mongo::BSONObj syn_bson = syns_cursor->next();
        bio_source_name = syn_bson["bio_source_name"].str();
        norm_bio_source_name = syn_bson["norm_bio_source_name"].str();

        c.done();

        return true;
      }

      bool set_bio_source_synonym(const std::string &input_bio_source_name, const std::string &synonym,
                                  bool is_bio_source, const bool is_syn, const std::string &user_key,
                                  std::string &msg)
      {
        std::string bio_source_name;
        std::string norm_bio_source_name;

        if (is_syn) {
          std::string norm_input_bio_source_name = utils::normalize_name(input_bio_source_name);
          if (!get_synonym_root(input_bio_source_name, norm_input_bio_source_name,
                                  bio_source_name, norm_bio_source_name, msg)) {
            return false;
          }
        } else {
          bio_source_name = input_bio_source_name;
          norm_bio_source_name = utils::normalize_name(input_bio_source_name);
        }

        std::string norm_synonym = utils::normalize_name(synonym);
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder index_name;
        index_name.append("name", 1);
        c->ensureIndex(helpers::collection_name(Collections::BIO_SOURCE_SYNONYMS()), index_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder index_norm_name;
        index_norm_name.append("norm_name", 1);
        c->ensureIndex(helpers::collection_name(Collections::BIO_SOURCE_SYNONYMS()), index_norm_name.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder index_syn;
        index_syn.append("synonym", 1);
        c->ensureIndex(helpers::collection_name(Collections::BIO_SOURCE_SYNONYMS()), index_syn.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder query_builder;
        query_builder.append("norm_name", norm_bio_source_name);
        mongo::BSONObj query = query_builder.obj();

        mongo::BSONObj value = BSON("synonyms" << norm_synonym);

        mongo::BSONObj append_value;
        if (is_bio_source) {
          mongo::BSONObjBuilder syn_insert_builder;
          std::string id = norm_bio_source_name + "_" + norm_synonym;
          syn_insert_builder.append("name", input_bio_source_name);
          syn_insert_builder.append("norm_name", norm_bio_source_name);
          append_value = BSON("$set" << syn_insert_builder.obj() << "$addToSet" << value);
        } else {
          append_value = BSON("$addToSet" << value);
        }

        c->update(helpers::collection_name(Collections::BIO_SOURCE_SYNONYMS()), query, append_value, true, false);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder index_syn_names;
        index_syn_names.append("norm_synonym", 1);
        c->ensureIndex(helpers::collection_name(Collections::BIO_SOURCE_SYNONYM_NAMES()), index_syn_names.obj(), true);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder syn_builder;
        syn_builder.append("synonym", synonym);
        syn_builder.append("norm_synonym", norm_synonym);
        syn_builder.append("bio_source_name", bio_source_name);
        syn_builder.append("norm_bio_source_name", norm_bio_source_name);
        syn_builder.append("user_key", user_key);
        syn_builder.append("public", true);
        c->insert(helpers::collection_name(Collections::BIO_SOURCE_SYNONYM_NAMES()), syn_builder.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();

        if (!__full_text_relation(bio_source_name, norm_bio_source_name,
                                  bio_source_name, norm_bio_source_name,
                                  msg)) {
          return false;
        }

        return true;
      }


      bool get_bio_source_synonyms(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                                   bool is_bio_source, const std::string &user_key,
                                   std::vector<utils::IdName> &syns, std::string &msg)
      {
        if (is_bio_source) {
          return __get_synonyms_from_bio_source(bio_source_name, norm_bio_source_name, user_key, syns, msg);
        } else {
          return __get_synonyms_from_synonym(bio_source_name, norm_bio_source_name, user_key, syns, msg);
        }
      }

      bool __is_connected(const std::string &norm_s1, const std::string &norm_s2,
                          bool &r, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder syn_query_builder;
        syn_query_builder.append("norm_bio_source_name", norm_s1);

        mongo::BSONObj query_obj = syn_query_builder.obj();
        mongo::Query query = mongo::Query(query_obj);
        std::auto_ptr<mongo::DBClientCursor> syns_cursor = c->query(helpers::collection_name(Collections::BIO_SOURCE_EMBRACING()), query);

        if (!syns_cursor->more()) {
          c.done();
          r = false;
          return true;
        }

        mongo::BSONObj syn_bson = syns_cursor->next();
        std::vector<mongo::BSONElement> e = syn_bson["subs"].Array();

        c.done();

        std::list<std::string> subs;

        BOOST_FOREACH(mongo::BSONElement be, e) {
          std::string sub = be.str();
          if (sub.compare(norm_s2) == 0) {
            r = true;
            return true;
          }
          subs.push_back(sub);
        }

        BOOST_FOREACH(std::string norm_sub, subs) {
          bool rr;
          if (!__is_connected(norm_sub, norm_s2, rr, msg)) {
            return false;
          }
          if (rr) {
            r = true;
            return true;
          }
        }

        r = false;
        return true;
      }

      bool set_bio_source_embracing(const std::string &bio_source_more_embracing, const std::string &norm_bio_source_more_embracing,
                                    const std::string &bio_source_less_embracing, const std::string &norm_bio_source_less_embracing,
                                    bool more_embracing_is_syn, const bool less_embracing_is_syn,
                                    const std::string &user_key, std::string &msg)
      {
        std::string more_embracing_root;
        std::string norm_more_embracing_root;
        std::string less_embracing_root;
        std::string norm_less_embracing_root;

        if (more_embracing_is_syn) {
          if (!get_synonym_root(bio_source_more_embracing, norm_bio_source_more_embracing,
                                  more_embracing_root, norm_more_embracing_root, msg)) {
            return false;
          }
        } else {
          more_embracing_root = bio_source_more_embracing;
          norm_more_embracing_root = norm_bio_source_more_embracing;
        }

        if (less_embracing_is_syn) {
          if (!get_synonym_root(bio_source_less_embracing, norm_bio_source_less_embracing,
                                  less_embracing_root, norm_less_embracing_root, msg)) {
            return false;
          }
        } else {
          less_embracing_root = bio_source_less_embracing;
          norm_less_embracing_root = norm_bio_source_less_embracing;
        }

        if (norm_more_embracing_root.compare(norm_less_embracing_root) == 0) {
          msg = bio_source_more_embracing + " and " + bio_source_less_embracing + " are synonyms.";
          return false;
        }

        bool is_connected(false);
        if (!__is_connected(norm_more_embracing_root, norm_less_embracing_root, is_connected, msg)) {
          return false;
        }

        if (is_connected) {
          std::string e = Error::m(ERR_MORE_EMBRACING_BIO_SOURCE_NAME,
                                   bio_source_more_embracing.c_str(), bio_source_less_embracing.c_str());
          EPIDB_LOG_TRACE(e);
          msg = e;
          return false;
        }

        if (!__is_connected(norm_less_embracing_root, norm_more_embracing_root, is_connected, msg)) {
          return false;
        }

        if (is_connected) {
          std::string e = Error::m(ERR_MORE_EMBRACING_BIO_SOURCE_NAME,
                                   bio_source_less_embracing.c_str(), bio_source_more_embracing.c_str());
          EPIDB_LOG_TRACE(e);
          msg = e;
          return false;
        }

        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder query_builder;
        query_builder.append("norm_bio_source_name", norm_bio_source_more_embracing);
        mongo::BSONObj query = query_builder.obj();

        mongo::BSONObj value = BSON("subs" << norm_bio_source_less_embracing);
        mongo::BSONObj append_value = BSON("$addToSet" << value);

        c->update(helpers::collection_name(Collections::BIO_SOURCE_EMBRACING()), query, append_value, true, false);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        mongo::BSONObjBuilder index_name;
        index_name.append("norm_bio_source_name", 1);
        c->ensureIndex(helpers::collection_name(Collections::BIO_SOURCE_EMBRACING()), index_name.obj());
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

        return true;
      }

      bool __get_down_connected(const std::string &norm_s1,
                                std::vector<std::string> &norm_names, std::string &msg)
      {
        norm_names.push_back(norm_s1);

        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder query_builder;
        query_builder.append("norm_bio_source_name", norm_s1);

        mongo::BSONObj query_obj = query_builder.obj();
        mongo::Query query = mongo::Query(query_obj);
        std::auto_ptr<mongo::DBClientCursor> syns_cursor = c->query(helpers::collection_name(Collections::BIO_SOURCE_EMBRACING()), query);

        if (!syns_cursor->more()) {
          c.done();
          return true;
        }

        mongo::BSONObj syn_bson = syns_cursor->next().getOwned();
        std::vector<mongo::BSONElement> e = syn_bson["subs"].Array();

        c.done();

        std::list<std::string> subs;
        BOOST_FOREACH(mongo::BSONElement be, e) {
          std::string sub = be.str();
          if (!__get_down_connected(sub, norm_names, msg)) {
            return false;
          }
        }

        return true;
      }

      bool __get_upper_connected(const std::string &norm_s1,
                                 std::vector<std::string> &norm_uppers, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder query_builder;
        query_builder.append("subs", BSON("$eq" << norm_s1));
        mongo::BSONObj query_obj = query_builder.obj();
        mongo::Query query = mongo::Query(query_obj);
        std::auto_ptr<mongo::DBClientCursor> syns_cursor = c->query(helpers::collection_name(Collections::BIO_SOURCE_EMBRACING()), query);

        if (!syns_cursor->more()) {
          c.done();
          return true;
        }

        mongo::BSONObj syn_bson = syns_cursor->next().getOwned();
        std::vector<mongo::BSONElement> e = syn_bson["subs"].Array();

        c.done();

        std::list<std::string> subs;
        BOOST_FOREACH(mongo::BSONElement be, e) {
          std::string sub = be.str();
          if (!__get_upper_connected(sub, norm_uppers, msg)) {
            return false;
          }
        }

        return true;
      }

      bool get_bio_source_embracing(const std::string &bio_source_name, const std::string &norm_bio_source_name,
                                    bool is_bio_source, const std::string &user_key,
                                    std::vector<std::string> &norm_subs, std::string &msg)
      {
        std::string more_embracing_root;
        std::string norm_more_embracing_root;

        if (!is_bio_source) {
          if (!get_synonym_root(bio_source_name, norm_bio_source_name,
                                  more_embracing_root, norm_more_embracing_root, msg)) {
            return false;
          }
        } else {
          more_embracing_root = bio_source_name;
          norm_more_embracing_root = norm_bio_source_name;
        }

        if (!__get_down_connected(norm_more_embracing_root, norm_subs, msg)) {
          return false;
        }
        return true;
      }
    }
  }
}
