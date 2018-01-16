## Data Identifier

Each datum in DeepBlue possesses a unique *Identifier*.
This identifier is returned in all listing, search, query, and insert operations.
The identifiers can be divided into two parts: the prefix, which is one or two letters,
indicating the data type, and a numeric value.

The following table contains the identifier prefixes and their data type:

| Identifier | Data Type       |
|:----------:|-----------------|
| a          | Annotation      |
| e          | Experiment      |
| g          | Genome          |
| em         | Epigenetic Mark |
| bs         | BioSource      |
| s          | Sample          |
| p          | Project         |
| t          | Technique       |
| q          | Query           |

The [info](http://deepblue.mpi-inf.mpg.de/api.php#api-info) command is to inspect the identifier content:
```python
server.info("e1", user_key)
```

The info command returns a list of maps, where each map contains the information of an id.

We can use the [info](http://deepblue.mpi-inf.mpg.de/api.php#api-info) command to view the samples' content:

```python
(s, related) = server.get_biosource_related("blood", user_key)
related_names = server.extract_names(related)[1] # get the BioSource names
(s, samples) = server.list_samples(related_names, {}, user_key)
samples_id = server.extract_ids(samples)[1] # get samples ID

for _id in samples_id[:20] : # the first 20 samples
 print  server.info(_id, user_key)
```