import helpers

from client import EpidbClient


class TestSelectRegions(helpers.TestCase):

  def test_select_full_experiment(self, format=None):
    epidb = EpidbClient()
    self.init_base()

    sample_id = self.sample_ids[0]

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
    full_experiment_regions = helpers.get_result("hg19_chr1_1_output")
    region_count = len(full_experiment_regions.split("\n"))

    format = "CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK"

    # test to retrieve the whole data with all parameters of the experiment
    # set or not set
    argument_combinations = [
      ("hg19_chr1_1", "hg19", "Methylation", sample_id, "tech1", "ENCODE", "chr1", 713240, 876330),
      ("hg19_chr1_1", "hg19", "Methylation", sample_id, "tech1", "ENCODE", "chr1", 713240, None),
      ("hg19_chr1_1", "hg19", "Methylation", sample_id, "tech1", "ENCODE", "chr1", None, None),
      ("hg19_chr1_1", "hg19", "Methylation", sample_id, "tech1", "ENCODE", None, None, None),
      ("hg19_chr1_1", "hg19", "Methylation", sample_id, "tech1", None, None, None, None),
      ("hg19_chr1_1", "hg19", "Methylation", sample_id, None, None, None, None, None),
      ("hg19_chr1_1", "hg19", "Methylation", None, None, None, None, None, None),
      ("hg19_chr1_1", "hg19", None, None, None, None, None, None, None)
    ]

    for args in argument_combinations:
      args = args + (self.admin_key,)

      res, qid = epidb.select_regions(*args)
      self.assertSuccess(res, qid)

      res, req = epidb.count_regions(qid, self.admin_key)
      self.assertSuccess(res, req)
      count = self.count_request(req)
      self.assertEqual(count, region_count)

      res, req = epidb.get_regions(qid, format, self.admin_key)
      self.assertSuccess(res, req)
      regions = self.get_regions_request(req)
      self.assertEqual(regions, full_experiment_regions)

  def test_minimum_parameters(self):
    # select_regions needs at least one of
    # experiment name, epigenetic_mark, sample, technique or project
    epidb = EpidbClient()
    self.init_base()

    sample_id = self.sample_ids[0]

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)

    # none of them: should fail
    res = epidb.select_regions(None, "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertFailure(res)

    # at least one: should pass
    expected_regions = helpers.get_result("hg19_chr1_1_output")
    format = "CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK"

    argument_combinations = [
      (None, "hg19", None, sample_id, None, None, None, None, None),
      ("hg19_chr1_1", "hg19", None, None, None, None, None, None, None),
      (None, "hg19", None, None, None, "ENCODE", None, None, None),
      (None, "hg19", None, None, "tech1", None, None, None, None),
      (None, "hg19", "Methylation", None, None, None, None, None, None)
    ]

    for args in argument_combinations:
      args = args + (self.admin_key,)

      res, qid = epidb.select_regions(*args)
      self.assertSuccess(res, qid)

      res, req = epidb.get_regions(qid, format, self.admin_key)
      self.assertSuccess(res, req)
      regions = self.get_regions_request(req)
      self.assertEqual(regions, expected_regions)

  def test_retrieve_with_defaults(self):
    epidb = EpidbClient()
    self.init_base()

    self.insert_experiment(epidb, "hg19_chr1_1")
    expected_regions = helpers.get_result("hg19_chr1_1_output")

    format = "CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK"

    res, qid = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, req = epidb.get_regions(qid, format, self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, expected_regions)

  def test_experiment_name_metacolumn(self):
    epidb = EpidbClient()
    self.init_base()

    self.insert_experiment(epidb, "hg19_chr1_1")

    expected_regions = helpers.get_result("experiment_name_metacolumn")

    format = "CHROMOSOME,START,END,@NAME"

    res, qid = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, req = epidb.get_regions(qid, format, self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, expected_regions)

  def test_experiment_name_metacolumn2(self):
    epidb = EpidbClient()
    self.init_base()

    self.insert_experiment(epidb, "hg19_chr1_1")
    self.insert_experiment(epidb, "hg19_chr1_2")

    res, qid1 = epidb.select_regions("hg19_chr1_2", "hg19", None, None, None, None, None, 750000, 770000, self.admin_key)
    self.assertSuccess(res, qid1)
    res, qid2 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None, None, None, 750000, 770000, self.admin_key)
    self.assertSuccess(res, qid2)

    res, qid3 = epidb.merge_queries(qid1, qid2, self.admin_key)
    self.assertSuccess(res, qid3)

    expected = helpers.get_result("experiment_name_multiple_experiments")
    res, req = epidb.get_regions(qid3, "CHROMOSOME,START,END,@NAME", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    l_regions = regions.split("\n")
    l_expected = expected.split("\n")
    self.assertEqual(set(l_regions), set(l_expected))

  def test_chromosome_explicit(self):
    # regression test: chromosome was put in the first column no matter
    # what the format string specified

    epidb = EpidbClient()
    self.init_base()

    self.insert_experiment(epidb, "hg19_chr1_1")

    (s, m) = epidb.create_column_type_simple("foobar", "", "string", self.admin_key)
    self.assertSuccess(s, m)

    full_experiment_regions = helpers.get_result("full_experiment_regions")

    res, qid = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                    None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    fmt = "foobar,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK"
    res, req = epidb.get_regions(qid, fmt, self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertNotEqual(regions, full_experiment_regions)

    fmt = "START,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK"
    res, req = epidb.get_regions(qid, fmt, self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertNotEqual(regions, full_experiment_regions)

    # leave out chromosome entirely
    regions_wo_chr = helpers.get_result("full_experiment_regions_wo_chr")

    # Creating another column, named NAME, that in the experiment is name.
    # This column should return empty.
    (s, m) = epidb.create_column_type_simple("COISA", "", "string", self.admin_key)
    self.assertSuccess(s, m)

    fmt = "START,END,COISA,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK"
    res, req = epidb.get_regions(qid, fmt, self.admin_key)
    regions = self.get_regions_request(req)

    self.assertSuccess(res, regions)
    self.assertEqual(regions, regions_wo_chr)

  def test_malformed_format(self):
    epidb = EpidbClient()
    self.init_base()

    self.insert_experiment(epidb, "hg19_chr1_1")

    # test various bad format strings that should fail
    bad_formats = [
      ",",
      "chr,start,,",
      "chr,start:0:,end",
      "chr,start:0:foo,end",
      "CHROMOSOME,START,,",
      "CHROMOSOME,START:0:,end",
      "CHROMOSOME,START:0:foo,end",
    ]

    res, qid = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None,
                                    None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    for fmt in bad_formats:
      res, req = epidb.get_regions(qid, fmt, self.admin_key)
      regions = self.get_regions_request_error(req)

  def test_select_all(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, msg = epidb.select_regions(None, None, None, None, None,
                               None, None, None, None, self.admin_key)

    self.assertFailure(res, msg)
    self.assertTrue("at least one of the following fields must be provided: 'experiment_name', 'epigenetic_mark', 'sample_id', 'project', 'technique'." in msg.lower())

  def test_unknown_parameters(self):
    epidb = EpidbClient()
    self.init_base()

    sample_id = self.sample_ids[0]

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)

    argument_combinations = [
      (None, "hg19", "_invalid_mark", sample_id, "tech1", "ENCODE", None, None, None, self.admin_key),
      (None, "hg19", "Methylation", "_invalid_sid", "tech1", "ENCODE", None, None, None, self.admin_key),
      (None, "hg19", "Methylation", sample_id, "_invalid_tech", "ENCODE", None, None, None, self.admin_key),
      (None, "hg19", "Methylation", sample_id, "tech1", "_invalid_project", None, None, None, self.admin_key)
    ]

    res, msg = epidb.select_regions(*argument_combinations[0])
    self.assertFailure(res, msg)
    self.assertEqual(msg, "Epigenetic mark _invalid_mark does not exists.")

    res, msg = epidb.select_regions(*argument_combinations[1])
    self.assertFailure(res, msg)
    self.assertEqual(msg, 'Sample ID _invalid_sid does not exists.')

    res, msg = epidb.select_regions(*argument_combinations[2])
    self.assertFailure(res, msg)
    self.assertEqual(msg, 'Technique _invalid_tech does not exists.')

    res, msg = epidb.select_regions(*argument_combinations[3])
    self.assertFailure(res, msg)
    self.assertEqual(msg, 'Project _invalid_project does not exists.')


  def test_argument_normalization(self):
    epidb = EpidbClient()
    self.init_base()

    sample_id = self.sample_ids[0]

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
    full_experiment_regions = helpers.get_result("full_experiment_regions")

    format = "CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK"

    res, qid = epidb.select_regions("HG19_CHR1_1", "hg19", "methylation", sample_id, " Tech1",
                                    "ENCode ", "chr1", 713240, 876330, self.admin_key)
    self.assertSuccess(res, qid)

    res, req = epidb.get_regions(qid, format, self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, full_experiment_regions)

  def test_select_range(self):
    epidb = EpidbClient()
    self.init_base()

    sample_id = self.sample_ids[0]

    format = "CHROMOSOME,START,END"

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
    range_regions = helpers.get_result("range_760k_875k")

    res, qid = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None, None, None,
                                    760000, 875000, self.admin_key)
    self.assertSuccess(res, qid)

    res, req = epidb.get_regions(qid, format, self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, range_regions)

  def test_multiple_experiments(self):
    epidb = EpidbClient()
    self.init_base()

    sample_id = self.sample_ids[0]

    format = "CHROMOSOME,START,END"

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
    self.insert_experiment(epidb, "hg19_chr1_2", sample_id)
    multiple_experiments_regions = helpers.get_result("multiple_experiments")

    res, qid = epidb.select_regions(["hg19_chr1_1", "hg19_chr1_2"], "hg19", None, None, None, None, None,
                                    None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, req = epidb.get_regions(qid, format, self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    self.assertEqual(regions, multiple_experiments_regions)

  def test_multiple_genomes(self):
    epidb = EpidbClient()
    self.init_base()

    sample_id = self.sample_ids[0]

    format = "CHROMOSOME,START,END"

    self.insert_experiment(epidb, "hg18_chr1_1", sample_id)
    self.insert_experiment(epidb, "hg19_chr1_2", sample_id)

    multiple_genomes_regions = helpers.get_result("multiple_genomes")

    # there was a bug in internal merge procedure triggered by the order of genomes
    for genomes in [["hg19", "hg18"], ["hg18", "hg19"]]:

      res, qid = epidb.select_regions(["hg18_chr1_1", "hg19_chr1_2"], genomes, None,
                                      None, None, None, None, None, None, self.admin_key)
      self.assertSuccess(res, qid)

      res, req = epidb.get_regions(qid, format, self.admin_key)
      self.assertSuccess(res, req)
      regions = self.get_regions_request(req)

      self.assertEqual(regions, multiple_genomes_regions)

  def test_multiple_genomes_2(self):
    epidb = EpidbClient()
    self.init_base()

    sample_id = self.sample_ids[0]

    format = "CHROMOSOME,START,END"

    self.insert_experiment(epidb, "hg18_chr1_1", sample_id)
    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
    self.insert_experiment(epidb, "hg19_chr1_2", sample_id)

    # retrieve every paired combination
    combinations = [
      (["hg18_chr1_1", "hg19_chr1_2"], "multiple_genomes"),
      (["hg18_chr1_1", "hg19_chr1_1"], "multiple_genomes_2"),
      (["hg19_chr1_1", "hg19_chr1_2"], "multiple_experiments")
    ]

    for (experiments, result_regions) in combinations:
      res, qid = epidb.select_regions(experiments, ["hg18", "hg19"], None, None, None, None, None,
                                      None, None, self.admin_key)
      self.assertSuccess(res, qid)
      res, req = epidb.get_regions(qid, format, self.admin_key)
      self.assertSuccess(res, req)
      regions = self.get_regions_request(req)

      regions_expected = helpers.get_result(result_regions)
      self.assertEqual(regions, regions_expected)
