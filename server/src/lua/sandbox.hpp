//
//  sandbox.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.10.2014.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
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