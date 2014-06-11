import xmlrpclib

#url = "http://localhost:31415"
#uk = "ML9zc2AZTSIgiDGU"

uk = "ov8yw7gg8JmO4V6W"
url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"

server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True, verbose=False)
print server.echo(uk)
(s, ann) = server.list_annotations("hg19", uk)
print ann
ann_names = [x[1] for x in ann]
(status, qid_hoxgenes) = server.select_annotations(ann_names, "hg19", ["chr1", "chr2", "chr3", "chr4", "chrX", "chrY"], None, None, uk)
print server.count_regions(qid_hoxgenes, uk)
x = server.get_regions(qid_hoxgenes, "CHROMOSOME,START,END", uk)
print len(x[1].split("\n"))
#print x

(status, qid_h3k27me3_gm12878) = server.select_regions('wgEncodeBroadHistoneGm12878H3k27me3StdPk', "hg19", None, None, None, None, None, None, None, uk)
print server.count_regions(qid_h3k27me3_gm12878, uk)
x = server.get_regions(qid_h3k27me3_gm12878, "CHROMOSOME,START,END,@NAME", uk)
print len(x[1].split("\n"))
