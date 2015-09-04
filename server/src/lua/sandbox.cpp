//
//  sandbox.cpp
//  epidb
//
//  Created by Felipe Albrecht on 27.10.2014.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <iostream>
#include <string>

#include <lua.hpp>

#include <memory>

#include "sandbox.hpp"

#include "../datatypes/column_types_def.hpp"
#include "../datatypes/regions.hpp"

#include "../dba/experiments.hpp"

#include "../extras/utils.hpp"

#include "../log.hpp"

namespace epidb {
  namespace lua {


    static std::string EMPTY_CHROMOSOME;
    static dba::Metafield EMPTY_METAFIELD;

    Sandbox::LuaPtr Sandbox::new_instance(processing::StatusPtr status)
    {
      return std::shared_ptr<Sandbox>(new Sandbox(status));
    }

    Sandbox::Sandbox(processing::StatusPtr status) :
      L(luaL_newstate()),
      current_chromosome(EMPTY_CHROMOSOME),
      current_region_ptr(nullptr),
      current_metafield(EMPTY_METAFIELD),
      status(status)
    {
      luaL_openlibs(L);

      lua_pushlightuserdata(L, this);
      lua_pushcclosure(L, &Sandbox::call_field_content, 1);
      lua_setglobal(L, "value_of");
    }

    Sandbox::~Sandbox()
    {
      lua_close(L);
    }

    bool Sandbox::store_row_code(const std::string &code, std::string &msg)
    {
      if (luaL_loadstring(L, epidb::lua::LUA_ENV) || lua_pcall(L, 0, 0, 0)) {
        msg = std::string(lua_tostring(L, -1));
        return false;
      }

      std::string full_function = LUA_FUNCTION_HEADER + code + LUA_FUNCTION_FOOTER;

      if (luaL_loadstring(L, full_function.c_str()) || lua_pcall(L, 0, 0, 0)) {
        msg = std::string(lua_tostring(L, -1));
        return false;
      }

      if (luaL_loadstring(L, LUA_SET_SANDBOX) || lua_pcall(L, 0, 0, 0)) {
        msg = std::string(lua_tostring(L, -1));
        return false;
      }

      return true;
    }

    void Sandbox::set_current_context(const std::string &chromosome, const AbstractRegion *region, dba::Metafield &metafield)
    {
      current_chromosome = chromosome;
      current_region_ptr = region;
      current_metafield = metafield;
    }

    bool Sandbox::execute_row_code(std::string &value,  std::string &msg) const
    {
      lua_sethook(L, &Sandbox::MaximumInstructionsReached, LUA_MASKCOUNT, 1000);
      lua_getglobal(L, "row_value");
      if (lua_pcall(L, 0, 1, 0)) {
        msg = lua_tostring(L, -1);
        return false;
      }
      lua_sethook(L, &Sandbox::MaximumInstructionsReached, 0, 0);

      if (lua_isnumber(L, -1)) {
        double z = lua_tonumber(L, -1);
        lua_pop(L, 1);
        value = std::to_string(z);
        return true;
      } else if (lua_isstring(L, -1)) {
        value = lua_tostring(L, -1);
        lua_pop(L, 1);
        return true;
      }
      msg = "Invalid return type from Lua code";

      return false;
    }

    int Sandbox::call_field_content(lua_State *lua_state)
    {
      Sandbox *sandbox = static_cast<Sandbox *>(lua_touserdata(lua_state, lua_upvalueindex(1)));
      return sandbox->field_content(lua_state);
    }

    int Sandbox::field_content(lua_State *lua_state)
    {
      int n = lua_gettop(lua_state);
      if (n != 1) {
        lua_pushstring(lua_state, "invalid number of arguments for the content() function");
      }
      const char *field_name_c = lua_tostring(lua_state, -1);

      const std::string field_name(field_name_c);

      std::string msg;
      if (dba::Metafield::is_meta(field_name)) {
        std::string result;
        if (!current_metafield.process(field_name, current_chromosome, current_region_ptr, status, result, msg)) {
          lua_pushstring(lua_state, msg.c_str());
          return 1;
        }
        std::string type = dba::Metafield::command_type(field_name);
        if (type == "string") {
          lua_pushstring(lua_state, result.c_str());
          return 1;
        } else {
          Score s;
          utils::string_to_score(result, s);
          lua_pushnumber(lua_state, s);
          return 1;
        }
      }

      if (field_name == "CHROMOSOME") {
        lua_pushstring(lua_state, current_chromosome.c_str());
        return 1;
      } else if (field_name == "START") {
        lua_pushnumber(lua_state, current_region_ptr->start());
        return 1;
      } else if (field_name == "END") {
        lua_pushnumber(lua_state, current_region_ptr->end());
        return 1;
      }

      DatasetId dataset_id = current_region_ptr->dataset_id();
      dba::columns::ColumnTypePtr column;

      // TODO: better error handling
      if (!dba::experiments::get_field_pos(dataset_id, field_name, column, msg)) {
        lua_pushstring(lua_state, "");
        return 1;
      }

      if (column->type() == datatypes::COLUMN_STRING || column->type() == datatypes::COLUMN_CATEGORY) {
        std::string content = current_region_ptr->get_string(column->pos());
        if (content.length() > 0) {
          lua_pushstring(lua_state, content.c_str());
          return 1;
        }

      } else if (column->type() == datatypes::COLUMN_INTEGER || column->type() == datatypes::COLUMN_DOUBLE || column->type() == datatypes::COLUMN_RANGE) {
        Score value = current_region_ptr->value(column->pos());
        if (value != std::numeric_limits<Score>::min()) {
          lua_pushnumber(lua_state, value);
          return 1;
        }
      }

      lua_pushstring(lua_state, "");
      return 1;
    }

    void Sandbox::MaximumInstructionsReached(lua_State *lua_state, lua_Debug *)
    {
      luaL_error(lua_state, "The maximum number of instructions has been reached");
    }
  }
}
