GENOMES = {
  "hg18": {
    "description": "Human genome 18",
    "info_file": "data/genomes/hg19"
  },
  "hg19": {
    "description": "Human genome 19",
    "info_file": "data/genomes/hg19"
  }
}

ANNOTATIONS = {
  "Cpg Islands": {
    "data_file": "data/cpgIslandExt.txt",
    "description": "CpG islands are associated ...",
    "format": "",
    "metadata": {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
    "genome": "hg19"
  },

  "Cpg Islands All Fields": {
    "data_file": "data/cpgIslandAllFields.txt",
    "description": "CpG islands are associated ... (all fields)",
    "format": "CHROMOSOME,START,END,NAME,LENGTH,NUM_CPG,NUM_GC,PER_CPG,PER_CG,OBS_EXP",
    "metadata": {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
    "genome": "hg19"
  }

}

TECHNIQUES = {
  "tech1": {
    "description": "some nice technique",
    "metadata": {"cost": "high"}
  },
  "tech2": {
    "description": "another cool technique",
    "metadata": {"cost": "low"}
  }
}

BIOSOURCES = {
  "K562": {
    "description": "some biosource",
    "metadata": {}
  },

  "Brain": {
      "description": "some other biosource",
      "metadata": {}
  }
}

PROJECTS = {
  "ENCODE": {
    "description": "The ENCODE Project: ENCyclopedia Of DNA Elements"
  },
  "Mouse ENCODE": {
      "description": "The Mouse ENCODE Project"
  }
}

EPIGENETIC_MARKS = {
  "Methylation": {
    "description": "DNA Methylation",
    "extra_metadata": {"category":"DNA Methylation"}
  },
  "H3K4me3": {
    "description": "lysine methylation",
    "extra_metadata": {"category":"Histone Modification"}
  }
}

SAMPLES = {
  "K562": {
    "metadata": {"karyotype":"cancer", "sex":"F"}
  },
  "Brain": {
      "metadata": {"karyotype":"normal", "sex":"M"}
  }
}

USERS = {
  "test_user": {
    "email": "test_user@example.com",
    "institution": "MPI-INF"
  }
}

SAMPLE_FIELDS = {
  "karyotype": {
    "type": "string",
    "description": "Sample Karyotype: cancer or normal"
  },
  "sex": {
    "type": "string",
    "description": "Sex of the element: M or F"
  }
}

EXPERIMENTS = {
  "hg19_chr1_1": {
    "genome": "hg19",
    "epigenetic_mark": "Methylation",
    "sample_id": None,
    "technique": "tech1",
    "project": "ENCODE",
    "description": "desc1",
    "format": ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "SIGNAL_VALUE",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])
  },

  "hg18_chr1_1": {
    "genome": "hg18",
    "epigenetic_mark": "Methylation",
    "sample_id": None,
    "technique": "tech1",
    "project": "ENCODE",
    "description": "desc1",
    "format": ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "SIGNAL_VALUE",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])
  },

  "hg19_chr1_2": {
    "genome": "hg19",
    "epigenetic_mark": "Methylation",
    "sample_id": None,
    "technique": "tech1",
    "project": "ENCODE",
    "description": "desc2",
    "format": ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "SIGNAL_VALUE",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])
  },

  "hg19_chr1_3": {
    "genome": "hg19",
    "epigenetic_mark": "Methylation",
    "sample_id": None,
    "technique": "tech1",
    "project": "ENCODE",
    "description": "desc3",
    "format": ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "SIGNAL_VALUE",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])
  },

  "hg19_chr2_1": {
    "genome": "hg19",
    "epigenetic_mark": "Methylation",
    "sample_id": None,
    "technique": "tech1",
    "project": "ENCODE",
    "description": "desc",
    "format": ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])
  },

  "hg19_big_1": {
    "genome": "hg19",
    "epigenetic_mark": "H3K4me3",
    "sample_id": None,
    "technique": "tech1",
    "project": "ENCODE",
    "description": "desc",
    "format": ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])
  },

  "hg19_big_2": {
    "genome": "hg19",
    "epigenetic_mark": "H3K4me3",
    "sample_id": None,
    "technique": "tech2",
    "project": "ENCODE",
    "description": "desc",
    "format": ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])
  },

  "hg19_small": {
    "genome": "hg19",
    "epigenetic_mark": "H3K4me3",
    "sample_id": None,
    "technique": "tech2",
    "project": "ENCODE",
    "description": "desc",
    "format": ""
  }
}

