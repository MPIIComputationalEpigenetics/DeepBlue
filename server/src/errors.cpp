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
#include <vector>

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
 * 12 - Gene
 * 13 - Gene Set
 * 14 - ...
 * 20 - Chromosome
 * 21 - Start
 * 22 - End
 * 23 - Column Name
 * 24 - Column Type
 * 25 - Metacolumn
 * 30 - Request
 * 40 - Tiling region
 * 50 - Dataset
 * 51 - GSM Identifier
 * 66 - Internal
 * 99 - Identifier
 */

/** Error :
 * 000 - Invalid/Non-Existent Name
 * 001 - Duplicated
 * 002 - Missing
 * 003 - Invalid ID
 * 004 - Unknown type
 *
 * 100 - Do not have permission
 *
 * 200 - Invalid User Key
 * 250 - Invalid email and password combination
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

  Error ERR_USER_USER_MISSING("100002", "The User is missing. Please, specify the User.");
  Error ERR_INSUFFICIENT_PERMISSION("100100", "Insufficient permission. Permission {} is required.");
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
  Error ERR_USER_GENE_MISSING("112002", "The Gene is missing. Please, specify the Gene.");
  Error ERR_USER_GENE_SET_MISSING("113002", "The Gene Set is missing. Please, specify the Gene Set.");

  Error ERR_FORMAT_CHROMOSOME_MISSING("120002", "The CHROMOSOME is missing in the format. Please, inform the CHROMOSOME column in the Format.");
  Error ERR_FORMAT_START_MISSING("121002", "The START is missing in the format. Please, inform the START column in the Format.");
  Error ERR_FORMAT_END_MISSING("122002", "The END is missing in the format. Please, inform the END column in the Format.");
  Error ERR_FORMAT_COLUMN_NAME_MISSING("123002", "The Column Name is missing in the format. Please, inform the column name in the Format.");

  Error ERR_INVALID_USER_NAME("100000", "Invalid User name '{}'.");
  Error ERR_INVALID_USER_ID("100003", "Invalid User ID '{}'.");
  Error ERR_INVALID_USER_KEY("100200", "Invalid User Key.");
  Error ERR_INVALID_USER_EMAIL_PASSWORD("100200", "Invalid User email and password combination.");

  Error ERR_DUPLICATED_EXPERIMENT_NAME("102001", "The experiment name '{}' is already being used.");

  Error ERR_DUPLICATED_GENE_SET_NAME("113001", "The gene set '{}' is already being used.");
  Error ERR_INVALID_GENE_SET_ID("113003", "Unable to find gene set ID '{}'.");

  Error ERR_INVALID_BIOSOURCE_NAME("104000", "Unable to find BioSource '{}'. No BioSource or Synonym was defined with this name.");
  Error ERR_INVALID_BIOSOURCE_ID("104003", "Uable to find BioSource ID '{}'. No BioSource or Synonym was defined with this ID.");
  Error ERR_DUPLICATED_BIOSOURCE_NAME("104001", "Duplicated BioSource Name '{}'. BioSource or Synonym with this name already exists.");
  Error ERR_MORE_EMBRACING_BIOSOURCE_NAME("104901", "'{}' is already more embracing than '{}'.");
  Error ERR_ALREADY_PARENT_BIOSOURCE_NAME("104902", "'{}' is already parent of '{}'.");
  Error ERR_ALREADY_CONECTED_BIOSOURCE_NAME("104903", "'{}' and '{}' are already connected.");

  Error ERR_INVALID_BIOSOURCE_SYNONYM("104400", "Invalid BioSource Synonym '{}'. A BioSource or a synonym with this name already exists.");

  Error ERR_DUPLICATED_EPIGENETIC_MARK_NAME("105001", "Duplicated Epigenetic Mark Name '{}'." );
  Error ERR_INVALID_EPIGENETIC_MARK_ID("105003", "Unable to find Epigenetic Mark ID '{}'." );

  Error ERR_INVALID_PROJECT_NAME("107000", "Unable to find the project '{}'.");
  Error ERR_DUPLICATED_PROJECT_NAME("107001", "Duplicated Project Name '{}'.");
  Error ERR_INVALID_PROJECT_ID("107003", "Unable to find the project ID '{}'.");
  Error ERR_PERMISSION_PROJECT("107100", "You are not the project '{}' owner and neither an administrator.");

  Error ERR_DUPLICATED_GENOME_NAME("111001", "Duplicated Genome name '{}'.");
  Error ERR_INVALID_GENOME_NAME("111000", "Unable to find the genome '{}'.");
  Error ERR_INVALID_GENOME_ID("111003", "Unable to find the genome ID '{}'.");

  Error ERR_INVALID_CHROMOSOME_NAME("120000", "Unable to find the chromosome '{}'.");
  Error ERR_INVALID_CHROMOSOME_NAME_GENOME("120002", "Unable to find the chromosome '{}' in the genome '{}'.");

  Error ERR_INVALID_TECHNIQUE_ID("106003", "Unable to find the technique ID '{}'.");
  Error ERR_DUPLICATED_TECHNIQUE_NAME("106001", "Duplicated Genome Name '{}'.");

  Error ERR_INVALID_EXPERIMENT_NAME("101000", "Unable to find the experiment '{}'.");
  Error ERR_INVALID_EXPERIMENT_ID("101003", "Unable to find the experiment ID '{}'.");

  Error ERR_INVALID_SAMPLE_ID("103000", "Unable to find the sample ID '{}'.");

  Error ERR_INVALID_QUERY_ID("110003", "Unable to find the query ID '{}'.");
  Error ERR_PERMISSION_QUERY("111003", "You are not the query ID '{}' owner and neither an administrator.");

  Error ERR_INVALID_ANNOTATION_ID("102003", "Unable to find the annotation ID '{}'.");

  Error ERR_INVALID_COLUMN_NAME("123000", "Unable to find the column name '{}' in the dataset format or in the DeepBlue columns.");
  Error ERR_INVALID_COLUMN_ID("123003", "Unable to find the column ID '{}'.");
  Error ERR_DUPLICATED_COLUMN_NAME("123001", "Duplicated column name '{}'.");

  Error ERR_INVALID_COLUMN_TYPE_ID("124003", "Unable to find the column type ID '{}'." );

  Error ERR_INVALID_META_COLUMN_NAME("125000", "The meta-column '{}' does not exist.");

  Error ERR_REQUEST_CANCELED("130300", "The request was canceled.");

  Error ERR_UNKNOW_QUERY_TYPE("310004", "Unknown query type '{}'");

  Error ERR_COLUMN_TYPE_MISSING("323002", "The Column Type '{}' does not exist.");
  Error ERR_COLUMN_TYPE_NAME_MISSING("323003", "The Column Type '{}' does not exist.");

  Error ERR_DATASET_NOT_FOUND("350000", "Dataset id {} not found.");

  Error ERR_INVALID_TILING_REGIONS_ID("140003", "Unable to find tiling regions ID '{}'.");

  Error ERR_INVALID_IDENTIFIER("19900", "Invalid identifier '{}'.");

  Error ERR_INVALID_GSM_IDENTIFIER("15100", "Invalid GSM identifier '{}'.");

  Error ERR_DATABASE_CONNECTION("466555", "MongoDB connection error: '{}'.");
  Error ERR_DATABASE_EXCEPTION("466666", "MongoDB exception at operation '{}': '{}'.");
  Error ERR_DATABASE_INVALID_BIOSOURCE("404666", "BioSource '{}' not found.");

};
