//
//  errors.hpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 23.06.14.
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


#ifndef EPIDB_ERRORS_HPP
#define EPIDB_ERRORS_HPP

#include <string>
#include <format.h>

#include <boost/algorithm/string.hpp>

namespace epidb {

  class Error {
  private:
    std::string code_value;
    std::string err_fmt;
  public:
    Error(const std::string &c, const std::string &f) :
      code_value(c),
      err_fmt(f) {}


    std::string code() const {
      return code_value;
    }

    template<typename... Args>
    static std::string m(const Error e, const Args & ... args)
    {
      std::stringstream ss;
      //ss << e.code_value;
      //ss << ":";
      ss << fmt::format("{}:" + e.err_fmt, e.code_value, args...);
      return ss.str();
    }

    static std::string code(const std::string &msg) {
      std::vector<std::string> strs;
      boost::split(strs, msg, boost::is_any_of("\t"));
      return strs[0];
    }

  };

  extern Error ERR_USER_USER_MISSING;
  extern Error ERR_INSUFFICIENT_PERMISSION;
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
  extern Error ERR_USER_GENE_MISSING;
  extern Error ERR_USER_GENE_MODEL_MISSING;

  extern Error ERR_FORMAT_CHROMOSOME_MISSING;
  extern Error ERR_FORMAT_START_MISSING;
  extern Error ERR_FORMAT_END_MISSING;
  extern Error ERR_FORMAT_COLUMN_NAME_MISSING;

  extern Error ERR_INVALID_EXPERIMENT_NAME;
  extern Error ERR_INVALID_EXPERIMENT_ID;
  extern Error ERR_INVALID_EXPERIMENT_COLUMN;

  extern Error ERR_INVALID_ANNOTATION_NAME;
  extern Error ERR_INVALID_ANNOTATION_ID;

  extern Error ERR_INVALID_PRA_PROCESSED_ANNOTATION_NAME;

  extern Error ERR_INVALID_PROJECT_NAME;
  extern Error ERR_INVALID_PROJECT_ID;
  extern Error ERR_PERMISSION_PROJECT;

  extern Error ERR_INVALID_TECHNIQUE_ID;
  extern Error ERR_INVALID_BIOSOURCE_ID;
  extern Error ERR_INVALID_EPIGENETIC_MARK_ID;
  extern Error ERR_INVALID_GENE_MODEL_ID;
  extern Error ERR_INVALID_TILING_REGIONS_ID;
  extern Error ERR_INVALID_COLUMN_TYPE_ID;

  extern Error ERR_INVALID_COLLECTION_NAME;

  extern Error ERR_INVALID_CHROMOSOME_NAME;
  extern Error ERR_INVALID_CHROMOSOME_NAME_GENOME;

  extern Error ERR_INVALID_GENOME_NAME;
  extern Error ERR_INVALID_GENOME_ID;

  extern Error ERR_INVALID_SAMPLE_ID;

  extern Error ERR_INVALID_BIOSOURCE_NAME;
  extern Error ERR_INVALID_QUERY_ID;
  extern Error ERR_PERMISSION_QUERY;

  extern Error ERR_INVALID_COLUMN_NAME;
  extern Error ERR_DUPLICATED_COLUMN_NAME;

  extern Error ERR_INVALID_META_COLUMN_NAME;

  extern Error ERR_DUPLICATED_BIOSOURCE_NAME;
  extern Error ERR_DUPLICATED_EXPERIMENT_NAME;
  extern Error ERR_DUPLICATED_GENE_MODEL_NAME;

  extern Error ERR_MORE_EMBRACING_BIOSOURCE_NAME;

  extern Error ERR_ALREADY_CONECTED_BIOSOURCE_NAME;
  extern Error ERR_ALREADY_PARENT_BIOSOURCE_NAME;

  extern Error ERR_INVALID_BIOSOURCE_SYNONYM;
  extern Error ERR_INVALID_USER_KEY;
  extern Error ERR_INVALID_USER_NAME;
  extern Error ERR_INVALID_USER_ID;
  extern Error ERR_INVALID_USER_EMAIL_PASSWORD;

  extern Error ERR_DUPLICATED_EPIGENETIC_MARK_NAME;
  extern Error ERR_DUPLICATED_PROJECT_NAME;
  extern Error ERR_DUPLICATED_GENOME_NAME;
  extern Error ERR_DUPLICATED_TECHNIQUE_NAME;

  extern Error ERR_INVALID_START;
  extern Error ERR_INVALID_LENGTH;

  extern Error ERR_UNKNOW_QUERY_TYPE;

  extern Error ERR_COLUMN_TYPE_MISSING;
  extern Error ERR_COLUMN_TYPE_NAME_MISSING;

  extern Error ERR_INVALID_IDENTIFIER;
  extern Error ERR_INVALID_GSM_IDENTIFIER;

  extern Error ERR_INVALID_INPUT_TYPE;

  extern Error ERR_INVALID_INPUT_SUB_ITEM_SIZE;
  extern Error ERR_INVALID_INPUT_SUB_ITEM_TYPE;

  extern Error ERR_DATASET_NOT_FOUND;

  extern Error ERR_NAME_NOT_FOUND;

  extern Error ERR_DATABASE_CONNECTION;
  extern Error ERR_DATABASE_EXCEPTION;
  extern Error ERR_DATABASE_INVALID_BIOSOURCE;

  extern Error ERR_REQUEST_CANCELED;
  extern Error ERR_REQUEST_ID_INVALID;

  extern Error ERR_INVALID_INTERNAL_NAME;
}

#endif
