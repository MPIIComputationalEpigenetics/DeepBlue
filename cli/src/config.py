#
#  config.py
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

class Config:
    def __init__(self):
        self._options = {}
        f = open('.deepblue_cli_config')
        for line in f:
            line = line.strip()
            s = line.split('=', 1)
            self._options[s[0]] = s[1]

    @property
    def email_server(self):
        return self._options['email_server']

    @property
    def email_port(self):
        return self._options['email_port']

    @property
    def email_user(self):
        return self._options['email_user']

    @property
    def email_password(self):
        return self._options['email_password']

    @property
    def user_key(self):
        return self._options['user_key']

    @property
    def email_sender(self):
        return self._options['email_sender']

c = Config()

print c
