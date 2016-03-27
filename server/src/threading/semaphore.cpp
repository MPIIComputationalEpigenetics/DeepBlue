//
//  semaphore.cpp
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
//
/*
* @Author: Felipe Albrecht
* @Date:   2016-03-23 19:24:40
* @Last Modified by:   Felipe Albrecht
* @Last Modified time: 2016-03-27 03:39:47
*/

#include <memory>
#include <mutex>
#include <condition_variable>

#include "semaphore.hpp"

namespace epidb {
  namespace threading {

    Semaphore::Semaphore(size_t c) : count(c) {}

    void Semaphore::down()
    {
      std::unique_lock<std::mutex> lk(mutex);
      cv.wait(lk, [this] {return count > 0;});

      count--;

      // Manual unlocking is done before notifying, to avoid waking up
      // the waiting thread only to block again (see notify_one for details)
      lk.unlock();
      cv.notify_one();
    }

    void Semaphore::up()
    {
      std::unique_lock<std::mutex> lk(mutex);
      count++;
      lk.unlock();
      cv.notify_all();
    }

    SemaphorePtr build_semaphore(size_t count)
    {
      return std::shared_ptr<Semaphore>(new Semaphore(count));
    }

  }
}
