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
                      default=31415, help="Server port")

    args = parser.parse_args()

    server = args[0].server
    port = args[0].port

    context = Context(server, port)
    shell = Shell(context)
    shell.loop()
