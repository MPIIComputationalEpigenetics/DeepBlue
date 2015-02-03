//
//  list.cpp
//  epidb
//
//  Created by Felipe Albrecht on 07.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <vector>

#include <boost/foreach.hpp>

#include <mongo/bson/bson.h>

#include "collections.hpp"
#include "column_types.hpp"
#include "config.hpp"
#include "dba.hpp"
#include "helpers.hpp"
#include "list.hpp"

#include "../algorithms/levenshtein.hpp"

#include "../connection/connection.hpp"

#include "../datatypes/metadata.hpp"

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace list {

      bool users(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::USERS(), result, msg);
      }

      bool genomes(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::GENOMES(), result, msg);
      }

      bool biosources(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::BIOSOURCES(), result, msg);
      }

      bool techniques(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::TECHNIQUES(), result, msg);
      }

      bool samples(const std::string &user_key, const mongo::BSONArray &biosources, const datatypes::Metadata &metadata,
                   std::vector<std::string> &result, std::string &msg)
      {
        mongo::BSONObjBuilder query_builder;

        if (!biosources.isEmpty()) {
          query_builder.append("norm_biosource_name", BSON("$in" << biosources));
        }

        datatypes::Metadata::const_iterator it;
        for (it = metadata.begin(); it != metadata.end(); ++it) {
          query_builder.append(it->first, it->second);
        }

        std::vector<mongo::BSONObj> samples;
        std::vector<std::string> fields;
        mongo::BSONObj query = query_builder.obj();
        if (!helpers::get(Collections::SAMPLES(), query, fields, samples, msg)) {
          return false;
        }

        BOOST_FOREACH(const mongo::BSONObj & o, samples) {
          result.push_back(o["_id"].String());
        }

        return true;
      }

      bool projects(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::PROJECTS(), result, msg);
      }

      bool epigenetic_marks(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::EPIGENETIC_MARKS(), result, msg);
      }

      bool experiments(const mongo::Query query, std::vector<utils::IdName> &result, std::string &msg)
      {

        /*
                fields.push_back("_id");
        fields.push_back("name");
        fields.push_back("genome");
        fields.push_back("epigenetic_mark");
        fields.push_back("sample_id");
        fields.push_back("technique");
        fields.push_back("description");
        */
        std::vector<std::string> fields;
        fields.push_back("_id");
        fields.push_back("name");

        std::vector<mongo::BSONObj> objects;
        if (!helpers::get(Collections::EXPERIMENTS(), query, fields, objects, msg)) {
          return false;
        }

        BOOST_FOREACH(const mongo::BSONObj & o, objects) {
          utils::IdName id_name(o["_id"].String(), o["name"].String());
          result.push_back(id_name);
        }

        return true;
      }

      bool annotations(const std::string &genome, const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        std::vector<mongo::BSONObj> objects;
        if (!helpers::get(Collections::ANNOTATIONS(), "genome", genome, objects, msg)) {
          return false;
        }

        BOOST_FOREACH(const mongo::BSONObj & o, objects) {
          utils::IdName names(o["_id"].String(), o["name"].String());
          result.push_back(names);
        }

        return true;
      }

      bool column_types(const std::string &user_key, std::vector<utils::IdName> &content, std::string  &msg)
      {
        return dba::columns::list_column_types(user_key, content, msg);
      }

      //-----

      bool column_types(const std::string &user_key, std::vector<std::string> &content, std::string  &msg)
      {
        Connection c;
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::COLUMN_TYPES()), mongo::BSONObj());

        while (data_cursor->more()) {
          mongo::BSONObj o = data_cursor->next().getOwned();
          columns::ColumnTypePtr column_type;
          if (! epidb::dba::columns::column_type_bsonobj_to_class(o, column_type, msg))  {
            return false;
          }
          content.push_back(column_type->str());
        }

        c.done();
        return true;
      }

      bool similar_biosources(const std::string name, const std::string &user_key,
                              std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::BIOSOURCES(), name, user_key, result, msg);
      }

      bool similar_techniques(const std::string name, const std::string &user_key,
                              std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::TECHNIQUES(), name, user_key, result, msg);
      }

      bool similar_projects(const std::string name, const std::string &user_key,
                            std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::PROJECTS(), name, user_key, result, msg);
      }

      bool similar_epigenetic_marks(const std::string name, const std::string &user_key,
                                    std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::EPIGENETIC_MARKS(), name, user_key, result, msg);
      }

      bool similar_genomes(const std::string name, const std::string &user_key,
                           std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::GENOMES(), name, user_key, result, msg);
      }

      bool similar_experiments(const std::string name, const std::string &genome, const std::string &user_key,
                               std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::EXPERIMENTS(), "name", name, "genome", genome, user_key, result, msg);
      }

      bool similar(const std::string &where, const std::string &what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        std::vector<utils::IdName> id_names;
        if (!helpers::get(where, id_names, msg)) {
          return false;
        }

        std::vector<std::string> names;
        std::map<std::string, std::string> id_name_map;
        BOOST_FOREACH(const utils::IdName & id_name, id_names) {
          id_name_map[id_name.name] = id_name.id;
          names.push_back(id_name.name);
        }
        std::vector<std::string> ordered = epidb::algorithms::Levenshtein::order_by_score(what, names);

        BOOST_FOREACH(const std::string & name, ordered) {
          utils::IdName id_name(id_name_map[name], name);
          result.push_back(id_name);
        }

        return true;
      }

      bool similar(const std::string &where, const std::string &field, const std::string &what,
                   const std::string &filter_field, const std::string &filter_what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        std::vector<mongo::BSONObj> docs;

        if (!helpers::get(where, filter_field, filter_what, docs, msg)) {
          return false;
        }

        std::vector<std::string> names;
        std::map<std::string, std::string> id_name_map;
        for (std::vector<mongo::BSONObj>::iterator it = docs.begin(); it != docs.end(); ++it) {
          std::string field_name(it->getField(field).str());
          names.push_back(field_name);
          id_name_map[field_name] = it->getField("_id").String();
        }

        std::vector<std::string> ordered = epidb::algorithms::Levenshtein::order_by_score(what, names);

        BOOST_FOREACH(const std::string & name, ordered) {
          utils::IdName id_name(id_name_map[name], name);
          result.push_back(id_name);
        }

        return true;
      }

      bool in_use(const std::string &collection, const std::string &key_name, const std::string &user_key,
                  std::vector<utils::IdNameCount> &names, std::string &msg)
      {
        mongo::BSONArray pipeline = BSON_ARRAY(
                                      BSON( "$group" <<
                                            BSON( "_id" << key_name << "total" << BSON( "$sum" << 1 ) )
                                          )
                                    );

        mongo::BSONObj agg_command = BSON( "aggregate" << Collections::EXPERIMENTS() << "pipeline" << pipeline);

        Connection c;

        mongo::BSONObj res;
        c->runCommand(config::DATABASE_NAME(), agg_command, res);

        if (!res.getField("ok").trueValue()) {
          msg = res.getStringField("errmsg");
          c.done();
          return false;
        }

        std::vector<mongo::BSONElement> result = res["result"].Array();


        BOOST_FOREACH(const mongo::BSONElement & be, result) {
          std::string norm_name = be["_id"].String();
          long count = be["total"].safeNumberLong();

          utils::IdNameCount inc;
          if (collection == Collections::SAMPLES()) {
            inc = utils::IdNameCount(norm_name, "", count);
          } else {
            utils::IdName id_name;
            if (!helpers::get_name(collection, norm_name, id_name, msg)) {
              c.done();
              return false;
            }
            inc = utils::IdNameCount(id_name.id, id_name.name, count);
          }
          names.push_back(inc);
        }

        c.done();
        return true;
      }
    }
  }
}
