## Removing Data

The [remove](http://deepblue.mpi-inf.mpg.de/api.php#api-remove) command requires the data ```ID``` and ```user_key``` parameters:


```python
(status, msg) = server.remove(data_id, user_key)
```

Please, note that **the data is irrecoverable after removed.**

Only the user that included the data can delete it.
