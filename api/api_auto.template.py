# Python helper for For DeepBlue version {{ version }}

import xmlrpclib

deepblue_URL = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
deepblue_USER_KEY = "anonymous_key"
deepblue_debug_VERBOSE = False

def key_required(func):
    def func_wrapper(self, *args, **kwargs):
        if self.key:
            return func(self, *args, **kwargs)
        else:
            raise AttributeError("To use this function a key is required. Set it using "
                                 "set_key or construct DeepBlueClient providing a key")
    return func_wrapper

class DeepBlueClient(object):
  """Conveniently access a DeepBlue server
  """
  def __init__(self, key=None, address=DEEPBLUE_url, port=None):
    """
    :param key: Authentication key to be used for all commands
    :param address: Address of the server DeepBlue runs on
    :param port: Port on the server DeepBlue listens on
    """
    self.key = key
    if port is not None:
      url = "http://%s:%d" % (address, port)
    else:
      url = "http://%s" % address
    self.server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True, verbose=False)


{{ commands }}

if __name__ == "__main__":
  server = DeepBlueClient()
  print server.echo(deepblue_USER_KEY)