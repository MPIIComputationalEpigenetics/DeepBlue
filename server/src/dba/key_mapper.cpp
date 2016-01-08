//
//  key_mapper.cpp
//  epidb
//
//  Created by Fabian Reinartz on 01.07.13.
//  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
//

#include <string>

#include <boost/thread/mutex.hpp>

#include <mongo/bson/bson.h>

#include "helpers.hpp"
#include "key_mapper.hpp"
#include "collections.hpp"

#include "../connection/connection.hpp"

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

      Connection c;

      mongo::BSONObjBuilder index;
      index.append("s", 1);
      c->createIndex(helpers::collection_name(Collections::KEY_MAPPER()), index.obj());

      auto cursor =
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

      Connection c;

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
      static std::string FEATURES = epidb::dba::KeyMapper::build_default("FEATURES");
      return FEATURES;
    }

    const std::string &KeyMapper::VALUE()
    {
      static std::string VALUE = epidb::dba::KeyMapper::build_default("VALUE");
      return VALUE;
    }


    const std::string &KeyMapper::WIG_STEP()
    {
      static std::string WIG_STEP = epidb::dba::KeyMapper::build_default("STEP_WIG");
      return WIG_STEP;
    }

    const std::string &KeyMapper::WIG_SPAN()
    {
      static std::string WIG_SPAN = epidb::dba::KeyMapper::build_default("SPAN_WIG");
      return WIG_SPAN;
    }

    const std::string &KeyMapper::WIG_DATA_SIZE()
    {
      static std::string WIG_DATA_SIZE = epidb::dba::KeyMapper::build_default("DATA_SIZE_WIG");
      return WIG_DATA_SIZE;
    }

    const std::string &KeyMapper::WIG_COMPRESSED()
    {
      static std::string WIG_COMPRESSED = epidb::dba::KeyMapper::build_default("COMPRESSED_DATA");
      return WIG_COMPRESSED;
    }

    const std::string &KeyMapper::WIG_TRACK_TYPE()
    {
      static std::string WIG_TRACK_TYPE = epidb::dba::KeyMapper::build_default("TRACK_TYPE_WIG");
      return WIG_TRACK_TYPE;
    }

    const std::string &KeyMapper::WIG_DATA()
    {
      static std::string WIG_DATA = epidb::dba::KeyMapper::build_default("DATA_WIG");
      return WIG_DATA;
    }

    const std::string& KeyMapper::SEQNAME()
    {
      static std::string SEQNAME = epidb::dba::KeyMapper::build_default("SEQNAME");
      return SEQNAME;
    }

    const std::string& KeyMapper::SOURCE()
    {
      static std::string SOURCE = epidb::dba::KeyMapper::build_default("SOURCE");
      return SOURCE;
    }

    const std::string& KeyMapper::CHROMOSOME()
    {
      static std::string CHROMOSOME = epidb::dba::KeyMapper::build_default("CHROMOSOME");
      return CHROMOSOME;
    }

    const std::string& KeyMapper::FEATURE()
    {
      static std::string FEATURE = epidb::dba::KeyMapper::build_default("DATA_WIG");
      return FEATURE;
    }
    const std::string& KeyMapper::SCORE()
    {
      static std::string SCORE = epidb::dba::KeyMapper::build_default("SCORE");
      return SCORE;
    }

    const std::string& KeyMapper::STRAND()
    {
      static std::string STRAND = epidb::dba::KeyMapper::build_default("STRAND");
      return STRAND;
    }

    const std::string& KeyMapper::FRAME()
    {
      static std::string FRAME = epidb::dba::KeyMapper::build_default("FRAME");
      return FRAME;
    }

    const std::string& KeyMapper::ATTRIBUTES()
    {
      static std::string ATTRIBUTES = epidb::dba::KeyMapper::build_default("ATTRIBUTES");
      return ATTRIBUTES;
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

      // Try to use some letter from the name
      size_t pos(0);
      while (!(stol_.find(sk) == stol_.end()) && (pos < l.size() - 1)) {
        sk = l[pos++];
      }

      // If not found useful letter, insert number
      if (stol_.find(sk) != stol_.end()) {
        int i = 0;
        while (!(stol_.find(sk) == stol_.end())) {
          sk = l[0] + utils::integer_to_string(i++);
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