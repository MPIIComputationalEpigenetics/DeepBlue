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

#include "../extras/utils.hpp"

namespace epidb {
  namespace dba {
    namespace columns {

      enum COLUMN_TYPES {
        COLUMN_STRING,
        COLUMN_INTEGER,
        COLUMN_DOUBLE,
        COLUMN_RANGE,
        COLUMN_CATEGORY,
        COLUMN_ERR,
      };

      typedef std::string Value;
      typedef std::pair<double, double> Range;
      typedef std::vector<std::string> Category;

      class AbstractColumnType {

      protected:
        std::string _name;
        std::string _ignore_if;
        bool no_ignore_if;

        AbstractColumnType(const std::string n, const std::string i) :
          _name(n),
          _ignore_if(i)
        {
          no_ignore_if = i.empty();
        }

      public:
        virtual COLUMN_TYPES type() const = 0;

        const std::string name()
        {
          return _name;
        }

        virtual bool check(const Value &verify) const
        {
          return false;
        }

        bool ignore(const Value &verify) const
        {
          if (no_ignore_if) {
            return false;
          }
          return (_ignore_if == "*") || (verify == _ignore_if);
        }

        virtual const std::string str() const
        {
          return "column type name: '" + _name + "' ignore if: '" + _ignore_if + "'";
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

        const std::string str() const
        {
          return AbstractColumnType::str();
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
      bool ColumnType<double>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<double>::str() const;

      template<>
      bool ColumnType<std::string>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<std::string>::str() const;

      template<>
      bool ColumnType<Range>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<Range>::str() const;

      template<>
      bool ColumnType<Category>::check(const std::string &verify) const;

      template<>
      const std::string ColumnType<Category>::str() const;

      bool list_column_types(const std::string &user_key, std::vector<utils::IdName> &content, std::string  &msg);

      bool column_type_simple(const std::string &name, COLUMN_TYPES type, const std::string &ignore_if,
                              ColumnTypePtr &column_type, std::string &msg);

      bool column_type_simple(const std::string &name, COLUMN_TYPES type,
                              ColumnTypePtr &column_type, std::string &msg);

      bool column_type_simple(const std::string &name, const std::string &type, const std::string &ignore_if,
                              ColumnTypePtr &column_type, std::string &msg);

      bool column_type_simple(const std::string &name, const std::string &type,
                              ColumnTypePtr &column_type, std::string &msg);

      bool load_column_type(const std::string &name, ColumnTypePtr &column_type, std::string &msg);

      bool is_column_type_name_valid(const std::string &name, const std::string &norm_name,
                                     std::string &msg);

      bool create_column_type_simple(const std::string &name, const std::string &norm_name,
                                     const std::string &description, const std::string &norm_description,
                                     const std::string &ignore_if, const std::string &type,
                                     const std::string &user_key,
                                     std::string &id, std::string &msg);

      bool create_column_type_category(const std::string &name, const std::string &norm_name,
                                       const std::string &description, const std::string &norm_description,
                                       const std::string &ignore_if, const std::vector<std::string> &items,
                                       const std::string &user_key,
                                       std::string &id, std::string &msg);

      bool create_column_type_range(const std::string &name, const std::string &norm_name,
                                    const std::string &description, const std::string &norm_description,
                                    const std::string &ignore_if,
                                    const double minimum, const double maximum,
                                    const std::string &user_key,
                                    std::string &id, std::string &msg);

      bool column_type_bsonobj_to_class(const mongo::BSONObj &obj, ColumnTypePtr &column_type, std::string &msg);
    }
  }
}

#endif /* defined(EPIDB_DBA_COLUMN_TYPES()_HPP) */