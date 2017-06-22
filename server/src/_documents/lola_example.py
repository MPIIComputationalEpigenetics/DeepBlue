import xmlrpclib

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

