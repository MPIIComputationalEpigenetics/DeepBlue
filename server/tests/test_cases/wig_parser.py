import time

import helpers

from client import EpidbClient

# XXX: check the exception in the fail_files with a predefined content
class TestWigFiles(helpers.TestCase):

  def test_wig_files(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    files = ["scores1", "scores2", "scores3", "scores4", "scores5",
             "scores6", "scores7", "yeast_pol2", "yeast_rap1"]

    for filename in files:
      wig_data = helpers.load_wig(filename)
      res = epidb.add_experiment(filename, "hg19", "Methylation", sample_id, "tech1",
                                 "ENCODE", "desc1", wig_data, "wig", None, self.admin_key)
      self.assertSuccess(res)


    (s, r) = epidb.select_regions(files, "hg19", None, None, None, None, None, None, None, self.admin_key)
    (s, rs) = epidb.get_regions(r, "CHROMOSOME, START, END, VALUE", self.admin_key)

    (s, req) = epidb.count_regions(r, self.admin_key)
    count = self.count_request(req)
    self.assertEqual(5667, count)

  def test_wig_files_pass(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    pass_files = ["chr_unsorted", "empty_lines", "fix_span", "line_breaks",
                  "null_scores", "tab_seperator", "var_simple", "comments",
                  "fix_simple", "inverted_header", "long_header", "negative_scores",
                  "strange_chromosome", "var_span"]

    for filename in pass_files:
      wig_data = helpers.load_wig("should_pass/%s" % filename)
      res = epidb.add_experiment(filename, "hg19", "Methylation", sample_id, "tech1",
                                 "ENCODE", "desc1", wig_data, "wig", None, self.admin_key)
      self.assertSuccess(res)

  def test_wig_files_fail(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    fail_files = ["overlap", "absent_chr", "bad_value", "fix_overshoot",
                  "missing_chr", "missing_start", "no_directive",
                  "null_step", "var_null_replace", "var_overwrite", "bad_fields",
                  "empty", "fix_overwrite", "missing_fields", "mix_overwrite",
                  "null_span", "rubbish", "var_overshoot", "multitrack", "ucsc_definition"]

    for filename in fail_files:
      wig_data = helpers.load_wig("should_fail/%s" % filename)
      res = epidb.add_experiment(filename, "hg19", "Methylation", sample_id, "tech1",
                                 "ENCODE", "desc1", wig_data, "wig", None, self.admin_key)
      self.assertFailure(res)

  """
  def test_include_big_file(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    wig_data = open("/Users/albrecht/Downloads/G199.CPG_methylation_sd.bs_call.20140106.wig").read()

    res = epidb.add_experiment("G199.CPG_methylation_sd.bs_call.20140106.wig", "hg19", "Methylation", sample_id, "tech1", "ENCODE", "desc1", wig_data, "wig", None, self.admin_key)

    self.assertSucess(res)
  """