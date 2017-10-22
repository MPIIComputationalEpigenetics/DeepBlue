import xmlrpclib
import time
import os.path
import errno

from collections import defaultdict
from pprint import pprint
from multiprocessing import Pool
from socket import error as socket_error

import cPickle

import itertools

# Internal configurations
# Do not change this value unless you really know what you are doing.
MAX_REQUEST_SIMULTANEOUS = 16
DEEPBLUE_URL = "http://localhost:31415"
USER_KEY = '1kbgOVXjTJuQ4v8s'

# User parameters (TODO: move to args)
GENOME = 'GRCh38'
DATA_DIR = "/home/albrecht/DeepBlue/DeepBlue-Populator/data"

max_threads = 4

import gzip

def insert_sequence(t):
    user_key = t[0]
    seq_info = t[1]

    print seq_info

    print "Inserting " + seq_info["genome"] + " for " + seq_info["chromosome"]

    sequence = gzip.open(seq_info["sequence_path"]).read()

    server = xmlrpclib.Server(DEEPBLUE_URL, allow_none=True)

    print server.upload_chromosome(seq_info["genome"], seq_info["chromosome"], sequence, USER_KEY)


def insert_chromosome_sequences(genome, user_key):
    seqs_dir = os.path.join(DATA_DIR, "genomes/", genome)

    seq_infos = []

    for file_name in os.listdir(seqs_dir):
        if file_name.endswith(".fa.gz"):
            chromosome = file_name[:-6] # remove .fa.gz
            f_full = os.path.join(seqs_dir, file_name)

            seq_info = {}
            seq_info["genome"] = genome
            seq_info["chromosome"] = chromosome
            seq_info["sequence_path"] = f_full

            seq_infos.append(seq_info)

    total = len(seq_infos)
    count = 0
    p = Pool(max_threads)
    print "Inserting Chromosome Sequence. Total of " + str(total) + " sequences."

    p.map(insert_sequence, itertools.izip(itertools.repeat(user_key), seq_infos))

    p.close()
    p.join()


insert_chromosome_sequences("GRCh38", USER_KEY)