COLUMNS = [
  ("AVG_METHYL_LEVEL", "Average methylation level in region", "double"),
  ("BIN", "Bin", "integer"),
  ("NAME", "Region name", "string"),
  ("SCORE", "Region score", "double"),
  ("SIGNAL_VALUE", "Signal value", "double"),
  ("P_VALUE", "P value", "double"),
  ("Q_VALUE", "Q value", "double"),
  ("PEAK", "Region peak", "integer"),
  ("THICK_START", "Thick start", "integer"),
  ("THICK_END", "Thick end", "integer"),
  ("ITEM_RGB", "Item RGB", "string"),
  ("BLOCK_COUNT", "Block count", "string"),
  ("BLOCK_SIZES", "Block sizes", "string"),
  ("BLOCK_STARTS", "Block starts", "string"),
  ("LEVEL", "Level", "double"),
  ("SIGN_IF", "Sign if", "double"),
  ("SCORE_2", "Level", "double"),
  ("LENGTH", "Length", "integer"),
  ("NUM_CPG", "Number of CpGs in region", "integer"),
  ("NUM_GC", "Number of GCs dinucleotide in region", "integer"),
  ("MEDIAN_NON_CONVERTED_CPG", "Median number of non-converted reads at CpGs in region", "integer"),
  ("MEDIAN_CONVERTED_CPG", "Median number of converted reads at CpGs in region", "integer"),
  ("MEDIAN_TOTAL_CPG", "Median number of total reads at CpGs in region", "integer"),
  ("PER_CPG", "Per CPG", "double"),
  ("PER_GC", "Per GC", "double"),
  ("PER_CG", "Per CG", "double"),
  ("OBS_EXP", "Obs Exp", "double"),
  ("GENE_ID_ENSEMBL", "Gene Ensemble ID", "string"),
  ("SPAN", "span", "integer"),
  ("COUNT", "count", "integer"),
  ("OFFSET", "offset", "integer"),
  ("FILE", "file", "string"),
  ("LOWER_LIMIT", "Lower limit", "double"),
  ("DATA_RANGE", "Data range", "double"),
  ("VALID_COUNT", "Valid count", "integer"),
  ("SUM_DATA", "Sum data", "double"),
  ("SUM_SQUARES", "Sum squares", "double"),
  ("SW_SCORE", "SW score", "integer"),
  ("MILLI_DIV", "milliDiv", "integer"),
  ("MILLI_DEL", "milliDel", "integer"),
  ("MILLI_INS", "milliIns", "integer"),
  ("GENO_LEFT", "genoLeft", "integer"),
  ("REF_GENES", "refGene annotation (union of refGene  annotations for all CpGs in region)", "string"),
  ("REP_NAME", "repName", "string"),
  ("REP_CLASS", "repClass", "string"),
  ("REP_FAMILY", "repFamily", "string"),
  ("REP_START", "repStart", "integer"),
  ("REP_END", " repEnd", "integer"),
  ("REP_LEFT", "repLeft", "integer"),
  ("ID", "Region id", "string"),
  ("ISLAND_SHELF_SHORE", "Island/Shelf/Shore (union of CpG Island annotations for all CpGs in region)",
   "string"),
  ("SIZE", "Size of region in base pairs", "integer"),
  ("IGNORE", "Ignored column", "string"),
  ("TRANSCRIPT_ID_ENSEMBL", "ENSEMBL transcript ID", "string"),
  ("TRANSCRIPT_SYMBOL", "Analogous to GENE_SYMBOL, usually GENE_SYMBOL with numeric suffix", "string"),
  ("GENE_SYMBOL", "Gene Symbol", "string"),
  ("PROBE_ID", "Microarray probe id", "string"),
  ("EXPRESSION_NORM_GCRMA", "GCRMA-normalized signal intensity", "double"),
  ("DNA_METH_U", "Number of unmethylated reads for a cytosine in a bisulfite sequencing experiment", "integer"),
  ("DNA_METH_M", "Number of methylated reads for a cytosine in a bisulfite sequencing experiment", "integer"),
  ("DNA_METH_T", "Number of total reads for a cytosine in a bisulfite sequencing experiment", "integer"),
  ("GENE_ID_ENTREZ", "Gene ID in Entrez", "string"),
  ("EXPRESSION_NORM", "", "double"),
  ("ATTRIBUTE", "A semicolon-separated list of tag-value pairs, providing additional information about each feature.", "string"),
  ("GFF_SCORE", "A floating point value. The type is a string because the value '.' is also acceptable.", "string"),
]