## Retieving Data

Using [info](http://deepblue.mpi-inf.mpg.de/api.php#api-info) the status of a request can be seen. As soon as the request is processed, the requested data can be retrieved using [get_request_data](http://deepblue.mpi-inf.mpg.de/api.php#api-get_request_data).

```python
>>> print server.get_regions(query_id, "CHROMOSOME,START,END,VALUE,STRAND", user_key)
['okay', 'r1']
>>> print server.get_request_regions('r1', user_key)
Request ID r1 was not finished. Please, check its status.
>>> print server.info('r1', user_key)
['okay', [{'state': 'running',  'create_time': '2015-May-02 19:48:44.162000',
           'query_id': 'q1', 'message': '', 'type': 'request'}]]
>>> print server.info('r1', user_key)
['okay', [{'finish_time': '2015-May-02 19:48:44.218000', 'state': 'done',
           'create_time': '2015-May-02 19:48:44.162000', 'query_id': 'q1',
           'message': '', 'type': 'request'}]]
>> print server.get_request_regions('r1', user_key)
```
