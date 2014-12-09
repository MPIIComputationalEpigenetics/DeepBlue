import helpers

from client import EpidbClient


class TestComplexQueries(helpers.TestCase):

  def test_complex1(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    res, qid_1 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1)
    res, qid_2 = epidb.select_regions("hg19_chr1_2", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_2)

    res, qid_3 = epidb.merge_queries(qid_1, qid_2, self.admin_key)
    self.assertSuccess(res, qid_3)

    res, qid_4 = epidb.select_regions("hg19_chr1_3", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_4)

    res, qid_5 = epidb.intersection(qid_3, qid_4, self.admin_key)
    self.assertSuccess(res, qid_5)

    res, regions = epidb.get_regions(qid_5, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)

    expected_regions = helpers.get_result("complex1")
    self.assertEqual(regions, expected_regions)

  def test_complex2(self):
    epidb = EpidbClient()
    self.init_full(epidb)


    self.insert_experiment(epidb, "hg19_big_1")
    self.insert_experiment(epidb, "hg19_big_2")

    res, qid_1_1 = epidb.select_regions("hg19_big_1", "hg19", None, None, None,
                                      None, None, 1000000, 3000000, self.admin_key)
    self.assertSuccess(res, qid_1_1)
    res, count = epidb.count_regions(qid_1_1, self.admin_key)
    self.assertSuccess(res, count)

    res, qid_1_2 = epidb.select_regions("hg19_big_1", "hg19", None, None, None,
                                      None, ["chr1", "chr3", "chr11", "chrX", "chr9"], None, None, self.admin_key)
    self.assertSuccess(res, qid_1_2)
    res, count = epidb.count_regions(qid_1_2, self.admin_key)
    self.assertSuccess(res, count)

    # this gives us regions from 1,000,000 to 3,000,000 on chromosomes chr1, chr3, chr9, chr11, chrY
    res, qid_2_1 = epidb.intersection(qid_1_1, qid_1_2, self.admin_key)
    self.assertSuccess(res, qid_2_1)
    res, count = epidb.count_regions(qid_2_1, self.admin_key)
    self.assertSuccess(res, count)

    res, qid_2_2 = epidb.tiling_regions(1000, "hg19", ["chr1", "chr2", "chr15", "chrX"], self.admin_key)
    self.assertSuccess(res, qid_2_2)
    res, count = epidb.count_regions(qid_2_2, self.admin_key)
    self.assertSuccess(res, count)

    res, qid_3_1 = epidb.merge_queries(qid_2_1, qid_2_2, self.admin_key)
    self.assertSuccess(res, qid_3_1)
    res, count = epidb.count_regions(qid_3_1, self.admin_key)
    self.assertSuccess(res, count)

    res, qid_4_1 = epidb.filter_regions(qid_3_1, "START",  ">=", "2000000", "number", self.admin_key)
    self.assertSuccess(res, qid_4_1)
    res, count = epidb.count_regions(qid_4_1, self.admin_key)
    self.assertSuccess(res, count)

    res, qid_4_2 = epidb.select_regions("hg19_big_2", "hg19", None, None, None,
                                      None, ["chr1", "chrX"], None, None, self.admin_key)
    self.assertSuccess(res, qid_4_2)
    res, count = epidb.count_regions(qid_4_2, self.admin_key)
    self.assertSuccess(res, count)

    res, qid_5_1 = epidb.intersection(qid_4_1, qid_4_2, self.admin_key)
    self.assertSuccess(res, qid_5_1)
    res, count = epidb.count_regions(qid_5_1, self.admin_key)
    self.assertSuccess(res, count)

    res, qid_6_1 = epidb.filter_regions(qid_5_1, "END",  "<", "2200000", "number", self.admin_key)
    self.assertSuccess(res, qid_6_1)

    res, regions = epidb.get_regions(qid_6_1, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)
