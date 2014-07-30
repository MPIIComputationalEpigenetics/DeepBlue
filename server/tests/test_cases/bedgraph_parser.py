import time

import helpers

from client import EpidbClient

class TestBedgraphFiles(helpers.TestCase):

  def test_bed_graph_files(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    files = ["reference_example", "test1", "bigwig"]

    for filename in files:
      wig_data = helpers.load_bedgraph(filename)
      res = epidb.add_experiment(filename, "hg19", "Methylation", sample_id, "tech1",
                                 "ENCODE", "desc1", wig_data, "bedgraph", None, self.admin_key)
      self.assertSuccess(res)

