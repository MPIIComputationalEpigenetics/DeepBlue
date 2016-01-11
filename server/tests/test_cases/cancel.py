import helpers

from deepblue_client import DeepBlueClient


class TestAggregateCommand(helpers.TestCase):

  def test_cancel_aggregation(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    self.insert_experiment(epidb, "hg19_big_2", sample_id)

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

    with open("data/cpgIslandExtFull.txt", 'r') as f:
      file_data = f.read()
      (res, a_1) = epidb.add_annotation("Cpg Islands", "hg19", "Complete CpG islands", file_data, cpg_island, None, self.admin_key)
      self.assertSuccess(res, a_1)
      res, q_cgi = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
      self.assertSuccess(res, q_cgi)

    res, qid_2 = epidb.tiling_regions(1000000, "hg19", None, self.admin_key)
    self.assertSuccess(res, qid_2)
    res, req_count = epidb.count_regions(qid_2, self.admin_key)
    self.assertSuccess(res, req_count)
    count = self.count_request(req_count)
    self.assertEquals(count, 3118)

    res, qid_3 = epidb.aggregate(q_cgi, qid_2, "@LENGTH", self.admin_key)
    self.assertSuccess(res, qid_3)

    res, qid_4 = epidb.filter_regions(qid_3, "@AGG.COUNT",  ">=", "100", "number", self.admin_key)
    (res, req_regions) = epidb.get_regions(qid_4, "CHROMOSOME,START,END,@AGG.MIN,@AGG.MAX,@AGG.MEDIAN,@AGG.MEAN,@AGG.VAR,@AGG.SD,@AGG.COUNT", self.admin_key)
    self.assertSuccess(res, req_regions)

    epidb.cancel_request(req_regions, self.admin_key)
    epidb.cancel_request(req_count, self.admin_key)

    (s, ss_count) = epidb.info(req_count  , self.admin_key)
    self.assertEquals(ss_count[0]["state"], "removed")
    (s, ss_regions) = epidb.info(req_regions, self.admin_key)
    self.assertEquals(ss_regions[0]["state"], "canceled")

    s, e1 = epidb.get_request_data(req_count, self.admin_key)
    self.assertEqual(e1, "Request ID r1 was not finished. Please, check its status.")
    s, e2 = epidb.get_request_data(req_regions, self.admin_key)
    self.assertEqual(e2, "Request ID r2 was not finished. Please, check its status.")
