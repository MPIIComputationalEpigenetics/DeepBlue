//
//  column_types.cpp
//  epidb
//
//  Created by Felipe Albrecht on 03.02.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <algorithm>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include <mongo/bson/bson.h>

#include "../extras/utils.hpp"

#include "collections.hpp"
#include "config.hpp"
#include "dba.hpp"
#include "full_text.hpp"
#include "helpers.hpp"

#include "column_types.hpp"

namespace epidb {
  namespace dba {
    namespace columns {

      template<>
      bool ColumnType<long long>::check(const std::string &verify) const
      {
        size_t ll;
        return utils::string_to_long(verify, ll);
      }

      template<>
      const std::string ColumnType<long long>::str() const
      {
        return AbstractColumnType::str() + " type: 'integer'";
      }

      template<>
      const mongo::BSONObj ColumnType<long long>::BSONObj() const
      {
        mongo::BSONObj super = AbstractColumnType::BSONObj();

        mongo::BSONObjBuilder builder;
        builder.appendElements(super);
        builder.append("column_type", "integer");

        return builder.obj();
      }

      template<>
      COLUMN_TYPES ColumnType<long long>::type() const
      {
        return COLUMN_INTEGER;
      }

      template <>
      bool ColumnType<double>::check(const std::string &verify) const
      {
        double d;
        return utils::string_to_double(verify, d);
      }

      template<>
      const std::string ColumnType<double>::str() const
      {
        return AbstractColumnType::str() + " type: 'double'";
      }

      template<>
      COLUMN_TYPES ColumnType<double>::type() const
      {
        return COLUMN_DOUBLE;
      }

      template<>
      const mongo::BSONObj ColumnType<double>::BSONObj() const
      {
        mongo::BSONObj super = AbstractColumnType::BSONObj();

        mongo::BSONObjBuilder builder;
        builder.appendElements(super);
        builder.append("column_type", "double");

        return builder.obj();
      }

      template<>
      bool ColumnType<std::string>::check(const std::string &verify) const
      {
        return true;
      }

      template<>
      const std::string ColumnType<std::string>::str() const
      {
        return AbstractColumnType::str() + " type: 'string'";
      }

      template<>
      COLUMN_TYPES ColumnType<std::string>::type() const
      {
        return COLUMN_STRING;
      }

      template<>
      const mongo::BSONObj ColumnType<std::string>::BSONObj() const
      {
        mongo::BSONObj super = AbstractColumnType::BSONObj();

        mongo::BSONObjBuilder builder;
        builder.appendElements(super);
        builder.append("column_type", "string");

        return builder.obj();
      }

      template<>
      bool ColumnType<Range>::check(const std::string &verify) const
      {
        double d;
        if (!utils::string_to_double(verify, d)) {
          return false;
        }
        return d >= _content.first && d <= _content.second;
      }

      template<>
      const std::string ColumnType<Range>::str() const
      {
        return AbstractColumnType::str() + " type: 'range' : "  +
               boost::lexical_cast<std::string>(_content.first) + "," +
               boost::lexical_cast<std::string>(_content.second);
      }

      template<>
      COLUMN_TYPES ColumnType<Range>::type() const
      {
        return COLUMN_RANGE;
      }

      template<>
      const mongo::BSONObj ColumnType<Range>::BSONObj() const
      {
        mongo::BSONObj super = AbstractColumnType::BSONObj();

        mongo::BSONObjBuilder builder;
        builder.appendElements(super);
        builder.append("column_type", "range");
        builder.append("minimum", _content.first);
        builder.append("maximum", _content.second);

        return builder.obj();
      }

      template<>
      bool ColumnType<Category>::check(const std::string &verify) const
      {
        return find(_content.begin(), _content.end(), verify) != _content.end();
      }

      template<>
      const std::string ColumnType<Category>::str() const
      {
        return AbstractColumnType::str() + " type: 'category' values: "  + utils::vector_to_string(_content) + "";
      }

      template<>
      COLUMN_TYPES ColumnType<Category>::type() const
      {
        return COLUMN_CATEGORY;
      }

