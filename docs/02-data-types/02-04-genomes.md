## Genomes

The *Genomes* controlled vocabulary contains the names, descriptions, and chromosome names and sizes of different Genome assemblies, e.g., *hg19*.
To begin with, DeepBlue contains the assembly *hg19*, but new genome assemblies can be inserted easily using the [add_genome](http://deepblue.mpi-inf.mpg.de/api.php#api-add_genome) command.
The following code example shows how to insert a new genome assembly with the chromosome names and sizes.
The variable ```genome_data``` contains text with the chromosome name and its size on each line.

```python
genome_data = """chr1 1000000
chr2 900000
chr3 500000
chrX 100000"""
print server.add_genome("hgX", "Example of Genome for the Manual",
                        genome_data, user_key)
```

Please be aware that not all users have permission to insert a new genome into DeepBlue.

The command [chromosomes](http://deepblue.mpi-inf.mpg.de/api.php#api-chromosomes) lists all the names and sizes of the chromosomes of a genome assembly:
```python
print server.chromosomes("hgX", user_key)
```

The ```add_genome``` command also creates an annotation containing all chromosomes and their sizes.
The annotation name is identical to that of the genome.
The annotation can be found using the [list_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-list_annotations):

```python
print server.list_annotations("hgX", user_key)
```

Use the [select_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-select_annotations) and [get_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-get_regions) commands to obtain the genome's annotation:
```python
(s, chromosomes_annotation) = server.select_annotations("hgX", "hgX", None,
                                                        None, None, user_key)
(s, regions) = server.get_regions(chromosomes_annotation, "CHROMOSOME,START,END",
                                  user_key)
print regions
```
It will print:
```
chr1	0	1000000
chr2	0	900000
chr3	0	500000
chrX	0	100000
```

The ```select_annotations``` and ```get_regions``` commands will be explained in more detail in the [Exploring the Data](../03-exploring/03-00-exploring.md) section.

### Genomic Sequences

DeepBlue also stores genomic sequences from the genome assemblies which can be used for data filtering and analysis.
The sequences of ```hg19``` genome assembly chromosomes are already included in DeepBlue.

The [upload_chromosome](http://deepblue.mpi-inf.mpg.de/api.php#api-upload_chromosome) command is used to upload the genomic sequences of other genome assemblies:
```python
# Example sequence!
data = "ACTGACTGCG" * 100000
print server.upload_chromosome("hgX", "chr1", data, user_key)
```
The [Working with Sequences](../06-sequences/06-01-obtaining-region-sequences.md) section discusses how to access and use the genomic sequences.

A list of all possible commands that can be applied within the genomes controlled vocabulary is available at [DeepBlue API - Inserting and listing Genomes](http://deepblue.mpi-inf.mpg.de/api.php#api-genomes).
