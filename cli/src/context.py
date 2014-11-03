from epidb_client import EpidbClient
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
        epidb = EpidbClient(self._address, self._port)
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
