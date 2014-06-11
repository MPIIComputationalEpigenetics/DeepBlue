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

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>

#include "../extras/utils.hpp"
#include "../extras/serialize.hpp"
#include "../extras/xmlrpc.hpp"

#include "parameters.hpp"

namespace epidb {
  typedef std::map<std::string, std::string> Metadata;

  bool read_metadata(const serialize::ParameterPtr &map,
                     Metadata &metadata, std::string &msg);

  struct CommandCategory {
    CommandCategory(const std::string &n, const std::string &d)
      : name(n), description(d)
    {}

    std::string name;
    std::string description;
  };

  namespace categories {
    static const CommandCategory ADMINISTRATION("Administration", "Administration commands");
    static const CommandCategory ANNOTATIONS("Annotations", "Inserting and listing annotations");
    static const CommandCategory BIO_SOURCES("Bio sources", "Inserting and listing bio sources");
    static const CommandCategory BIO_SOURCE_RELATIONSHIP("Bio source relationship", "Set the relationship between different bio sources");
    static const CommandCategory COLUMN_TYPES("Column Types", "Inserting and listing different column types");
    static const CommandCategory EPIGENETIC_MARKS("Epigenetic marks", "Inserting and listing epigenetic marks");
    static const CommandCategory EXPERIMENTS("Experiments", "Inserting and listing experiments");
    static const CommandCategory GENERAL_INFORMATION("General Information", "Commands for all types of data");
    static const CommandCategory GENOMES("Genomes", "Inserting and listing genomes");
    static const CommandCategory HELP("Help", "Help information about DeepBlue usage");
    static const CommandCategory OPERATIONS("Operations", "Different querying operations");
    static const CommandCategory PROJECTS("Projects", "Inserting and listing projects");
    static const CommandCategory SAMPLES("Samples", "Inserting and listing samples");
    static const CommandCategory STATUS("Status", "Checking Deep Blue status");
    static const CommandCategory TECHNIQUES("Techniques", "Inserting and listing techniques");
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

    bool check_local(const std::string &ip, bool &r, std::string &msg) const;

    bool check_init(bool &r, std::string &msg) const;

    bool check_email(const std::string &email, bool &r, std::string &msg) const;

    const std::string gen_random(const size_t len) const;

    void set_id_names_return(const std::vector<utils::IdName> &id_names, serialize::Parameters &result) const;

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
} // namespace epidb

#endif
