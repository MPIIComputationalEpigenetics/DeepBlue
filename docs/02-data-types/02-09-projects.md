## Projects

The *Projects* controlled vocabulary contains information on projects or consortia that generate epigenomic data.
For example, DeepBlue stores data from the ENCODE and Blueprint projects.

It is possible to list all projects with the [list_projects](http://deepblue.mpi-inf.mpg.de/api.php#api-list_projects) command:
```python
>>> server.list_projects(user_key)
['okay', [['p1', 'ENCODE'], ['p2', 'Blueprint Epigenetics']]]
```

Similar project names can be found using [list_similar_projects](http://deepblue.mpi-inf.mpg.de/api.php#api-list_similar_projects).

The [add_project](http://deepblue.mpi-inf.mpg.de/api.php#api-add_project) command is used to add a new project to the database.
Note that not all users have permission to insert new terms into this controlled vocabulary.
