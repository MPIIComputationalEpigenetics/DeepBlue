import helpers

from deepblue_client import DeepBlueClient

class TestCoverageCommand(helpers.TestCase):

  def test_coverage(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    file_data = None
    with open("data/cpgIslandExtFull.txt", 'r') as f:
      file_data = f.read()

    cpg_island =  ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "LENGTH",
      "NUM_CPG",
      "NUM_GC",
      "PER_CPG",
      "PER_CG",
      "OBS_EXP"
      ])

    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
          file_data,
          cpg_island,
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)

    res, qid = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
    self.assertSuccess(res, qid)
    status, req = epidb.coverage(qid, "hg19", self.admin_key)
    coverage = self.get_regions_request(req)
    print coverage


    self.insert_experiment(epidb, "hg19_big_1")

    res, qid = epidb.select_experiments("hg19_big_1", None, None, None, self.admin_key)
    self.assertSuccess(res, qid)
    status, req = epidb.coverage(qid, "hg19", self.admin_key)
    coverage = self.get_regions_request(req)
