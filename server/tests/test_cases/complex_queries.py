import helpers

from client import EpidbClient


class TestComplexQueries(helpers.TestCase):

  def test_complex1(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    res, qid_1_1 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    #server.get_regions(qid_1_1, )

    self.assertSuccess(res, qid_1_1)
    res, qid_1_2 = epidb.select_regions("hg19_chr1_2", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1_2)

    res, qid_2 = epidb.merge_queries(qid_1_1, qid_1_2, self.admin_key)
    self.assertSuccess(res, qid_2)

    res, regions = epidb.get_regions(qid_2, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)

    res, qid_1_3 = epidb.select_regions("hg19_chr1_3", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1_3)

    res, qid_5 = epidb.intersection(qid_2, qid_1_3, self.admin_key)
    self.assertSuccess(res, qid_5)

    res, req = epidb.get_regions(qid_5, "CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

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
    res, req = epidb.count_regions(qid_1_1, self.admin_key)
    self.assertSuccess(res, req)
    c = self.count_request(req)

    res, qid_1_2 = epidb.select_regions("hg19_big_1", "hg19", None, None, None,
                                      None, ["chr1", "chr3", "chr11", "chrX", "chr9"], None, None, self.admin_key)
    self.assertSuccess(res, qid_1_2)
    res, req = epidb.count_regions(qid_1_2, self.admin_key)
    self.assertSuccess(res, req)
    c = self.count_request(req)

    # this gives us regions from 1,000,000 to 3,000,000 on chromosomes chr1, chr3, chr9, chr11, chrY
    res, qid_2_1 = epidb.intersection(qid_1_1, qid_1_2, self.admin_key)
    self.assertSuccess(res, qid_2_1)
    res, req = epidb.count_regions(qid_2_1, self.admin_key)
    self.assertSuccess(res, req)
    c = self.count_request(req)
    self.assertEqual(c, 247)

    res, qid_2_2 = epidb.tiling_regions(1000, "hg19", ["chr1", "chr2", "chr15", "chrX"], self.admin_key)
    self.assertSuccess(res, qid_2_2)
    res, req = epidb.count_regions(qid_2_2, self.admin_key)
    self.assertSuccess(res, req)
    c = self.count_request(req)

    res, qid_3_1 = epidb.merge_queries(qid_2_1, qid_2_2, self.admin_key)
    self.assertSuccess(res, qid_3_1)
    res, req = epidb.count_regions(qid_3_1, self.admin_key)
    self.assertSuccess(res, req)
    c = self.count_request(req)

    res, qid_4_1 = epidb.filter_regions(qid_3_1, "START",  ">=", "2000000", "number", self.admin_key)
    self.assertSuccess(res, qid_4_1)
    res, req = epidb.count_regions(qid_4_1, self.admin_key)
    self.assertSuccess(res, req)
    c = self.count_request(req)

    res, qid_4_2 = epidb.select_regions("hg19_big_2", "hg19", None, None, None,
                                      None, ["chr1", "chrX"], None, None, self.admin_key)
    self.assertSuccess(res, qid_4_2)
    res, req = epidb.count_regions(qid_4_2, self.admin_key)
    self.assertSuccess(res, req)
    c = self.count_request(req)
    self.assertEqual(c, 8961)

    res, qid_5_1 = epidb.intersection(qid_4_1, qid_4_2, self.admin_key)
    self.assertSuccess(res, qid_5_1)
    res, req = epidb.count_regions(qid_5_1, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)

    self.assertEqual(count, 14370)

    res, qid_6_1 = epidb.filter_regions(qid_5_1, "END",  "<", "2200000", "number", self.admin_key)
    self.assertSuccess(res, qid_6_1)

    res, req = epidb.count_regions(qid_6_1, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEqual(count, 52)


    res, req = epidb.get_regions(qid_6_1, "CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK,@NAME", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected_regions = helpers.get_result("complex2")
    self.assertEqual(regions, expected_regions)

  def test_complex_input_regions(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    regions = "chr1\t1\t10000\nchr2\t2\t20000\nchr3\t3\t30000"

    (s, q) = epidb.input_regions("hg19", regions, self.admin_key)
    res, req = epidb.count_regions(q, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEqual(count, 3)

    res, req = epidb.get_regions(q, "CHROMOSOME,START,END,NAME,@NAME,@EPIGENETIC_MARK,@CALCULATED(return value_of('END') - value_of('START') )", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    output = """chr1\t1\t10000\t\tQuery q1 regions set\t\t9999.000000
chr2\t2\t20000\t\tQuery q1 regions set\t\t19998.000000
chr3\t3\t30000\t\tQuery q1 regions set\t\t29997.000000"""

    self.assertEqual(regions, output)