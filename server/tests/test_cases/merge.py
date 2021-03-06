import helpers

from deepblue_client import DeepBlueClient


class TestMerge(helpers.TestCase):

  def test_merge_queries(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    res, qid_1 = epidb.select_experiments("hg19_chr1_1", None, 760000, 860000, self.admin_key)
    self.assertSuccess(res, qid_1)

    res, qid_2 = epidb.select_regions("hg19_chr1_2", "hg19", None, None, None,
                                 None, None, 760000, 860000, self.admin_key)
    self.assertSuccess(res, qid_2)

    res, qid_3 = epidb.merge_queries(qid_1, qid_2, self.admin_key)
    self.assertSuccess(res, qid_3)

    expected_regions = helpers.get_result("merge_760k_860k")

    res, req = epidb.get_regions(qid_3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, expected_regions)


  def test_merge_different_chromosomes(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    res, qid_1 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                 None, "chr1", None, None, self.admin_key)
    self.assertSuccess(res, qid_1)

    res, qid_2 = epidb.select_regions("hg19_chr2_1", "hg19", None, None, None,
                                 None, "chr2", None, None, self.admin_key)
    self.assertSuccess(res, qid_2)


    res, qid_3 = epidb.merge_queries(qid_1, qid_2, self.admin_key)
    self.assertSuccess(res, qid_3)

    res, req = epidb.get_regions(qid_3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected_regions = helpers.get_result("merge_different_chromosomes")
    self.assertEqual(regions, expected_regions)


  def test_multiple_merge(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sizes = [10000, 20000, 25000, 30000, 35000, 40000, 50000]
    qs = []

    for s in sizes:
      res, q_t = epidb.tiling_regions(s, "hg19", "chr21", self.admin_key)
      qs.append(q_t)

    res, qid_3 = epidb.merge_queries(qs[0], qs[1:], self.admin_key)
    self.assertSuccess(res, qid_3)

    res, req = epidb.count_regions(qid_3, self.admin_key)

    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEqual(count, 14287)

