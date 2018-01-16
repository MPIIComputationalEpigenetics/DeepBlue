## Inserting Samples

The [add_sample](http://deepblue.mpi-inf.mpg.de/api.php#api-add_sample) command requires a ```name``` and ```description```; the parameter ```extra_metadata``` is optional:

```python
(status, bs_ids) = server.search("Astrocy", "biosources", user_key)
bs_id = bs_ids[0][0]
extra_metadata = {"karyotype":"normal", "tier":"3"}
(status, bs_id) = server.add_sample(bs_id,
                  "Astrocy sample from ENCODE cv", extra_metadata, user_key)
```
