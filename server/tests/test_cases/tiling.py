import helpers

from client import EpidbClient


class TestTilingRegions(helpers.TestCase):

  def test_full_genome(self):
    epidb = EpidbClient()
    self.init_base()

    res, qid = epidb.tiling_regions(1000000, "hg19", None, self.admin_key)
    self.assertSuccess(res, qid)

    res, regions = epidb.get_regions(qid, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)


  def test_chromosomes(self):
    epidb = EpidbClient()
    self.init_base()

    res, qid = epidb.tiling_regions(1000000, "hg19", ["chr15", "chrX", "chr3"], self.admin_key)
    self.assertSuccess(res, qid)

    res, regions = epidb.get_regions(qid, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)

    chr3_tiles = 198022430 / 1000000
    chr15_tiles = 102531392 / 1000000
    chrX_tiles = 155270560 / 1000000

    self.assertEqual(len(regions.strip().split('\n')), chr3_tiles + chr15_tiles + chrX_tiles)


  def test_filter_tiling(self):
    epidb = EpidbClient()
    self.init_base()

    res, qid = epidb.tiling_regions(10000, "hg19", "chr1", self.admin_key)
    self.assertSuccess(res, qid)

    res, qid2 = epidb.filter_regions(qid, "END",  "<=", "100000", "number", self.admin_key)
    self.assertSuccess(res, qid2)

    res, regions = epidb.get_regions(qid2, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)

    expected_regions = helpers.get_result("filter_tiling")
    self.assertEqual(regions, expected_regions)


  def test_intersect_tiling(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    res, qid1 = epidb.tiling_regions(1000, "hg19", "chr1", self.admin_key)
    self.assertSuccess(res, qid1)

    res, qid2 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid2)

    res, qid3 = epidb.intersection(qid1, qid2, self.admin_key)
    self.assertSuccess(res, qid3)

    res, regions = epidb.get_regions(qid3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)

    expected_regions = helpers.get_result("intersect_tiling")
    self.assertEqual(regions, expected_regions)


  def test_merge_tiling(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    res, qid1 = epidb.tiling_regions(10000, "hg19", "chr1", self.admin_key)
    self.assertSuccess(res, qid1)

    res, qid2 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid2)

    # limit the tilings on the range of the experiment
    res, qid3 = epidb.filter_regions(qid1, "START",  ">=", "713240", "number", self.admin_key)
    self.assertSuccess(res, qid3)
    res, qid4 = epidb.filter_regions(qid3, "END",  "<=", "876330", "number", self.admin_key)
    self.assertSuccess(res, qid4)

    res, qid5 = epidb.merge_queries(qid4, qid2, self.admin_key)
    self.assertSuccess(res, qid5)

    res, regions = epidb.get_regions(qid5, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, regions)

    expected_regions = helpers.get_result("merge_tiling")
    self.assertEqual(regions, expected_regions)

  def test_metacolumn_experiment_name(self):
    epidb = EpidbClient()
    self.init_base()

    res, qid = epidb.tiling_regions(1000000, "hg19", "chrY", self.admin_key)
    self.assertSuccess(res, qid)

    res, regions = epidb.get_regions(qid, "CHROMOSOME,START,END,experiment_name", self.admin_key)
    self.assertSuccess(res, regions)

    expected = helpers.get_result("experiment_name_default")
    res, regions = epidb.get_regions(qid, "CHROMOSOME,START,END,experiment_name", self.admin_key)
    self.assertSuccess(res, regions)
    self.assertEqual(regions, expected)

    expected = helpers.get_result("experiment_name_custom")
    res, regions = epidb.get_regions(qid, "CHROMOSOME,START,END,experiment_name:no_name", self.admin_key)
    self.assertSuccess(res, regions)
    self.assertEqual(regions, expected)

    expected = """chrY\t0\t1000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t1000000\t2000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t2000000\t3000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t3000000\t4000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t4000000\t5000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t5000000\t6000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t6000000\t7000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t7000000\t8000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t8000000\t9000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t9000000\t10000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t10000000\t11000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t11000000\t12000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t12000000\t13000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t13000000\t14000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t14000000\t15000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t15000000\t16000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t16000000\t17000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t17000000\t18000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t18000000\t19000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t19000000\t20000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t20000000\t21000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t21000000\t22000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t22000000\t23000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t23000000\t24000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t24000000\t25000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t25000000\t26000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t26000000\t27000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t27000000\t28000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t28000000\t29000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t29000000\t30000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t30000000\t31000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t31000000\t32000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t32000000\t33000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t33000000\t34000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t34000000\t35000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t35000000\t36000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t36000000\t37000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t37000000\t38000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t38000000\t39000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t39000000\t40000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t40000000\t41000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t41000000\t42000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t42000000\t43000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t43000000\t44000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t44000000\t45000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t45000000\t46000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t46000000\t47000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t47000000\t48000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t48000000\t49000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t49000000\t50000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t50000000\t51000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t51000000\t52000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t52000000\t53000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t53000000\t54000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t54000000\t55000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t55000000\t56000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t56000000\t57000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t57000000\t58000000\tTiling regions of 1000000 (Genome hg19)\t1000000\nchrY\t58000000\t59000000\tTiling regions of 1000000 (Genome hg19)\t1000000"""
    res, regions = epidb.get_regions(qid, "CHROMOSOME,START,END,@NAME,@LENGTH", self.admin_key)
    self.assertSuccess(res, regions)
    self.assertEqual(regions, expected)


  def test_metacolumn_sequence(self):
    epidb = EpidbClient()
    self.init_base()

    sequence = open("data/genomes/chromosomes/chrM.fa").read()
    res = epidb.upload_chromosome("hg19", "chrM", sequence, self.admin_key)
    self.assertSuccess(res)

    res, qid = epidb.tiling_regions(10000, "hg19", "chrM", self.admin_key)
    self.assertSuccess(res, qid)

    expected = helpers.get_result("experiment_name_custom_sequence")
    res, regions = epidb.get_regions(qid, "CHROMOSOME,START,END,@NAME,@LENGTH,@SEQUENCE", self.admin_key)
    self.assertSuccess(res, regions)
    self.assertEqual(regions, expected)