import xmlrpclib

uk = "bnuEsTzM3l640iOl"
url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"

server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True, verbose=False)

print server.echo(uk)

print server.list_projects(uk)

genome = "hg19"
epigenetic_mark = "Methylation"
sample = None
technique = None
project = 'Blueprint Epigenetics'
project = "ENCODE"

sample = "s1"
(s, e) = server.list_experiments(genome, epigenetic_mark, sample, technique, project, uk)

print len(e)