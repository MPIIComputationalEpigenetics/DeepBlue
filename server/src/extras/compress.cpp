//
//  compress.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.07.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <iostream>
#include <minilzo.h>

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


    bool compress(const lzo_bytep in, const lzo_uint in_size, lzo_bytep out, lzo_uint &out_size)
    {
      lzo_voidp wrkmem = (lzo_voidp) malloc(LZO1X_1_MEM_COMPRESS);
      int r = lzo1x_1_compress(in, in_size, out, &out_size, wrkmem);
      free(wrkmem);

      if (r == LZO_E_OK) {
        EPIDB_LOG_TRACE("compressed " << in_size << "  bytes into " << out_size  << " bytes");
      }

      if (out_size >= in_size) {
        EPIDB_LOG_TRACE("This block contains incompressible data.");
      }
      return true;
    }

    bool decompress(const lzo_bytep data, const lzo_uint lzo_size, const lzo_uint orig_size,
                    lzo_bytep &in, lzo_uint &new_len)
    {
      bool r = lzo1x_decompress(data, lzo_size, in, &new_len, NULL);
      if (r == LZO_E_OK  && new_len == orig_size) {
      }
      EPIDB_LOG_TRACE("decompressed " << lzo_size << " bytes back into " << new_len << " bytes");
      return 1;
    }
  }
}