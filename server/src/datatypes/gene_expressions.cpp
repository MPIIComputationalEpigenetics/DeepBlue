//
//  gene_expressions.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.10.16.
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

#include "../datatypes/metadata.hpp"

#include "../dba/collections.hpp"
#include "../dba/data.hpp"
#include "../dba/full_text.hpp"
#include "../dba/genes.hpp"
#include "../dba/helpers.hpp"
#include "../dba/key_mapper.hpp"
#include "../dba/remove.hpp"

#include "expressions.hpp"
#include "gene_expressions.hpp"

#include "../errors.hpp"

namespace epidb {
  namespace datatypes {

    bool GeneExpressionType::info(const std::string & id, mongo::BSONObj & obj_metadata, std::string & msg)
    {
      mongo::BSONObj data_obj;
      if (!data(id, data_obj, msg)) {
        return false;
      }

      mongo::BSONObjBuilder bob;

      bob.append(data_obj["_id"]);
      bob.append(data_obj["format"]);
      bob.append(data_obj["upload_info"]["content_format"]);
      bob.append(data_obj["upload_info"]["total_genes"]);
      bob.append(data_obj["replica"]);
      bob.append(data_obj["sample_id"]);
      bob.append(data_obj["extra_metadata"]);
      bob.append(data_obj["columns"]);

      mongo::BSONObjBuilder sample_bob;
      const auto sample = data_obj["sample_info"].Obj();
      for (auto it = sample.begin(); it.more(); ) {
        mongo::BSONElement e = it.next();
        if (strncmp("norm_", e.fieldName(), 5) != 0) {
          sample_bob.append(e.fieldName(), utils::bson_to_string(e));
        }
      }

      bob.append("sample_info", sample_bob.obj());

      obj_metadata = bob.obj();

      return true;
    }

    bool GeneExpressionType::data(const std::string & id, mongo::BSONObj & result, std::string & msg)
    {
      if (dba::helpers::get_one(dba::Collections::GENE_EXPRESSIONS(), mongo::Query(BSON("_id" << id)), result)) {
        return true;
      } else {
        msg = Error::m(ERR_INVALID_EXPRESSION_ID, id);
        return false;
      }
    }

    bool GeneExpressionType::exists(const std::string & sample_id, const int replica)
    {
      return dba::helpers::check_exist(dba::Collections::GENE_EXPRESSIONS(),
                                       BSON("sample_id" << sample_id << "replica" << replica));
    }

    bool GeneExpressionType::update_upload_info(const std::string & collection, const std::string & annotation_id,
        const size_t total_size, const size_t total_genes, std::string & msg)
    {
      Connection c;
      c->update(dba::helpers::collection_name(collection), BSON("_id" << annotation_id),
                BSON("$set" << BSON("upload_info.total_genes" << (long long) total_genes << "upload_info.total_size" << (long long) total_size << "upload_info.done" << true << "upload_info.upload_end" << mongo::DATENOW)), false, true);

      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }
      c.done();

      return true;
    }

    mongo::BSONObj GeneExpressionType::to_bson(const int dataset_id, const std::string & gene_id, const ISerializablePtr & row)
    {
      mongo::BSONObjBuilder bob;

      auto row_bson = row->to_BSON();

      bob.append("_id", gene_id);
      bob.append(dba::KeyMapper::DATASET(), dataset_id);
      bob.appendElements(row_bson);

      return bob.obj();
    }

    bool GeneExpressionType::insert(const datatypes::User& user,
                                    const std::string & sample_id, const int replica, datatypes::Metadata extra_metadata,
                                    const ISerializableFilePtr file, const std::string & format,
                                    const std::string & project, const std::string & norm_project,
                                    const std::string & ip,
                                    std::string & expression_id, std::string & msg)
    {

      mongo::BSONObj gene_expression_metadata;
      mongo::BSONObj extra_metadata_obj = datatypes::metadata_to_bson(extra_metadata);
      int dataset_id;

      if (!build_expression_type_metadata(sample_id, replica, format, project, norm_project, extra_metadata_obj,
                                          ip, dataset_id, expression_id, gene_expression_metadata, msg)) {
        return false;
      }

      mongo::BSONObj upload_info;
      if (!build_upload_info(user, ip, format, upload_info, msg)) {
        return false;
      }
      mongo::BSONObjBuilder gene_expression_builder;
      gene_expression_builder.appendElements(gene_expression_metadata);
      gene_expression_builder.append("upload_info", upload_info);

      mongo::BSONObj e = gene_expression_builder.obj();
      Connection c;
      c->insert(dba::helpers::collection_name(dba::Collections::GENE_EXPRESSIONS()), e);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!dba::search::insert_full_text(dba::Collections::GENE_EXPRESSIONS(), expression_id, gene_expression_metadata, msg)) {
        c.done();
        std::string new_msg;
        if (!dba::remove::gene_expression(user, expression_id, new_msg)) {
          msg = msg + " " + new_msg;
        }
        return false;
      }

      size_t total_size = 0;
      size_t total_genes = 0;

      std::vector<mongo::BSONObj> rows_obj_bulk;

      for (const auto& row : file->rows()) {
        int _id;
        if (!dba::helpers::get_increment_counter("gene_single_expressions", _id, msg) ||
            !dba::helpers::notify_change_occurred(dba::Collections::GENE_SINGLE_EXPRESSIONS(), msg))  {
          return false;
        }

        std::string gene_id = "gsx" + utils::integer_to_string(_id);
        mongo::BSONObj row_obj = to_bson(dataset_id, gene_id, row);

        rows_obj_bulk.emplace_back(std::move(row_obj));
        total_genes++;
      }

