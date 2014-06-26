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
#include <boost/lexical_cast.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

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
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        // if the collection is dropped MongoDB doesn't realize the index is deleted
        // and will not recreate it even though it's missing. So we have to reset the
        // index cache manually.
        c->resetIndexCache();
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c->ensureIndex(helpers::collection_name(Collections::TEXT_SEARCH()),
                       mongo::fromjson("{\"epidb_id\": 1}"));
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c->ensureIndex(helpers::collection_name(Collections::TEXT_SEARCH()),
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
          if (std::string(e.fieldName()) == "_id") {
            continue;
          }
          if (e.isSimpleType()) {
            create_text_search_builder.append(e.fieldName(), e.str());
            continue;
          }


          if (std::string(e.fieldName()) == "extra_metadata") {
            for (mongo::BSONObj::iterator itt = e.Obj().begin(); itt.more(); ) {
              mongo::BSONElement em = itt.next();
              if (em.isSimpleType()) {
                create_text_search_builder.append(std::string("extra_metadata_") + em.fieldName(), em.str());
              }
            }
          }
        }

        if (type == "experiments" || type == "samples") {
          std::string norm_bio_source_name = data["norm_bio_source_name"].str();
          std::auto_ptr<mongo::DBClientCursor> cursor = c->query(helpers::collection_name(Collections::TEXT_SEARCH()),
              mongo::fromjson("{\"norm_name\": \"" + norm_bio_source_name + "\", \"type\": \"bio_sources\"}"));

          if (!cursor->more()) {
            std::string s = Error::m(ERR_DATABASE_INVALID_BIO_SOURCE, norm_bio_source_name.c_str());
            EPIDB_LOG_TRACE(s);
            msg = s;
            return false;
          }

          mongo::BSONObj bio_source = cursor->next().getOwned();
          if (bio_source.hasField("related_terms")) {
            create_text_search_builder.append(bio_source["related_terms"]);
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

      bool insert_related_term(const std::string &id, const std::string &name, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder query_builder;
        query_builder.append("epidb_id", id);
        mongo::BSONObj query = query_builder.obj();

        mongo::BSONObj value = BSON("related_terms" << name);
        mongo::BSONObj append_value = BSON("$addToSet" << value);

        // Update the bio_source term
        c->update(helpers::collection_name(Collections::TEXT_SEARCH()), query, append_value, true, false);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        // Find the bio source to update the experiments and samples that use it
        std::auto_ptr<mongo::DBClientCursor> cursor =
          c->query(helpers::collection_name(Collections::TEXT_SEARCH()), query);

        if (!cursor->more()) {
          msg = "Unable to find bio source " + id;
          c.done();
          return false;
        }

        mongo::BSONObj o = cursor->next();
        if (o["type"].str() != "bio_sources") {
          msg = "Data id " + id + " is not a bio source";
          c.done();
          return false;
        }

        std::string norm_bio_source_name = o["norm_name"].str();

        mongo::BSONObjBuilder update_related_query_builder;
        update_related_query_builder.append("norm_bio_source_name", norm_bio_source_name);
        mongo::BSONObj update_related_query = update_related_query_builder.obj();

        c->update(helpers::collection_name(Collections::TEXT_SEARCH()), update_related_query, append_value, false, true);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        c.done();
        return true;
      }


      static bool __sort_search_full_text_result(TextSearchResult i, TextSearchResult j)
      {
        return i.score > j.score;
      }

      bool search_full_text(const std::string &text, std::vector<TextSearchResult> &results,
                            std::string &msg)
      {
        std::vector<std::string> e;
        return search_full_text(text, e, results, msg);
      }

      bool search_full_text(const std::string &text, const std::vector<std::string> &types,
                            std::vector<TextSearchResult> &results, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());

        mongo::BSONObjBuilder cmd_builder;
        cmd_builder.append("text", Collections::TEXT_SEARCH());
        cmd_builder.append("search", text);
        if (types.size() > 0) {
          cmd_builder.append("filter", BSON("type" << helpers::build_condition_array(types, "$in")));
        }

        mongo::BSONObj result;
        mongo::BSONObj o = cmd_builder.obj();
        c->runCommand(config::DATABASE_NAME(), o, result);

        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        if (!result.getField("ok").trueValue()) {
          msg = result.getStringField("errmsg");
          c.done();
          return false;
        }

        TextSearchResult res;
        std::vector<mongo::BSONElement> result_entries = result.getField("results").Array();
        std::vector<mongo::BSONElement>::iterator it;
        for (it = result_entries.begin(); it != result_entries.end(); ++it) {
          res.id = it->Obj().getFieldDotted("obj.epidb_id").str();
          res.name = it->Obj().getFieldDotted("obj.name").str();
          res.type = it->Obj().getFieldDotted("obj.type").str();
          res.score = it->Obj().getField("score").number();
          results.push_back(res);
        }
        c.done();

        std::sort(results.begin(), results.end(), __sort_search_full_text_result);

        return true;
      }
    }
  }
}