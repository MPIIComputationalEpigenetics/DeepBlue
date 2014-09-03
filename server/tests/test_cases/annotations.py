import helpers

from client import EpidbClient


class TestAnnotationCommands(helpers.TestCase):

  def test_annotation(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    file_data = None
    with open("data/cpgIslandExt.txt", 'r') as f:
      file_data = f.read()

    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
          file_data,
          "",
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)

    res, annotations = epidb.list_annotations("hg19", self.admin_key)
    self.assertSuccess(res, annotations)
    self.assertEqual(len(annotations), 2)

    self.assertEqual(annotations[0][1], "hg19")
    self.assertEqual(annotations[1][1], "Cpg Islands")

    size = len(file_data.split("\n"))

    res, qid = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, count = epidb.count_regions(qid, self.admin_key)
    self.assertSuccess(res, count)
    self.assertEqual(size, count)

    res, regions = epidb.get_regions(qid, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)
    self.assertEqual(regions, file_data)

  def test_annotation_full_cpg_islands(self):
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

    with open("data/cpgIslandExtFull.txt", 'r') as f:
      file_data = f.read()
      regions_count = len(file_data.split("\n")) -1
      (res, a_1) = epidb.add_annotation("Cpg Islands", "hg19", "Complete CpG islands", file_data, cpg_island, None, self.admin_key)
      self.assertSuccess(res, a_1)
      res, qid_1 = epidb.select_annotations("Cpg Islands", "hg19", "chr1", None, None, self.admin_key)
      self.assertSuccess(res, qid_1)

      (s, c) = epidb.count_regions(qid_1, self.admin_key)

      self.assertEqual(regions_count, c)