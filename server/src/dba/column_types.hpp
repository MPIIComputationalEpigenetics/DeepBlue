//
//  column_types.hpp
//  epidb
//
//  Created by Felipe Albrecht on 03.02.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_DBA_COLUMN_TYPES_HPP
#define EPIDB_DBA_COLUMN_TYPES_HPP

#include <string>
#include <vector>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

#include <mongo/bson/bson.h>

#include "key_mapper.hpp"

#include "../extras/utils.hpp"

#include "../lua/sandbox.hpp"

#include "../log.hpp"

namespace epidb {
  namespace dba {
    namespace columns {

      enum COLUMN_TYPES {
        COLUMN_STRING,
        COLUMN_INTEGER,
        COLUMN_DOUBLE,
        COLUMN_RANGE,
        COLUMN_CATEGORY,
        COLUMN_CALCULATED,
        COLUMN_ERR,
      };

      typedef std::string Token;
      typedef std::pair<std::string, lua::Sandbox::LuaPtr> Code;
      typedef std::pair<double, double> Range;
      typedef std::vector<std::string> Category;

      class AbstractColumnType {

      protected:
        std::string _name;
        std::string _default_value;
        bool _has_internal_name;
        std::string _internal_name;

        AbstractColumnType(const std::string n, const std::string i) :
          _name(n),
          _default_value(i),
          _has_internal_name(false),
          _internal_name()
        {}

      public:
        virtual COLUMN_TYPES type() const = 0;

        const std::string name()
        {
          return _name;
        }

        const std::string internal_name()
        {
          if (!_has_internal_name) {
            std::string msg;
            if (!dba::KeyMapper::to_short(_name, _internal_name, msg)) {
              EPIDB_LOG_ERR(msg);
              _internal_name = "There was not possible to load " + _name;
            }
            _has_internal_name = true;
          }

          return _internal_name;
        }

        const std::string default_value()
        {
          return _default_value;
        }

        virtual bool check(const Token &verify) const
        {
          return false;
        }

        bool ignore(const Token &verify) const
        {
          return _default_value == verify;
        }

        virtual bool execute(const std::string& chromosome, const Region& region, dba::Metafield &metafield,std::string& result, std::string& msg)
        {
          msg = "Execute method not implemented for this class: " + str();
          return false;
        }

        virtual const std::string str() const
        {
          return "column type name: '" + _name + "' default: '" + _default_value + "'";
        }

        virtual const mongo::BSONObj BSONObj() const
        {
          mongo::BSONObjBuilder builder;
          builder.append("name", _name);
          builder.append("default_value", _default_value);

          return builder.obj();
        }

        virtual ~AbstractColumnType() {}
      };

      typedef boost::shared_ptr<AbstractColumnType> ColumnTypePtr;

      template< class Type>
      class ColumnType : public AbstractColumnType {
      private:
        Type _content;

      public:
        ColumnType(const std::string &n, const Type c, const std::string &i) :
          AbstractColumnType(n, i),
          _content(c)
        {}

        COLUMN_TYPES type() const
        {
          return COLUMN_ERR;
        }

        bool check(const std::string &verify) const
        {
          return AbstractColumnType::check(verify);
        }

        bool execute(const std::string& chromosome, const Region &region, dba::Metafield &metafield, std::string &result, std::string &msg)
        {
          return AbstractColumnType::execute(chromosome, region, metafield, result, msg);
        }

        const std::string str() const
        {
          return AbstractColumnType::str();
        }

        virtual const mongo::BSONObj BSONObj() const
        {
          return AbstractColumnType::BSONObj();
        }

        const Type content()
        {
          return _content;
        }

        std::ostream &operator<<(std::ostream &str)
        {
          str << &this->str();
          return str;
        }
      };

      template<>
      bool ColumnType<long long>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<long long>::str() const;

      template<>
      const mongo::BSONObj ColumnType<long long>::BSONObj() const;

      template<>
      bool ColumnType<double>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<double>::str() const;

      template<>
      const mongo::BSONObj ColumnType<double>::BSONObj() const;

      template<>
      bool ColumnType<std::string>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<std::string>::str() const;

      template<>
      const mongo::BSONObj ColumnType<std::string>::BSONObj() const;

      template<>
      bool ColumnType<Range>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<Range>::str() const;

      template<>
      const mongo::BSONObj ColumnType<Range>::BSONObj() const;

      template<>
      bool ColumnType<Category>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<Category>::str() const;

      template<>
      bool ColumnType<Code>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<Code>::str() const;

      template<>
      const mongo::BSONObj ColumnType<Code>::BSONObj() const;

      bool list_column_types(const std::string &user_key, std::vector<utils::IdName> &content, std::string  &msg);

      bool column_type_simple(const std::string &name, COLUMN_TYPES type, const std::string &default_value,
                              ColumnTypePtr &column_type, std::string &msg);

      bool column_type_simple(const std::string &name, const std::string &type, const std::string &default_value,
                              ColumnTypePtr &column_type, std::string &msg);

      bool load_column_type(const std::string &name, mongo::BSONObj &obj_column_type, std::string &msg);

      bool load_column_type(const std::string &name, ColumnTypePtr &column_type, std::string &msg);

      bool is_column_type_name_valid(const std::string &name, const std::string &norm_name,
                                     std::string &msg);

      bool create_column_type_simple(const std::string &name, const std::string &norm_name,
                                     const std::string &description, const std::string &norm_description,
                                     const std::string &default_value, const std::string &type,
                                     const std::string &user_key,
                                     std::string &id, std::string &msg);

      bool create_column_type_category(const std::string &name, const std::string &norm_name,
                                       const std::string &description, const std::string &norm_description,
                                       const std::string &default_value, const std::vector<std::string> &items,
                                       const std::string &user_key,
                                       std::string &id, std::string &msg);

      bool create_column_type_range(const std::string &name, const std::string &norm_name,
                                    const std::string &description, const std::string &norm_description,
                                    const std::string &default_value,
                                    const double minimum, const double maximum,
                                    const std::string &user_key,
                                    std::string &id, std::string &msg);

      bool create_column_type_calculated(const std::string &name, const std::string &norm_name,
                                         const std::string &description, const std::string &norm_description,
                                         const std::string &code,
                                         const std::string &user_key,
                                         std::string &id, std::string &msg);

      bool column_type_bsonobj_to_class(const mongo::BSONObj &obj, ColumnTypePtr &column_type, std::string &msg);

      bool get_column_type(const std::string &id, mongo::BSONObj& result, std::string &msg);

      bool get_column_type(const std::string &id, std::map<std::string, std::string> &res, std::string &msg);

      std::map<std::string, std::string> dataset_column_to_map(const mongo::BSONObj &o);
    }
  }
}

#endif /* defined(EPIDB_DBA_COLUMN_TYPES()_HPP) */