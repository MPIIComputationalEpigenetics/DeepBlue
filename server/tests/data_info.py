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
    "format": "CHROMOSOME,START,END,name:String,length:Integer,cpgNum:Integer,gcNum:Integer,perCpg:Double,perGc:Double,obsExp:Double",
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

  "BS2": {
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
    "description": "DNA Methylation"
  },
  "H3K4me3": {
    "description": "lysine methylation"
  }
}

SAMPLES = {
  "K562": {
    "metadata": {"karyotype":"cancer", "sex":"F"}
  },
  "BS2": {
      "metadata": {"karyotype":"cancer", "sex":"F"}
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
      "name:String",
      "score:Integer",
      "strand:String",
      "signalValue:Integer",
      "pValue:Double",
      "qValue:Integer",
      "peak:Integer"
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
      "name:String",
      "score:Integer",
      "strand:String",
      "signalValue:Double",
      "pValue:Double",
      "qValue:Double",
      "peak:Integer"
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
      "name:String",
      "score:Integer",
      "strand:String",
      "signalValue:Double",
      "pValue:Double",
      "qValue:Double",
      "peak:Integer"
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
      "name:String",
      "score:Integer",
      "strand:String",
      "signalValue:Double",
      "pValue:Double",
      "qValue:Double",
      "peak:Integer"
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
      "name:String",
      "score:Integer",
      "strand:String",
      "pValue:Double",
      "qValue:Double",
      "peak:Integer"
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
      "name:String",
      "score:Integer",
      "strand:String",
      "pValue:Double",
      "qValue:Double",
      "peak:Integer"
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
      "name:String",
      "score:Integer",
      "strand:String",
      "pValue:Double",
      "qValue:Double",
      "peak:Integer"
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