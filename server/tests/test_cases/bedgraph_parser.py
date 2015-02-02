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


    (s, q) = epidb.select_regions(files, "hg19", None, None, None, None, None, None, None, self.admin_key)

    (s, req) = epidb.count_regions(q, self.admin_key)
    self.assertSuccess(s, req)
    count = self.count_request(req)

    # 3997106 // grep -v # *.bg | grep -v browser | grep -v track | wc -l
    self.assertEqual(3997106, count)
