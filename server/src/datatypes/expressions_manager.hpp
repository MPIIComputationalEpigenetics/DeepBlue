//
//  expressions.hpp
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

#ifndef DATATYPES_EXPRESSIONS_MANAGER_HPP
#define DATATYPES_EXPRESSIONS_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>

#include "metadata.hpp"
#include "regions.hpp"

#include "expressions.hpp"
#include "gene_expressions.hpp"

#include "../log.hpp"

namespace epidb {
  namespace datatypes {

    class ExpressionManager {

    private:
      static std::vector<ExpressionTypePtr> __registered;

      bool register_expression_type(ExpressionTypePtr expression_type);

      ExpressionManager()
      {
        ExpressionTypePtr etp = std::unique_ptr<GeneExpressionType>(new GeneExpressionType());
        EPIDB_LOG(etp->name() << " initialized");
        register_expression_type(std::move(etp));
      }

    public:
      bool is_expression_type(const std::string& name) const;
      const std::vector<ExpressionTypePtr>& registered_expression_types() const;

      const ExpressionTypePtr& get_manager(const std::string& name) const ;

      const ExpressionTypePtr& GENE_EXPRESSION() const;

      static const ExpressionManager* INSTANCE();
    };


  }
}

#endif