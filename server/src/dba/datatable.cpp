//
//  datatable.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 22.03.2016.
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
#include <vector>

#include <mongo/bson/bson.h>

#include "../connection/connection.hpp"

#include "../processing/processing.hpp"

#include "column_types.hpp"
#include "collections.hpp"
#include "genes.hpp"
#include "helpers.hpp"
#include "list.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace dba {
    namespace datatable {


      bool format_row(const std::string collection, const std::vector<std::string>& fields, const mongo::BSONObj& obj,
                      std::vector<std::string>& row, std::string& msg)
      {
        for (const auto& field : fields) {
          std::string s;
          if (field.compare("data_type") == 0) {
            s = obj["upload_info"]["content_format"].String();
          } else if (field.compare("biosource") == 0) {
            s = obj["sample_info"]["biosource_name"].String();
          } else if (field.compare("extra_metadata") == 0) {
            StringBuilder sb;

            if (collection == "experiments") {
              sb.append("<div class='exp-metadata'>");
            }

            std::string extra_metadata = utils::format_extra_metadata(obj["extra_metadata"].Obj());
            if (!extra_metadata.empty()) {
              sb.append(extra_metadata);
              sb.append("<br/>");
            }

            if (obj.hasField("upload_info")) {
              sb.append("<b>Upload information</b>:<br/><br/>");

              if (collection == "gene_models") {
                sb.append("<b>number of genes</b>: ");
                sb.append(utils::bson_to_string(obj["upload_info"]["total_genes"]));
                sb.append("<br/>");
              } else {
                sb.append("<b>data size</b>: ");
                sb.append(utils::bson_to_string(obj["upload_info"]["total_size"]));
                sb.append("kbytes<br/>");
              }

              sb.append("<b>data inserted</b>: ");
              sb.append(utils::bson_to_string(obj["upload_info"]["upload_end"]));
              sb.append("<br/>");
            }

            if (obj.hasField("sample_info")) {
              sb.append("<br/>");
              sb.append("<b>Sample metadata</b><br/><br />");
              sb.append(utils::format_extra_metadata(obj["sample_info"].Obj()));
            }
            if (collection == "experiments") {
              sb.append("</div><div class='exp-metadata-more-view'>-- View metadata --</div>");
            }

            s = sb.to_string();

          } else {
            s = utils::bson_to_string(obj[field]);
          }
          row.emplace_back(std::move(s));
        }

        return true;
      }



      bool format_column_type(const mongo::BSONObj& obj, std::vector<std::string> &row, std::string& msg)
      {
        std::map<std::string, std::string> res = columns::dataset_column_to_map(obj);
        row.emplace_back(res["_id"]);
        row.emplace_back(res["name"]);
        row.emplace_back(res["description"]);

        std::string column_type = res["column_type"];

        row.emplace_back(column_type);

        std::string information;
        if (column_type == "category") {
          information = "Acceptable items: " + res["items"];
        } else if (column_type == "range") {
          information = res["minimum"] + " " + res["maximum"];
        } else if (column_type == "calculated") {
          information = res["code"];
        }
        row.emplace_back(information);

        return true;
      }

      std::string element_value(const mongo::BSONObj& o, const std::string& key)
      {
        if (o.hasField(key)) {
          return o[key].String();
        }

        return std::string("");
      }

      bool format_gene(const mongo::BSONObj& obj, std::vector<std::string> &row, std::string& msg)
      {
        mongo::BSONObj attributes = obj[KeyMapper::ATTRIBUTES()].Obj();

        row.emplace_back(obj["_id"].String());

        std::string gene_model;
        if (!genes::get_gene_model_by_dataset_id(obj[KeyMapper::DATASET()].Int(), gene_model, msg)) {
          return false;
        }

        row.emplace_back(gene_model);
        row.emplace_back(element_value(obj, KeyMapper::SOURCE()));
        row.emplace_back(element_value(obj, KeyMapper::CHROMOSOME()));
        row.emplace_back(utils::integer_to_string( obj[KeyMapper::START()].Int()));
        row.emplace_back(utils::integer_to_string( obj[KeyMapper::END()].Int()));
        row.emplace_back(element_value(obj, KeyMapper::STRAND()));
        row.emplace_back(element_value(attributes, "gene_id"));
        row.emplace_back(element_value(attributes, "gene_name"));
        row.emplace_back(element_value(attributes, "gene_type"));
        row.emplace_back(element_value(attributes, "gene_status"));
        row.emplace_back(element_value(attributes, "level"));

        return true;
      }

      bool format_sample(const mongo::BSONObj& obj, std::vector<std::string> &row, std::string& msg)
      {
        auto it = obj.begin();

        row.emplace_back(obj["_id"].String());
        row.emplace_back(obj["biosource_name"].String());

        StringBuilder sb;
        while (it.more()) {
          const auto& e = it.next();
          std::string elem_name(e.fieldName());
          if (elem_name.compare("type") && elem_name.compare("_id") && elem_name.compare("biosource_name") && elem_name.compare("user") &&
              elem_name.compare(0, 2, "__")  && elem_name.compare(0, 5, "norm_")) {

            std::string content = std::string(utils::bson_to_string(e));
            sb.append("<b>");
            sb.append(elem_name);
            sb.append("</b>: ");
            sb.append(content);
            sb.append("<br/>");
          }
        }
        row.emplace_back(sb.to_string());

        return true;
      }

      std::string get_gene_sort_column(std::string name)
      {
        if (name == "_id") {
          return "_id";
        }

        if (name == "gene_model") {
          return KeyMapper::DATASET();
        }

        if (name == "level" || name == "gene_name" || name == "gene_id" || name == "gene_status" || name == "gene_type") {
          return KeyMapper::ATTRIBUTES() + "." + name;
        }

        std::transform(name.begin(), name.end(), name.begin(), ::toupper);

        std::string msg;
        std::string key;
        if (!KeyMapper::KeyMapper::to_short(name, key, msg)) {
          return name;
        }
        return key;
      }

      bool build_gene_query(const datatypes::Metadata& columns_filters, mongo::BSONObjBuilder& query_builder, std::string& msg )
      {
        for (const auto& column : columns_filters) {
          std::string name = column.first;
          std::string value = column.second;

          std::string key;
          if (name == "_id") {
            query_builder.appendRegex("_id", value, "i");

          } else if (name == "level" || name == "gene_name" || name == "gene_id" || name == "gene_status" || name == "gene_type") {
            key = KeyMapper::ATTRIBUTES() + "." + name;
            query_builder.appendRegex(key, value, "i"); // case insensitivity and ignore spaces and special characters

          } else if (name == "start" || name == "end") {
            int int_value;
            if (!utils::string_to_int(value, int_value)) {
              continue;
            }
            std::transform(name.begin(), name.end(), name.begin(), ::toupper);
            if (!KeyMapper::KeyMapper::to_short(name, key, msg)) {
              return false;
            }

            if (name == "START") {
              query_builder.append(key, BSON("$gte" << int_value));
            } else {
              query_builder.append(key, BSON("$lte" << int_value));
            }

          } else if (name == "strand") {
            query_builder.append(KeyMapper::STRAND(), value);

          } else if (name == "gene_model") {
            mongo::BSONObjBuilder gene_datasets_query_builder;
            gene_datasets_query_builder.appendRegex("norm_name", value, "i");

            mongo::BSONArray IDs = helpers::build_dataset_ids_arrays(Collections::GENE_MODELS(), gene_datasets_query_builder.obj());
            query_builder.append(KeyMapper::DATASET(),  BSON("$in" << IDs));

          } else {
            std::transform(name.begin(), name.end(), name.begin(), ::toupper);

            if (!KeyMapper::KeyMapper::to_short(name, key, msg)) {
              return false;
            }
            query_builder.appendRegex(key, value, "i"); // case insensitivity and ignore spaces and special characters
          }

        }

        return true;
      }

      bool datatable(const std::string collection, const std::vector<std::string> columns,
                     const long long start, const long long length,
                     const std::string& global_search, const std::string& sort_column, const std::string& sort_direction,
                     const bool has_filter, const datatypes::Metadata& columns_filters,
                     const std::string& user_key,
                     size_t& total_elements, std::vector<std::vector<std::string>>& results,
                     std::string& msg)
      {
        if (!dba::Collections::is_valid_search_collection(collection)) {
          msg = Error::m(ERR_INVALID_COLLECTION_NAME, collection, utils::vector_to_string(dba::Collections::valid_search_Collections()));
          return false;
        }

        if (start < 0) {
          msg = Error::m(ERR_INVALID_START, start);
          return false;
        }

        if (length <= 0) {
          msg = Error::m(ERR_INVALID_LENGTH, length);
          return false;
        }

        mongo::BSONObj projection;


        if (collection == Collections::SAMPLES() ||
            collection == Collections::COLUMN_TYPES() ||
            collection == Collections::GENES()) {
          projection = mongo::BSONObj();
        } else {
          mongo::BSONObjBuilder b;
          for (const std::string & c : columns) {
            if (c == "data_type") {
              b.append("upload_info.content_format", 1);
            } else if (c == "biosource") {
              b.append("sample_info.biosource_name", 1);
            } else if (c == "extra_metadata") {
              b.append("extra_metadata", 1);
              b.append("sample_info", 1);
              b.append("upload_info", 1);
            } else {
              b.append(c, 1);
            }
          }
          projection = b.obj();
        }

        mongo::BSONObjBuilder query_builder;
        if (!global_search.empty()) {
          mongo::BSONObjBuilder text_builder;
          text_builder.append("$search", global_search);
          query_builder.append("$text", text_builder.obj());
        }

        mongo::BSONObj query_obj;
        if (collection == Collections::GENES()) {
          if (!build_gene_query(columns_filters, query_builder, msg)) {
            return false;
          }
        } else {
          for (const auto& filter : columns_filters) {
            if (filter.first == "extra_metadata" && global_search.empty()) {
              mongo::BSONObjBuilder text_builder;
              text_builder.append("$search", filter.second);
              query_builder.append("$text", text_builder.obj());
            } else if (filter.first == "data_type") {
              query_builder.appendRegex("upload_info.content_format", filter.second, "ix");
            } else if (filter.first == "biosource") {
              query_builder.appendRegex("sample_info.norm_biosource_name", filter.second, "ix");
            } else {
              query_builder.appendRegex(filter.first, filter.second, "i"); // case insensitivity and ignore spaces and special characters
            }
          }
        }

        if (collection == Collections::EXPERIMENTS() ||
            collection == Collections::PROJECTS() ||
            collection == Collections::GENE_EXPRESSIONS()) {

          std::vector<utils::IdName> user_projects;
          if (!dba::list::projects(user_key, user_projects, msg)) {
            return false;
          }

          std::vector<std::string> user_projects_names;
          for (const auto& project : user_projects) {
            user_projects_names.push_back(project.name);
          }

          query_builder.append("project", utils::build_array(user_projects_names));
          query_builder.append("norm_project", utils::build_normalized_array(user_projects_names));
        }

        query_obj = query_builder.obj();
        mongo::Query query(query_obj);

        int sort = 1;
        if (sort_direction == "desc") {
          sort = -1;
        }

        if (!sort_column.empty()) {
          if (collection == Collections::GENES()) {
            query.sort(get_gene_sort_column(sort_column), sort);
          } else if (sort_column == "data_type") {
            query.sort(std::string("upload_info.content_format"), sort);
          } else if (sort_column == "biosource") {
            query.sort(std::string("sample_info.biosource_name"), sort);
          } else {
            query.sort(sort_column, sort);
          }
        }


        Connection c;
        auto cursor = c->query(helpers::collection_name(collection), query, length, start, &projection);

        std::vector<std::vector<std::string>> rows;

        while (cursor->more()) {
          std::vector<std::string> row;
          mongo::BSONObj obj = cursor->next().getOwned();

          if (collection == Collections::SAMPLES()) {
            if (!format_sample(obj, row, msg)) {
              c.done();
              return false;
            }
          } else if (collection == Collections::COLUMN_TYPES()) {
            if (!format_column_type(obj, row, msg)) {
              c.done();
              return false;
            }
          } else if (collection == Collections::GENES()) {
            if (!format_gene(obj, row, msg)) {
              c.done();
              return false;
            }
          } else {
            if (!format_row(collection, columns, obj, row, msg)) {
              c.done();
              return false;
            }
          }
          results.emplace_back(std::move(row));
        }
        c.done();

        if (!helpers::collection_size(collection, query_obj, total_elements, msg )) {
          return false;
        }

        return true;
      }
    }
  }
}