      c->insert(dba::helpers::collection_name(dba::Collections::GENE_SINGLE_EXPRESSIONS()), rows_obj_bulk);
      if (!c->getLastError().empty()) {
        msg = c->getLastError();
        c.done();
        return false;
      }

      if (!update_upload_info(dba::Collections::GENE_EXPRESSIONS(), expression_id, total_size, total_genes, msg)) {
        std::string new_msg;
        if (!dba::remove::gene_model(user, expression_id, new_msg)) {
          msg = msg + " " + new_msg;
        }
        return false;
      }

      c.done();
      return true;

    }


    bool GeneExpressionType::load_data(const std::vector<std::string> &sample_ids, const  std::vector<long>& replicas,
                                       const std::vector<std::string> &genes, const std::vector<std::string> &project,
                                       const std::string & norm_gene_model,  ChromosomeRegionsList & chromosomeRegionsList, std::string & msg)
    {
      Connection c;
      mongo::BSONObj gene_model_obj = c->findOne(dba::helpers::collection_name(dba::Collections::GENE_MODELS()),
                                      BSON("norm_name" << norm_gene_model));

      if (gene_model_obj.isEmpty()) {
        msg = "gene model " + norm_gene_model + " does not exists";
        c.done();
        return false;
      }

      mongo::BSONObjBuilder ges_builder;
      if (!sample_ids.empty()) {
        ges_builder.append("sample_id",  BSON("$in" << utils::build_array(sample_ids)));
      }

      if (!replicas.empty()) {
        ges_builder.append("replica", BSON("$in" << utils::build_array_long(replicas)));
      }

      if (!project.empty()) {
        ges_builder.append("norm_project", BSON("$in" << utils::build_array(project)));
      }

      mongo::BSONObj ges_query = ges_builder.obj();

      mongo::BSONArray ges_datasets = dba::helpers::build_dataset_ids_arrays(dba::Collections::GENE_EXPRESSIONS(), ges_query);

      mongo::BSONObjBuilder bob;
      // Look at the tracking ID, gene id, and short name
      bob.append(dba::KeyMapper::DATASET(), BSON("$in" << ges_datasets));
      if (!genes.empty()) {
        mongo::BSONObj b_in_tracking_id = BSON(dba::KeyMapper::TRACKING_ID() << BSON("$in" << utils::build_array(genes)));
        mongo::BSONObj b_in_gene_id = BSON(dba::KeyMapper::GENE_ID() << BSON("$in" << utils::build_array(genes)));
        mongo::BSONObj b_in_short_name = BSON(dba::KeyMapper::GENE_SHORT_NAME() <<  BSON("$in" << utils::build_array(genes)));
        bob.append("$or", BSON_ARRAY(b_in_tracking_id << b_in_gene_id << b_in_short_name ));
      }

      auto query = bob.obj();

      std::string collection = dba::helpers::collection_name(dba::Collections::GENE_SINGLE_EXPRESSIONS());
      auto data_cursor = c->query(collection, query);

      std::unordered_map<std::string, Regions> gene_expressions;

      while (data_cursor->more()) {
        mongo::BSONObj gene = data_cursor->next().getOwned();

        std::string gene_short_name;
        std::string tracking_id;

        DatasetId dataset_id = gene[dba::KeyMapper::DATASET()].Int();

        if (gene.hasElement(dba::KeyMapper::GENE_SHORT_NAME())) {
          gene_short_name = gene[dba::KeyMapper::GENE_SHORT_NAME()].str();
        }

        if (gene.hasElement(dba::KeyMapper::TRACKING_ID())) {
          tracking_id = gene[dba::KeyMapper::TRACKING_ID()].str();
        }

        std::string chromosome;
        Position start;
        Position end;
        std::string strand;

        if (!dba::genes::map_gene_location(tracking_id, gene_short_name, norm_gene_model, chromosome, start, end, strand, msg)) {
          return false;
        }

        RegionPtr region = build_stranded_region(start, end, dataset_id, strand);

        auto it = gene.begin();

        // Jump the _id field
        it.next();
        // Jump the dataset_id field
        it.next();

        while ( it.more() ) {
          const mongo::BSONElement &e = it.next();
          switch (e.type()) {
          case mongo::String :
            region->insert(e.str());
            break;
          case mongo::NumberDouble :
            region->insert((float) e._numberDouble());
            break;
          case mongo::NumberInt :
            region->insert(e._numberInt());
            break;
          default:
            region->insert(e.toString(false));
          }
        }

        gene_expressions[chromosome].emplace_back(std::move(region));
      }

      for (auto &chromosome_regions : gene_expressions) {
        std::sort(chromosome_regions.second.begin(), chromosome_regions.second.end(), RegionPtrComparer);
        chromosomeRegionsList.emplace_back(chromosome_regions.first, std::move(chromosome_regions.second));
      }

      c.done();

      return true;
    }

    bool GeneExpressionType::list(const mongo::BSONObj & query, std::vector<utils::IdName> &result, std::string & msg)
    {
      std::vector<std::string> fields;
      fields.push_back("_id");

      std::vector<mongo::BSONObj> objects;
      if (!dba::helpers::get(dba::Collections::GENE_EXPRESSIONS(), query, fields, objects, msg)) {
        return false;
      }

      for (const mongo::BSONObj & o : objects) {
        utils::IdName id_name(o["_id"].String(), "");
        result.push_back(id_name);
      }

      return true;
    }
  }
}
