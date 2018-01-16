## Inserting BioSources

The [add_biosource](http://deepblue.mpi-inf.mpg.de/api.php#api-add_biosource) command requires a ```name``` and ```description```; the parameter ```extra_metadata``` is optional:


```python
extra_metadata = {"term_url":
  "http://www.ebi.ac.uk/ontology-lookup/browse.do?ontName=BTO&termId=BTO%3A0000099"}
(status, bs_id) = server.add_biosource("Astrocy",
  "astrocytes, Astrocy is the same as cell line NH-A", extra_metadata, user_key)
```
