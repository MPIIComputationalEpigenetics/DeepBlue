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
