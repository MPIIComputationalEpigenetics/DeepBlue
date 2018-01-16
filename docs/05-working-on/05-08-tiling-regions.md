## Tiling Regions

DeepBlue has an operation for generating [tiling regions](http://deepblue.mpi-inf.mpg.de/api.php#api-tiling_regions):

```python
>>> (s, tiling) = server.tiling_regions(100000, "hg19", None, user_key)
>>> (s, request_id) = server.count_regions(tiling, user_key)
>>> print(server.get_request_data(request_id, user_key))
['okay', 31326]
```
