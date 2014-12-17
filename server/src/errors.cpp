//
//  errors.cpp
//  epidb
//
//  Created by Felipe Albrecht on 23.06.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

#include <boost/shared_ptr.hpp>

#include "errors.hpp"

/** Error level:
 * 1 - User
 * 2 - Logical
 * 3 - System bug
 * 4 - Database error
 */


/** Data Type:
 * 00 - User
 * 01 - Experiment
 * 02 - Annotation
 * 03 - Sample
 * 04 - BioSource
 * 05 - Epigenetic Mark
 * 06 - Technique
 * 07 - Project
 * 08 - Data
 * 09 - Format
 * 10 -
 * 11 - Genome
 * 12 - ..
 * 50 - Dataset
 * 66 - Internal
 */

/** Error :
 * 000 - Invalid/Non-Existent
 * 001 - Duplicated
 *
 * 400 - Synonym already exists
 *
 * 555 - Connection Error
 * 666 - Internal Error
 *
 * // Controlled vocabulary errors
 * 901 - More Embracing
 * 902 - Already parent
 * 903 - Already connected
 */

namespace epidb {

  std::string Error::m(const Error e, ...)
  {
    int final_n, n = ((int)e.err_fmt.size()) * 2; /* reserve 2 times as much as the length of the fmt_str */
    std::string str;
    boost::shared_ptr<char[]> formatted;
    va_list ap;
    while (1) {
      formatted.reset(new char[n]); /* wrap the plain char array into the unique_ptr */
      strcpy(&formatted[0], e.err_fmt.c_str());
      va_start(ap, e);
      final_n = vsnprintf(&formatted[0], n, e.err_fmt.c_str(), ap);
      va_end(ap);
      if (final_n < 0 || final_n >= n)
        n += abs(final_n - n + 1);
      else
        break;
    }
    std::stringstream ss;
    ss << e.code_value;
    ss << ":";
    ss << formatted.get();
    return ss.str();
  }


  Error ERR_INVALID_USER_KEY("100000", "Invalid User Key.");

  Error ERR_DUPLICATED_EXPERIMENT_NAME("102001", "The experiment name '%s' is already being used.");

  Error ERR_INVALID_BIOSOURCE_NAME("104000", "Invalid BioSource Name '%s'. No BioSource or Synonym was defined with this name.");
  Error ERR_DUPLICATED_BIOSOURCE_NAME("104001", "Duplicated BioSource Name '%s'. BioSource or Synonym with this name already exists.");
  Error ERR_MORE_EMBRACING_BIOSOURCE_NAME("104901", "'%s' is already more embracing than '%s'.");
  Error ERR_ALREADY_PARENT_BIOSOURCE_NAME("104902", "'%s' is already parent of '%s'.");
  Error ERR_ALREADY_CONECTED_BIOSOURCE_NAME("104903", "'%s' and '%s' are already connected.");

  Error ERR_INVALID_BIOSOURCE_SYNONYM("104400", "Invalid BioSource Synonym '%s'. A BioSource or a synonym with this name already exists.");

  Error ERR_DUPLICATED_EPIGENETIC_MARK_NAME("105001", "Duplicated Epigenetic Mark Name '%s'." );

  Error ERR_DUPLICATE_PROJECT_NAME("107001", "Duplicated Project Name '%s'.");

  Error ERR_DUPLICATE_GENOME_NAME("111101", "Duplicated Genome Name '%s'.");

  Error ERR_DATASET_NOT_FOUND("350000", "Dataset '%ld' not found.");

  Error ERR_DATABASE_CONNECTION("466555", "MongoDB connection error: '%s'.");
  Error ERR_DATABASE_EXCEPTION("466666", "MongoDB exception at operation '%s': '%s'.");
  Error ERR_DATABASE_INVALID_BIOSOURCE("404666", "BioSource '%s' not found.");
};
