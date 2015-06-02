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

#include <mongo/bson/bson.h>

#include "key_mapper.hpp"

#include "../datatypes/column_types_def.hpp"
#include "../extras/utils.hpp"
#include "../lua/sandbox.hpp"
#include "../log.hpp"

namespace epidb {
  namespace dba {
    namespace columns {

      typedef std::pair<std::string, lua::Sandbox::LuaPtr> Code;
      typedef std::pair<Score, Score> Range;
      typedef std::vector<std::string> Category;

      class AbstractColumnType {

      protected:
        std::string _name;
        int _pos;

        AbstractColumnType(const std::string &n) :
          _name(n),
          _pos(-1)
        {


        }

        AbstractColumnType(const std::string &n, int pos) :
          _name(n),
          _pos(pos)
        { }

      public:
        virtual datatypes::COLUMN_TYPES type() const = 0;

        const std::string name() const
        {
          return _name;
        }

        int pos() const
        {
          return _pos;
        }

        virtual bool check(const std::string &verify) const
        {
          return false;
        }

        virtual bool execute(const std::string &chromosome,  const AbstractRegion *region, dba::Metafield &metafield, std::string &result, std::string &msg)
        {
          msg = "Execute method not implemented for this class: " + str();
          return false;
        }

        virtual const std::string str() const
        {
          return "column type name: '" + _name + "'";
        }

        virtual const mongo::BSONObj BSONObj() const
        {
          mongo::BSONObjBuilder builder;
          builder.append("name", _name);

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
        ColumnType(const std::string &n, const Type c, int pos) :
          AbstractColumnType(n, pos),
          _content(c)
        {}

        datatypes::COLUMN_TYPES type() const
        {
          return datatypes::COLUMN_ERR;
        }

        bool check(const std::string &verify) const
        {
          return AbstractColumnType::check(verify);
        }

        virtual bool execute(const std::string &chromosome, const AbstractRegion *region, dba::Metafield &metafield, std::string &result, std::string &msg)
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

        const Type content() const
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
      bool ColumnType<Score>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<Score>::str() const;

      template<>
      const mongo::BSONObj ColumnType<Score>::BSONObj() const;

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

      bool column_type_simple(const std::string &name, datatypes::COLUMN_TYPES type, ColumnTypePtr &column_type, std::string &msg);

      bool column_type_simple(const std::string &name, const std::string &type, ColumnTypePtr &column_type, std::string &msg);

      bool load_column_type(const std::string &name, mongo::BSONObj &obj_column_type, std::string &msg);

      bool load_column_type(const std::string &name, processing::StatusPtr status, ColumnTypePtr &column_type, std::string &msg);

      bool is_column_type_name_valid(const std::string &name, const std::string &norm_name,
                                     std::string &msg);

      bool create_column_type_simple(const std::string &name, const std::string &norm_name,
                                     const std::string &description, const std::string &norm_description,
                                     const std::string &type, const std::string &user_key,
                                     std::string &id, std::string &msg);

      bool create_column_type_category(const std::string &name, const std::string &norm_name,
                                       const std::string &description, const std::string &norm_description,
                                       const std::vector<std::string> &items, const std::string &user_key,
                                       std::string &id, std::string &msg);

      bool create_column_type_range(const std::string &name, const std::string &norm_name,
                                    const std::string &description, const std::string &norm_description,
                                    const Score minimum, const Score maximum,
                                    const std::string &user_key,
                                    std::string &id, std::string &msg);

      bool create_column_type_calculated(const std::string &name, const std::string &norm_name,
                                         const std::string &description, const std::string &norm_description,
                                         const std::string &code,
                                         const std::string &user_key,
                                         std::string &id, std::string &msg);

      bool column_type_bsonobj_to_class(const mongo::BSONObj &obj, processing::StatusPtr status, ColumnTypePtr &column_type, std::string &msg);

      bool get_column_type(const std::string &id, mongo::BSONObj &result, std::string &msg);

      bool get_column_type(const std::string &id, std::map<std::string, std::string> &res, std::string &msg);

      std::map<std::string, std::string> dataset_column_to_map(const mongo::BSONObj &o);
    }
  }
}

#endif /* defined(EPIDB_DBA_COLUMN_TYPES_HPP) */