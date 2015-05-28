//
//  errors.hpp
//  epidb
//
//  Created by Felipe Albrecht on 23.06.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//


#ifndef EPIDB_ERRORS_HPP
#define EPIDB_ERRORS_HPP

#include <string>

namespace epidb {

  class Error {
  private:
    std::string code_value;
    std::string err_fmt;
  public:
    Error(const std::string &c, const std::string &f) :
      code_value(c),
      err_fmt(f) {}

    static std::string m(const Error e, ...);
  };

  extern Error ERR_USER_USER_MISSING;
  extern Error ERR_USER_EXPERIMENT_MISSING;
  extern Error ERR_USER_ANNOTATION_MISSING;
  extern Error ERR_USER_SAMPLE_MISSING;
  extern Error ERR_USER_BIOSOURCE_MISSING;
  extern Error ERR_USER_EPIGNETIC_MARK_MISSING;
  extern Error ERR_USER_TECHNIQUE_MISSING;
  extern Error ERR_USER_PROJECT_MISSING;
  extern Error ERR_USER_DATA_MISSING;
  extern Error ERR_USER_FORMAT_MISSING;
  extern Error ERR_USER_GENOME_MISSING;

  extern Error ERR_FORMAT_CHROMOSOME_MISSING;
  extern Error ERR_FORMAT_START_MISSING;
  extern Error ERR_FORMAT_END_MISSING;
  extern Error ERR_FORMAT_COLUMN_NAME_MISSING;

  extern Error ERR_INVALID_EXPERIMENT_NAME;
  extern Error ERR_INVALID_EXPERIMENT_ID;

  extern Error ERR_INVALID_PROJECT_NAME;
  extern Error ERR_INVALID_PROJECT_ID;

  extern Error ERR_INVALID_BIOSOURCE_NAME;
  extern Error ERR_INVALID_QUERY_ID;
  extern Error ERR_INVALID_COLUMN_NAME;

  extern Error ERR_DUPLICATED_BIOSOURCE_NAME;
  extern Error ERR_DUPLICATED_EXPERIMENT_NAME;

  extern Error ERR_MORE_EMBRACING_BIOSOURCE_NAME;

  extern Error ERR_ALREADY_CONECTED_BIOSOURCE_NAME;
  extern Error ERR_ALREADY_PARENT_BIOSOURCE_NAME;

  extern Error ERR_INVALID_BIOSOURCE_SYNONYM;
  extern Error ERR_INVALID_USER_KEY;

  extern Error ERR_DUPLICATED_EPIGENETIC_MARK_NAME;
  extern Error ERR_DUPLICATED_PROJECT_NAME;
  extern Error ERR_DUPLICATED_GENOME_NAME;
  extern Error ERR_DUPLICATED_TECHNIQUE_NAME;

  extern Error ERR_DATASET_NOT_FOUND;

  extern Error ERR_DATABASE_CONNECTION;
  extern Error ERR_DATABASE_EXCEPTION;
  extern Error ERR_DATABASE_INVALID_BIOSOURCE;
}

#endif