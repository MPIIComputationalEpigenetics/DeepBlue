## Inserting Annotations

The [add_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-add_annotation) command requires only a ```name```, ```genome```, ```description```, ```data```, ```format```, and ```extra_metadata```.
All these parameters have the same meaning as the corresponding  parameters in the  [add_experiment](http://deepblue.mpi-inf.mpg.de/api.php#api-add_experiment) command. The ```format```is the only exception, because [add_annotations](http://deepblue.mpi-inf.mpg.de/api.php#api-add_annotation) *only accepts data in the BED format*.
