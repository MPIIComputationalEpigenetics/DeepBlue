# Features

Among the DeepBlue Features, we can highlight:
* DeepBlue contains the Methylation (WGBS, RRBS, and Infinium 450k), Histone Modifications and Variants, DNaseI, Transcription Factors Binding Sites, Chromatin State Segmetation, and gene expression (mRNA-seq) datasets from the major epigenome consortia: ENCODE, Roadmap Epigenomic (planning), DEEP (planning), and BLUEPRINT Epigenomics
* Update module that retrieves the latest epigenome datasets from the major epigenome consortia
* Access via an XML-RPC protocol that is supported by all major programming languages
* Web Interface (in development)
* User data upload facility
* Analysis operations performed directly on the data server
* Reproducibility by automatic documenting and storing of the analysis steps
* Controlled vocabulary to ensure data consistency
* Full text search command, expanded for all experiments metadata and controlled vocabularies
* BioSources data hierarchy and synonyms
* Flexible data model for storing experiments metadata
* Data Compression
* Data scalability through data distribution across several compute nodes (sharding), provided by MongoDB
* A set of operations for working with the data (see below)

Operations:
* Select epigenomic data from pre-installed datasets or datasets uploaded by the user
* Filter epigenomic data by metadata
* Filter epigenomic data by region attributes
* Find overlapping regions sets
* Group regions
* Retrieve DNA sequences
* DNA pattern matching operations
* Aggregate and summarize regions
* Count regions
* Output in BED format with desired columns
