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


  def test_overlap(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)


    ##### SET 1 ######
    res, qid_1 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1)
    res, qid_2 = epidb.select_regions("hg19_chr1_2", "hg19", None, None, None,
                                      None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid_2)

    res, qid_3 = epidb.overlap(qid_1, qid_2, True, 0, "bp", self.admin_key)
    self.assertSuccess(res, qid_3)
    res, req = epidb.get_regions(qid_3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected_regions = helpers.get_result("intersection")
    self.assertEqual(regions, expected_regions)

    res, qid_31 = epidb.overlap(qid_1, qid_2, True, 165, "bp", self.admin_key)
    self.assertSuccess(res, qid_31)
    res, req = epidb.get_regions(qid_31, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected_regions = """chr1\t713240\t713390
chr1\t713900\t714050
chr1\t714160\t714310
chr1\t715060\t715210
chr1\t762420\t762570
chr1\t840080\t840230
chr1\t840600\t840750
chr1\t858880\t859030
chr1\t859600\t859750
chr1\t860240\t860390
chr1\t875900\t876050"""

    self.assertEqual(regions, expected_regions)

    res, qid_4 = epidb.overlap(qid_1, qid_2, False, 0, "bp", self.admin_key)
    self.assertSuccess(res, qid_4)
    res, req = epidb.get_regions(qid_4, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions2 = self.get_regions_request(req)

    expected = """chr1	761180	761330
chr1	763020	763170
chr1	839540	839690
chr1	875400	875550
chr1	876180	876330"""

    self.assertEquals(regions2, expected)



    res, qid_41 = epidb.overlap(qid_1, qid_2, False, 0, "bp", self.admin_key)
    self.assertSuccess(res, qid_41)
    res, req = epidb.get_regions(qid_41, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions2 = self.get_regions_request(req)

    expected = """chr1	761180	761330
chr1	763020	763170
chr1	839540	839690
chr1	875400	875550
chr1	876180	876330"""


    res, qid_41 = epidb.overlap(qid_1, qid_2, False, 0, "%", self.admin_key)
    self.assertSuccess(res, qid_41)
    res, req = epidb.get_regions(qid_41, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions2 = self.get_regions_request(req)

    expected = """chr1	761180	761330
chr1	763020	763170
chr1	839540	839690
chr1	875400	875550
chr1	876180	876330"""

    self.assertEquals(regions2, expected)

    res, qid_5 = epidb.overlap(qid_1, qid_2, False, 350, "bp", self.admin_key)
    self.assertSuccess(res, qid_5)
    res, req = epidb.get_regions(qid_5, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected = """chr1	761180	761330
chr1	763020	763170
chr1	875400	875550"""

    self.assertEquals(regions, expected)

    res, qid_6 = epidb.overlap(qid_1, qid_2, False, 500, "%", self.admin_key)
    self.assertSuccess(res, qid_6)
    res, req = epidb.get_regions(qid_6, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEquals(regions, "chr1\t763020\t763170")


    ##### SET 2 ######
    res, q1 = epidb.input_regions("hg19", "chr1\t1000\t1010\nchr1\t1100\t1110\nchr1\t1200\t1210\nchr1\t1300\t1310\nchr1\t1400\t1410\nchr1\t1500\t1510", self.admin_key)
    self.assertSuccess(res, q1)

    res, q2 = epidb.input_regions("hg19", "chr1\t1011\t1020\nchr1\t1111\t1120\nchr1\t1211\t1220\nchr1\t1311\t1320\nchr1\t1411\t1420\nchr1\t1511\t1520", self.admin_key)
    self.assertSuccess(res, q2)


    res, q3 = epidb.overlap(q1, q2, False, 0, "bp", self.admin_key)
    self.assertSuccess(res, q3)
    res, req = epidb.get_regions(q3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    expected = 'chr1\t1000\t1010\nchr1\t1100\t1110\nchr1\t1200\t1210\nchr1\t1300\t1310\nchr1\t1400\t1410\nchr1\t1500\t1510'
    self.assertEquals(regions, expected)


    res, q3 = epidb.overlap(q1, q2, False, 20, "%", self.admin_key)
    self.assertSuccess(res, q3)
    res, req = epidb.get_regions(q3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    expected = ''
    self.assertEquals(regions, expected)


    res, q3 = epidb.overlap(q1, q2, False, 1, "%", self.admin_key)
    self.assertSuccess(res, q3)
    res, req = epidb.get_regions(q3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    expected = 'chr1\t1000\t1010\nchr1\t1100\t1110\nchr1\t1200\t1210\nchr1\t1300\t1310\nchr1\t1400\t1410\nchr1\t1500\t1510'
    self.assertEquals(regions, expected)

    res, q3 = epidb.overlap(q2, q1, False, 0, "bp", self.admin_key)
    self.assertSuccess(res, q3)
    res, req = epidb.get_regions(q3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEquals(regions, 'chr1\t1011\t1020\nchr1\t1111\t1120\nchr1\t1211\t1220\nchr1\t1311\t1320\nchr1\t1411\t1420\nchr1\t1511\t1520')


    res, q4 = epidb.input_regions("hg19", "chr1\t1000\t1010\nchr1\t1200\t1210\nchr1\t1300\t1310", self.admin_key)
    self.assertSuccess(res, q4)
    res, q5 = epidb.input_regions("hg19", "chr1\t1011\t1020\nchr1\t1111\t1120\nchr1\t1211\t1220\nchr1\t1311\t1320\nchr1\t1411\t1420\nchr1\t1511\t1520", self.admin_key)
    self.assertSuccess(res, q5)

    res, q6 = epidb.overlap(q4, q5, False, 0, "bp", self.admin_key)
    self.assertSuccess(res, q6)
    res, req = epidb.get_regions(q6, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEquals(regions, 'chr1\t1000\t1010\nchr1\t1200\t1210\nchr1\t1300\t1310')

    res, q4 = epidb.input_regions("hg19", "chr1\t1000\t1010", self.admin_key)
    self.assertSuccess(res, q4)
    res, q5 = epidb.input_regions("hg19", "chr1\t1011\t1020\nchr1\t1111\t1120\nchr1\t1211\t1220\nchr1\t1311\t1320\nchr1\t1411\t1420\nchr1\t1511\t1520", self.admin_key)
    self.assertSuccess(res, q5)

    res, q6 = epidb.overlap(q4, q5, False, 0, "bp", self.admin_key)
    self.assertSuccess(res, q6)
    res, req = epidb.get_regions(q6, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, "chr1\t1000\t1010")

    res, q7 = epidb.overlap(q5, q4, False, 0, "bp", self.admin_key)
    self.assertSuccess(res, q7)
    res, req = epidb.get_regions(q7, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, "chr1\t1011\t1020\nchr1\t1111\t1120\nchr1\t1211\t1220\nchr1\t1311\t1320\nchr1\t1411\t1420\nchr1\t1511\t1520")


  def test_big_files_intersect(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)


    self.insert_experiment(epidb, "hg19_big_1")
    self.insert_experiment(epidb, "hg19_big_2")


    ## Testing if the total of regions is the same as overlap and not-overlap hg19_big_1
    res, qid_1_1 = epidb.select_experiments("hg19_big_1", None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1_1)

    res, req = epidb.count_regions(qid_1_1, self.admin_key)
    self.assertSuccess(res, req)
    c = self.count_request(req)
    self.assertEquals(c, 54307)

    res, qid_1_2 = epidb.select_experiments("hg19_big_2", None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1_2)

    res, q6 = epidb.overlap(qid_1_1, qid_1_2, False, 0, "bp", self.admin_key)
    self.assertSuccess(res, q6)
    res, req = epidb.count_regions(q6, self.admin_key)
    self.assertSuccess(res, req)
    c1 = self.count_request(req)
    self.assertEquals(c1, 17180)

    res, q6 = epidb.overlap(qid_1_1, qid_1_2, True, 0, "bp", self.admin_key)
    self.assertSuccess(res, q6)
    res, req = epidb.count_regions(q6, self.admin_key)
    self.assertSuccess(res, req)
    c2 = self.count_request(req)
    self.assertEquals(c2, 37127)

    self.assertEquals(c, c1+c2)

    ## Testing if the total of regions is the same as overlap and not-overlap hg19_big_2
    res, req = epidb.count_regions(qid_1_2, self.admin_key)
    self.assertSuccess(res, req)
    cc1 = self.count_request(req)
    self.assertEquals(cc1, 77543)

    res, q7 = epidb.overlap(qid_1_2, qid_1_1, False, 0, "bp", self.admin_key)
    self.assertSuccess(res, q7)
    res, req1 = epidb.count_regions(q7, self.admin_key)
    self.assertSuccess(res, req1)
    c3 = self.count_request(req1)
    self.assertEquals(c3, 39771)

    res, q8 = epidb.overlap(qid_1_2, qid_1_1, True, 0, "bp", self.admin_key)
    self.assertSuccess(res, q8)
    res, req2 = epidb.count_regions(q8, self.admin_key)
    self.assertSuccess(res, req2)
    c4 = self.count_request(req2)
    self.assertEquals(c4, 37772)
    self.assertEquals(cc1, c3+c4)

    res, q9 = epidb.overlap(qid_1_2, q7, False, 0, "bp", self.admin_key)
    res, req3X = epidb.count_regions(q9, self.admin_key)
    self.assertSuccess(res, req3X)
    c3XX = self.count_request(req3X)
    self.assertEquals(c3XX, 37772)


    res, q10 = epidb.overlap(q9, q8, False, 0, "bp", self.admin_key)
    res, req3 = epidb.count_regions(q10, self.admin_key)
    self.assertSuccess(res, req3)
    cXX = self.count_request(req3)
    self.assertEquals(cXX, 0)

    res, q11 = epidb.overlap(qid_1_2, q8, False, 0, "bp", self.admin_key)
    res, req4X = epidb.count_regions(q11, self.admin_key)
    self.assertSuccess(res, req4X)
    c4XX = self.count_request(req4X)
    self.assertEquals(c4XX, 39771)

    res, q12 = epidb.overlap(q9, q7, False, 0, "bp", self.admin_key)
    res, req5X = epidb.count_regions(q10, self.admin_key)
    self.assertSuccess(res, req5X)
    c5XX = self.count_request(req5X)
    self.assertEquals(c5XX, 0)
