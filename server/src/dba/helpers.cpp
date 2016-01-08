//
//  helpers.cpp
//  epidb
//
//  Created by Felipe Albrecht on 08.07.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include <sstream>
#include <set>

#include <boost/thread/mutex.hpp>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "../extras/serialize.hpp"
#include "../extras/utils.hpp"

#include "collections.hpp"
#include "config.hpp"
#include "dba.hpp"
#include "helpers.hpp"

namespace epidb {
  namespace dba {
    namespace helpers {

      const std::string collection_name(const std::string &name)
      {
        return config::DATABASE_NAME() + "." + name;
      }

      const std::string region_collection_name(const std::string &genome, const std::string &chromosome)
      {
        std::stringstream ss;
        ss << collection_name(Collections::REGIONS()) << "." << utils::normalize_name(genome) << "." << chromosome;
        return ss.str();
      }

      bool get(const std::string &where,
               std::vector<std::string> &result, std::string &msg)
      {
        return get(where, "", result, msg);
      }

      // Get content where the field content match with the expected_content parameter.
      bool get(const std::string &where, const std::string &field, const std::string &expected_content,
               std::vector<mongo::BSONObj> &result, std::string &msg)
      {
        std::vector<QueryPair> data;
        QueryPair p(field, expected_content);
        data.push_back(p);
        return get(where, data, result, msg);
      }

      bool get(const std::string &where, const std::string &returned_field,
               std::vector<std::string> &result, std::string &msg)
      {
        std::vector<std::string> fields;
        if (returned_field.length() > 0) {
          fields.push_back(returned_field);
        }

        std::vector<mongo::BSONObj> objs;
        if (!get(where, mongo::BSONObj(), fields, objs, msg)) {
          return false;
        }

        for(const mongo::BSONObj & o: objs) {
          result.push_back(o[returned_field].String());
        }

        return true;
      }

      // Get content where the field content match with the all conditions
      bool get(const std::string &where, const std::vector<QueryPair> &conditions,
               std::vector<mongo::BSONObj> &results, std::string &msg)
      {
        mongo::BSONObjBuilder query_builder;
        for(const QueryPair & p: conditions) {
          query_builder.append(p.first, p.second);
        }
        mongo::BSONObj query = query_builder.obj();
        return get(where, query, results, msg);
      }

      // Get content where the field content match with the query object
      bool get(const std::string &where, const mongo::BSONObj &query,
               std::vector<mongo::BSONObj> &results, std::string &msg)
      {
        std::vector<std::string> v;
        return get(where, mongo::Query(query), v, results, msg);
      }

      // Get content where the field content match with the query object
      bool get(const std::string &where, const mongo::Query &query,
               std::vector<mongo::BSONObj> &results, std::string &msg)
      {
        std::vector<std::string> v;
        return get(where, query, v, results, msg);
      }

      // Get Id and Name from the given collection documents and a filter BSon
      bool get(const std::string &where, mongo::BSONObj filter,
               std::vector<utils::IdName> &results, std::string &msg)
      {
        std::vector<std::string> v;
        v.push_back("_id");
        v.push_back("name");

        std::vector<mongo::BSONObj> r;
        if (!get(where, mongo::Query(filter), v, r, msg)) {
          return false;
        }

        results = utils::bsons_to_id_names(r);

        return true;
      }

      // Get Id and Name from the given collection documents.
      bool get(const std::string &where, std::vector<utils::IdName> &results, std::string &msg)
      {
        std::vector<std::string> v;
        v.push_back("_id");
        v.push_back("name");

        std::vector<mongo::BSONObj> r;
        if (!get(where, mongo::Query(), v, r, msg)) {
          return false;
        }

        results = utils::bsons_to_id_names(r);

        return true;
      }

      // Get content where the field content match with the query object and the fields.
      // Return all elements if fields is empty.
      bool get(const std::string &where, const mongo::Query &query, const std::vector<std::string> &fields,
               std::vector<mongo::BSONObj> &results, std::string &msg)
      {
        Connection c;

        mongo::BSONObjBuilder b;
        for(const std::string & f: fields) {
          b.append(f, 1);
        }
        mongo::BSONObj projection = b.obj();
        const std::string collection = collection_name(where);
        auto data_cursor = c->query(collection, query, 0, 0, &projection);
        while (data_cursor->more()) {
          mongo::BSONObj obj = data_cursor->next().getOwned();
          results.push_back(obj);
        }

        c.done();
        return true;
      }

      // Return true and one element that match the given query.
      // Return false if does not find any element.
      bool get_one(const std::string &where, const mongo::BSONObj &query,
                   mongo::BSONObj &result)
      {
        return get_one(where, mongo::Query(query), result);
      }

      // Return true and one element that match the given query.
      // Return false if does not find any element.
      bool get_one(const std::string &where, const mongo::Query &query,
                   mongo::BSONObj &result)
      {
        Connection c;
        const std::string collection = collection_name(where);
        auto data_cursor = c->query(collection, query);
        if (data_cursor->more()) {
          result = data_cursor->next().getOwned();
          c.done();
          return true;
        } else {
          c.done();
          return false;
        }
      }

