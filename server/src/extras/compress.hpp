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
    bool compress(const lzo_bytep in, const lzo_uint in_size, lzo_bytep out, lzo_uint &out_size);
    bool decompress(const lzo_bytep data, const lzo_uint lzo_size, const lzo_uint orig_size,
                    lzo_bytep &in, lzo_uint &new_len);
  }
}