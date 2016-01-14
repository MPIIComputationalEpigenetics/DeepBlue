#
#  context.py
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

from deepblue_client import DeepBlueClient
from config import Config


class Context:
    def __init__(self, address, port):
        self._address = address
        self._port = port
        self._config = Config()

    @property
    def address(self):
        return self._address

    @property
    def port(self):
        return self._port

    @property
    def epidb(self):
        epidb = DeepBlueClient("deepblue.mpi-inf.mpg.de/xmlrpc")
        return epidb

    @property
    def email_user(self):
        return self._config.email_user

    @property
    def email_password(self):
        return self._config.email_password

    @property
    def user_key(self):
        return self._config.user_key
