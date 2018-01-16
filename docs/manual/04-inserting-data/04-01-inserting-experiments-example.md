### Example: Inserting an Experiment

In this example we insert an experiment from the ENCODE project. Note: this is just an example.
This experiment has already been included in DeepBlue.

```python
import gzip

# Reading
f = gzip.open("wgEncodeHaibMethyl450Ag04449SitesRep1.bed.gz", 'rb')
data = f.read()

# Even though DeepBlue accepts unsorted BED files,
# sorting them will make the insertion process much faster
data_split = data.split("\n")
data_split = [x for x in data_splited if x]  # Remove empty lines
data_split.sort()
sorted_data = "\n".join(data_split)


# Setting metadata
name = "wgEncodeHaibMethyl450Ag04450SitesRep1"
genome = "hg19"
epigenetic_mark = "Methylation"
biosource = "AG04450"
(status, samples) = server.list_samples(biosource, {}, user_key)
sample = samples[0]
technique = "Infinium 450k"
project = "Encode"
description = "Some example"
extra_metadata = {"dateSubmitted":"2011-09-29", "replicate":"1" :"size":"8.7M"}

# File format
format = "NAME,SCORE,STRAND,THICK_START,THICK_END,ITEM_RGB"

# Inserting
(status, _id) = server.add_experiment(name, genome, epigenetic_mark,
                                      sample, technique, project,
                                      description, sorted_data, format,
                                      extra_metadata, user_key)

# Checking result
if (status == "okay"):
 print "Experiment " + _id + " successfully inserted"
else:
 print "Problem inserting the experiment " + name + " : " + _id
```
