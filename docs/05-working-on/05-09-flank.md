## Flanking

DeepBlue has an operation for generating [flank regions](http://deepblue.mpi-inf.mpg.de/api.php#api-flank) from existing regions.

The flanking regions are calculated as following:

* If the ```start``` parameter is positive, the new regions will start after the end of the region.
* If the ```start``` parameter is negative, the new regions will start before the start of the region plus the defined ```length```.

If the ```use_strand``` parameter is true, then DeepBlue will use the content of the STRAND column for calculating the positions. It only changes the behavior of the regions that have negative strand (-).
When the ```use_strand``` parameter is true and also the region has negative strand, the flanking regions are calculated as following:

* If the ```start``` parameter is positive, the new regions will start before the start of the region plus the defined ```length```.
* If the ```start``` parameter is negative, the new regions will start after the end of the region.

Example:
```python
import xmlrpclib
import time

url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
user_key = "anonymous_key"

server = xmlrpclib.Server(url, allow_none=True)

# Select the RP11-34P13 gene locations from gencode v23
(status, q_genes) = server.select_genes(["RNU6-1100P", "CICP7", "MRPL20", "ANKRD65",
                                         "HES2", "ACOT7", "HES3", "ICMT"],
                                         "gencode v19", user_key)

# Obtain the regions that starts 2500 bases pair before the regions start and
# have 2000 base pairs.
# The 4th argument inform that DeepBlue must consider the region's column STRAND
# to calculate the new region
(s, before_flank_id) = server.flank(q_genes, -2500, 2000, True, user_key)

# Obtain the regions that starts 1500 bases pair after the regions end and
# have 500 base pairs.
# The 4th argument inform that DeepBlue must consider the region's column STRAND
# to calculate the new region
(s, after_flank_id) = server.flank(q_genes, 1500, 500, True, user_key)

# Merge both flanking regions set and genes set
(s, flank_merge_id) = server.merge_queries(before_flank_id, after_flank_id, user_key)
(s, all_merge_id) = server.merge_queries(q_genes, flank_merge_id, user_key)

# Request the regions
(status, request_id) = server.get_regions(all_merge_id,
                                          "CHROMOSOME,START,END,STRAND,@LENGTH",
                                          user_key)

# Wait for the server processing
(status, info) = server.info(request_id, user_key)
request_status = info[0]["state"]
while request_status != "done" and request_status != "failed":
  time.sleep(1)
  (status, info) = server.info(request_id, user_key)
  request_status = info[0]["state"]

(status, regions) = server.get_request_data(request_id, user_key)

print regions
```


