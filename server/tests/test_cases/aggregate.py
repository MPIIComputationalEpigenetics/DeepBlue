import helpers

from deepblue_client import DeepBlueClient


class TestAggregateCommand(helpers.TestCase):

  def test_correct_limits(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    regions_range = "chr1\t150\t225"
    regions_data = "chr1\t125\t150\nchr1\t150\t175\nchr1\t175\t200\nchr1\t200\t225\nchr1\t225\t250"

    (s, q_range) = epidb.input_regions("hg19", regions_range, self.admin_key)
    (s, q_date) = epidb.input_regions("hg19", regions_data, self.admin_key)

    (s, q_agg) = epidb.aggregate(q_date, q_range, "START", self.admin_key)

    status, req = epidb.get_regions(q_agg, "CHROMOSOME,START,END,@AGG.COUNT", self.admin_key)
    rs = self.get_regions_request(req)

    self.assertEquals("chr1\t150\t225\t3", rs)

  def test_correction_score(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
    (s, data) = epidb.select_experiments("hg19_chr1_1", "chr1", None, None, self.admin_key)
    res, qid_2 = epidb.tiling_regions(1000, "hg19", "chr1", self.admin_key)
    self.assertSuccess(res, qid_2)

    (s, q_agg) = epidb.aggregate(data, qid_2, "SIGNAL_VALUE", self.admin_key)
    res, qid_4 = epidb.filter_regions(q_agg, "@AGG.COUNT",  ">", "0", "number", self.admin_key)
    status, req = epidb.get_regions(qid_4, "CHROMOSOME,START,END,@AGG.MEAN,@AGG.COUNT", self.admin_key)

    rs = self.get_regions_request(req)

    self.assertEquals(rs, "chr1\t713000\t714000\t27.1111\t3\nchr1\t714000\t715000\t39.5556\t3\nchr1\t715000\t716000\t24.0000\t1\nchr1\t761000\t762000\t6.0000\t1\nchr1\t762000\t763000\t54.6667\t3\nchr1\t763000\t764000\t12.0000\t1\nchr1\t839000\t840000\t15.0000\t1\nchr1\t840000\t841000\t14.0000\t2\nchr1\t858000\t859000\t16.8000\t1\nchr1\t859000\t860000\t10.6000\t2\nchr1\t860000\t861000\t41.0000\t1\nchr1\t861000\t862000\t22.0000\t1\nchr1\t875000\t876000\t13.0000\t2\nchr1\t876000\t877000\t13.5000\t2")

  def test_aggregation(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

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
    res, req = epidb.count_regions(qid_2, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEquals(count, 3118)

    res, _qid_3 = epidb.aggregate(q_cgi, qid_2, "@LENGTH", self.admin_key)
    self.assertSuccess(res, _qid_3)

    res, qid_3 = epidb.query_cache(_qid_3, True, self.admin_key)
    self.assertSuccess(res, qid_3)


    res, qid_4 = epidb.filter_regions(qid_3, "@AGG.COUNT",  ">", "0", "number", self.admin_key)
    res, req = epidb.count_regions(qid_4, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 2574)

    res, qid_4 = epidb.filter_regions(qid_2, "@AGG.COUNT",  "<", "0", "number", self.admin_key)
    res, req = epidb.count_regions(qid_4, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 0)

    res, qid_4 = epidb.filter_regions(qid_3, "@AGG.COUNT",  ">=", "100", "number", self.admin_key)
    (res, req) = epidb.get_regions(qid_4, "CHROMOSOME,START,END,@AGG.MIN,@AGG.MAX,@AGG.MEDIAN,@AGG.MEAN,@AGG.VAR,@AGG.SD,@AGG.COUNT,@AGG.SUM", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected = 'chr1\t1000000\t2000000\t201.0000\t5585.0000\t469.0000\t766.0082\t589695.4375\t767.9163\t122\t93453.0000\nchr16\t0\t1000000\t201.0000\t6377.0000\t484.0000\t746.6083\t674998.0625\t821.5826\t120\t89593.0000\nchr16\t1000000\t2000000\t201.0000\t5449.0000\t398.0000\t666.6393\t630197.3125\t793.8497\t122\t81330.0000\nchr16\t2000000\t3000000\t201.0000\t4843.0000\t533.0000\t780.4951\t559994.2500\t748.3276\t101\t78830.0000\nchr16\t88000000\t89000000\t202.0000\t3785.0000\t347.0000\t553.3461\t295400.6875\t543.5078\t104\t57548.0000\nchr19\t0\t1000000\t201.0000\t7814.0000\t424.0000\t776.2705\t944608.4375\t971.9097\t122\t94705.0000\nchr19\t1000000\t2000000\t201.0000\t6035.0000\t430.0000\t738.8853\t625527.1250\t790.9027\t183\t135216.0000\nchr19\t2000000\t3000000\t201.0000\t3978.0000\t395.0000\t673.9907\t444749.5000\t666.8954\t107\t72117.0000\nchr19\t3000000\t4000000\t201.0000\t2753.0000\t387.0000\t531.0648\t172512.1094\t415.3458\t108\t57355.0000\nchr20\t62000000\t63000000\t202.0000\t5019.0000\t501.0000\t716.2427\t427763.9375\t654.0366\t103\t73773.0000\nchr7\t0\t1000000\t201.0000\t6234.0000\t348.0000\t556.3500\t475220.5625\t689.3624\t100\t55635.0000\nchr9\t139000000\t140000000\t202.0000\t6342.0000\t406.0000\t777.3303\t817548.5625\t904.1839\t109\t84729.0000'

    self.assertEquals(regions, expected)
    (s, req) = epidb.count_regions(qid_4, self.admin_key)
    self.assertSuccess(s, req)
    count = self.count_request(req)
    self.assertEquals(count, 12)

  def test_aggregation_bug_on_normalfields(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    res = epidb.add_technique("ChIP-seq", "ChIP-sequencing", {}, self.admin_key)
    self.assertSuccess(res)

    sample_id = self.sample_ids[0]

    broad_peak_format = ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "SIGNAL_VALUE",
      "P_VALUE",
      "Q_VALUE",
      ])

    with open("data/wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed", 'r') as f:
      file_data = f.read()
      (res, exp) = epidb.add_experiment("wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed", "hg19", "H3k4me3", "s1", "ChIPseq", "ENCODE", "wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed from ENCODE",  file_data, broad_peak_format, None, self.admin_key)
      self.assertSuccess(res, exp)
      res, q_exp = epidb.select_experiments("wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed", "chr1", None, None,self.admin_key)
      self.assertSuccess(res, q_exp)

    # Insert annotation
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


    res, qid_3 = epidb.aggregate(q_exp, q_cgi, "SCORE", self.admin_key)
    self.assertSuccess(res, qid_3)

    res, chr_sizes = epidb.select_annotations("Chromosomes size for hg19", "hg19", None, None, None, self.admin_key)
    res, qid_5 = epidb.aggregate(qid_3, chr_sizes, "@AGG.COUNT", self.admin_key)

    status, req = epidb.get_regions(qid_5, "CHROMOSOME,@AGG.MIN,@AGG.MAX,@AGG.MEDIAN,@AGG.MEAN,@AGG.VAR,@AGG.SD,@AGG.COUNT,@AGG.SUM", self.admin_key)
    rs = self.get_regions_request(req)

    expected = "chr1\t0.0000\t8.0000\t1.0000\t0.8018\t0.3498\t0.5915\t2462\t1974.0000"
    self.assertEquals(rs, expected)

  def test_aggregation_bug_on_normalfields_better_data(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    res = epidb.add_technique("ChIP-seq", "ChIP-sequencing", {}, self.admin_key)
    self.assertSuccess(res)

    sample_id = self.sample_ids[0]

    broad_peak_format = ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "SIGNAL_VALUE",
      "P_VALUE",
      "Q_VALUE",
      ])

    with open("data/wgEncodeBroadHistoneH1hescH3k27me3StdPk.bed", 'r') as f:
      file_data = f.read()
      (res, a_1) = epidb.add_experiment("wgEncodeBroadHistoneH1hescH3k27me3StdPk.bed", "hg19", "H3k4me3", "s1", "ChIPseq", "ENCODE", "wgEncodeBroadHistoneH1hescH3k27me3StdPk.bed from ENCODE",  file_data, broad_peak_format, None, self.admin_key)
      self.assertSuccess(res, a_1)
      res, q_exp = epidb.select_experiments("wgEncodeBroadHistoneH1hescH3k27me3StdPk.bed", "chr1", None, None, self.admin_key)
      self.assertSuccess(res, q_exp)

    # Insert annotation
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

    res, qid_3 = epidb.aggregate(q_exp, q_cgi, "SCORE", self.admin_key)
    self.assertSuccess(res, qid_3)

    res, chr_sizes = epidb.select_annotations("Chromosomes size for hg19", "hg19", None, None, None, self.admin_key)
    res, qid_5 = epidb.aggregate(qid_3, chr_sizes, "@AGG.COUNT", self.admin_key)

    status, req = epidb.get_regions(qid_5, "CHROMOSOME,@AGG.MIN,@AGG.MAX,@AGG.MEDIAN,@AGG.MEAN,@AGG.VAR,@AGG.SD,@AGG.COUNT", self.admin_key)
    rs = self.get_regions_request(req)

    expected = 'chr1\t0.0000\t8.0000\t0.0000\t0.5142\t0.5301\t0.7281\t2462'
    self.assertEquals(rs, expected)
