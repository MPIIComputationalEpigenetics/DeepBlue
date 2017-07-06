import xmlrpclib


"""
Lola algorithm in DeepBlue

 ## main variables
 - query_set -> regions what the user want to anaylize
 - universe_regions -> background regions
 - datasets -> set of regions, here, it will be mainly the DeepBlue experiments

 1. query_overlap_with_universe = regions from query_set that overlap with regions from the universe_regions.

 # We get the regions of each dataset
 for each dataset_regions in datasets:
  -

  ## Support
  support = dataset_regions that overlap with query_overlap_with_universe

  ## Please, here is my main question. Is it right?
  universe_overlap_total = universe_regions that overlap with the dataset_regions
  b = universe_overlap_total - a

  ##
  c = regions count  of query_set minus support

  ##
  d = regions count of universe_regions - a - b - c


  fisher_result = fisher(a, b, c, d)
  log_score = abs(log(fisher_result)) # Positive value of the log score

"""

server = xmlrpclib.Server("http://localhost:31415", allow_none=True)

uk = 'anonymous_key'

print server.echo(uk)

def request_data(request_id):
  (status, info) = server.info(request_id, user_key)
  request_status = info[0]["state"]

  while request_status != "done" and request_status != "failed":
    print request_status
    time.sleep(1)
    (status, info) = server.info(request_id, user_key)
    request_status = info[0]["state"]

  (status, data) = server.get_request_data(request_id, user_key)
  if status != "okay":
    print data
    return None

  return data


h3k27ac = server.list_experiments ("GRCh38", "peaks", "h3k27ac", None, None, "ChIP-seq", None, uk)
h3k27ac_ids = server.extract_names(h3k27ac[1])[1]

h3k27me3 = server.list_experiments ("GRCh38", "peaks", "h3k27me3", None, None, "ChIP-seq", None, uk)
h3k27me3_ids = server.extract_names(h3k27me3[1])[1]

databases = {
  "h3k27ac" : h3k27ac_ids,
  "h3k27me3" : h3k27ac_ids
}

print databases

ss, query_id = server.select_annotations("Promoters", "grch38", None, None, None, uk)
ss, universe_query_id = server.select_regions(None, "grch38", None, None, "chip-seq", None, None, None, None, uk)

print query_id
print ss, universe_query_id

s, request = server.lola(query_id, universe_query_id, databases, "grch38", uk)

print request

