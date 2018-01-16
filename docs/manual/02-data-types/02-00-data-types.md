# Data Types

The *Genomic Regions* from the *Experiments* and *Annotations*, which we will now simply refer to as regions, are the DeepBlue central data.
Every experiment has a set of regions. Each region must contain a chromosome, start, and end fields.
Others fields, e.g., region name, scores, strand, etc. are also possible.
DeepBlue allows any number of fields to be associated with a region.
The annotations, that are auxiliary data, are also formed by a set of regions in same way as Experiments.

Every experiment included in DeepBlue contains at least 6 data fields as part of the experiment metadata:
 * Experiment Name
 * Genome Assembly
 * Epigenetic Mark
 * BioSource
 * Project
 * Technique

These fields are organized into [controlled vocabularies](02-03-controlled-vocabulary.md).
Controlled vocabularies are collections of unique terms with information about them.
For example, the *Genome Assembly Controlled Vocabulary* contains the name of all genomic assemblies that can be used in experiment metadata.
Terms can be included and removed from a controlled vocabulary, but not modified.

The next sections explain how experiments and annotations are organized before going through the DeepBlue controlled vocabularies individually.
