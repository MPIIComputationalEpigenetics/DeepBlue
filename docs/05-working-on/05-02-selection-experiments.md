## Selecting Experiments

The [select_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-select_regions) command is used to access the experiments' data.
The [select_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-select_regions) command is similar to [select_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-select_annotations). The difference is that [select_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-select_regions) accepts ```epigenetic_mark```, ```sample```, ```technique```, and ```project``` parameters.


### Selecting Experiments Example


*Example:*
Firstly, we use the [get_biosource_related](http://deepblue.mpi-inf.mpg.de/api.php#api-get_biosource_related) command to retrieve all biosources related to the term "blood" and then we select only the biosources names from the ```blood_related``` list.
Them, we select all samples that use these biosources and get their IDs.
Finally, we select all ```chromosome``` *chr1* regions from the experiments that have ```genome``` *hg19*, ```epigenetic_mark``` *methylation* and the found samples and print the chromosome, start, and end of these regions:

```python
(status, blood_related) = server.get_biosource_related("blood", user_key)
blood_related_names = server.extract_names(blood_related)[1]
(status, blood_samples) = server.list_samples(blood_related_names,
                                              {"karyotype":"cancer"}, user_key)
blood_samples_ids = server.extract_ids(blood_samples)[1]
(status, query_id) = server.select_regions(None, "hg19", "DNA Methylation",
                                           blood_samples_ids, None, None,
                                           "chr1", None, None, user_key)
(status, request_id) = server.get_regions(query_id,
                                          "CHROMOSOME, START, END", user_key)
(status, regions) = server.get_request_data(request_id, user_key)
print regions
```
