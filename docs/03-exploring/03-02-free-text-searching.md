## Free Text Search

The listing operations are useful for listing the IDs and names, but not for searching through data content.
For example, if we wish to search for an [Experiment](../02-data-types/02-01-experiments.md) with associated metadata, we must [list all the experiments](http://deepblue.mpi-inf.mpg.de/api.php#api-list_experiments), execute [info](http://deepblue.mpi-inf.mpg.de/api.php#api-info) and analyze the result of each experiment.
Also, the [list_samples](http://deepblue.mpi-inf.mpg.de/api.php#api-list_samples) command needs the correct metadata content to find the expected sample.
In short, it is not an optimal way to search data.
The [search](http://deepblue.mpi-inf.mpg.de/api.php#api-search) command is the optimal method for data searching in DeepBluebecause it simplifies the task of finding data.

The [search](http://deepblue.mpi-inf.mpg.de/api.php#api-search) command has three parameters: the free text for which to do search, the data type (optional), and the *user_key*. The data type defines the collection on which the search will be made.
The available collections are: *annotations*, *biosources*, *epigenetic_marks*, *experiments*, *genomes*, *projects*, *samples*, *sample fields*, *techniques*, *tilings*, and *column_types*. Multiple data types can be combined, i.e., it is possible to search for the term "cancer" in all experiments and samples.
The search command will search for the free text input in all data content.
This means that the command will look at the data name, description, other data type attributes, and also at the extra metadata.

The [search](http://deepblue.mpi-inf.mpg.de/api.php#api-search) command result is a set of tuples, ordered by relevance, containing the data id, data name, and data type. The command returns a maximum of 100 values.
The command returns a set containing the IDs and names; the [info](http://deepblue.mpi-inf.mpg.de/api.php#api-info) command offers more information about the retrieved items.

In the following example, we search for all samples containing either the word "cancer" **or** "blood" in their description:
```python
server.search("cancer blood", "samples", user_key)
```

The following example searches for cancer **or** blood **or** methylation.
This means that an experiment with only the word "methylation" in its metadata will also be returned.
Experiments containing all three given words will have higher scores and will appear before those experiments with only one or two of the given words:
```python
server.search("cancer blood methylation", "experiments", user_key)
```

More complex searches can also be performed:
 * Words that must be in the metadata should be placed in double quotes
 * Text can be exactly matched by enclosing in escaped double quotes, e.g. *"white cells"*
 * The hyphen symbol (*-*) can be used to exclude a word


To search all experiments including "cancer" **and** "blood" ***and*** "methylation":
```python
server.search("\"cancer\" \"blood\" \"methylation\"", "samples", user_key)
```

And, to search for "methylation" **and** **not** "histone" **and** **not** "modification" in the epigenetic marks:
```python
server.search("methylation -histone -modification", "epigenetic_marks", user_key)
```

To search for "methylation" **and** "cancer"  **and** **not** "rrbs" **and** **not** "histone" **and** **not** "modification" in experiments:
```python
server.search("\"methylation\" \"cancer\" -rrbs -histone -modification ",
              "experiments", user_key)
```

Please be aware that the terms inside  ```\" \"``` are searched for exactly as they are entered, turning off the default functionality of searching for similar terms.


#### More Examples
[Search](http://deepblue.mpi-inf.mpg.de/api.php#api-search) can be used together with the [info](http://deepblue.mpi-inf.mpg.de/api.php#api-info) command:

```python
(s, result) = server.search("methylation  -histone -modification cancer ",
                            "experiments", user_key)
for r in result[:3]:   # Select first three elements
	print server.info(r[0], user_key)
```


In this example, we search for experiments whose metadata does not contain the term "rrbs":
```python
(s, result) = server.search("methylation -rrbs  -histone -modification cancer ",
                             "experiments", user_key)
for r in result[:3]:   # Select first three elements
	print server.info(r[0], user_key)
```