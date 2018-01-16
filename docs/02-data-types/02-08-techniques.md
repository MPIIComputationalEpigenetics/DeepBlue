## Techniques

The *Technique* controlled vocabulary contains information on experimental techniques which refines the epigenetic mark metadata.
The technique controlled vocabulary contains the technique name and its description.

The command [list_techniques](http://deepblue.mpi-inf.mpg.de/api.php#api-list_techniques) lists all techniques included in DeepBlue:

```python
>>> print server.list_techniques("anonymous_key")
['okay', [['t1', 'RRBS'], ['t2', 'Infinium 450k'], ['t3', 'BisulfiteSeq'],
          ['t4', 'ChipSeq'], ['t5', 'ChipSeq Uniform'], ['t6', 'DNaseSeq'],
          ['t7', 'DNaseSeq Uniform'], ['t8', 'Chromatin State Segmentation by HMM'],
          ['t9', 'RNASeq']]]
```

Similar technique names can be found using the [list_similar_techniques](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_techniques) command.

The [add_technique](http://deepblue.mpi-inf.mpg.de/api.php#api-add_technique) command is used to insert a technique.
Note that not all users have permission to add new techniques.
