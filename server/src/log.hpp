//
//  log.cpp
//  epidb
//
//  Created by Felipe Albrecht on 04.04.2014.
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

#ifndef EPIDB_LOG_HPP
#define EPIDB_LOG_HPP


#include <boost/log/trivial.hpp>


#define EPIDB_LOG_TRACE(s) { BOOST_LOG_TRIVIAL(trace) << s; }
#define EPIDB_LOG(s) { BOOST_LOG_TRIVIAL(info) << s; }
#define EPIDB_LOG_ERR(s) { BOOST_LOG_TRIVIAL(error) << s; }
#define EPIDB_LOG_WARN(s) { BOOST_LOG_TRIVIAL(warning) << s;}
#define EPIDB_LOG_DBG(s) { BOOST_LOG_TRIVIAL(debug) << s; }

/*

#define EPIDB_LOG_TRACE(s) { std::cerr  << s << std::endl; }
#define EPIDB_LOG(s) { std::cerr  << s<< std::endl; }
#define EPIDB_LOG_ERR(s) { std::cerr  << s<< std::endl; }
#define EPIDB_LOG_WARN(s) { std::cerr  << s<< std::endl;}
#define EPIDB_LOG_DBG(s) { std::cerr  << s<< std::endl; }

*/

#endif // EPIDB_LOG_HPP