      bool get_name(const std::string &where, const std::string &norm_name,
                    utils::IdName &id_name, std::string &msg)
      {
        std::string field;
        std::vector<mongo::BSONObj> results;

        if (where.compare("users") == 0) {
          field = "key";
        } else {
          field = "norm_name";
        }
        if (!get(where, field, norm_name, results, msg)) {
          return false;
        }

        if (results.size() == 0) {
          msg = "Unable to retrieve the name of the internal name '" + norm_name + "'.";
          return false;
        }

        id_name = utils::bson_to_id_name(results[0]);

        return true;
      }

      bool get_id(const std::string &where, const std::string &norm_name,
                  std::string &id, std::string &msg)
      {
        std::string field;
        std::vector<mongo::BSONObj> results;

        if (where.compare("users") == 0) {
          field = "key";
        } else {
          field = "norm_name";
        }
        if (!get(where, field, norm_name, results, msg)) {
          return false;
        }

        if (results.size() == 0) {
          msg = "Unable to retrieve the id of the '" + norm_name + "''. Where: " + where;
          return false;
        }

        id = results[0]["_id"].str();

        return true;
      }

      bool check_exist(const std::string &where, const std::string &field, const std::string &content)
      {
        Connection c;
        const std::string collection = collection_name(where);

        unsigned long long count = c->count(collection, BSON(field << content));
        c.done();

        return count > 0;
      }

      bool check_exist(const std::string &where, const std::string &field, const bool content)
      {
        Connection c;
        const std::string collection = collection_name(where);

        unsigned long long count = c->count(collection, BSON(field << content));
        c.done();

        return count > 0;
      }

      bool check_exist(const std::string &where, const mongo::BSONObj& query)
      {
        Connection c;
        const std::string collection = collection_name(where);

        unsigned long long count = c->count(collection, query);
        c.done();

        return count > 0;
      }

      bool remove_one(const std::string &collection, const std::string &id, std::string &msg, const std::string &field)
      {
        Connection c;
        c->remove(collection, BSON(field << id), 1);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();
        return true;
      }

      bool remove_all(const std::string &collection, const mongo::Query &query, std::string &msg)
      {
        Connection c;
        c->remove(collection, query);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();
        return true;
      }

      bool collection_size(const std::string &where, unsigned long long &size, std::string &msg)
      {
        Connection c;
        const std::string collection = collection_name(where);
        size = c->count(collection);
        c.done();
        return true;
      }

      bool remove_collection(const std::string &collection, std::string &msg)
      {
        Connection c;
        mongo::BSONObj info;
        c->dropCollection(collection, &info);

        if (!info.getField("ok").trueValue()) {
          msg = info.getStringField("errmsg");
          c.done();
          return false;
        }

        c.done();
        return true;
      }

      boost::mutex counter_mutex;
      bool get_increment_counter(const std::string &name, int &id, std::string &msg)
      {
        boost::mutex::scoped_lock lock(counter_mutex);
        Connection c;

        if (!check_exist(Collections::COUNTERS(), "_id", name)) {
          mongo::BSONObjBuilder b;
          b.append("_id", name);
          b.append("seq", 0);
          c->insert(helpers::collection_name(Collections::COUNTERS()), b.obj());
          if (!c->getLastError().empty()) {
            msg = c->getLastError();
            c.done();
            return false;
          }
        }

        mongo::BSONObj fnm_obj =
          BSON("findandmodify" << Collections::COUNTERS() <<
               "query" << BSON("_id" << name) <<
               "update" << BSON("$inc" << BSON("seq" << 1)) <<
               "new" << true);

        mongo::BSONObj info;
        bool result = c->runCommand(config::DATABASE_NAME(), fnm_obj, info);
        if (!result) {
          // TODO: get info error
          msg = "error in generate the counter '" + name + "'.";
          c.done();
          return  false;
        }
        mongo::BSONElement f = info["value"]["seq"];
        // We use number() not Int() because the seq may be "automatically" converted to double when we copy the collections using the mongodb console
        id = f.number();
        c.done();
        return result;
      }

      bool get_counter(const std::string &name, int &count, std::string &msg)
      {
        mongo::BSONObjBuilder data_builder;
        data_builder.append("_id", name);

        if (!check_exist(Collections::COUNTERS(), "_id", name)) {
          return get_increment_counter(name, count, msg);
        }

        mongo::BSONObj data = data_builder.obj();

        Connection c;
        auto cursor = c->query(helpers::collection_name(Collections::COUNTERS()), data);
        if (cursor->more()) {
          mongo::BSONObj count_cursor = cursor->next();
          count = count_cursor["seq"].Int();
          c.done();
          return true;
        } else {
          msg += "error reading the counter '" + name + "'.";
        }
        c.done();
        return false;
      }

      bool notify_change_occurred(const std::string &name, std::string &msg)
      {
        int tmp;
        return get_increment_counter(name + "_operations", tmp, msg);
      }
    }
  }
}
