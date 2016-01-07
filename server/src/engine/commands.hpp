//
//  commands.hpp
//  epidb
//
//  Created by Felipe Albrecht on 29.05.13.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#ifndef EPIDB_ENGINE_COMMANDS_HPP
#define EPIDB_ENGINE_COMMANDS_HPP

#include <iostream>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "../datatypes/metadata.hpp"
#include "../datatypes/user.hpp"

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"
#include "../extras/xmlrpc.hpp"

#include "parameters.hpp"

namespace epidb {
  bool read_metadata(const serialize::ParameterPtr &map,
                     datatypes::Metadata &metadata, std::string &msg);

  bool extract_items(std::vector<serialize::ParameterPtr> input_list, size_t position, serialize::Type type,
                     serialize::Parameters &result);

  struct CommandCategory {
    CommandCategory(const std::string &n, const std::string &d)
      : name(n), description(d)
    {}

    std::string name;
    std::string description;
  };

  bool operator==(const CommandCategory &c1, const CommandCategory &c2);

  namespace categories {
    static const CommandCategory ADMINISTRATION("Administration", "Administration commands");
    static const CommandCategory ANNOTATIONS("Annotations", "Inserting and listing annotations");
    static const CommandCategory BIOSOURCES("BioSources", "Inserting and listing biosources");
    static const CommandCategory BIOSOURCE_RELATIONSHIP("BioSources relationship", "Set the relationship between different biosources");
    static const CommandCategory COLUMN_TYPES("Column Types", "Inserting and listing different column types");
    static const CommandCategory DATA_MODIFICATION("Data Modification", "Operations that modify the data content");
    static const CommandCategory EPIGENETIC_MARKS("Epigenetic marks", "Inserting and listing epigenetic marks");
    static const CommandCategory EXPERIMENTS("Experiments", "Inserting and listing experiments");
    static const CommandCategory GENERAL_INFORMATION("General Information", "Commands for all types of data");
    static const CommandCategory GENOMES("Genomes", "Inserting and listing genomes");
    static const CommandCategory GENES("Genes", "Operations on gene sets and genes identifiers");
    static const CommandCategory HELP("Help", "Help information about DeepBlue usage");
    static const CommandCategory OPERATIONS("Regions Operations", "Operating on the data regions");
    static const CommandCategory REQUESTS("Requests", "Requests status information and results");
    static const CommandCategory PROJECTS("Projects", "Inserting and listing projects");
    static const CommandCategory SAMPLES("Samples", "Inserting and listing samples");
    static const CommandCategory STATUS("Status", "Checking DeepBlue status");
    static const CommandCategory TECHNIQUES("Techniques", "Inserting and listing techniques");
    static const CommandCategory UTILITIES("Utilities", "Utilities for connecting operations");
  }

  struct CommandDescription {
    CommandDescription(const CommandCategory &c, const std::string &d)
      : category(c), description(d)
    {}

    CommandCategory category;
    std::string description;
  };

  class Command {

  protected:
    std::string name_;
    CommandDescription desc_;
    Parameters parameters_;
    Parameters results_;

    static std::map<std::string, Command *> *commands_;

    static bool checks(const std::string &user_key, std::string &msg);

    bool check_local(const std::string &ip, bool &r, std::string &msg) const;

    bool check_init(bool &r, std::string &msg) const;

    bool check_permissions(const std::string& user_key, const datatypes::PermissionLevel& permission, datatypes::User &user, std::string& msg) const;

    void set_id_names_return(const std::vector<utils::IdName> &id_names, serialize::Parameters &result) const;

    void set_id_names_count_return(const std::vector<utils::IdNameCount> &id_name_counts, serialize::Parameters &result) const;

    void init();

  public:
    Command(const std::string &name, const Parameters &params, const Parameters &results, const CommandDescription &desc);
    virtual ~Command() {}

    const Parameters parameters() const
    {
      return parameters_;
    }

    const Parameters results() const
    {
      return results_;
    }

    const CommandDescription description() const
    {
      return desc_;
    }

    virtual bool run(const std::string &ip, const serialize::Parameters &params,
                     serialize::Parameters &result) const = 0;

    bool check_parameters(serialize::Parameters &parameters,
                          std::string &msg) const;

    static const Command *get_command(const std::string &name);
  };

  serialize::ParameterPtr build_command_info(const Command* command);
} // namespace epidb

#endif
