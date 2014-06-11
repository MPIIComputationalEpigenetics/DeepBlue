import helpers

from client import EpidbClient


class TestMerge(helpers.TestCase):

  def test_merge_queries(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    res, qid_1 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                 None, None, 760000, 860000, self.admin_key)
    self.assertSuccess(res, qid_1)

    res, qid_2 = epidb.select_regions("hg19_chr1_2", "hg19", None, None, None,
                                 None, None, 760000, 860000, self.admin_key)
    self.assertSuccess(res, qid_2)

    res, qid_3 = epidb.merge_queries(qid_1, qid_2, self.admin_key)
    self.assertSuccess(res, qid_3)

    expected_regions = helpers.get_result("merge_760k_860k")

    res, regions = epidb.get_regions(qid_3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)
    self.assertEqual(regions, expected_regions)


  def test_merge_different_chromosomes(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    res, qid_1 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                 None, "chr1", None, None, self.admin_key)
    self.assertSuccess(res, qid_1)

    res, qid_2 = epidb.select_regions("hg19_chr2_1", "hg19", None, None, None,
                                 None, "chr2", None, None, self.admin_key)
    self.assertSuccess(res, qid_2)


    res, qid_3 = epidb.merge_queries(qid_1, qid_2, self.admin_key)
    self.assertSuccess(res, qid_3)

    res, regions = epidb.get_regions(qid_3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)

    expected_regions = helpers.get_result("merge_different_chromosomes")

    res, regions = epidb.get_regions(qid_3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)
    self.assertEqual(regions, expected_regions)
