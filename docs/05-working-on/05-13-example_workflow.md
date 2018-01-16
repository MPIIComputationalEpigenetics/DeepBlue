### Example: Workflow

![DeepBlue Epigenomic Data Server](http://deepblue.mpi-inf.mpg.de/img/workflow.png)

```python
import xmlrpclib
import time

url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"
server = xmlrpclib.Server(url, allow_none=True)
user_key = "anonymous_key"

(status, experiment) = server.select_regions("ENCFF129SEF", None,
                                             None, None, None, None,
                                             "chr1", None, None, user_key)

(status, cpg) = server.select_annotations("CpG Islands", "hg19", "chr1",
                                          None, None, user_key)

(status, exp_filtered) = server.filter_regions(experiment,
                                               "SCORE", ">", "10", "number",
                                               user_key)

(status, intersected) = server.intersection(exp_filtered, cpg, user_key)

(status, exp_H3k4me3) = server.select_regions("ENCFF594HUT", None,
                                              None, None, None, None,
                                              "chr1", None, None, user_key)

(status, final) = server.intersection(intersected, exp_H3k4me3, user_key)

(status, count_request_id) = server.count_regions(final, user_key)

# Wait for the server processing
(status, info) = server.info(count_request_id, user_key)
request_status = info[0]["state"]
while request_status != "done" and request_status != "failed":
  time.sleep(1)
  (status, info) = server.info(count_request_id, user_key)
  print info
  request_status = info[0]["state"]
  print request_status


(status, count_result) = server.get_request_data(count_request_id, user_key)

print count_result
```