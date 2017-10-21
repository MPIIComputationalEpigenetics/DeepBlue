//
//  storage.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 21.10.17.
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


#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>


#include "../config/config.hpp"

#include "../connection/connection.hpp"

#include "../log.hpp"


namespace epidb {
  namespace storage {

    void clear_all()
    {
      Connection c;
      c->dropCollection(config::DATABASE_NAME() + ".fs.chunks");
      c->dropCollection(config::DATABASE_NAME() + ".fs.files");
      c.done();
    }

    std::string store(const std::string filename, const char *ptr, size_t len)
    {
      Connection c;
      mongo::GridFS gridfs(c.conn(), config::get_mongodb_server(), "fs");
      gridfs.setChunkSize(2 << 22); // 8MB
      mongo::BSONObj ret = gridfs.storeFile(ptr, len, filename);

      mongo::BSONObjBuilder bob;
      bob.appendElements(ret);
      c->update(config::DATABASE_NAME() + ".fs.files",
                BSON("filename" << ret.getField("filename")),
                bob.obj(), false, false);


      std::string e = c->getLastError();
      if(!e.empty()) {
        EPIDB_LOG_ERR("MDBQC: error_code!=0, failing: " << e << "\n" << c->getLastErrorDetailed().toString() );
      }

      c.done();

      return filename;
    }

    bool get_file_info(const std::string &filename, mongo::OID &oid, size_t &chunk_size, size_t &file_size, std::string &msg)
    {
      Connection c;
      auto data_cursor = c->query(config::DATABASE_NAME() + ".fs.files", mongo::Query(BSON("filename" << filename)));

      if (data_cursor->more()) {
        auto fileinfo = data_cursor->next();
        oid = fileinfo["_id"].OID();
        chunk_size = fileinfo["chunkSize"].numberLong();
        file_size = fileinfo["length"].numberLong();
        c.done();
        return true;
      }

      msg = "The result data under the request '" + filename + "'' was not found";
      c.done();
      return false;
    }

    //bool get_result(const std::string &filename, std::string &content, std::string &msg)
    bool load(const std::string &filename, std::string &content, std::string &msg)
    {
      mongo::OID oid;
      size_t file_size;
      size_t chunk_size;
      if (!get_file_info(filename, oid, chunk_size, file_size, msg)) {
        return false;
      }

      mongo::BSONObj projection = BSON("data" << 1);

      size_t remaining = file_size;
      size_t n = 0;

      std::stringstream ss;

      Connection c;
      while (remaining > 0) {
        mongo::Query q(BSON("files_id" << oid << "n" << (long long) n));
        auto data_cursor = c->query(config::DATABASE_NAME() + ".fs.chunks", q, 0, 0, &projection);
        if (data_cursor->more()) {
          int read;
          char* compressed_data = (char *) data_cursor->next().getField("data").binData(read);
          ss.write(compressed_data, read);

          n++;
          remaining -= read;
        } else {
          msg = "Chunk for file " + filename + " not found.";
          c.done();
          return false;
        }
      }
      c.done();
      content = ss.str();
      return true;
    }
  }
}