      template<>
      const mongo::BSONObj ColumnType<Category>::BSONObj() const
      {
        mongo::BSONObj super = AbstractColumnType::BSONObj();

        mongo::BSONObjBuilder builder;
        builder.appendElements(super);
        builder.append("column_type", "range");
        mongo::BSONArray arr = helpers::build_array(_content);
        builder.append("values", arr);

        return builder.obj();
      }

      bool get_column_type(const std::string &type_name, COLUMN_TYPES type, std::string &msg)
      {
        if (type_name == "string") {
          type = COLUMN_STRING;
          return true;
        } else if (type_name == "integer") {
          type = COLUMN_INTEGER;
          return true;
        } else if (type_name == "double") {
          type = COLUMN_DOUBLE;
          return true;
        } else {
          msg = "Invalid column type '" + type_name + "'";
          return false;
        }
      }

      bool column_type_simple(const std::string &name, COLUMN_TYPES type, const std::string &default_value,
                              ColumnTypePtr &column_type, std::string &msg)
      {
        switch (type) {
        case COLUMN_STRING:
          column_type = boost::shared_ptr<ColumnType<std::string > >(new ColumnType<std::string>(name, "", default_value));
          return true;

        case COLUMN_INTEGER:
          column_type = boost::shared_ptr<ColumnType<size_t> >(new ColumnType<size_t>(name, 0, default_value));
          return true;

        case COLUMN_DOUBLE:
          column_type = boost::shared_ptr<ColumnType<double> >(new ColumnType<double>(name, 0.0, default_value));
          return true;

        case COLUMN_RANGE:
          msg = "Range Columns should be created first and them loaded.";
          return false;
        case COLUMN_CATEGORY:
          msg = "Category Columns should be created first and them loaded.";
          return false;

        default:
          msg = "Invalid column type: " +  boost::lexical_cast<std::string>(type);;
          return false;
        }
      }

      bool column_type_simple(const std::string &name, const std::string &type, const std::string &default_value,
                              ColumnTypePtr &column_type, std::string &msg)
      {
        std::string type_l(utils::lower(type));
        if (type_l == "string") {
          column_type = boost::shared_ptr<ColumnType<std::string > >(new ColumnType<std::string>(name, "", default_value));
          return true;
        } else if (type_l == "integer") {
          column_type = boost::shared_ptr<ColumnType<long long> >(new ColumnType<long long>(name, 0, default_value));
          return true;
        } else if (type_l == "double") {
          column_type = boost::shared_ptr<ColumnType<double> >(new ColumnType<double>(name, 0.0, default_value));
          return true;
        } else {
          msg = "Invalid column type '" + type_l + "'";
          return false;
        }
      }

      bool is_column_type_name_valid(const std::string &name, const std::string &norm_name,
                                     std::string &msg)
      {
        bool exists;
        if (!helpers::check_exist(Collections::COLUMN_TYPES(), "norm_name", norm_name, exists, msg)) {
          return false;
        }
        if (exists) {
          msg = "Column type '" + name + "' already exists.";
          return false;
        }
        return true;
      }

      bool __check_value_type(const std::string &type, std::string &msg)
      {
        if (type == "string" || type == "integer" || type == "double") {
          return true;
        }
        return false;
      }

      bool __create_column_base(const std::string &name, const std::string &norm_name,
                                const std::string &description, const std::string &norm_description,
                                const std::string &default_value, const std::string &type,
                                const std::string &user_key,
                                std::string &column_type_id, mongo::BSONObj &obj, std::string &msg)
      {
        int id;
        if (!helpers::get_counter(Collections::COLUMN_TYPES(), id, msg))  {
          return false;
        }
        column_type_id = "ct" + boost::lexical_cast<std::string>(id);

        mongo::BSONObjBuilder create_column_type_builder;
        create_column_type_builder.append("_id", column_type_id);
        create_column_type_builder.append("name", name);
        create_column_type_builder.append("norm_name", norm_name);
        create_column_type_builder.append("default_value", default_value);
        create_column_type_builder.append("description", description);
        create_column_type_builder.append("norm_description", norm_description);
        create_column_type_builder.append("column_type", type);

        std::string user_name;
        if (!get_user_name(user_key, user_name, msg)) {
          return false;
        }
        create_column_type_builder.append("user", user_name);

        obj = create_column_type_builder.obj();
        return true;

      }

