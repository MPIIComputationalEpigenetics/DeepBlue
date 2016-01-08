//
//  sandbox.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 27.10.2014.
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

#ifndef EPIDB_LUA_SANDBOX_HPP
#define EPIDB_LUA_SANDBOX_HPP

#include <string>

#include <lua.hpp>

#include "../dba/metafield.hpp"

#include "../datatypes/regions.hpp"

namespace epidb {
  namespace lua {

    class Sandbox {

    private:
      lua_State *L;
      std::string error_msg;
      std::string &current_chromosome;
      const AbstractRegion *current_region_ptr;
      dba::Metafield &current_metafield;
      processing::StatusPtr status;

    public:
      typedef std::shared_ptr<Sandbox> LuaPtr;
      static LuaPtr new_instance(processing::StatusPtr status);

      Sandbox(processing::StatusPtr status);
      ~Sandbox();

      bool store_row_code(const std::string &code, std::string &msg);
      void set_current_context(const std::string &chromosome, const AbstractRegion * region, dba::Metafield &metafield);
      bool execute_row_code(std::string &value, std::string &msg) const;

      static int call_field_content(lua_State *L);

      int field_content(lua_State *L);

      static void MaximumInstructionsReached(lua_State *, lua_Debug *);
    };
  }
}

#endif