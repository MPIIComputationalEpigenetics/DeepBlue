## Merging Regions Sets

It is possible to merge regions from two different operations results using the [merge_queries](http://deepblue.mpi-inf.mpg.de/api.php#api-merge_queries) command:

```python
(s, hemoglobin_alpha) = server.select_annotations("Genes", "hg19", "chr16",
                                                  222845, 227521,  user_key)
(s, hemoglobin_beta) = server.select_annotations("Genes", "hg19", "chr11",
                                                 5246693, 5250625, user_key)

(s, hemoglobin) = server.merge_queries(hemoglobin_alpha, hemoglobin_beta, user_key)

(s, request_id) = server.get_regions(hemoglobin,
                                     "CHROMOSOME,START,END,ENSEMBLE_ID,VALUE,STRAND",
                                     user_key)
(s, regions) = server.get_request_data(request_id, user_key)
```
