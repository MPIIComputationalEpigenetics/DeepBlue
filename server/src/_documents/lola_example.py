import xmlrpclib
import time

from collections import defaultdict

def request_data(request_id, uk):
    (status, info) = server.info(request_id, uk)
    request_status = info[0]["state"]

    while request_status != "done" and request_status != "failed":
        time.sleep(1)
        (status, info) = server.info(request_id, uk)
        request_status = info[0]["state"]

    (status, data) = server.get_request_data(request_id, uk)
    if status != "okay":
        print data
        return None

    return data


def get_chromatin_stes(genome, uk):

    status, csss_query_id = server.select_regions(
        None, genome, "Chromatin State Segmentation", None, None, None, None, None, None, uk)
    print status, csss_query_id

    status, csss_names_request_id = server.distinct_column_values(
        csss_query_id, "NAME", uk)
    distinct_values = request_data(csss_names_request_id, uk)
    print distinct_values['distinct']

    return distinct_values['distinct']


def build_chromatin_state_files(uk):
    datasets = defaultdict(list)
    css_files = server.list_experiments(
        "GRCh38", "peaks", "Chromatin State Segmentation", None, None, None, None, uk)
    css_files_names = server.extract_names(css_files[1])[1]
    states = get_chromatin_stes('GRCh38', uk)
    for css_file in css_files_names:
        for state in states:
            ss, css_file_query_id = server.select_experiments(
                css_file, None, None, None, uk)
            ss, css_file_query_cache_id = server.query_cache(css_file_query_id, True, uk)
            ss, css_file_filter_query_id = server.filter_regions(
                css_file_query_cache_id, "NAME", "==", state, "string", uk)
            datasets[css_file].append(css_file_filter_query_id)
    return dict(datasets)


####
uk = 'anonymous_key'
server = xmlrpclib.Server("http://localhost:31415", allow_none=True)
print server.echo(uk)

# Query data
# Select the hypo methilated regions where the AVG methyl level is lower than 0.0025
ss, query_id = server.select_experiments(
    "S00VEQA1.hypo_meth.bs_call.GRCh38.20150707.bed", None, None, None, uk)
ss, query_id = server.filter_regions(
    query_id, "AVG_METHYL_LEVEL", "<", "0.0025", "number", uk)


# Universe will be tiling regions of 1000bp
ss, universe_query_id = server.tiling_regions(1000, "grch38", None, uk)

# Datasets will be the chromatin segmentation files divided by segments
datasets = build_chromatin_state_files(uk)
print "total of " + str(len(datasets)) + " datasets"

s, request = server.lola(query_id, universe_query_id, datasets, "grch38", uk)
print request

distinct_values = request_data(request, uk)

print distinct_values
