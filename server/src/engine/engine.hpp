//
//  engine.hpp
//  epidb
//
//  Created by Felipe Albrecht on 29.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ENGINE_ENGINE_HPP
#define EPIDB_ENGINE_ENGINE_HPP

#include <boost/noncopyable.hpp>

#include "commands.hpp"

#include "../log.hpp"

namespace epidb {
  class Engine : private boost::noncopyable {
  private:
    Engine()
    {
      EPIDB_LOG("Creating Engine");
    }
    Engine(Engine const &);
    void operator=(Engine const &);

  public:
    const static Engine &instance()
    {
      static Engine instance;
      return instance;
    }

    bool execute(const std::string &name, const std::string &ip, unsigned long long id,
                 serialize::Parameters &parameters, serialize::Parameters &result) const;
  };
}

#endif
