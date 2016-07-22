import helpers

from deepblue_client import DeepBlueClient


class TestFilterCommand(helpers.TestCase):

  def _test_filter_regions(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    res, qid = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, qid2 = epidb.filter_regions(qid, "START",  ">=", "875400 ", "number", self.admin_key)
    self.assertSuccess(res, qid2)

    res, req = epidb.get_regions(qid2, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    # Test filter with string values
    expected_regions = helpers.get_result("filter_ge_875400")
    self.assertEqual(regions, expected_regions)

    res, qid3 = epidb.filter_regions(qid, "STRAND",  "==", "+", "string", self.admin_key)
    self.assertSuccess(res, qid3)

    res, req = epidb.get_regions(qid3, "CHROMOSOME,START,END,STRAND", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, 'chr1\t713240\t713390\t+\nchr1\t713900\t714050\t+\nchr1\t714160\t714310\t+\nchr1\t714540\t714690\t+\nchr1\t715060\t715210\t+\nchr1\t762060\t762210\t+\nchr1\t839540\t839690\t+\nchr1\t840080\t840230\t+\nchr1\t860240\t860390\t+\nchr1\t875400\t875550\t+\nchr1\t876180\t876330\t+')

    res, qid3 = epidb.filter_regions(qid, "STRAND",  "!=", "+", "string", self.admin_key)
    self.assertSuccess(res, qid3)

    res, req = epidb.get_regions(qid3, "CHROMOSOME,START,END,STRAND", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, 'chr1\t713520\t713670\t-\nchr1\t761180\t761330\t-\nchr1\t762420\t762570\t.\nchr1\t762820\t762970\t-\nchr1\t763020\t763170\t-\nchr1\t840600\t840750\t-\nchr1\t858880\t859030\t.\nchr1\t859600\t859750\t.\nchr1\t861040\t861190\t-\nchr1\t875900\t876050\t-')

  def test_remove_full_chromosome_data(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    self.insert_experiment(epidb, "hg19_big_2")
    res, qid_1_1 = epidb.select_regions("hg19_big_2", "hg19", None, None, None,
                                      None, None, 0, 9841558, self.admin_key)

    res, req = epidb.count_regions(qid_1_1, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)

    (status, filtered_chr) = epidb.filter_regions(qid_1_1,"CHROMOSOME", "==", "chr21", "string", self.admin_key)
    res, req = epidb.get_regions(filtered_chr, "CHROMOSOME,START,END,STRAND", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    self.assertEquals(regions, "chr21\t9656828\t9656920\t.\nchr21\t9700370\t9700415\t.\nchr21\t9825445\t9826573\t.\nchr21\t9826759\t9827609\t.\nchr21\t9829381\t9829420\t.\nchr21\t9831594\t9831981\t.\nchr21\t9833197\t9833459\t.\nchr21\t9833733\t9833902\t.\nchr21\t9841288\t9841558\t.")


  def _test_filter_two_genomes(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    res, qid = epidb.select_regions(["hg19_chr1_1", "hg18_chr1_1"], ["hg19", "hg18"], None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, qid2 = epidb.filter_regions(qid, "START",  ">=", "875400 ", "number", self.admin_key)
    self.assertSuccess(res, qid2)

    res, req = epidb.get_regions(qid2, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected_regions = helpers.get_result("filter_multiple_genomes_ge_875400")

    self.assertEqual(regions, expected_regions)