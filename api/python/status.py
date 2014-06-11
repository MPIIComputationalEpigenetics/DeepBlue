import xmlrpclib

address = "infao6940"
port = 31415

url = "http://%s:%d" % (address, port)
server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True)
r = server.echo(None)

print r