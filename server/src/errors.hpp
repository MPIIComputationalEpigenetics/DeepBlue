//
//  errors.hpp
//  epidb
//
//  Created by Felipe Albrecht on 23.06.14.
//  Copyright (c) 2013,2014 Max Planck Institute for Computer Science. All rights reserved.
//

#include <string>
#include <map>

namespace epidb {

	class Error {
	private:
		std::string code_value;
		std::string err_fmt;
	public:
		Error(const std::string& c, const std::string& f) :
			code_value(c),
			err_fmt(f) {}

		static std::string m(const Error e, ...);
	};

	extern Error ERR_INVALID_BIO_SOURCE_NAME;
	extern Error ERR_DUPLICATED_BIO_SOURCE_NAME;
	extern Error ERR_MORE_EMBRACING_BIO_SOURCE_NAME;

	extern Error ERR_DUPLICATED_EPIGENETIC_MARK_NAME;

	extern Error ERR_DATABASE_EXCEPTION;
	extern Error ERR_DATABASE_INVALID_BIO_SOURCE;
}