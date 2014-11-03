

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
