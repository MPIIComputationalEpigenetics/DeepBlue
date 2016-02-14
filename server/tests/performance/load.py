import gzip
from pymongo import MongoClient
import xmlrpclib
import time

url = "http://localhost:31415"
server = xmlrpclib.Server(url, allow_none=True)
uk= "lsUz0a5NbqXbmXaj"

mongo = MongoClient("localhost", 27017)
mongo.drop_database("tests_suite")

(status, uk) = server.init_system("est_admin", "deepblue@mpi-inf.mpg.de", "MPI-Inf")

hg19_info = open("../data/genomes/hg19").read().replace(",", "")
print server.add_genome("hg19", "hg19", hg19_info, uk)

f = gzip.open("bigwig.bg.gz")
data = f.read()
(status, a1) = server.add_annotation("exp_wig", "hg19", "bla", data, "bedgraph", None, uk)


cpg_island =  ",".join([
 "CHROMOSOME",
 "START",
 "END",
 "NAME",
 "LENGTH",
 "COUNT1",
 "COUNT2",
 "COUNT3",
 "COUNT4",
 "COUNT5"
])

print server.create_column_type_simple("NAME", 'name', "string", uk)

print server.create_column_type_simple("LENGTH", 'len', "integer", uk)
print server.create_column_type_simple("COUNT1", 'len', "integer", uk)
print server.create_column_type_simple("COUNT2", 'len', "integer", uk)
print server.create_column_type_simple("COUNT3", 'len', "double", uk)
print server.create_column_type_simple("COUNT4", 'len', "double", uk)
print server.create_column_type_simple("COUNT5", 'len', "double", uk)

data = open("cpgIslandExtFull.bed").read()
(status, a2) = server.add_annotation("cpg", "hg19", "bla", data, cpg_island, None, uk)

(status, q1) = server.select_annotations("exp_wig", "hg19", None, None, None, uk)
print q1
(status, q2) = server.select_annotations("cpg", "hg19", None, None, None, uk)
print q2

(status, q3) = server.intersection(q1, q2, uk)
print q3

(status, r1) = server.count_regions(q3, uk)
print r1

# Wait the processing
(status, info) = server.info(r1, uk)
while info[0]["state"] != "done" and info[0]["state"] != "error":
    time.sleep(5)
    (status, info) = server.info(r1, uk)

(status, regions) = server.get_request_data(r1, uk)
print regions


(status, q3) = server.intersection(q2, q1, uk)
print q3

(status, r1) = server.count_regions(q3, uk)
print r1

# Wait the processing
(status, info) = server.info(r1, uk)
while info[0]["state"] != "done" and info[0]["state"] != "error":
    time.sleep(5)
    (status, info) = server.info(r1, uk)

(status, regions) = server.get_request_data(r1, uk)
print regions

(status, q3) = server.intersection(q2, q2, uk)
print q3

(status, r1) = server.count_regions(q3, uk)
print r1

# Wait the processing
(status, info) = server.info(r1, uk)
while info[0]["state"] != "done" and info[0]["state"] != "error":
    time.sleep(5)
    (status, info) = server.info(r1, uk)
(status, regions) = server.get_request_data(r1, uk)
print regions

(status, q3) = server.intersection(q1, q1, uk)
print q3

(status, r1) = server.count_regions(q3, uk)
print r1

# Wait the processing
(status, info) = server.info(r1, uk)
while info[0]["state"] != "done" and info[0]["state"] != "error":
    time.sleep(5)
    (status, info) = server.info(r1, uk)

(status, regions) = server.get_request_data(r1, uk)
print regions

"""
            old code (intersect)        -> iterators  -> check end data -> fixing bug (clone) -> remove data it -> multithread
32,107      453, 453, 453               -> 264        -> 257            -> 370, 324, 320      -> 301             -> 287,281
3,904       1210, 1128, 1277            -> 282        -> 267            -> 367, 355, 371      -> 280             -> 300, 294
28,691      21, 24 , 19                 -> 11         -> 11             -> 19, 19, 25         -> 10              -> 28, 19
3,996,097 - 2101, 2249 , 2247           -> 546        -> 536            -> 895, 1095, 857     -> 620             -> 570, 535
"""


