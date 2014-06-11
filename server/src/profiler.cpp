//
//  profiler.hpp
//  epidb
//
//  Created by Felipe Albrecht on 17.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//
//
// based on:


/**
#include <boost/thread/tss.hpp>
static boost::thread_specific_ptr< MyClass> instance;
if( ! instance.get() ) {
    // first time called by this thread
    // construct test element to be used in all subsequent calls from this thread
    instance.reset( new MyClass);
}
    instance->doSomething();
*/

/*
 * Profiny - Lightweight Profiler Tool
 * Copyright (C) 2013 Sercan Tutar
 *
 */

#include <vector>
#include <map>
#include <sstream>
#include <fstream>

#include <boost/timer/timer.hpp>
#include <boost/intrusive_ptr.hpp>

#include "profiler.hpp"

namespace profiny {
  inline BaseObject::BaseObject() :
    m_ref(0)
  {
  }

  inline BaseObject::~BaseObject()
  {
  }

  inline void BaseObject::incrRef()
  {
    m_ref++;
  }

  inline void BaseObject::decrRef()
  {
    m_ref--;
  }

  inline int BaseObject::getRef() const
  {
    return m_ref;
  }


  inline Profile::Profile(const std::string &name) :
    m_name(name), m_callCount(0), m_wallTime(0.0), m_userTime(0.0), m_systemTime(0.0)
  {
  }

  inline Profile::~Profile()
  {
  }

  inline bool Profile::start()
  {
    Profiler::getInstance()->pushProfile(this);
    m_timer.start();
    return true;
  }

  inline bool Profile::stop()
  {
    Profiler::getInstance()->popProfile();
    m_timer.stop(); // TODO: check if we need this line
    boost::timer::cpu_times t = m_timer.elapsed();
    m_wallTime += NANOSEC_TO_SEC(t.wall);
    m_userTime += NANOSEC_TO_SEC(t.user);
    m_systemTime += NANOSEC_TO_SEC(t.system);
    ++m_callCount;
    return true;
  }

  inline unsigned int Profile::getCallCount() const
  {
    return m_callCount;
  }

  inline std::string Profile::getName() const
  {
    return m_name;
  }

  inline void Profile::getTimes(double &wall, double &user, double &system) const
  {
    wall = m_wallTime;
    user = m_userTime;
    system = m_systemTime;
  }

  inline std::map<std::string, boost::intrusive_ptr<Profile> > &Profile::getSubProfiles()
  {
    return m_subProfiles;
  }

  /**********************************************************************/

  ScopedProfile::ScopedProfile(const std::string &name)
  {
    std::string n(name);

    if (Profiler::getInstance()->isInStack(n)) {
      // profile is already in stack (probably a recursive call)
      if (Profiler::getInstance()->getOmitRecursiveCalls()) {
        m_profile = NULL;
        return;
      } else {
        n = "RECURSIVE@" + n;
      }
    }

    m_profile = Profiler::getInstance()->getProfile(n);
    if (m_profile != NULL) {
      if (!m_profile->start()) {
        // cannot start profiler (probably a recursive call for flat profiler)
        m_profile = NULL;
      }
    } else {
      std::cerr << "Cannot start scoped profiler: " << n << std::endl;
    }
  }

  inline ScopedProfile::~ScopedProfile()
  {
    if (m_profile != NULL) {
      m_profile->stop();
    }
  }

  /**********************************************************************/

  boost::intrusive_ptr<Profiler> Profiler::m_instance = NULL;

  inline Profiler::Profiler()
    : m_omitRecursiveCalls(true)
  {
  }

  inline Profiler::~Profiler()
  {
  }

  inline boost::intrusive_ptr<Profiler> Profiler::getInstance()
  {
    if (m_instance == NULL) {
      m_instance = new Profiler;
      atexit(printStats);
    }
    return m_instance;
  }

  inline boost::intrusive_ptr<Profile> Profiler::getProfile(const std::string &name)
  {
    std::map<std::string, boost::intrusive_ptr<Profile> > &profiles = getCurrentProfilesRoot();
    std::map<std::string, boost::intrusive_ptr<Profile> >::iterator it = profiles.find(name);
    if (it != profiles.end()) {
      return it->second;
    } else {
      boost::intrusive_ptr<Profile> result = new Profile(name);
      profiles[name] = result;
      return result;
    }
  }

  inline std::map<std::string, boost::intrusive_ptr<Profile> > &Profiler::getCurrentProfilesRoot()
  {
    return m_profileStack.empty() ? m_profiles : m_profileStack.back()->getSubProfiles();
  }

  inline void Profiler::pushProfile(boost::intrusive_ptr<Profile> p)
  {
    m_profileStack.push_back(p);
  }

  inline void Profiler::popProfile()
  {
    if (!m_profileStack.empty()) {
      m_profileStack.pop_back();
    }
  }

  inline bool Profiler::isInStack(const std::string &name)
  {
    for (unsigned int i = 0; i < m_profileStack.size(); i++) {
      if (m_profileStack[i]->getName() == name) {
        return true;
      }
    }
    return false;
  }

  inline void Profiler::printStats(std::ofstream &fs, std::map<std::string, boost::intrusive_ptr<Profile> > *p, int depth)
  {
    std::cerr << "printStats" << std::endl;
    std::ostringstream oss;
    for (int i = 0; i < depth; i++) {
      oss << "\t";
    }

    std::map<std::string, boost::intrusive_ptr<Profile> >::iterator it = p->begin();
    for (; it != p->end(); it++) {
      unsigned int cc = it->second->getCallCount();
      double wall, user, system;
      it->second->getTimes(wall, user, system);
      fs << oss.str() << it->second->getName() << "  T:" << wall << "  #:" << cc << "  %:" << 100 * ((user + system) / wall) << std::endl;
      printStats(fs, &(it->second->getSubProfiles()), depth + 1);
    }
  }

  inline void Profiler::printStats()
  {
    printStats("profiny.out");
  }

  inline void Profiler::printStats(const std::string &filename)
  {
    std::ofstream fs;
    fs.open(filename.c_str());
    if (!fs.is_open()) {
      std::cerr << "Cannot open profiler output file: " << filename << std::endl;
      return;
    }
    Profiler::printStats(fs, &(getInstance()->m_profiles), 0);
    fs.close();
  }

  inline void Profiler::setOmitRecursiveCalls(bool omit)
  {
    getInstance()->m_omitRecursiveCalls = omit;
  }

  inline bool Profiler::getOmitRecursiveCalls()
  {
    return getInstance()->m_omitRecursiveCalls;
  }

  inline void intrusive_ptr_add_ref(profiny::BaseObject *p)
  {
    if (p != NULL) {
      // pointer is not NULL
      p->incrRef();
    }
  }

  inline void intrusive_ptr_release(profiny::BaseObject *p)
  {
    if (p != NULL) {
      // pointer is not NULL
      p->decrRef();
      if (p->getRef() <= 0) {
        // reference count is zero or less
        delete p;
      }
    }
  }
} // namespace profiny
