## Experiments

The epigenomic data is organized as *Experiments* in DeepBlue.
Each experiment is compounded by a region set and associated metadata.

One experiment has the following metadata:
  * *Name* — experiment name.
  * *Genome* — genome assembly used by the experiment. More details in the [Genomes](02-04-genomes.md) section.
  * *Epigenetic mark* — epigenetic mark of the experiment, for example: methylation or some histone modification. More details in the [Epigenetic Marks](02-05-epigenetic-marks.md) section.
  * *Sample* — identification of the sample used in the experiment. More details in the [BioSources and Samples](02-06-biosources.md) section.
  * *Technique* — technique used by the experiment, for example, ChipSeq or DNaseSeq. More details in the [Technique](02-08-techniques.md) section
  * *Project* — project associated with the experiment, for example, ENCODE or Blueprint. More details in the [Project](02-09-projects.md) section.
  * *Description* — description of the experiment.
  * *Extra metadata* — additional metadata. A key-value dictionary with extra information about the experiment
  * *User* — User who inserted the experiment.

The [list_experiments](http://deepblue.mpi-inf.mpg.de/api.php#api-list_experiments) command is used to obtain all Experiments that match the given metadata content.
This command has the parameters: genome assembly, epigenetic mark, sample ID, technique, project, and *user_key*.
All parameters, with the exception of *user_key*, are optional. Setting a parameter to *None* means that this parameter will not be used for the experiment selection. For example, it is possible to list all experiments from the genome assembly *hg19*, by entering the genome assembly and passing an empty string to all other metadata parameters:

```python
import xmlrpclib
user_key = "anonymous_key"
url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"

server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True)

all_experiments = server.list_experiments("hg19", "", "", "", "", "", "", user_key)
```

To list all Experiments from the human genome assembly *hg19* with the epigenetic mark *H3K27me3* from the *ENCODE* project:

```python
import xmlrpclib
user_key = "anonymous_key"
url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"

server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True)

h3k27me3_encode = server.list_experiments("hg19", "h3k27me3", None, None, None,
                                          None, "ENCODE", user_key)
```
