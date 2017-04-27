import os
import xmlrpclib
import time

import StringIO
import gzip

url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
genome = "grch38"
user_key = "anonymous_key"

server = xmlrpclib.Server(url, allow_none=True)

status, annotations = server.list_annotations (genome, user_key )
status, annotations_ids = server.extract_ids(annotations)
status, annotations_info = server.info(annotations_ids, user_key)

requests = []

for annotation in annotations_info:
  info = []

  # we are not going to store the generated "Pattern" regions
  if annotation["name"].startswith("Pattern "):
    continue

  if annotation["name"].startswith("Chromosomes size"):
    continue

  info.append(("name", annotation["name"]))
  info.append(("description", annotation["description"]))
  info.append(("format", annotation["format"]))
  info.extend( [("extra_metadata__"+k, v) for k, v in annotation.get("extra_metadata", {}).iteritems()] )

  status, q_id = server.select_annotations(annotation["name"], annotation["genome"], None, None, None, user_key)
  status, request_id = server.get_regions(q_id, annotation["format"], user_key)

  # Storing the annotation dict for the download part... and info for serialization to the file
  requests.append((request_id, annotation, info))

from pprint import pprint
print "Requests:"
pprint(requests)
print "-" * 30

if not os.path.isdir("data/"):
  os.mkdir("data/")

_dir = "data/" + genome + "/"
if not os.path.isdir(_dir):
    os.mkdir(_dir)

while len(requests) > 0:
    for req in requests[:]:
        print req[0]
        (s, ss) = server.info(req[0], user_key)
        print ss
        if ss[0]["state"] == "done":
            print "getting data from " + ss[0]["_id"]
            (s, request_data) = server.get_request_data(req[0], user_key)
            out = StringIO.StringIO()
            # Compress in memory
            with gzip.GzipFile(fileobj=out, mode="w") as f:
              f.write(request_data)

            #Store the compressed data into the file
            file_path = _dir + req[1]["name"] +".bed.gz"
            print file_path
            with open(file_path, 'wb') as data_file:
              data_file.write(out.getvalue())

            # Now, store the metadata
            with open(file_path+".metadata", 'w') as metadata_file:
              metadata_file.write("\n".join([line[0] + "\t" + line[1] for line in req[2]]))

            requests.remove(req)
        if ss[0]["state"] == "failed":
            print ss
            requests.remove(req)
    time.sleep(1.0)