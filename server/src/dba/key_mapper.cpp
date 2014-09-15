//
//  key_mapper.cpp
//  epidb
//
//  Created by Fabian Reinartz on 01.07.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "key_mapper.hpp"
#include "config.hpp"
#include "helpers.hpp"
#include "collections.hpp"
#include "../log.hpp"


namespace epidb {
  namespace dba {

    std::map<std::string, std::string> KeyMapper::stol_;
    std::map<std::string, std::string> KeyMapper::ltos_;

    boost::mutex KeyMapper::read_database_mutex;
    boost::mutex KeyMapper::set_shortcut_mutex;

    bool KeyMapper::loaded_ = false;


    bool KeyMapper::read_database()
    {
      boost::mutex::scoped_lock scoped_lock(read_database_mutex);
      if (loaded_)
        return true;

      mongo::ScopedDbConnection c(config::get_mongodb_server());

      mongo::BSONObjBuilder index;
      index.append("s", 1);
      c->ensureIndex(helpers::collection_name(Collections::KEY_MAPPER()), index.obj(), true);

      std::auto_ptr<mongo::DBClientCursor> cursor =
        c->query(helpers::collection_name(Collections::KEY_MAPPER()), mongo::BSONObj());

      if (!(c->getLastErrorDetailed().getField("err").isNull())) {
        c.done();
        loaded_ = false;
        return false;
      }

      while (cursor->more()) {
        mongo::BSONObj e = cursor->next();
        const std::string l = e.getStringField("_id");
        const std::string s = e.getStringField("s");
        stol_[s] = l;
        ltos_[l] = s;
      }
      c.done();
      loaded_ = true;
      return true;
    }

    bool KeyMapper::set_shortcut(const std::string &s, const std::string &l, std::string &msg)
    {
      boost::mutex::scoped_lock scoped_lock(set_shortcut_mutex);

      // Check if the value was not included were was waiting for the mutex
      std::map<std::string, std::string>::iterator it = ltos_.find(l);
      if (it != ltos_.end()) {
        return true;
      }

      mongo::ScopedDbConnection c(config::get_mongodb_server());

      mongo::BSONObjBuilder b;
      b.append("_id", l);
      b.append("s", s);
      c->insert(helpers::collection_name(Collections::KEY_MAPPER()), b.obj());
      // TODO: handle error
      mongo::BSONObj err = c->getLastErrorDetailed();
      c.done();

      if (!err.getField("err").isNull()) {
        msg = err.toString();
        return false;
      }
      stol_[s] = l;
      ltos_[l] = s;
      return true;
    }

    const std::string &KeyMapper::BED_COMPRESSED()
    {
      static std::string BED_COMPRESSED = epidb::dba::KeyMapper::build_default("BED_COMPRESSED");
      return BED_COMPRESSED;
    }

    const std::string &KeyMapper::BED_DATA()
    {
      static std::string BED_DATA = epidb::dba::KeyMapper::build_default("BED_DATA");
      return BED_DATA;
    }

    const std::string &KeyMapper::BED_DATASIZE()
    {
      static std::string BED_DATA = epidb::dba::KeyMapper::build_default("BED_DATASIZE");
      return BED_DATA;
    }

    const std::string &KeyMapper::DATASET()
    {
      static std::string DATASET = epidb::dba::KeyMapper::build_default("DATASET");
      return DATASET;
    }

    const std::string &KeyMapper::CHROMOSOME()
    {
      static std::string CHROMOSOME = epidb::dba::KeyMapper::build_default("CHROMOSOME");
      return CHROMOSOME;
    }

    const std::string &KeyMapper::START()
    {
      static std::string START = epidb::dba::KeyMapper::build_default("START");
      return START;
    }

    const std::string &KeyMapper::END()
    {
      static std::string END = epidb::dba::KeyMapper::build_default("END");
      return END;
    }

    const std::string &KeyMapper::FEATURES()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("FEATURES");
      return VALUE;
    }

    const std::string &KeyMapper::VALUE()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("VALUE");
      return VALUE;
    }


    const std::string &KeyMapper::WIG_TYPE()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("TYPE_WIG");
      return VALUE;
    }

    const std::string &KeyMapper::WIG_STEP()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("STEP_WIG");
      return VALUE;
    }

    const std::string &KeyMapper::WIG_SPAN()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("SPAN_WIG");
      return VALUE;
    }

    const std::string &KeyMapper::WIG_DATA_SIZE()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("DATA_SIZE_WIG");
      return VALUE;
    }

    const std::string &KeyMapper::WIG_COMPRESSED()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("COMPRESSED_DATA");
      return VALUE;
    }

    const std::string &KeyMapper::WIG_TRACK_TYPE()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("TRACK_TYPE_WIG");
      return VALUE;
    }

    const std::string &KeyMapper::WIG_DATA()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("DATA_WIG");
      return VALUE;
    }

    std::string KeyMapper::build_default(const std::string &s)
    {
      std::string r;
      std::string err;
      if (!to_short(s, r, err)) {
        EPIDB_LOG_ERR(err);
      }
      return r;
    }

    bool KeyMapper::to_short(const std::string &l, std::string &res, std::string &err)
    {
      if (l.compare("_id") == 0) {
        res = l;
        return true;
      }

      if (!loaded_) {
        if (!read_database()) {
          err = "Unable to read key mappings from database.";
          return false;
        }
      }

      std::map<std::string, std::string>::iterator it = ltos_.find(l);
      if (it != ltos_.end()) {
        res = it->second;
        return true;
      }

      std::string sk;
      sk = l[0];
      int i = 0;

      // Try to use some letter from the name
      size_t pos(0);
      while (!(stol_.find(sk) == stol_.end()) && (pos < l.size() - 1)) {
        sk = l[pos++];
      }

      // If not found useful letter, insert number
      if (stol_.find(sk) != stol_.end()) {
        while (!(stol_.find(sk) == stol_.end())) {
          sk = l[0] + boost::lexical_cast<std::string>(i++);
        }
      }

      if (set_shortcut(sk, l, err)) {
        res = sk;
        return true;
      }
      err = "Unable to set shortcut: " + err;
      return false;
    }

    bool KeyMapper::to_long(const std::string &s, std::string &res, std::string &err)
    {
      if (s.compare("_id") == 0) {
        res = s;
        return true;
      }

      if (!loaded_)
        read_database();

      std::map<std::string, std::string>::iterator it = stol_.find(s);
      if (it != stol_.end()) {
        res = it->second;
        return true;
      }
      err = "No mapping found for key '" + s + "'";
      return false;
    }

  } // namespace dba
} // namespace epidb