## Selecting Annotations

The [select_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-select_annotations) command accepts the name of one or more annotations, the genome, chromosomes, and start and end positions. Any combination of these parameters is possible.

*Example 1*: Selecting the CpG Island annotation:
```python
(s, cpg_island) = server.select_annotations("Cpg Islands",
                                            "hg19", None, None, None, user_key)
```

*Example 2*: Only the chromosomes X and Y:
```python
(s, cpg_island) = server.select_annotations("Cpg Islands",
                                            "hg19", ["chrX", "chrY"],
                                            None, None, user_key)
```


*Example 3*: Selecting the *hox* genes:
```python
(s, hox_genes) = server.select_annotations("Genes", "hg19",
                                           "chr7", 27130000, 27250000,
                                           user_key)
```

The [select_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-select_annotations) return value is a query identifier (that will be) used in other operations or to retrieve data with the [get_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-get_regions) command.