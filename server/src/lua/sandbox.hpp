//
//  sandbox.hpp
//  epidb
//
//  Created by Felipe Albrecht on 27.10.2014.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
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

    static const char *LUA_ENV =
      "-- sample sandbox environment\n"
      "sandbox_env = {\n"
      " print = print,\n"
      " value_of = value_of,\n"
      " ipairs = ipairs,\n"
      " next = next,\n"
      " pairs = pairs,\n"
      " pcall = pcall,\n"
      " tonumber = tonumber,\n"
      " tostring = tostring,\n"
      " type = type,\n"
      " unpack = unpack,\n"
      " string = { byte = string.byte, char = string.char, find = string.find,\n"
      "   format = string.format, gmatch = string.gmatch, gsub = string.gsub,\n"
      "   len = string.len, lower = string.lower, match = string.match,\n"
      "   rep = string.rep, reverse = string.reverse, sub = string.sub,\n"
      "   upper = string.upper },\n"
      " table = { insert = table.insert, maxn = table.maxn, remove = table.remove,\n"
      " sort = table.sort },\n"
      " math = { abs = math.abs, acos = math.acos, asin = math.asin,\n"
      " atan = math.atan, atan2 = math.atan2, ceil = math.ceil, cos = math.cos,\n"
      " cosh = math.cosh, deg = math.deg, exp = math.exp, floor = math.floor,\n"
      " fmod = math.fmod, frexp = math.frexp, huge = math.huge,\n"
      " ldexp = math.ldexp, log = math.log, log10 = math.log10, max = math.max,\n"
      " min = math.min, modf = math.modf, pi = math.pi, pow = math.pow,\n"
      " rad = math.rad, random = math.random, sin = math.sin, sinh = math.sinh,\n"
      " sqrt = math.sqrt, tan = math.tan, tanh = math.tanh },\n"
      " os = { clock = os.clock, difftime = os.difftime, time = os.time },\n"
      "}\n";


    static const char *LUA_FUNCTION_HEADER = "function row_value()\n";

    static const char *LUA_FUNCTION_FOOTER = "\nend";

    static const char *LUA_SET_SANDBOX = "setfenv(row_value, sandbox_env)\n";
  }
}

#endif