## Epigenetic Marks

The *Epigenetic Marks* controlled vocabulary contains epigenetic modifications and variations.
Examples of epigenetic marks are DNA methylation, DNaseI hypersensitive sites, mRNA, histone variants and post-translational modification.
We also included chromatin state segmentation as an epigenetic mark.

The command [list_epigenetic_marks](http://deepblue.mpi-inf.mpg.de/api.php#api-list_epigenetic_marks) is used to list all epigenetic marks included in DeepBlue:

```python
print server.list_epigenetic_marks(user_key)
```

Insert new epigenetic marks by using the [add_epigenetic_mark](http://deepblue.mpi-inf.mpg.de/api.php#api-add_epigenetic_marks) command:

```python
print server.add_epigenetic_mark("Epigenetic Mark Example",
                                 "A example of epigenetic mark", user_key)
```

Before inserting a new epigenetic mark, please check if a similar name already exists in order to avoid data duplication.
Use [list_similar_epigenetic_marks](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_epigenetic_marks):
```python
print server.list_similar_epigenetic_marks("methylation", user_key)
```
This command will return the most similar epigenetic mark names:
```python
['okay', ['Methylation', 'mRNA-seq', 'PAX5-N19', 'H4K12bio', 'eGFP-FOS']]
```
