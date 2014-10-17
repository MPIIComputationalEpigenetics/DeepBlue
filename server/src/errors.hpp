//
//  errors.hpp
//  epidb
//
//  Created by Felipe Albrecht on 23.06.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

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

  extern Error ERR_INVALID_USER_KEY;

  extern Error ERR_DUPLICATED_EXPERIMENT_NAME;

  extern Error ERR_INVALID_BIOSOURCE_NAME;
  extern Error ERR_DUPLICATED_BIOSOURCE_NAME;
  extern Error ERR_MORE_EMBRACING_BIOSOURCE_NAME;
  extern Error ERR_INVALID_BIOSOURCE_SYNONYM;

  extern Error ERR_DUPLICATED_EPIGENETIC_MARK_NAME;

  extern Error ERR_DUPLICATE_PROJECT_NAME;

  extern Error ERR_DUPLICATE_SAMPLE_FIELD_NAME;

  extern Error ERR_DUPLICATE_GENOME_NAME;

  extern Error ERR_DATASET_NOT_FOUND;

  extern Error ERR_DATABASE_CONNECTION;
  extern Error ERR_DATABASE_EXCEPTION;
  extern Error ERR_DATABASE_INVALID_BIOSOURCE;
}