      bool create_column_type_simple(const std::string &name, const std::string &norm_name,
                                     const std::string &description, const std::string &norm_description,
                                     const std::string &default_value, const std::string &type,
                                     const std::string &user_key,
                                     std::string &column_type_id, std::string &msg)
      {
        std::string norm_type = type;
        boost::algorithm::to_lower(norm_type);

        if (!__check_value_type(type, msg)) {
          msg = "The acceptable values for a column type are: string, integer, double";
          return false;
        }

        mongo::BSONObj obj;
        if (!__create_column_base(name, norm_name, description, norm_description,
                                  default_value, type, user_key, column_type_id, obj, msg)) {
          return false;
        }

        mongo::ScopedDbConnection c(config::get_mongodb_server());
        c->insert(helpers::collection_name(Collections::COLUMN_TYPES()), obj);
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!search::insert_full_text(Collections::COLUMN_TYPES(), column_type_id, obj, msg)) {
          c.done();
          return false;
        }

        c.done();

        return true;
      }

      bool create_column_type_category(const std::string &name, const std::string &norm_name,
                                       const std::string &description, const std::string &norm_description,
                                       const std::string &default_value, const std::vector<std::string> &items,
                                       const std::string &user_key,
                                       std::string &column_type_id, std::string &msg)
      {
        mongo::BSONObj obj;
        if (!__create_column_base(name, norm_name, description, norm_description,
                                  default_value, "category", user_key, column_type_id, obj, msg)) {
          return false;
        }

        mongo::BSONObjBuilder create_column_type_category_builder;
        create_column_type_category_builder.appendElements(obj);

        mongo::BSONArray arr = helpers::build_array(items);
        create_column_type_category_builder.append("items", arr);

        mongo::ScopedDbConnection c(config::get_mongodb_server());
        c->insert(helpers::collection_name(Collections::COLUMN_TYPES()), create_column_type_category_builder.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }

        if (!search::insert_full_text(Collections::COLUMN_TYPES(), column_type_id, obj, msg)) {
          c.done();
          return false;
        }

        c.done();

        return true;
      }

      bool create_column_type_range(const std::string &name, const std::string &norm_name,
                                    const std::string &description, const std::string &norm_description,
                                    const std::string &default_value,
                                    const double minimum, const double maximum,
                                    const std::string &user_key,
                                    std::string &column_type_id, std::string &msg)
      {
        mongo::BSONObj obj;
        if (!__create_column_base(name, norm_name, description, norm_description,
                                  default_value, "range", user_key, column_type_id, obj, msg)) {
          return false;
        }

        mongo::BSONObjBuilder create_column_type_range_builder;
        create_column_type_range_builder.appendElements(obj);

        create_column_type_range_builder.append("minimum", minimum);
        create_column_type_range_builder.append("maximum", maximum);

        mongo::ScopedDbConnection c(config::get_mongodb_server());
        c->insert(helpers::collection_name(Collections::COLUMN_TYPES()), create_column_type_range_builder.obj());
        if (!c->getLastError().empty()) {
          msg = c->getLastError();
          c.done();
          return false;
        }
        c.done();

        if (!search::insert_full_text(Collections::COLUMN_TYPES(), column_type_id, obj, msg)) {
          c.done();
          return false;
        }

        return true;
      }

      bool list_column_types(const std::string &user_key, std::vector<utils::IdName> &content, std::string  &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::COLUMN_TYPES()), mongo::BSONObj());

        while (data_cursor->more()) {
          mongo::BSONObj o = data_cursor->next().getOwned();
          ColumnTypePtr column_type;
          if (!column_type_bsonobj_to_class(o, column_type, msg))  {
            return false;
          }
          utils::IdName name(o["_id"].str(), column_type->str());
          content.push_back(name);
        }

