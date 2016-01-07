import helpers

from deepblue_client import DeepBlueClient


class TestIntersection(helpers.TestCase):

  def test_intersection(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    res, qid_1 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1)
    res, qid_2 = epidb.select_regions("hg19_chr1_2", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_2)

    res, qid_3 = epidb.intersection(qid_1, qid_2, self.admin_key)
    self.assertSuccess(res, qid_3)

    res, req = epidb.get_regions(qid_3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected_regions = helpers.get_result("intersection")
    self.assertEqual(regions, expected_regions)


  def test_intersection_two_genomes(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    res, qid_1 = epidb.select_regions("hg19_chr1_1", ["hg19", "hg18"], None, None, None,
                                        None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1)
    res, qid_2 = epidb.select_regions("hg18_chr1_1", ["hg18", "hg19"], None, None, None,
                                        None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_2)

    res, qid_3 = epidb.intersection(qid_1, qid_2, self.admin_key)
    self.assertSuccess(res, qid_3)

    res, req = epidb.get_regions(qid_3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected_regions = helpers.get_result("intersection_multiple_genomes")
    self.assertEqual(regions, expected_regions)
