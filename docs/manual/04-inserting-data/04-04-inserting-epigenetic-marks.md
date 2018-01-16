## Inserting Epigenetic Marks

The [add_epigenetic_mark](http://deepblue.mpi-inf.mpg.de/api.php#api-add_epigenetic_mark) command requires a ```name``` and ```description```.

```python
(status, em_id) = server.add_epigenetic_mark("h3k27me3",
                                             "Histone H3 (tri-methyl K27).", user_key)
```
