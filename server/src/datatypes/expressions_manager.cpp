//
//  expressions_manager.hpp
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

#include <algorithm>
#include <string>
#include <vector>

#include "../log.hpp"

#include "expressions_manager.hpp"
#include "expressions.hpp"

namespace epidb {
  namespace datatypes {

    const ExpressionManager* ExpressionManager::INSTANCE()
    {
      static ExpressionManager* __expression_manager = new ExpressionManager();
      return __expression_manager;
    }

    std::vector<ExpressionTypePtr> ExpressionManager::__registered;

    const ExpressionTypePtr& ExpressionManager::GENE_EXPRESSION() const
    {
      static const ExpressionTypePtr& etp_ref = ExpressionManager::get_manager("gene");
      return etp_ref;
    }

    bool ExpressionManager::register_expression_type(ExpressionTypePtr expression_type)
    {
      __registered.emplace_back(std::move(expression_type));
      return true;
    }

    bool ExpressionManager::is_expression_type(const std::string& name) const
    {
      return std::find_if(__registered.begin(), __registered.end(),
      [&name](ExpressionTypePtr const & et) {
        return et->name() == name;
      }) != __registered.end();
    }

    const std::vector<ExpressionTypePtr>& ExpressionManager::registered_expression_types() const
    {
      return __registered;
    }

    const ExpressionTypePtr& ExpressionManager::get_manager(const std::string& name) const
    {
      auto etp = std::find_if(__registered.begin(), __registered.end(),
      [&name](ExpressionTypePtr const & et) {
        return et->name() == name;
      });

      if (etp == __registered.end()) {
        return nullptr;
      }
      return *etp;
    }
  }
}
