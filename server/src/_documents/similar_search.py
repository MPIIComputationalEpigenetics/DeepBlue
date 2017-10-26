import xmlrpclib
import time
import os.path
import errno

from collections import defaultdict
from pprint import pprint
from multiprocessing import Pool
from socket import error as socket_error

import cPickle

# Internal configurations
# Do not change this value unless you really know what you are doing.
MAX_REQUEST_SIMULTANEOUS = 16
CACHE_PATH = ".cache"
STATES_CACHE_FILE = "css_queries_cache.deepblue"
#DEEPBLUE_URL = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
DEEPBLUE_URL = "http://localhost:31415"
USER_KEY = 'anonymous_key'

# User parameters (TODO: move to args)
GENOME = 'GRCh38'


def request_data(request_id):
    (status, info) = SERVER.info(request_id, USER_KEY)
    request_status = info[0]["state"]

    while request_status != "done" and request_status != "failed":
        print info
        time.sleep(10)
        (status, info) = SERVER.info(request_id, USER_KEY)
        request_status = info[0]["state"]

    (status, data) = SERVER.get_request_data(request_id, USER_KEY)
    if status != "okay":
        print data
        return None

    return data


def get_chromatin_stes(genome, user_key):

    status, csss_query_id = SERVER.select_regions(
        None, genome, "Chromatin State Segmentation",
        None, None, None, None, None, None, user_key)
    print status, csss_query_id

    status, csss_names_request_id = SERVER.distinct_column_values(
        csss_query_id, "NAME", user_key)
    print csss_names_request_id
    distinct_values = request_data(csss_names_request_id)
    print distinct_values['distinct']

    return distinct_values['distinct']


def split_file(data):
    server_worker = xmlrpclib.Server(DEEPBLUE_URL, allow_none=True)

    [css_file, states] = data
    result = []

    while True:
        try:
            _, css_file_query_id = server_worker.select_experiments(
                css_file, None, None, None, USER_KEY)
            _, css_file_query_cache_id = server_worker.query_cache(
                css_file_query_id, True, USER_KEY)
            for state in states:
                _, css_file_filter_query_id = server_worker.filter_regions(
                    css_file_query_cache_id, "NAME", "==", state, "string", USER_KEY)
                result.append((state, css_file_filter_query_id))

            print css_file
            return css_file, result
        except socket_error as serr:
            print serr


def build_chromatin_state_files(server, genome, user_key):
    cache_file = os.path.join(CACHE_PATH, STATES_CACHE_FILE)

    if os.path.exists(cache_file):
        _file = open(cache_file, "r")
        queries = cPickle.load(_file)
        return queries
    else:
        _, css_files_list = server.list_experiments(
            genome, "peaks", "Chromatin State Segmentation", None, None, None, None, user_key)
        _, css_files_names = server.extract_names(css_files_list)
        states = get_chromatin_stes(genome, user_key)

        css_files = []
        for css_file in css_files_names:
            css_files.append([css_file, states])

        pool = Pool(MAX_REQUEST_SIMULTANEOUS)
        queries = pool.map(split_file, css_files)
        pprint(queries)
        datasets = defaultdict(list)

        for query in queries:
            for state in query[1]:
                datasets[query[0]].append([state[1], state[0]])

        datasets = dict(datasets)

        print datasets

        pool.close()
        pool.join()

        try:
            os.makedirs(CACHE_PATH)
        except OSError as exc:  # Python >2.5
            if exc.errno == errno.EEXIST and os.path.isdir(CACHE_PATH):
                pass
            else:
                raise

        _file = open(cache_file, "w+")
        # print queries
        cPickle.dump(datasets, _file)
        return datasets


def build_annotations(server, user_key):
    pass

if __name__ == "__main__":
    ####
    SERVER = xmlrpclib.Server(DEEPBLUE_URL, allow_none=True)
    print SERVER.echo(USER_KEY)

    # Query data
    # Select the hypo methylated regions where the AVG methyl level is lower
    # than 0.0025
    _, QUERY_ID = SERVER.select_experiments(
        "S0145XH1.ERX1007389.H3K27ac.bwa.GRCh38.20151015.bed", None, None, None, USER_KEY)
    print QUERY_ID

    _, request_id = SERVER.enrich_regions_fast(QUERY_ID, "GRCh38", None, None, None, "Chip-Seq", None, USER_KEY)

    data = request_data(request_id)
    pprint(data)


    #'CD4-positive, alpha-beta T cell'