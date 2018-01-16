## Counting Regions

The [count_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-count_regions) command requests the number of regions in a query. The result can be retrieved using [get_request_data](http://deepblue.mpi-inf.mpg.de/api.php#api-get_request_data).

```python
>>> (status, request_id) = server.count_regions(query_id, user_key)
>>> print server.get_request_data(request_id, user_key)
['okay', 1338676]
```
