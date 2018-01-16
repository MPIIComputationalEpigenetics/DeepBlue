## Intersecting Regions

The command [intersection](http://deepblue.mpi-inf.mpg.de/api.php#api-intersection) selects the regions that intersect with another set of regions.
The correct usage is to obtain the regions from experiments that overlap with some annotation, e.g., all sites that overlap with some CpG Island.
The command has three parameters: the ID of the regions to be filtered, and the ID of the regions to be used for the filtering, and the user key. The result is a query of the regions of the first set that overlap with the regions of the second set:

```python
>>> (status, cpg_islands) = server.select_annotations("CpG Islands",
                                                      "hg19", None, None,
                                                      None, user_key)
>>> (status, overlaps) = server.intersection(filtered, cpg_islands, user_key)
>>> (status, request_id) = server.count_regions(overlaps, user_key)
>>> print server.get_request_data(request_id, user_key)
['okay', 611800]
```
