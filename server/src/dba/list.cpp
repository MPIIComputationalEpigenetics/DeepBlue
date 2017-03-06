//
//  list.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 07.04.14.
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

#include <future>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <mongo/bson/bson.h>

#include "collections.hpp"
#include "column_types.hpp"
#include "config.hpp"
#include "dba.hpp"
#include "helpers.hpp"
#include "list.hpp"

#include "../algorithms/levenshtein.hpp"

#include "../connection/connection.hpp"

#include "../dba/genes.hpp"
#include "../dba/users.hpp"
#include "../dba/queries.hpp"

#include "../datatypes/metadata.hpp"
#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"

#include "../errors.hpp"

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

      bool biosources(const datatypes::Metadata &metadata,
                      std::vector<utils::IdName> &result, std::string &msg)
      {
        mongo::BSONObjBuilder query_builder;
        datatypes::Metadata::const_iterator it;
        for (const auto& p : metadata) {
          query_builder.append("extra_metadata." + p.first, p.second);
        }
        return helpers::get(Collections::BIOSOURCES(), query_builder.obj(), result, msg);
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

        for (const mongo::BSONObj & o : samples) {
          result.push_back(o["_id"].String());
        }

        return true;
      }

      /*
      * \brief List all projects that MUST NOT be available to the user
      */
      bool private_projects(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        datatypes::User user;
        if (!dba::users::get_user_by_key(user_key, user, msg)) {
          return false;
        }

        if (user.is_admin()) {
          result.clear();
          return true;
        }

        mongo::BSONObj private_projects = BSON("public" << false);
        mongo::BSONObj full_query;

        std::vector<std::string> user_projects = user.get_projects();

        if (!user_projects.empty()) {
          mongo::BSONObj user_projects_bson = BSON("_id" << BSON("$nin" << utils::build_array(user_projects)));
          full_query = BSON("$and" << BSON_ARRAY(private_projects << user_projects_bson));
        } else {
          full_query = private_projects;
        }

        std::vector<mongo::BSONObj> projects;
        if (!helpers::get(Collections::PROJECTS(), full_query, projects, msg)) {
          return false;
        }

        result = utils::bsons_to_id_names(projects);

        return true;
      }

      /*
      * \brief List all projects that are available to the user
      */
      bool projects(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        datatypes::User user;
        if (!dba::users::get_user_by_key(user_key, user, msg)) {
          return false;
        }
        mongo::BSONObj full_query;

        // list all project if is admin
        if (!user.is_admin()) {
          mongo::BSONObj public_projects = BSON("public" << true);
          full_query = BSON("$or" << BSON_ARRAY(public_projects << BSON("_id" << BSON("$in" << utils::build_array(user.get_projects())))));
        }

        std::vector<mongo::BSONObj> projects;
        if (!helpers::get(Collections::PROJECTS(), full_query, projects, msg)) {
          return false;
        }

        result = utils::bsons_to_id_names(projects);

        return true;
      }

      bool epigenetic_marks(const datatypes::Metadata &metadata,
                            std::vector<utils::IdName> &result, std::string &msg)
      {
        mongo::BSONObjBuilder query_builder;
        datatypes::Metadata::const_iterator it;
        for (const auto& p : metadata) {
          query_builder.append("extra_metadata." + p.first, p.second);
        }
        return helpers::get(Collections::EPIGENETIC_MARKS(), query_builder.obj(), result, msg);
      }

      bool experiments(const mongo::BSONObj& query, std::vector<utils::IdName> &result, std::string &msg)
      {
        return experiments(mongo::Query(query), result, msg);
      }

      bool experiments(const mongo::Query& query, std::vector<utils::IdName> &result, std::string &msg)
      {
        std::vector<std::string> fields;
        fields.push_back("_id");
        fields.push_back("name");

        std::vector<mongo::BSONObj> objects;
        if (!helpers::get(Collections::EXPERIMENTS(), query, fields, objects, msg)) {
          return false;
        }

        for (const mongo::BSONObj & o : objects) {
          utils::IdName id_name(o["_id"].String(), o["name"].String());
          result.push_back(id_name);
        }

        return true;
      }

      bool annotations(const std::string &genome, const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        std::vector<mongo::BSONObj> objects;
        if (!helpers::get(Collections::ANNOTATIONS(), "norm_genome", genome, objects, msg)) {
          return false;
        }

        for (const mongo::BSONObj & o : objects) {
          utils::IdName names(o["_id"].String(), o["name"].String());
          result.push_back(names);
        }

        return true;
      }

      bool annotations(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::ANNOTATIONS(), result, msg);
      }

      bool column_types(const std::string &user_key, std::vector<utils::IdName> &content, std::string  &msg)
      {
        return dba::columns::list_column_types(user_key, content, msg);
      }

      //-----

      bool column_types(const std::string &user_key, std::vector<std::string> &content, std::string  &msg)
      {
        Connection c;
        auto data_cursor = c->query(helpers::collection_name(Collections::COLUMN_TYPES()), mongo::BSONObj());

        processing::StatusPtr status = processing::build_dummy_status();

        while (data_cursor->more()) {
          mongo::BSONObj o = data_cursor->next().getOwned();
          columns::ColumnTypePtr column_type;
          if (! epidb::dba::columns::column_type_bsonobj_to_class(o, status, column_type, msg))  {
            return false;
          }
          content.push_back(column_type->str());
        }

        c.done();
        return true;
      }

      // --

      bool gene_models(const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg)
      {
        return helpers::get(Collections::GENE_MODELS(), result, msg);
      }

      bool genes(const std::string &user_key,
                 const std::vector<std::string> &genes_id_or_name, const std::vector<std::string> &go_terms,
                 const std::vector<std::string> &chromosomes,
                 const Position start, const Position end,
                 const std::string &norm_gene_model,  std::vector<mongo::BSONObj> &genes, std::string &msg)
      {
        return dba::genes::get_genes(chromosomes, start, end, "", genes_id_or_name, go_terms, user_key, norm_gene_model, genes, msg);
      }

      bool similar_biosources(const std::string &name, const std::string &user_key,
                              std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::BIOSOURCES(), utils::normalize_name(name), user_key, result, msg);
      }

      bool similar_techniques(const std::string &name, const std::string &user_key,
                              std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::TECHNIQUES(), utils::normalize_name(name), user_key, result, msg);
      }

      bool similar_projects(const std::string &name, const std::string &user_key,
                            std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::PROJECTS(), utils::normalize_name(name), user_key, result, msg);
      }

      bool similar_epigenetic_marks(const std::string &name, const std::string &user_key,
                                    std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::EPIGENETIC_MARKS(),  utils::normalize_epigenetic_mark(name), user_key, result, msg);
      }

      bool similar_genomes(const std::string &name, const std::string &user_key,
                           std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::GENOMES(), utils::normalize_epigenetic_mark(name), user_key, result, msg);
      }

      bool similar_experiments(const std::string &name, const std::string &genome, const std::string &user_key,
                               std::vector<utils::IdName> &result, std::string &msg)
      {
        return similar(Collections::EXPERIMENTS(), "name", name, "genome", genome, user_key, result, msg);
      }

      bool similar(const std::string &where, const std::string &what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg,
                   const size_t total)
      {
        std::vector<utils::IdName> id_names;
        if (!helpers::get(where, id_names, msg)) {
          return false;
        }

        std::vector<std::string> names;
        std::map<std::string, std::string> id_name_map;
        for (const utils::IdName & id_name : id_names) {
          id_name_map[id_name.name] = id_name.id;
          names.push_back(id_name.name);
        }
        std::vector<std::string> ordered = epidb::algorithms::Levenshtein::order_by_score(what, names);

        size_t count = 0;
        for (const std::string & name : ordered) {
          utils::IdName id_name(id_name_map[name], name);
          result.push_back(id_name);
          count++;
          if (count >= total) {
            break;
          }
        }

        return true;
      }

      bool similar(const std::string &where, const std::string &field, const std::string &what,
                   const std::string &filter_field, const std::string &filter_what,
                   const std::string &user_key, std::vector<utils::IdName> &result, std::string &msg,
                   const size_t total)
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

        size_t count = 0;
        for (const std::string & name : ordered) {
          utils::IdName id_name(id_name_map[name], name);
          result.push_back(id_name);
          count++;
          if (count >= total) {
            break;
          }
        }

        return true;
      }

      bool build_list_experiments_query(const std::vector<serialize::ParameterPtr> genomes, const std::vector<serialize::ParameterPtr> types,
                                        const std::vector<serialize::ParameterPtr> epigenetic_marks, const std::vector<serialize::ParameterPtr> biosources,
                                        const std::vector<serialize::ParameterPtr> sample_ids, const std::vector<serialize::ParameterPtr> techniques,
                                        const std::vector<serialize::ParameterPtr> projects, const std::string user_key,
                                        mongo::BSONObj& query, std::string& msg)
      {
        mongo::BSONObjBuilder args_builder;

        if (!types.empty()) {
          args_builder.append("upload_info.content_format", utils::build_normalized_array(types));
        }

        if (!genomes.empty()) {
          args_builder.append("genome", utils::build_array(genomes));
          args_builder.append("norm_genome", utils::build_normalized_array(genomes));
        }


        if (!biosources.empty()) {
          args_builder.append("sample_info.biosource_name", utils::build_array(biosources));
          args_builder.append("sample_info.norm_biosource_name", utils::build_normalized_array(biosources));
        }

        // epigenetic mark
        if (!epigenetic_marks.empty()) {
          args_builder.append("epigenetic_mark", utils::build_array(epigenetic_marks));
          args_builder.append("norm_epigenetic_mark", utils::build_epigenetic_normalized_array(epigenetic_marks));
        }
        // sample id
        if (!sample_ids.empty()) {
          args_builder.append("sample_id", utils::build_array(sample_ids));
        }

        std::vector<utils::IdName> user_projects;
        if (!dba::list::projects(user_key, user_projects, msg)) {
          return false;
        }

        // project.
        if (!projects.empty()) {

          // Filter the projects that are available to the user
          std::vector<serialize::ParameterPtr> filtered_projects;
          for (const auto& project : projects) {
            std::string project_name = project->as_string();
            std::string norm_project = utils::normalize_name(project_name);
            bool found = false;
            for (const auto& user_project : user_projects) {
              std::string norm_user_project = utils::normalize_name(user_project.name);
              if (norm_project == norm_user_project) {
                filtered_projects.push_back(project);
                found = true;
                break;
              }
            }

            if (!found) {
              msg = Error::m(ERR_INVALID_PROJECT, project_name);
              return false;
            }
          }
          args_builder.append("project", utils::build_array(filtered_projects));
          args_builder.append("norm_project", utils::build_normalized_array(filtered_projects));
        } else {
          std::vector<std::string> user_projects_names;
          for (const auto& project : user_projects) {
            user_projects_names.push_back(project.name);
          }

          args_builder.append("project", utils::build_array(user_projects_names));
          args_builder.append("norm_project", utils::build_normalized_array(user_projects_names));
        }

        // technique
        if (!techniques.empty()) {
          args_builder.append("technique", utils::build_array(techniques));
          args_builder.append("norm_technique", utils::build_normalized_array(techniques));
        }

        query = dba::query::build_query(args_builder.obj());

        return true;
      }

      bool get_controlled_vocabulary_key(const std::string& controlled_vocabulary,
                                         std::string &collection_key, std::string &msg)
      {
        if (controlled_vocabulary == dba::Collections::EPIGENETIC_MARKS()) {
          collection_key = "$norm_epigenetic_mark";
          return true;
        }

        if (controlled_vocabulary == dba::Collections::GENOMES()) {
          collection_key = "$norm_genome";
          return true;
        }

        if (controlled_vocabulary == dba::Collections::BIOSOURCES()) {
          collection_key = "$sample_info.norm_biosource_name";
          return true;
        }

        if (controlled_vocabulary == dba::Collections::SAMPLES()) {
          collection_key = "$sample_id";
          return true;
        }

        if (controlled_vocabulary == dba::Collections::TECHNIQUES()) {
          collection_key = "$norm_technique";
          return true;
        }

        if (controlled_vocabulary == dba::Collections::PROJECTS()) {
          collection_key = "$norm_project";
          return true;
        }

        if (controlled_vocabulary == "types") {
          collection_key = "$upload_info.content_format";
          return true;
        }


        msg = "Controlled vocabulary '" + controlled_vocabulary + "' does not exist. Available controlled vocabularies: types, " +
              dba::Collections::EPIGENETIC_MARKS() + ", " + dba::Collections::GENOMES() + ", " +
              dba::Collections::BIOSOURCES() + ", " + dba::Collections::SAMPLES() + ", " +
              dba::Collections::TECHNIQUES() + ", " + dba::Collections::PROJECTS();
        return false;
      }


      bool in_use(const std::string &collection, const std::string &key_name, const std::string &user_key,
                  std::vector<utils::IdNameCount> &names, std::string &msg)
      {
        std::vector<utils::IdName> user_projects;
        if (!projects(user_key, user_projects, msg)) {
          return false;
        }

        std::vector<std::string> project_names;
        for (const auto& project : user_projects) {
          project_names.push_back(project.name);
        }

        mongo::BSONArray projects_array = utils::build_array(project_names);

        // Select experiments that are uploaded and from πublic projects or that the user has permission
        mongo::BSONObj done = BSON("upload_info.done" << true);
        mongo::BSONObj user_projects_bson = BSON("project" << BSON("$in" << projects_array));
        mongo::BSONObj query = BSON("$and" << BSON_ARRAY(done <<  user_projects_bson));
        mongo::BSONObj match = BSON("$match" << query);

        // Group by count
        mongo::BSONObj group = BSON( "$group" << BSON( "_id" << key_name << "total" << BSON( "$sum" << 1 ) ) );

        mongo::BSONArray pipeline = BSON_ARRAY( match << group );

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

        for (const mongo::BSONElement & be : result) {
          std::string norm_name = utils::normalize_name(be["_id"].String());
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

      std::tuple<bool, std::string> __collection_experiments_count(const mongo::BSONObj & experiments_query, const mongo::BSONArray & projects_array,
          const std::string collection, const std::string key_name,
          std::unordered_map<std::string, std::vector<utils::IdNameCount> > &faceting_result)
      {
        // Select experiments that are uploaded and from πublic projects or that the user has permission
        mongo::BSONObj done = BSON("upload_info.done" << true);
        mongo::BSONObj user_projects_bson = BSON("project" << BSON("$in" << projects_array));
        mongo::BSONObj query = BSON("$and" << BSON_ARRAY(done <<  user_projects_bson << experiments_query));
        mongo::BSONObj match = BSON("$match" << query);

        // Group by count
        mongo::BSONObj group = BSON( "$group" << BSON( "_id" << key_name << "total" << BSON( "$sum" << 1 ) ) );
        mongo::BSONArray pipeline = BSON_ARRAY( match << group );

        Connection c;
        auto cursor = c->aggregate(helpers::collection_name(Collections::EXPERIMENTS()), pipeline);

        std::vector<utils::IdNameCount> names;

        while  (cursor->more()) {
          mongo::BSONObj be = cursor->next();
          const mongo::BSONElement& _id = be["_id"];
          std::string norm_name;
          if (_id.isNull()) {
            norm_name = "null";
          } else {
            norm_name = utils::normalize_name(_id.String());
          }

          long count = be["total"].safeNumberLong();

          utils::IdNameCount inc;
          if (collection == Collections::SAMPLES())
            inc = utils::IdNameCount(norm_name, "", count);
          else if (collection == "types") {
            inc = utils::IdNameCount("", norm_name, count);
          } else {
            utils::IdName id_name;
            std::string msg;
            if (!helpers::get_name(collection, norm_name, id_name, msg)) {
              c.done();
              return std::make_tuple(false, msg);
            }
            inc = utils::IdNameCount(id_name.id, id_name.name, count);
          }
          names.push_back(inc);
        }

        c.done();
        faceting_result[collection] = names;
        return std::make_tuple(true, std::string(""));
      }

      bool collection_experiments_count(const std::string controlled_vocabulary,
                                        const mongo::BSONObj & experiments_query, const std::string &user_key,
                                        std::vector<utils::IdNameCount> &experiments_count, std::string& msg)
      {
        std::vector<utils::IdName> user_projects;
        if (!projects(user_key, user_projects, msg)) {
          return false;
        }
        std::vector<std::string> project_names;
        for (const auto& project : user_projects) {
          project_names.push_back(project.name);
        }
        mongo::BSONArray projects_array = utils::build_array(project_names);

        std::string key_name;
        if (!get_controlled_vocabulary_key(controlled_vocabulary, key_name, msg)) {
          return false;
        }


        std::unordered_map<std::string, std::vector<utils::IdNameCount>> faceting_result;
        auto result = __collection_experiments_count(experiments_query, projects_array,
                      controlled_vocabulary, key_name, faceting_result);
        if (!std::get<0>(result)) {
          msg = std::get<1>(result);
          return false;
        }
        experiments_count = faceting_result[controlled_vocabulary];
        return true;
      }

      bool faceting(const mongo::BSONObj& experiments_query, const std::string &user_key,
                    std::unordered_map<std::string, std::vector<utils::IdNameCount>> &faceting_result,
                    std::string &msg)
      {
        std::vector<utils::IdName> user_projects;
        if (!projects(user_key, user_projects, msg)) {
          return false;
        }
        std::vector<std::string> project_names;
        for (const auto& project : user_projects) {
          project_names.push_back(project.name);
        }
        mongo::BSONArray projects_array = utils::build_array(project_names);

        std::vector<std::pair<std::string, std::string> > collums = {
          {"epigenetic_marks", "$norm_epigenetic_mark"},
          {"genomes", "$norm_genome"},
          {"biosources", "$sample_info.norm_biosource_name"},
          {"samples", "$sample_id"},
          {"techniques", "$norm_technique"},
          {"projects", "$norm_project"},
          {"types", "$upload_info.content_format"}

        };


        std::vector<std::future<std::tuple<bool, std::string> > > threads;
        for (const auto& column : collums) {
          std::string collection = column.first;
          std::string key_name = column.second;

          auto t = std::async(std::launch::async,
                              &__collection_experiments_count,
                              std::ref(experiments_query), std::ref(projects_array),
                              collection, key_name, std::ref(faceting_result));
          threads.push_back(std::move(t));
        }

        size_t thread_count = threads.size();
        for (size_t i = 0; i < thread_count; ++i) {
          threads[i].wait();
          auto result = threads[i].get();
          if (!std::get<0>(result)) {
            msg = std::get<1>(result);
            return false;
          }
        }

        return true;
      }
    }
  }
}
