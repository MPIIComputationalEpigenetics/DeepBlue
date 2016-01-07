import xmlrpclib

class DeepBlueClient:
    """Conveniently access a DeepBlue server
    """
    def __init__(self, user_key=None, address="deepblue.mpi-inf.mpg.de/xmlrpc", port=None):
      """
      :param user_key: Authentication key to be used for all commands
      :param address: Address of the server DeepBlue runs on
      :param port: Port on the server DeepBlue listens on
      """
      self._user_key = user_key
      if port is not None:
        url = "http://%s:%d" % (address, port)
      else:
        url = "http://%s" % address

      self._server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True, verbose=False)
      self._commands = self._server.commands()[1]
      self._commands.update(self._server.admin_commands()[1])

      self._require_user_key = {}
      self._parameters_count = {}

      for k in self._commands:
        k_params = self._commands[k]["parameters"]
        need_user_key = k_params[-1][0] == "user_key" if len(k_params) > 0  else False
        self._require_user_key[k] = need_user_key
        self._parameters_count[k] = len(k_params)

    def __nonzero__(self):
      return True

    def __getattr__(self, name):
      call_proxy = getattr(self._server, name)
      def _call(*args, **kwargs):
        if self._require_user_key[name] and len(args) < self._parameters_count[name]:
          return call_proxy(*args + (self._user_key,))
        else:
          return call_proxy(*args)
      return _call
