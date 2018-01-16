## Experiments by Query

In a [previous section](05-02-selection-experiments.md), we selected all experiments that matched a set of parameters. DeepBlue provides the command [get_experiments_by_query](http://deepblue.mpi-inf.mpg.de/api.php#api-get_experiments_by_query) to request the experiments which where selected by the [select_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-select_regions) command. The result can be retrieved using [get_request_data](http://deepblue.mpi-inf.mpg.de/api.php#api-get_request_data).

```python
>>> (status, request_id) = server.get_experiments_by_query(query_id, user_key)
>>> (status, experiments) = server.get_request_data(request_id, user_key)
>>> for experiment in experiments[:5]:
>>>   (status, info) = server.info(experiment[0], user_key)
>>>   print info["name"], info["format"]
```
