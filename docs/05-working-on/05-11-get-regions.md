## Get Regions

The command [get_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-get_regions) is used for requesting sequences from the server. The result can be retrieved using [get_request_data](http://deepblue.mpi-inf.mpg.de/api.php#api-get_request_data).
The command parameters are the ``query_id`` and the ``format``.

The format is the name of the fields that were given in the [add_experiment](http://deepblue.mpi-inf.mpg.de/api.php#api-add_experiment) and [add_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-add_annotation).

*Example:* Retrieving all regions from chromosome 1 with *blood* as biosource, *methylation* as epigenetic mark and containing the key *karyotype* with the value *cancer* in the sample:

```python
(status, blood_related) = server.get_biosource_related("blood", user_key)
blood_related_names = [x[1] for x in blood_related]

(status, blood_samples) = server.list_samples(blood_related_names,
                                         {"karyotype":"cancer"}, user_key)
blood_samples_ids = [x[0] for x in blood_samples]

(status, query_id) = server.select_regions(None, "hg19", "DNA Methylation",
                                           blood_samples_ids, None, None,
                                           "chr1", None, None, user_key)

(status, request_id) = server.get_regions(query_id,
                                          "CHROMOSOME,START,END", user_key)

(status, regions) = server.get_request_data(request_id, user_key)

print regions
```

### Metafields

Metafields are fields used to obtain more information about the retrieved region.
It is possible to obtain the region length and the name of the experiment of the given region.
The following table shows all metafields in the regions experiment metadata:

| Metafield Name   | Data Type                                             |
|------------------|-------------------------------------------------------|
| @NAME            | Region experiment name                                |
| @LENGTH          | Region length                                         |
| @EPIGENETIC_MARK | Region epigenetic mark                                |
| @PROJECT         | Region project name                                   |
| @BIOSOURCE       | Region biosource name                                 |
| @SAMPLE_ID       | Region sample ID                                      |
| @SEQUENCE        | Obtain the DNA sequence                               |
| @STRAND          | Obtain region strand                                  |
| @GENE_ID         | Obtain the gene ID from this region                   |
| @GENE_NAME       | Obtain the gene name from this region                 |
| @COUNT.MOTIF     | Count how many times the motif appears in this region |
| @CALCULATED      | Execute a Lua code                                    |


The next example works as the previous one, but now returning the experiment name, length and BioSource name:

```python
(status, regions) = server.get_regions(query_id,
                                       "CHROMOSOME,START,END,@NAME,@LENGTH,@BIOSOURCE",
                                       user_key)
```
