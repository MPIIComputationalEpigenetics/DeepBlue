## Filtering Regions

The [filter_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-filter_regions) command is used to filter regions according to their content. This command receives the ID of a query, the region field name that will be filtered, the operation (```==```, ```!=```, ```>```,```>=```, ```<```, ```<=``), and the field type:

Example:
```python
>>> (status, filtered) = server.filter_regions(query_id,
                                               "SCORE", ">","5", "integer",
                                               user_key)
>>> (status, request_id) = server.count_regions(filtered, user_key)
>>> print server.get_request_data(request_id, user_key
['okay', 1007799]
```