        c.done();
        return true;
      }

      bool column_type_bsonobj_to_class(const mongo::BSONObj &obj, ColumnTypePtr &column_type, std::string &msg)
      {
        const std::string name = obj["name"].String();
        const std::string type = obj["column_type"].String();
        std::string default_value;
        if (obj.hasField("default_value")) {
          default_value = obj["default_value"].String();
        } else {
          default_value = "";
        }

        if (type == "string") {
          column_type = boost::shared_ptr<ColumnType<std::string > >(new ColumnType<std::string>(name, "", default_value));
          return true;
        } else if (type == "integer") {
          column_type = boost::shared_ptr<ColumnType<long long > >(new ColumnType<long long>(name, 0, default_value));
        } else if (type == "double") {
          column_type = boost::shared_ptr<ColumnType<double > >(new ColumnType<double>(name, 0.0, default_value));
        } else if (type == "category") {
          Category category;
          std::vector<mongo::BSONElement> e = obj["items"].Array();
          BOOST_FOREACH(const mongo::BSONElement & be, e) {
            category.push_back(be.str());
          }
          column_type = boost::shared_ptr<ColumnType<Category > >(new ColumnType<Category>(name, category, default_value));
        } else if (type == "range") {
          double minimum = obj["minimum"].Double();
          double maximum = obj["maximum"].Double();
          Range range(minimum, maximum);
          column_type = boost::shared_ptr<ColumnType<Range > >(new ColumnType<Range>(name, range, default_value));
        } else {
          msg = "Column type '" + type + "' is invalid";
          return false;
        }

        return true;
      }


      bool load_column_type(const std::string &name, mongo::BSONObj &obj_column_type, std::string &msg)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());
        const std::string norm_name = utils::normalize_name(name);
        mongo::BSONObj query = BSON("norm_name" << norm_name);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::COLUMN_TYPES()), query, 1);
        if (!data_cursor->more()) {
          msg = "Column type " + name + " not found";
          c.done();
          return false;
        }
        c.done();

        obj_column_type = data_cursor->next().getOwned();

        return true;
      }


      bool load_column_type(const std::string &name, ColumnTypePtr &column_type, std::string &msg)
      {
        mongo::BSONObj obj_column_type;
        if (!load_column_type(name, obj_column_type, msg)) {
          return false;
        }

        return column_type_bsonobj_to_class(obj_column_type, column_type, msg);
      }

      bool get_column_type(const std::string &id, std::map<std::string, std::string> &res, std::string &msg, bool full)
      {
        mongo::ScopedDbConnection c(config::get_mongodb_server());
        mongo::BSONObj query = BSON("_id" << id);
        std::auto_ptr<mongo::DBClientCursor> data_cursor = c->query(helpers::collection_name(Collections::COLUMN_TYPES()), query, 1);
        if (!data_cursor->more()) {
          msg = "Column type id " + id + " not found";
          c.done();
          return false;
        }

        ColumnTypePtr column_type;
        mongo::BSONObj o = data_cursor->next().getOwned();
        if (!column_type_bsonobj_to_class(o, column_type, msg)) {
          return false;
        }
        c.done();

        res["name"] = column_type->name();
        res["default_value"] = column_type->default_value();
        switch (column_type->type()) {
        case COLUMN_STRING: {
          res["column_type"] = "string";
          break;
        }

        case COLUMN_INTEGER: {
          res["column_type"] = "integer";
          break;
        }

        case COLUMN_DOUBLE: {
          res["column_type"] = "double";
          break;
        }

        case COLUMN_RANGE: {
          res["column_type"] = "range";
          ColumnType<Range> *column = static_cast<ColumnType<Range> *>(column_type.get());
          res["minimum"] = utils::integer_to_string(column->content().first);
          res["maximum"] = utils::integer_to_string(column->content().second);
          break;
        }

        case COLUMN_CATEGORY: {
          res["column_type"] = "category";
          ColumnType<Category> *column = static_cast<ColumnType<Category> *>(column_type.get());
          res["values"] = utils::vector_to_string(column->content());
          break;
        }

        case COLUMN_ERR: {
          msg = "Invalid column id: " + id;
          return false;
        }
        }

        return true;
      }
    }
  }
}