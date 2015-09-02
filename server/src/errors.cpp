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
 * 10 - Query
 * 11 - Genome
 * 12 - ..
 * 20 - Chromosome
 * 21 - Start
 * 22 - End
 * 23 - Column Name
 * 24 - Column Type
 * 25 - Metacolumn
 * 30 - Request
 * 50 - Dataset
 * 66 - Internal
 */

/** Error :
 * 000 - Invalid/Non-Existent Name
 * 001 - Duplicated
 * 002 - Missing
 * 003 - Invalid ID
 *
 * 100 - Do not have permission
 *
 * 200 - Invalid User Key
 *
 * 300 - Canceled
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
    int n = ((int)e.err_fmt.size()) * 2; /* reserve 2 times as much as the length of the fmt_str */
    boost::shared_ptr<char[]> formatted;
    va_list ap;
    while (1) {
      formatted.reset(new char[n]); /* wrap the plain char array into the unique_ptr */
      strcpy(&formatted[0], e.err_fmt.c_str());
      va_start(ap, e);
      int final_n = vsnprintf(&formatted[0], n, e.err_fmt.c_str(), ap);
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

  Error ERR_USER_USER_MISSING("100002", "The User is missing. Please, specify the User.");
  Error ERR_INSUFFICIENT_PERMISSION("100100", "Insufficient permission. Permission %s is required.");
  Error ERR_USER_EXPERIMENT_MISSING("101002", "The Experiment is missing. Please, specify the Experiment.");
  Error ERR_USER_ANNOTATION_MISSING("102002", "The Annotation is missing. Please, specify the Annotation.");
  Error ERR_USER_SAMPLE_MISSING("103002", "The Sample is missing. Please, specify the Sample.");
  Error ERR_USER_BIOSOURCE_MISSING("104002", "The BioSource is missing. Please, specify the BioSource.");
  Error ERR_USER_EPIGNETIC_MARK_MISSING("105002", "The Epigenetic Mark is missing. Please, specify the Epigenetic Mark.");
  Error ERR_USER_TECHNIQUE_MISSING("106002", "The Technique is missing. Please, specify the Technique.");
  Error ERR_USER_PROJECT_MISSING("107002", "The Project is missing. Please, specify the Project.");
  Error ERR_USER_DATA_MISSING("108002", "The Data is missing. Please, specify the Data.");
  Error ERR_USER_FORMAT_MISSING("109002", "The Format is missing. Please, specify the Format.");
  Error ERR_USER_GENOME_MISSING("111002", "The Genome is missing. Please, specify the Genome.");

  Error ERR_FORMAT_CHROMOSOME_MISSING("120002", "The CHROMOSOME is missing in the format. Please, inform the CHROMOSOME column in the Format.");
  Error ERR_FORMAT_START_MISSING("121002", "The START is missing in the format. Please, inform the START column in the Format.");
  Error ERR_FORMAT_END_MISSING("122002", "The END is missing in the format. Please, inform the END column in the Format.");
  Error ERR_FORMAT_COLUMN_NAME_MISSING("123002", "The Column Name is missing in the format. Please, inform the column name in the Format.");

  Error ERR_INVALID_USER_KEY("100200", "Invalid User Key.");
  Error ERR_INVALID_USER_NAME("100000", "Invalid User name '%s'.");

  Error ERR_DUPLICATED_EXPERIMENT_NAME("102001", "The experiment name '%s' is already being used.");

  Error ERR_INVALID_BIOSOURCE_NAME("104000", "Invalid BioSource Name '%s'. No BioSource or Synonym was defined with this name.");
  Error ERR_DUPLICATED_BIOSOURCE_NAME("104001", "Duplicated BioSource Name '%s'. BioSource or Synonym with this name already exists.");
  Error ERR_MORE_EMBRACING_BIOSOURCE_NAME("104901", "'%s' is already more embracing than '%s'.");
  Error ERR_ALREADY_PARENT_BIOSOURCE_NAME("104902", "'%s' is already parent of '%s'.");
  Error ERR_ALREADY_CONECTED_BIOSOURCE_NAME("104903", "'%s' and '%s' are already connected.");

  Error ERR_INVALID_BIOSOURCE_SYNONYM("104400", "Invalid BioSource Synonym '%s'. A BioSource or a synonym with this name already exists.");

  Error ERR_DUPLICATED_EPIGENETIC_MARK_NAME("105001", "Duplicated Epigenetic Mark Name '%s'." );

  Error ERR_INVALID_PROJECT_NAME("107000", "Unable to find the project '%s'.");
  Error ERR_DUPLICATED_PROJECT_NAME("107001", "Duplicated Project Name '%s'.");
  Error ERR_INVALID_PROJECT_ID("107003", "Unable to find the project ID '%s'.");
  Error ERR_PROJECT_PERMISSION("107100", "You are not the project '%s' owner and neither an administrator.");

  Error ERR_DUPLICATED_GENOME_NAME("111001", "Duplicated Genome Name '%s'.");

  Error ERR_DUPLICATED_TECHNIQUE_NAME("106001", "Duplicated Genome Name '%s'.");

  Error ERR_INVALID_EXPERIMENT_NAME("101000", "Unable to find the experiment '%s'.");
  Error ERR_INVALID_EXPERIMENT_ID("101000", "Unable to find the experiment ID '%s'.");

  Error ERR_INVALID_QUERY_ID("110000", "Unable to find the query ID '%s'.");
  Error ERR_INVALID_COLUMN_NAME("123000", "Unable to find the column '%s' in the dataset format or in the DeepBlue columns.");
  Error ERR_DUPLICATED_COLUMN_NAME("123001", "Duplicated column name '%s'.");

  Error ERR_INVALID_META_COLUMN_NAME("125000", "The meta-column '%s' does not exist.");

  Error ERR_REQUEST_CANCELED("130300", "The request was canceled.");

  Error ERR_COLUMN_TYPE_MISSING("323002", "The Column Type '%d' does not exist.");
  Error ERR_COLUMN_TYPE_NAME_MISSING("323003", "The Column Type '%s' does not exist.");

  Error ERR_DATASET_NOT_FOUND("350000", "Dataset '%ld' not found.");

  Error ERR_DATABASE_CONNECTION("466555", "MongoDB connection error: '%s'.");
  Error ERR_DATABASE_EXCEPTION("466666", "MongoDB exception at operation '%s': '%s'.");
  Error ERR_DATABASE_INVALID_BIOSOURCE("404666", "BioSource '%s' not found.");
};
