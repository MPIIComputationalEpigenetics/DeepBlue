import helpers

from client import EpidbClient


class TestBugs(helpers.TestCase):

  # Bug: Error when insert repeat_masker annotation - 67203100
  def test_no_start_position(self):
    file_content = """1036\t1146\t43\t0\t18\tchr19\t59118819\t1000\t-10000\t+\t(TTAGGG)n\tSimple_repeat\tSimple_repeat\t5\t165\t0\t1
    585\t1080\t161\t1\t0\tchr19_gl000208_random\t0\t719\t-91970\t-\tALR/Alpha\tSatellite\tcentr\t-41\t719\t1\t4"""

    rmsk = [
  "swScore:Integer:0",
  "milliDiv:Integer:0",
  "milliDel:Integer:0",
  "milliIns:Integer:0",
  "chromosome:String",
  "start:Integer:0",
  "end:Integer:0",
  "genoLeft:Integer:0",
  "strand:String",
  "repName:String",
  "repClass:String",
  "repFamily:String",
  "repStart:Integer:0",
  "repEnd:Integer:0",
  "repLeft:Integer:0",
  "id:String"
]
    format = ",".join(rmsk)

    epidb = EpidbClient()
    self.init_base(epidb)

    (r, a) = epidb.add_annotation("repeat_masker", "hg19", None, file_content, format, None, self.admin_key)

    self.assertFailure(r, a)

    self.assertEquals(a, "Start position was not informed or was ignored. Line content: 1146\t43\t0\t18\tchr19\t59118819\t1000\t-10000\t+\t(TTAGGG)n\tSimple_repeat\tSimple_repeat\t5\t165\t0\t1\t")

    rmsk2 = [
  "swScore:Integer:0",
  "milliDiv:Integer:0",
  "milliDel:Integer:0",
  "milliIns:Integer:0",
  "CHROMOSOME:String",
  "START:Integer",
  "END:Integer",
  "genoLeft:Integer:0",
  "strand:String",
  "repName:String",
  "repClass:String",
  "repFamily:String",
  "repStart:Integer:0",
  "repEnd:Integer:0",
  "repLeft:Integer:0",
  "id:String"
]
    format2 = ",".join(rmsk2)

    (r, a) = epidb.add_annotation("repeat_masker2", "hg19", None, file_content, format2, None, self.admin_key)

    self.assertSuccess(r, a)

  def test_invalid_eamp_character(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    epidb.add_biosource("rostrolateral thalamic nucleus of Butler <methodName>Saidel", "", {}, self.admin_key)
    epidb.add_biosource("testing <b>cool", "", {}, self.admin_key)
    epidb.add_biosource("testing ugly &Saidel", "", {}, self.admin_key)
    epidb.add_biosource("testing weird <Saidel", "", {}, self.admin_key)
    epidb.add_biosource("testing open Saidel>", "", {}, self.admin_key)
    epidb.add_biosource("testing closed <Saidel>", "", {}, self.admin_key)
    epidb.add_biosource("!'234456789<<<<><<<;;.,.,-,>", "", {}, self.admin_key)

    (r,a) = epidb.list_biosources(self.admin_key)
    self.assertSuccess(r,a)

    biosource_names = [x[1] for x in a]
    self.assertEquals(biosource_names, ['K562', 'rostrolateral thalamic nucleus of Butler <methodName>Saidel', 'testing <b>cool', 'testing ugly &Saidel', 'testing weird <Saidel', 'testing open Saidel>', 'testing closed <Saidel>', "!'234456789<<<<><<<;;.,.,-,>"])

  def test_wrong_chromosomes_usage(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    cpg_island =  ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "name:String",
      "length:Integer:0",
      "cpgNum:Integer:0",
      "gcNum:Integer:0",
      "perCpg:Double",
      "perGc:Double",
      "obsExp:Double"
      ])

    file_data = None
    with open("data/cpgIslandExtFull.txt", 'r') as f:
      file_data = f.read()

      res = epidb.add_annotation("Cpg Islands", "hg19", "Complete CpG islands", file_data, cpg_island, None, self.admin_key)
      self.assertSuccess(res)

    size_total = len(file_data.split("\n"))-1 # -1 last line

    (status, qid_cpg) = epidb.select_annotations("Cpg Islands", "hg19", "chr1", None, None, self.admin_key)

    (s, c) = epidb.count_regions(qid_cpg, self.admin_key)

    self.assertEquals(2462, c)

    total = 0
    (status, cq1) = epidb.select_annotations("Cpg Islands", "hg19", "chr1", None, None, self.admin_key)
    (s, c1) = epidb.count_regions(cq1, self.admin_key)

    (status, cq2) = epidb.select_annotations("Cpg Islands", "hg19", "chr7", None, None, self.admin_key)
    (s, c2) = epidb.count_regions(cq2, self.admin_key)

    (status, cq3) = epidb.select_annotations("Cpg Islands", "hg19", "chr18", None, None, self.admin_key)
    (s, c3) = epidb.count_regions(cq3, self.admin_key)

    (status, cq4) = epidb.select_annotations("Cpg Islands", "hg19", "chrX", None, None, self.admin_key)
    (s, c4) = epidb.count_regions(cq4, self.admin_key)

    total = int(c1) + int(c2) + int(c3) + int(c4)

    (status, qid_count) = epidb.select_annotations("Cpg Islands", "hg19", ["chr1","chr7","chr18","chrX"], None, None, self.admin_key)
    (s, c) = epidb.count_regions(qid_count, self.admin_key)
    self.assertEquals(c, total)

    cpg_island_chrs = """chr1
chr10
chr11
chr11_gl000202_random
chr12
chr13
chr14
chr15
chr16
chr17
chr17_ctg5_hap1
chr17_gl000204_random
chr17_gl000205_random
chr18
chr19
chr1_gl000191_random
chr1_gl000192_random
chr2
chr20
chr21
chr22
chr3
chr4
chr4_ctg9_hap1
chr4_gl000193_random
chr4_gl000194_random
chr5
chr6
chr6_apd_hap1
chr6_cox_hap2
chr6_dbb_hap3
chr6_mann_hap4
chr6_mcf_hap5
chr6_qbl_hap6
chr6_ssto_hap7
chr7
chr8
chr8_gl000197_random
chr9
chr9_gl000199_random
chr9_gl000200_random
chr9_gl000201_random
chrUn_gl000211
chrUn_gl000212
chrUn_gl000213
chrUn_gl000214
chrUn_gl000215
chrUn_gl000216
chrUn_gl000217
chrUn_gl000218
chrUn_gl000219
chrUn_gl000220
chrUn_gl000221
chrUn_gl000222
chrUn_gl000223
chrUn_gl000224
chrUn_gl000225
chrUn_gl000228
chrUn_gl000229
chrUn_gl000231
chrUn_gl000235
chrUn_gl000236
chrUn_gl000237
chrUn_gl000240
chrUn_gl000241
chrUn_gl000242
chrUn_gl000243
chrX
chrY"""

    (status, qid_count) = epidb.select_annotations("Cpg Islands", "hg19", cpg_island_chrs.split("\n"), None, None, self.admin_key)
    (s, c) = epidb.count_regions(qid_count, self.admin_key)
    self.assertEquals(size_total, c)

  def test_not_find_genome_and_in_order_chromosoms(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    genome_data = """chr1 1000000
chr2 900000
chr3 500000
chrX 100000"""
    epidb.add_genome("Genome Example", "Example of Genome for the Manual", genome_data, self.admin_key)

    x = epidb.chromosomes("Genome Example", self.admin_key)
    self.assertEquals(x, ['okay', [['chr1', 1000000], ['chr2', 900000], ['chr3', 500000], ['chrX', 100000]]])

