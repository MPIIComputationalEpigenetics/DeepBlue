## Listing

The [DeepBlue API](http://deepblue.mpi-inf.mpg.de/api.php) provides a set of Listing methods.
Each [data type](../02-data-types/02-00-data-types.md) has ```list``` and ```list_similar``` operations.
All listing operations return a set of IDs and names, with the exception of the samples, which return a set of IDs and a key-value table.
It is possible to read the [data identifier](../02-data-types/02-10-data-identifier.md) using the[info](http://deepblue.mpi-inf.mpg.de/api.php#api-info)  command.

The [list_experiments](http://deepblue.mpi-inf.mpg.de/api.php#api-list_experiments) command lists experiments by genome, experiment type, epigenetic_mark, biosource, sample, technique, project. It accepts any combination of the parameters, allowing users to filter the selection.
Hence, without any filter at all, [list_experiments](http://deepblue.mpi-inf.mpg.de/api.php#api-list_experiments) returns all the experiments in DeepBlue.

The [list_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-list_annotations) command requires the genome name parameter because the same annotation name can be used for different genomes, for example, the genes location annotations.

The ```list_similar``` command list all elements from the controlled vocabulary that are similar to the given input.
It is available for
 [Experiments](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_experiments), [Genomes](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_genomes), [Epigenetic Marks](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_epigenetic_marks), [BioSources](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_biosources), [Projects](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_projects), and [Techniques](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_techniques).
