import helpers

from client import EpidbClient


class TestScoreMatrixCommand(helpers.TestCase):

  def test_ascore_simple(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    res = epidb.add_technique("ChIP-seq", "ChIP-sequencing", {}, self.admin_key)
    self.assertSuccess(res)

    sample_id = self.sample_ids[0]

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
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

    with open("data/wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed", 'r') as f:
      file_data = f.read()
      (res, exp) = epidb.add_experiment("wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed", "hg19", "H3k4me3", "s1", "ChIPseq", "ENCODE", "wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed from ENCODE",  file_data, broad_peak_format, None, self.admin_key)
      self.assertSuccess(res, exp)

    # Insert annotation
    cpg_island =  ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "length:Integer",
      "cpgNum:Integer",
      "gcNum:Integer",
      "perCpg:Double",
      "perGc:Double",
      "obsExp:Double"
      ])

    """
    with open("data/cpgIslandExtFull.txt", 'r') as f:
      file_data = f.read()
      (res, a_1) = epidb.add_annotation("Cpg Islands", "hg19", "Complete CpG islands", file_data, cpg_island, None, self.admin_key)
      self.assertSuccess(res, a_1)
      res, q_cgi = epidb.select_annotations("Cpg Islands", "hg19", "chr1", None, None, self.admin_key)
      self.assertSuccess(res, q_cgi)
    """

    (s, q_tiling) = epidb.tiling_regions(10000000, "hg19", "chr1", self.admin_key)
    self.assertSuccess(res, q_tiling)

    expected = 'CHROMOSOME\tSTART\tEND\twgEncodeBroadHistoneH1hescH3k27me3StdPk.bed\twgEncodeBroadHistoneH1hescH3k4me3StdPk.bed\nchr1\t0\t10000000\t537.702\t550.688\nchr1\t10000000\t20000000\t605.834\t560.192\nchr1\t20000000\t30000000\t569.547\t569.46\nchr1\t30000000\t40000000\t511.311\t578.14\nchr1\t40000000\t50000000\t572.76\t567.424\nchr1\t50000000\t60000000\t549.296\t569.072\nchr1\t60000000\t70000000\t621.303\t568.744\nchr1\t70000000\t80000000\t530.706\t671.114\nchr1\t80000000\t90000000\t570.735\t597.719\nchr1\t90000000\t100000000\t655.342\t575.797\nchr1\t100000000\t110000000\t669.194\t667.227\nchr1\t110000000\t120000000\t600.521\t570.044\nchr1\t120000000\t130000000\t768.08\t621.833\nchr1\t130000000\t140000000\t\t\nchr1\t140000000\t150000000\t679.529\t601.084\nchr1\t150000000\t160000000\t575.793\t613.414\nchr1\t160000000\t170000000\t607.294\t603.071\nchr1\t170000000\t180000000\t478.058\t599.973\nchr1\t180000000\t190000000\t577.574\t583.5\nchr1\t190000000\t200000000\t573.467\t548.2\nchr1\t200000000\t210000000\t542.957\t544.742\nchr1\t210000000\t220000000\t633.094\t591.817\nchr1\t220000000\t230000000\t568.256\t588.62\nchr1\t230000000\t240000000\t577.107\t577.795\n'

    (s, req) = epidb.score_matrix({"wgEncodeBroadHistoneH1hescH3k27me3StdPk.bed":"SCORE","wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed":"SCORE"}, "mean",  q_tiling, self.admin_key)

    self.assertSuccess(s, req)
    rs = self.get_regions_request(req)

    self.assertEquals(rs, expected)