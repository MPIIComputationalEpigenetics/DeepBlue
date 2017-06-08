
//
//  version.cpp
//  DeepBlue Epigenomic Data Server
//  File created by Felipe Albrecht on 10.09.13.
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

#include "version.hpp"

namespace epidb {
  const std::string Version::name = "DeepBlue Epigenomic Data Server";
  const std::string Version::copyright = "Copyright Copyright (c) 2015,2016,2017 Max Planck Institute for Informatics. All rights reserved.";
  const std::string Version::license = "This program comes with ABSOLUTELY NO WARRANTY.\nThis is free software, and you are welcome to redistribute it under certain conditions.\n";
  const std::string Version::terms = "This program is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n"
	"\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
	"\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program.  If not, see <http://www.gnu.org/licenses/>.";
  const std::string Version::author = "Felipe Albrecht - felipe.albrecht@mpi-inf.mpg.de";
  const size_t Version::major_version = 1;
  const size_t Version::minor_version = 12;
  const size_t Version::fix_version = 13;
}
