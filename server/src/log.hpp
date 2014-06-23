//
//  log.cpp
//  epidb
//
//  Created by Felipe Albrecht on 04.04.2014.
//  Copyright (c) 2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_LOG_HPP
#define EPIDB_LOG_HPP

#include <boost/log/trivial.hpp>

#define EPIDB_LOG_TRACE(s) { BOOST_LOG_TRIVIAL(trace) << s; }
#define EPIDB_LOG(s) { BOOST_LOG_TRIVIAL(info) << s; }
#define EPIDB_LOG_ERR(s) { BOOST_LOG_TRIVIAL(error) << s; }
#define EPIDB_LOG_WARN(s) { BOOST_LOG_TRIVIAL(warning) << s;}
#define EPIDB_LOG_DBG(s) { BOOST_LOG_TRIVIAL(debug) << s; }


#endif // EPIDB_LOG_HPP
