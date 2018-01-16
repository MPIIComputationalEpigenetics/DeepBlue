## BioSources and Samples

The *BioSources* controlled vocabulary contains different types of biological sources: cell lines, cell types, lineage, tissues, organs, and others.
A BioSource contains a name, description, and additional metadata.
DeepBlue currently imports BioSources from three ontologies: [Cell Type](http://www.ontobee.org/browser/index.php?o=CL), [Experimental Factor Ontology](http://www.ontobee.org/browser/index.php?o=EFO), and [Uber Anatomy Ontology](http://www.ontobee.org/browser/index.php?o=UBERON).
Each term in the BioSource controlled vocabulary has a name, description (if available), the address of a full description of the term, ontology name, namespace of the term, and the ontology comment about the term.

The command [add_biosource](http://deepblue.mpi-inf.mpg.de/api.php#api-add_biosource) is used to insert a new BioSource. It is also possible to list all BioSources using the command [list_biosources](http://deepblue.mpi-inf.mpg.de/api.php#api-list_biosources) and to search all BioSources with a similar name through the command [list_similar_biosources](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_biosources).

Identical BioSources can have different names, referred to as synonyms.
For example, the *leukocyte* BioSource has the synonyms *leucocyte* and *white blood cells*.
A synonym can be set using the [set_biosource_synonym](http://deepblue.mpi-inf.mpg.de/api.php#api-set_biosource_synonym) command.
A BioSource list of synonyms may be retrieved with the [get_biosource_synonyms](http://deepblue.mpi-inf.mpg.de/api.php#api-get_biosource_synonyms) command:

```python
(s, synonyms) = server.get_biosource_synonyms("blood", user_key)
```
The ```synonyms``` variable will contain ```['blood', 'portion of blood', 'vertebrate blood']```.

DeepBlue also organizes all available BioSources into a hierarchy.
The hierarchy establishes which terms embrace each other.
For example, the BioSource *blood* is more embracing than *leukocyte*, and the term *leukocyte* is more embracing than *lymphocyte*.
The command [set_biosource_parent](http://deepblue.mpi-inf.mpg.de/api.php#api-set_biosource_parent) is used to set a relationship between two BioSources.
Use the [get_biosource_children](http://deepblue.mpi-inf.mpg.de/api.php#api-get_biosource_children) command to list the BioSources that fall under another BioSource within the BioSources hierarchy.

```python
(s, children) = server.get_biosource_children("blood", user_key)
```

The command [get_biosource_related](http://deepblue.mpi-inf.mpg.de/api.php#api-get_biosource_related) returns all BioSources under the given BioSource and their synonyms:

```python
(s, related) = server.get_biosource_related("blood", user_key)
```

Some BioSources may have duplicated IDs, for example, ```blood```and ```portion of blood``` have the ID ```bs2```.
This is because ```portion of blood```is a synonym of ```blood```.


### Samples

BioSources are not used directly by [Experiments](02-01-experiments.md), but rather through *Samples*.
From a data organization perspective, **Samples are BioSources with metadata**.
The metadata may contain any kind of information about the source such as the organism, laboratory, age, karyotype, cell lineage, strain, date, donor sex, or donor ethnicity.
The metadata fields are very flexible, and it is recommended that all sample information be included here.
We will try to include as much metadata as possible during a sample's import process.
DeepBlue imports samples from ENCODE, BLUEPRINT and others future projects. Look in the field *source* in the sample *extra_metadata* for obtaining the *Sample* original source.

The sample metadata fields should be included before using the command [add_sample_field](http://deepblue.mpi-inf.mpg.de/api.php#api-add_sample_field).
The necessary information to create a new field is the field name, the type (string or numeral), description, and the *user_key*.
An error message will be returned if a field with the same name already exists.
A list of all similar field names can be obtained using the command [list_similar_sample_fields](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_sample_fields), and a list of all sample fields with the command [list_sample_fields](http://deepblue.mpi-inf.mpg.de/api.php#api-list_sample_fields).

The [add_sample](http://deepblue.mpi-inf.mpg.de/api.php#api-add_sample) command is used to insert a new sample together with its metadata.
The command [list_samples](http://deepblue.mpi-inf.mpg.de/api.php#api-list_samples) lists all samples belonging to a given BioSource and their metadata.
The [list_samples](http://deepblue.mpi-inf.mpg.de/api.php#api-list_samples) command returns a list of key-value elements, where the key is the sample ID, and the elements are the metadata of the sample:

```python
server.list_samples("T_cells_CD4+", {}, user_key)
```
This command returns:
```python
['okay', ['s342', {'lineage': 'mesoderm', 'karyotype': 'unknown',
                    'description': 'Parent cell line for T cells CD4+.',
                    'biosource_name': 'T_cells_CD4+', 'lab': 'Crawford', 'sex': 'B',
                    'user': 'Populator', 'tier': '3', '_id': 's342',
                    'organism': 'human'}]]
```

The [list_samples](http://deepblue.mpi-inf.mpg.de/api.php#api-list_samples) command may be used to retrieve samples based on their metadata. For instance, to retrieve all samples that have "tier 3" in their metadata:
```python
server.list_samples(None, {"tier":"3"}, user_key)
```

N.B.: Not all samples have a *tier* in their metadata; it depends on the source from which the data was imported.
