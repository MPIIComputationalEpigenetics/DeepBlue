# Working with the Data

DeepBlue has the following commands for working with the data:

* [select_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-select_regions) — select experiment regions that match the given parameters
* [select_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-select_annotations) — select annotation regions that match the given parameters
* [filter_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-filter_regions) — filter the regions
* [intersection](http://deepblue.mpi-inf.mpg.de/api.php#api-intersection) — return the regions that intersect the regions from another query
* [merge_queries](http://deepblue.mpi-inf.mpg.de/api.php#api-merge_queries) — merge two queries into one
* [tiling_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-tiling_regions) — create regions of the given tiling size over the chromosomes
* [get_experiments_by_query](http://deepblue.mpi-inf.mpg.de/api.php#api-get_experiments_by_query) — request experiment names and IDs of the selected regions
* [count_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-count_regions) — request the number of regions
* [get_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-get_regions) — request the query regions
* [aggregate](http://deepblue.mpi-inf.mpg.de/api.php#api-aggregate) — get the regions for the given query in the requested BED format
* [get_request_data](http://deepblue.mpi-inf.mpg.de/api.php#api-get_request_data) - get the requested data
* [info](http://deepblue.mpi-inf.mpg.de/api.php#api-info) - get information about an entity or the data request

The usual workflow is: select the data with [select_experiments](http://deepblue.mpi-inf.mpg.de/api.php#api-select_regions) and [select_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-select_annotations).
Next, filter the data with [filter_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-filter_regions) or [intersection](http://deepblue.mpi-inf.mpg.de/api.php#api-intersection).
If necessary, select more data and merge them using the [merge_queries](http://deepblue.mpi-inf.mpg.de/api.php#api-merge_queries).
To view the results; it is possible to request the experiments containing selected data with the [get_experiments_by_query](http://deepblue.mpi-inf.mpg.de/api.php#api-get_experiments_by_query), [count_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-count_regions), or [get_regions](http://deepblue.mpi-inf.mpg.de/api.php#api-get_regions) command (in the BED format), or in a data summary using [aggregate](http://deepblue.mpi-inf.mpg.de/api.php#api-aggregate). The status of the request can be viewed using [info](http://deepblue.mpi-inf.mpg.de/api.php#api-info) and the data retrieved using [get_request_data](http://deepblue.mpi-inf.mpg.de/api.php#api-get_request_data).

The use of these commands will be explained in the following sections.
