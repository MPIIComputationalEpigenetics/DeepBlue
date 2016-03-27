//
//  semaphore.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 24.03.15.
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

#ifndef EPIDB_ALGORITHMS_THREADS_SEMAPHORE_HPP
#define EPIDB_ALGORITHMS_THREADS_SEMAPHORE_HPP

#include <memory>

namespace epidb {
  namespace threading {

    class Semaphore {
      size_t count;
      std::mutex mutex;
      std::condition_variable cv;

    public:
      Semaphore(size_t c);
      void down();
      void up();
    };

    typedef std::shared_ptr<Semaphore> SemaphorePtr;

    SemaphorePtr build_semaphore(size_t count);
  }
}

#endif