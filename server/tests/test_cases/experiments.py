import time

import helpers

from client import EpidbClient
import data_info


class TestExperiments(helpers.TestCase):

  def test_experiments_pass(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_experiment("test_exp2", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertSuccess(res)

    res, experiments = epidb.list_experiments("hg19", None, None, None, None, self.admin_key)
    self.assertSuccess(res, experiments)
    self.assertEqual(len(experiments), 2)

    experiments_names = [x[1] for x in experiments]

    self.assertTrue("test_exp1" in experiments_names)
    self.assertTrue("test_exp2" in experiments_names)


  def test_insert_local_file(self):
    epidb = EpidbClient()
    self.init_base(epidb)
    sample_id = self.sample_ids[0]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "inexistent_file.wig"}, self.admin_key)
    self.assertFailure(res)

    res, experiments = epidb.list_experiments("hg19", None, None, None, None, self.admin_key)
    self.assertSuccess(res, experiments)
    self.assertEqual(len(experiments), 1)

    res, qid1 = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)
    (s, regions_1) = epidb.get_regions(qid1, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(s, regions_1)

  def test_double_experiment_same_user_fail(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    uid, user_key = self.insert_user(epidb, "test_user")

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    # adding the same experiments with different users should work
    exp = ("test_exp1", "hg19", "Methylation", sample_id, "tech1",
           "ENCODE", "desc1", regions_data, format, None)

    res = epidb.add_experiment(*(exp + (self.admin_key,)))
    self.assertSuccess(res)

    res = epidb.add_experiment(*(exp + (user_key,)))
    self.assertFailure(res)
    self.assertEqual(res[1], "102001:The experiment name 'test_exp1' is already being used.")


  def test_double_experiment_fail(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]


    # adding the same experiment with the same user should fail
    exp = ("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None)

    res = epidb.add_experiment(*(exp + (self.admin_key,)))
    self.assertSuccess(res)

    res = epidb.add_experiment(*(exp + (self.admin_key,)))
    self.assertFailure(res)

  def test_list_recent(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]


    res, eid1 = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertSuccess(res, eid1)

    # get some distance in insertion times
    time.sleep(5)

    res, eid2 = epidb.add_experiment("test_exp2", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertEqual(res, "okay")

    time.sleep(3)
    ago = 1.0 / 24 / 60 / 60 * 5

    res, experiments = epidb.list_recent_experiments(ago, "hg19", None, None, None, None, self.admin_key)
    self.assertSuccess(res, experiments)

    experiments_names = [x[1] for x in experiments]

    self.assertTrue("test_exp1" not in experiments_names)
    self.assertTrue("test_exp2" in experiments_names)

    res, experiments = epidb.list_recent_experiments(1.0, "hg19", None, None, None, None, self.admin_key)
    self.assertSuccess(res, experiments)

    experiments_names = [x[1] for x in experiments]
    self.assertTrue("test_exp1" in experiments_names)
    self.assertTrue("test_exp2" in experiments_names)

  def test_add_with_invalid_sample(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", "_s123", "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res)


  def test_add_with_invalid_genome(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "_hg123", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res)


  def test_add_with_invalid_project(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "No project", "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res)


  def test_add_with_invalid_epigenetic_mark(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "hg19", "No Epigenetic ", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res)


  def test_get_by_query(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    eid1 = self.insert_experiment(epidb, "hg18_chr1_1")
    eid2 = self.insert_experiment(epidb, "hg19_chr1_1")
    eid3 = self.insert_experiment(epidb, "hg19_chr1_2")
    eid4 = self.insert_experiment(epidb, "hg19_chr1_3")

    res, qid1 = epidb.select_regions("hg18_chr1_1", "hg18", None, None, None,
        None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    res, req = epidb.get_experiments_by_query(qid1, self.admin_key)
    exps = self.get_regions_request(req)
    self.assertEqual(len(exps), 1)
    self.assertEqual(exps[0], [eid1, "hg18_chr1_1"])

    (res, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END", self.admin_key)
    exps = self.get_regions_request(req)

    self.assertEqual(exps, 'chr1\t683240\t690390\nchr1\t697520\t697670\nchr1\t702900\t703050\nchr1\t714160\t714310\nchr1\t714540\t714690\nchr1\t715060\t715210\nchr1\t761180\t761330\nchr1\t762060\t762210\nchr1\t762420\t762570\nchr1\t762820\t762970\nchr1\t763020\t763170\nchr1\t839540\t839690\nchr1\t840080\t840230\nchr1\t840600\t840750\nchr1\t858880\t859030\nchr1\t859600\t859750\nchr1\t860240\t860390\nchr1\t861040\t861190\nchr1\t875400\t875550\nchr1\t875900\t876050\nchr1\t876400\t876650\nchr1\t877900\t878050\nchr1\t879180\t880330')

    res, qid2 = epidb.select_regions(None, "hg19", "Methylation", None, None,
        None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    res, req = epidb.get_experiments_by_query(qid2, self.admin_key)
    self.assertSuccess(res, req)
    exps = self.get_regions_request(req)

    self.assertSuccess(res, exps)
    self.assertEqual(len(exps), 3)
    self.assertTrue([eid2, "hg19_chr1_1"] in exps)
    self.assertTrue([eid3, "hg19_chr1_2"] in exps)
    self.assertTrue([eid4, "hg19_chr1_3"] in exps)

    (res, req) = epidb.get_regions(qid2, "CHROMOSOME,START,END", self.admin_key)
    x = self.get_regions_request(req)
    self.assertEqual(x, 'chr1\t567500\t567650\nchr1\t713000\t713070\nchr1\t713240\t713390\nchr1\t713280\t713430\nchr1\t713520\t713670\nchr1\t713520\t713670\nchr1\t713900\t714050\nchr1\t713920\t714070\nchr1\t714160\t714310\nchr1\t714200\t714350\nchr1\t714300\t714350\nchr1\t714460\t714590\nchr1\t714540\t714690\nchr1\t714540\t714690\nchr1\t715060\t715210\nchr1\t715080\t715230\nchr1\t719100\t719330\nchr1\t761180\t761330\nchr1\t762060\t762210\nchr1\t762060\t762210\nchr1\t762100\t762250\nchr1\t762420\t762570\nchr1\t762440\t762590\nchr1\t762460\t762500\nchr1\t762620\t762790\nchr1\t762820\t762970\nchr1\t762820\t762970\nchr1\t763020\t763170\nchr1\t839540\t839690\nchr1\t840000\t840150\nchr1\t840080\t840230\nchr1\t840100\t840310\nchr1\t840600\t840750\nchr1\t840640\t840790\nchr1\t840850\t840990\nchr1\t857740\t857890\nchr1\t858740\t858890\nchr1\t858880\t859030\nchr1\t859600\t859750\nchr1\t859640\t859790\nchr1\t859650\t859790\nchr1\t860220\t860370\nchr1\t860240\t860390\nchr1\t860600\t860750\nchr1\t861040\t861190\nchr1\t861040\t861190\nchr1\t875220\t875370\nchr1\t875400\t875550\nchr1\t875900\t876050\nchr1\t875920\t876070\nchr1\t876180\t876330\nchr1\t876420\t876570\nchr1\t877000\t877150')
