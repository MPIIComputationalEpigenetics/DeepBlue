//
//  add_sample_from_gsm.cpp
//  epidb
//
//  Created by Felipe Albrecht on 10.12.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#import "../datatypes/metadata.hpp"

#import "url_loader.hpp"

namespace epidb {
  namespace external {
    namespace geo {

      const std::string GSM_QUERY_PAGE="http://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?targ=gsm&form=text&view=quick&acc=";

      bool load_gsm(const std::string &gsm_id, datatypes::Metadata &metadata, std::string &msg)
      {

      	std::string url_address = GSM_QUERY_PAGE + gsm_id;

      	std::string content;
      	if (!url_loader::load(url_address, content, msg)) {
      		return false;
      	}

      	std::cerr << content << std::endl;

        return true;
      }

    }
  }
}
