//
//  compress.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.07.14.
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

#include <new>

#include <iostream>
#include <minilzo.h>

#include <memory>

#include "log.hpp"

namespace epidb {
  namespace compress {

    bool init()
    {
      if (lzo_init() != LZO_E_OK) {
        EPIDB_LOG_ERR("internal error - lzo_init() failed !!!");
        EPIDB_LOG_ERR("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics)")
        return false;
      }

      EPIDB_LOG("LZO initialized");

      return true;
    }


    bool __compress(const lzo_bytep in, const lzo_uint in_size, lzo_bytep out, lzo_uint &out_size)
    {
      lzo_voidp wrkmem = (lzo_voidp) malloc(LZO1X_1_MEM_COMPRESS);
      int r = lzo1x_1_compress(in, in_size, out, &out_size, wrkmem);
      free(wrkmem);

      if (r == LZO_E_OK && out_size >= in_size + 32) { // 32 for the overhead in the bson object
        return false;
      }
      return true;
    }

    // TODO : Return smart pointer
    std::shared_ptr<char> compress(const char *in, const size_t in_size, size_t &out_size, bool &compress)
    {
      char *ptr = const_cast<char *>(in);
      lzo_bytep lzo_ptr = reinterpret_cast<lzo_bytep &>(ptr);

      lzo_bytep lzo_out = (lzo_bytep) malloc((in_size + in_size / 16 + 64 + 3));
      bool r = __compress(lzo_ptr, in_size, lzo_out, out_size);
      if (!r) {
        free(lzo_out);
        compress = false;
        return std::shared_ptr<char>();
      }
      compress = true;
      return std::shared_ptr<char>( (char *) lzo_out);
    }

    lzo_bytep decompress(const lzo_bytep data, const lzo_uint compressed_size, const size_t uncompressed_size, size_t &out_size)
    {
      lzo_bytep lzo_out = (lzo_bytep) malloc(sizeof(lzo_byte) * uncompressed_size);
      bool r = lzo1x_decompress(data, compressed_size, lzo_out, &out_size, NULL);
      if (r != LZO_E_OK ) {
        free(lzo_out);
        return NULL;
      }
      return lzo_out;
    }

  }
}