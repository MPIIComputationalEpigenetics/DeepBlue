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

#include <boost/shared_ptr.hpp>

#include "sandbox.hpp"

#include "../regions.hpp"

namespace epidb {
  namespace lua {


    Sandbox::LuaPtr Sandbox::new_instance()
    {
      return boost::shared_ptr<Sandbox>(new Sandbox());
    }

    Sandbox::Sandbox()
    {
      L = luaL_newstate();
      luaL_openlibs(L);
      if (luaL_loadstring(L, epidb::lua::LUA_ENV) || lua_pcall(L, 0, 0, 0)) {
        std::string msg = std::string(lua_tostring(L, -1));
        std::cerr << msg << std::endl;
      }
      current_region = nullptr;
    }

    Sandbox::~Sandbox()
    {
      lua_close(L);
    }

    bool Sandbox::store_row_code(const std::string &code, std::string &msg)
    {
      std::string full_function = LUA_FUNCTION_HEADER + code + LUA_FUNCTION_FOOTER;

      if (luaL_loadstring(L, full_function.c_str()) || lua_pcall(L, 0, 0, 0)) {
        msg = std::string(lua_tostring(L, -1));
        return false;
      }

      if (luaL_loadstring(L, LUA_SET_SANDBOX) || lua_pcall(L, 0, 0, 0)) {
        msg = std::string(lua_tostring(L, -1));
        return false;
      }

      lua_pushlightuserdata(L, this);
      lua_pushcclosure(L, &Sandbox::call_field_content, 1);
      lua_setglobal(L, "field_content");
      return true;
    }

    bool Sandbox::execute_row_code(std::string &value, const Region *region, std::string &msg)
    {
      current_region = region;

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

      msg = "Invalid return type from the Lua code";

      return false;
    }

    int Sandbox::call_field_content(lua_State *lua_state)
    {
      Sandbox *sandbox = (Sandbox *) lua_touserdata(lua_state, lua_upvalueindex(1));
      return sandbox->field_content(lua_state);
    }

    int Sandbox::field_content(lua_State *lua_state)
    {
      int n = lua_gettop(lua_state);
      if (n != 1) {
        lua_pushstring(lua_state, "invalid number of arguments for the content() function");
      }
      const char *field_name = lua_tostring(lua_state, -1);

      std::string content = current_region->get(std::string(field_name));
      if (content.length() > 0) {
        lua_pushstring(lua_state, content.c_str());
        return 1;
      }

      Score value = current_region->value(field_name);
      if (value != std::numeric_limits<Score>::min()) {
        lua_pushnumber(lua_state, value);
        return 1;
      }

      lua_pushstring(lua_state, "");
      return 1;
    }

    void Sandbox::MaximumInstructionsReached(lua_State *, lua_Debug *)
    {
      std::cerr <<  "The maximum number of instructions has been reached" << std::endl;
    }
  }
}
