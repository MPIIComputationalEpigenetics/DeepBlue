#
#  shell.py
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

from commands import get_command

class Shell:
    def __init__(self, context):
        self._context = context

    def loop(self):
        ok = True
        while ok is True:
            epidb = self._context.epidb
            line = raw_input(str(epidb.echo(self._context.user_key)) + " > ")
            cmd = line.strip()
            if len(cmd) == 0:
                continue
            command = get_command(cmd)
            if command is None:
                continue
            command(self._context)
