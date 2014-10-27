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

      return true;
    }


    bool Sandbox::execute_row_code(std::string &value, std::string &msg)
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

      msg = "Invalid return type from the Lua code";

      return false;
    }

    void Sandbox::MaximumInstructionsReached(lua_State *, lua_Debug *)
    {
      std::cerr <<  "The maximum number of instructions has been reached" << std::endl;
    }
  }
}

/*
int main (void)
{
  char buff[256];
  int error;
  epidb::lua::Sandbox::LuaPtr lua_ptr = epidb::lua::Sandbox::new_instance();
  std::string msg;
  std::string value;

  if (!lua_ptr->store_row_code("value = 1312 return math.log(value)", msg)) {
    std::cerr << msg << std::endl;
    return -1;
  }
  if (!lua_ptr->execute_row_code(value, msg)) {
    std::cerr << msg << std::endl;
    return -2;
  }
  std::cerr << value << std::endl;
  return 0;
}
*/
