import helpers

from client import EpidbClient


class TestFilterCommand(helpers.TestCase):

  def test_filter_regions(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    res, qid = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, qid2 = epidb.filter_regions(qid, "START",  ">=", "875400 ", "number", self.admin_key)
    self.assertSuccess(res, qid2)

    res, regions = epidb.get_regions(qid2, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)

    expected_regions = helpers.get_result("filter_ge_875400")
    self.assertEqual(regions, expected_regions)

  def test_filter_two_genomes(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    res, qid = epidb.select_regions(["hg19_chr1_1", "hg18_chr1_1"], ["hg19", "hg18"], None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, qid2 = epidb.filter_regions(qid, "START",  ">=", "875400 ", "number", self.admin_key)
    self.assertSuccess(res, qid2)

    res, regions = epidb.get_regions(qid2, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)

    expected_regions = helpers.get_result("filter_multiple_genomes_ge_875400")
    self.assertEqual(regions, expected_regions)