## Inserting Genomes

The [add_genome](http://deepblue.mpi-inf.mpg.de/api.php#api-add_genome) command requires a ```name```, ```description```, and ```data```.
The ```data``` parameter is a string containing all the chromosomes and their sizes.

Example:

```python
data = """chr1	197195432
chr2	181748087
chr3	159599783
chr4	155630120
chr5	152537259
chr6	149517037
chr7	152524553
chr8	131738871
chr9	124076172
chr10	129993255
chr11	121843856
chr12	121257530
chr13	120284312
chr14	125194864
chr15	103494974
chr16	98319150
chr17	95272651
chr18	90772031
chr19	61342430
chrX	166650296
chrY	15902555
chrM	16299"""

(status, genome_id) = server.add_genome("mm9_simple",
                                        "mm9 genome with the main chromosomes",
                                        data, user_key)
```
