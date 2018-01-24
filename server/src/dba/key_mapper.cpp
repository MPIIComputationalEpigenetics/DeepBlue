//
//  key_mapper.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Fabian Reinartz on 01.07.13.
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
      static std::string FEATURE = epidb::dba::KeyMapper::build_default("FEATURE");
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

    const std::string& KeyMapper::TRACKING_ID()
    {
      static std::string TRACKING_ID = epidb::dba::KeyMapper::build_default("TRACKING_ID");
      return TRACKING_ID;
    }

    const std::string& KeyMapper::GENE_SHORT_NAME()
    {
      static std::string GENE_SHORT_NAME = epidb::dba::KeyMapper::build_default("GENE_SHORT_NAME");
      return GENE_SHORT_NAME;
    }

    const std::string& KeyMapper::GENE_ID()
    {
      static std::string GENE_ID = epidb::dba::KeyMapper::build_default("GENE_ID");
      return GENE_ID;
    }

    const std::string& KeyMapper::FPKM()
    {
      static std::string FPKM = epidb::dba::KeyMapper::build_default("FPKM");
      return FPKM;
    }

    const std::string& KeyMapper::FPKM_LO()
    {
      static std::string FPKM_LO = epidb::dba::KeyMapper::build_default("FPKM_LO");
      return FPKM_LO;
    }

    const std::string& KeyMapper::FPKM_HI()
    {
      static std::string FPKM_HI = epidb::dba::KeyMapper::build_default("FPKM_HI");
      return FPKM_HI;
    }

    const std::string& KeyMapper::NUM_READS()
    {
      static std::string NUM_READS = epidb::dba::KeyMapper::build_default("NUM_READS");
      return NUM_READS;
    }

    const std::string& KeyMapper::TRANSCRIPT_IDS()
    {
      static std::string TRANSCRIPT_IDS = epidb::dba::KeyMapper::build_default("TRANSCRIPT_IDS");
      return TRANSCRIPT_IDS;
    }

    const std::string& KeyMapper::LENGTH()
    {
      static std::string LENGTH = epidb::dba::KeyMapper::build_default("LENGTH");
      return LENGTH;
    }

    const std::string& KeyMapper::EFFECTIVE_LENGTH()
    {
      static std::string EFFECTIVE_LENGTH = epidb::dba::KeyMapper::build_default("EFFECTIVE_LENGTH");
      return EFFECTIVE_LENGTH;
    }

    const std::string& KeyMapper::EXPECTED_COUNT()
    {
      static std::string EXPECTED_COUNT = epidb::dba::KeyMapper::build_default("EXPECTED_COUNT");
      return EXPECTED_COUNT;
    }

    const std::string& KeyMapper::TPM()
    {
      static std::string TPM = epidb::dba::KeyMapper::build_default("TPM");
      return TPM;
    }

    const std::string& KeyMapper::POSTERIOR_MEAN_COUNT()
    {
      static std::string POSTERIOR_MEAN_COUNT = epidb::dba::KeyMapper::build_default("POSTERIOR_MEAN_COUNT");
      return POSTERIOR_MEAN_COUNT;
    }

    const std::string& KeyMapper::POSTERIOR_STANDARD_DEVIATION_OF_COUNT()
    {
      static std::string POSTERIOR_STANDARD_DEVIATION_OF_COUNT = epidb::dba::KeyMapper::build_default("POSTERIOR_STANDARD_DEVIATION_OF_COUNT");
      return POSTERIOR_STANDARD_DEVIATION_OF_COUNT;
    }

    const std::string& KeyMapper::PME_TPM()
    {
      static std::string PME_TPM = epidb::dba::KeyMapper::build_default("PME_TPM");
      return PME_TPM;
    }

    const std::string& KeyMapper::PME_FPKM()
    {
      static std::string PME_FPKM = epidb::dba::KeyMapper::build_default("PME_FPKM");
      return PME_FPKM;
    }

    const std::string& KeyMapper::TPM_CI_LOWER_BOUND()
    {
      static std::string TPM_CI_LOWER_BOUND = epidb::dba::KeyMapper::build_default("TPM_CI_LOWER_BOUND");
      return TPM_CI_LOWER_BOUND;
    }

    const std::string& KeyMapper::TPM_CI_UPPER_BOUND()
    {
      static std::string TPM_CI_UPPER_BOUND = epidb::dba::KeyMapper::build_default("TPM_CI_UPPER_BOUND");
      return TPM_CI_UPPER_BOUND;
    }

    const std::string& KeyMapper::FPKM_CI_LOWER_BOUND()
    {
      static std::string FPKM_CI_LOWER_BOUND = epidb::dba::KeyMapper::build_default("FPKM_CI_LOWER_BOUND");
      return FPKM_CI_LOWER_BOUND;
    }

    const std::string& KeyMapper::FPKM_CI_UPPER_BOUND()
    {
      static std::string FPKM_CI_UPPER_BOUND = epidb::dba::KeyMapper::build_default("FPKM_CI_UPPER_BOUND");
      return FPKM_CI_UPPER_BOUND;
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