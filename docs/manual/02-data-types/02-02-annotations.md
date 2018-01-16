## Annotations

The auxiliary genomic regions data is organized as *Annotations* in DeepBlue.
Each Annotation is composed by a region set, a name, genome, and *user* attributes.

Annotations are used to store genomic regions that do not come from an epigenomic experiment, but contain other (epi-)genetic information.
Examples of annotations are: CpG islands and genes.
It is possible to list all available annotations using the [list_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-list_annotations) command. The *list_annotations* command has two parameters: *Genome* and *user_key*.

The following source code demonstrates how to list all available annotations for the genome assembly *hg19*:

```python
import xmlrpclib
user_key = "anonymous_key"
url = "http://deepblue.mpi-inf.mpg.de/xmlrpc"

server = xmlrpclib.Server(url, encoding='UTF-8', allow_none=True)

hg19_annotations = server.list_annotations("hg19", user_key)
```
