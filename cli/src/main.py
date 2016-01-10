#
#  main.py
#  DeepBlue Epigenomic Data Server - CLI
#
#  Created by Felipe Albrecht
#  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

from optparse import OptionParser

from context import Context
from shell import Shell


def main():
    pass

if __name__ == '__main__':
    parser = OptionParser()

    parser.add_option("--server", dest="server",
                      default="localhost", help="Server address")
    parser.add_option("--port", type="int", dest="port",
                      default=0, help="Server port")

    args = parser.parse_args()

    server = args[0].server
    port = args[0].port

    context = Context(server, port)
    shell = Shell(context)
    shell.loop()
