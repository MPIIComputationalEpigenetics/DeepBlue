//
//  compress.hpp
//  epidb
//
//  Created by Felipe Albrecht on 10.07.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <minilzo.h>

namespace epidb {
  namespace compress {
    bool init();
    // TODO : Return smart pointer
    boost::shared_ptr<char> compress(const char *in, const size_t in_size, size_t &out_size, bool& compress);
    lzo_bytep decompress(const lzo_bytep data, const lzo_uint compressed_size, const size_t uncompressed_size, size_t &out_size);
  }
}