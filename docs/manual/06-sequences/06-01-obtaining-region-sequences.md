## Obtaining Regions' Sequences

Just use the metafield ```@SEQUENCE``` for obtaining the regions' sequences:

```python
import xmlrpclib
import time

url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
server = xmlrpclib.Server(url, allow_none=True)
user_key = "anonymous_key"


(status, experiments) = server.list_experiments("hg19", "peaks",
                                      "H3K27ac", "HCT116", "",
                                      "", "ENCODE", user_key)

experiments_name = server.extract_names(experiments)[1]
(status, query_id) = server.select_regions(experiments_name, None, None,
                                          None, None, None,
                                          "chr22", None, None, user_key)

(status, regions_request_id) = server.get_regions(query_id,
                                       "CHROMOSOME,START,END,@SEQUENCE,@SAMPLE_ID",
                                       user_key)


# Wait for the server processing
(status, info) = server.info(regions_request_id, user_key)
request_status = info[0]["state"]
while request_status != "done" and request_status != "failed":
  time.sleep(1)
  (status, info) = server.info(regions_request_id, user_key)
  request_status = info[0]["state"]
  print request_status


(status, regions) = server.get_request_data(regions_request_id, user_key)
print regions
```
