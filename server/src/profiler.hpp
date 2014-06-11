//
//  profiler.hpp
//  epidb
//
//  Created by Felipe Albrecht on 17.04.14.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//
//
// based on:
/*
 * Profiny - Lightweight Profiler Tool
 * Copyright (C) 2013 Sercan Tutar
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 * USAGE:
 *   First PROFINY_CALL_GRAPH_PROFILER or PROFINY_FLAT_PROFILER must be defined
 *   (giving as a compiler option is advised). If you;
 *
 *     - define PROFINY_CALL_GRAPH_PROFILER, it will work as a call-graph profiler
 *     - define PROFINY_FLAT_PROFILER, it will work as a flat profiler
 *     - define neither, Profiny macros will be set to blank (i.e. profiling will be off)
 *     - define both, it will give an error
 *
 *   Later, if you chose PROFINY_CALL_GRAPH_PROFILER, you may want to determine
 *   whether recursive calls will be omitted or not (omitted by default) by calling:
 *
 *     Profiler::setOmitRecursiveCalls(bool)
 *
 *   By default (if the profiling is not off), if your program exits normally, Profinity
 *   will print results in "profinity.out" file. Also, the user can force printing results
 *   at any time by calling:
 *
 *     Profiler::printStats("filename")
 *
 *   See ReadMe.txt for more info.
 *
 *
 * Happy profiling!
 *
 */


#ifndef PROFINY_H_
#define PROFINY_H_

#include <vector>
#include <map>
#include <sstream>
#include <fstream>

#include <boost/timer/timer.hpp>
#include <boost/intrusive_ptr.hpp>


#if defined(PROFINY_CALL_GRAPH_PROFILER) && defined(PROFINY_FLAT_PROFILER)

# error "PROFINY_CALL_GRAPH_PROFILER and PROFINY_FLAT_PROFILER should not be defined at the same time!"

#elif defined(PROFINY_CALL_GRAPH_PROFILER) || defined(PROFINY_FLAT_PROFILER)

# define //PROFINY_SCOPE \
  std::ostringstream _oss; \
  _oss << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__; \
  profiny::ScopedProfile _sco_pro(_oss.str());

# define //PROFINY_SCOPE_WITH_ID(ID) \
  std::ostringstream _oss; \
  _oss << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ":" << (ID); \
  profiny::ScopedProfile _sco_pro(_oss.str());

# define PROFINY_NAMED_SCOPE(NAME) \
  std::ostringstream _oss; \
  _oss << (NAME); \
  profiny::ScopedProfile _sco_pro(_oss.str());

# define PROFINY_NAMED_SCOPE_WITH_ID(NAME, ID) \
  std::ostringstream _oss; \
  _oss << (NAME) << ":" << (ID); \
  profiny::ScopedProfile _sco_pro(_oss.str());

#else

# define //PROFINY_SCOPE

# define //PROFINY_SCOPE_WITH_ID(ID)

# define PROFINY_NAMED_SCOPE(NAME)

# define PROFINY_NAMED_SCOPE_WITH_ID(NAME, ID)

#endif


#define NANOSEC_TO_SEC(X) ((X) / 1000000000.0)


namespace profiny {
  class BaseObject {
  public:
    BaseObject();

    virtual ~BaseObject();

    void incrRef();

    void decrRef();

    int getRef() const;

  private:
    int m_ref;
  };

  /**********************************************************************/

  class Profile : public BaseObject {
    friend class ScopedProfile;
    friend class Profiler;

  private:
    Profile(const std::string &name);

    ~Profile();

    bool start();

    bool stop();

    unsigned int getCallCount() const;

    std::string getName() const;

    void getTimes(double &wall, double &user, double &system) const;

    std::map<std::string, boost::intrusive_ptr<Profile> > &getSubProfiles();

    std::map<std::string, boost::intrusive_ptr<Profile> > m_subProfiles;

    std::string m_name;

    unsigned int m_callCount;

    double m_wallTime;
    double m_userTime;
    double m_systemTime;

    boost::timer::cpu_timer m_timer;
  };

  /**********************************************************************/

  class ScopedProfile : public BaseObject {
  public:
    ScopedProfile(const std::string &name);

    ~ScopedProfile();

  private:
    boost::intrusive_ptr<Profile> m_profile;
  };

  /**********************************************************************/

  class Profiler : public BaseObject {
    friend class Profile;
    friend class ScopedProfile;

  public:
    static void printStats(const std::string &filename);

    static void setOmitRecursiveCalls(bool omit);

    static bool getOmitRecursiveCalls();

  private:
    Profiler();

    ~Profiler();

    static boost::intrusive_ptr<Profiler> getInstance();

    boost::intrusive_ptr<Profile> getProfile(const std::string &name);

    static void printStats();

    static void printStats(std::ofstream &fs, std::map<std::string, boost::intrusive_ptr<Profile> > *p, int depth);

    std::map<std::string, boost::intrusive_ptr<Profile> > &getCurrentProfilesRoot();

    void pushProfile(boost::intrusive_ptr<Profile> p);

    void popProfile();

    bool isInStack(const std::string &name);

    std::map<std::string, boost::intrusive_ptr<Profile> > m_profiles;

    static boost::intrusive_ptr<Profiler> m_instance;

    std::vector<boost::intrusive_ptr<Profile> > m_profileStack;

    bool m_omitRecursiveCalls;
  };

} // namespace profiny

#endif /* PROFINY_H_ */
