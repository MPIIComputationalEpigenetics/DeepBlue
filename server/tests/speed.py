import xmlrpclib

#url = "http://localhost:31415"
#uk = "ML9zc2AZTSIgiDGU"

uk = "ov8yw7gg8JmO4V6W"
url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True, verbose=False)

print server.echo(None)

(status, qid_hoxgenes) = server.select_annotations("Genes", "hg19", "chr7", 20000000, 27250000, uk)

print status

print server.count_regions(qid_hoxgenes, uk)

x = server.get_regions(qid_hoxgenes, "CHROMOSOME,START,END", uk)
print len(x[1].split("\n"))

print x