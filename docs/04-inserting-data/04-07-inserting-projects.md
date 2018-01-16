## Inserting Projects

The [add_project](http://deepblue.mpi-inf.mpg.de/api.php#api-add_project) command requires ```name``` and ```description``` parameters:


```python
(status, p_id) = server.add_project("DEEP",
                  "The German epigenome programme ‘DEEP’", user_key)
```
