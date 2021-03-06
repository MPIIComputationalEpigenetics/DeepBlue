import time

import helpers

from deepblue_client import DeepBlueClient
import data_info
import gzip


class TestExperiments(helpers.TestCase):

  def test_load_bedgraph(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = gzip.open("data/bedgraph/chr19.txt.gz").read()

    # adding two experiments with the same data should work
    res = epidb.add_experiment("S0022IH2.ERX300681.H3K36me3.bwa.GRCh38.20150528.bedgraph", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, "bedgraph", {"md5sum": "afd4af5afd5afd4af5afd5afd4af5afd5"}, self.admin_key)
    self.assertSuccess(res)

    (status, query_id) = epidb.select_regions ("#afd4af5afd5afd4af5afd5afd4af5afd5", None, None, None, None, None, "chr19", 49388217, 49417994, self.admin_key)

    self.assertSuccess(status, query_id)

    (status, input) = epidb.input_regions("hg19", "chr19\t49388217\t49417994", self.admin_key)
    self.assertSuccess(status, input)

    (status, query_overlap) = epidb.intersection(query_id, input, self.admin_key)
    self.assertSuccess(status, query_overlap)

    (status, request_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END,VALUE", self.admin_key)
    self.assertSuccess(status, request_id)
    (status, overlap_request_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END,VALUE", self.admin_key)
    self.assertSuccess(status, overlap_request_id)

    by_select = self.get_regions_request(request_id)
    by_overlap = self.get_regions_request(overlap_request_id)

    self.assertEqual(by_overlap, by_select)
    self.assertTrue(len(by_select) > 0)

    (status, info) = epidb.info("#afd4af5afd5afd4af5afd5afd4af5afd5", self.admin_key)
    self.assertEquals(info[0]["_id"], "e1")

  def test_experiments_preview(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertSuccess(res)

    status, preview = epidb.preview_experiment('test_exp1', self.admin_key)
    self.assertEqual(preview, 'CHROMOSOME\tSTART\tEND\tNAME\tSCORE\tSTRAND\tSIGNAL_VALUE\tP_VALUE\tQ_VALUE\tPEAK\nchr1\t713240\t713390\t.\t0.0000\t+\t21.0000\t69.6000\t-1.0000\t-1\nchr1\t713520\t713670\t.\t0.0000\t-\t21.0000\t22.4866\t-1.0000\t-1\nchr1\t713900\t714050\t.\t0.0000\t+\t59.0000\t71.2352\t-1.0000\t-1\nchr1\t714160\t714310\t.\t0.0000\t+\t22.0000\t101.8740\t-1.0000\t-1\nchr1\t714540\t714690\t.\t0.0000\t+\t77.0000\t105.3120\t-1.0000\t-1')

  def test_experiments_pass(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
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

    res, experiments = epidb.list_experiments("hg19", "peaks", None, "NO_BIOSOURCE", None, None, None, self.admin_key)
    self.assertSuccess(res, experiments)
    self.assertEqual(len(experiments), 0)

    res, experiments = epidb.list_experiments(None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, experiments)
    self.assertEqual(len(experiments), 2)

    res, experiments = epidb.list_experiments("hg19", "peaks", None, "K562", None, None, None, self.admin_key)
    self.assertSuccess(res, experiments)
    self.assertEqual(len(experiments), 2)

    experiments_names = [x[1] for x in experiments]

    self.assertTrue("test_exp1" in experiments_names)
    self.assertTrue("test_exp2" in experiments_names)

    s, ids = epidb.name_to_id(['test_exp1'], 'experiments', self.admin_key)
    self.assertEqual(ids, [['e1', 'test_exp1']])
    s, ids = epidb.name_to_id(['test_exp1', 'test_exp2'], 'experiments', self.admin_key)
    self.assertEqual([['e1', 'test_exp1'], ['e2', 'test_exp2']], ids)
    s, ids = epidb.name_to_id('test_exp1', 'experiments', self.admin_key)
    self.assertEqual([['e1', 'test_exp1']], ids)

  def test_insert_local_file(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    sample_id = self.sample_ids[0]

    # adding two experiments with the same data should work
    (res, _id) = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res, _id)

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "inexistent_file.wig"}, self.admin_key)
    self.assertFailure(res)

    res, experiments = epidb.list_experiments("hg19", "signal", None, "K562", None, None, None, self.admin_key)
    self.assertSuccess(res, experiments)
    self.assertEqual(len(experiments), 1)

    res, qid1 = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)
    (s, req1) = epidb.get_regions(qid1, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(s, req1)
    data1 = self.get_regions_request(req1)

    res, qid1 = epidb.select_regions(_id, None, None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)
    (s, req2) = epidb.get_regions(qid1, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(s, req2)
    data2 = self.get_regions_request(req2)

    self.assertEqual(data1, data2)

  def test_double_experiment_same_user_fail(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    uid, user_key = self.insert_user(epidb, "test_user")
    s, tmp_user = epidb.modify_user_admin(uid, "permission_level", "INCLUDE_EXPERIMENTS", self.admin_key)
    self.assertSuccess(s)

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
    epidb = DeepBlueClient(address="localhost", port=31415)
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
    epidb = DeepBlueClient(address="localhost", port=31415)
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
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", "_s123", "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res)


  def test_add_with_invalid_genome(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "_hg123", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res)


  def test_add_with_invalid_project(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "No project", "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res)


  def test_add_with_invalid_chromosome(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg18_invalid_chr")
    _format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, _format, None, self.admin_key)
    self.assertFailure(res)

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, _format, {"__ignore_unknow_chromosomes__": True}, self.admin_key)
    self.assertSuccess(res)

    res, q_exp = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None,self.admin_key)
    res, req = epidb.get_regions(q_exp, _format, self.admin_key)

    data = self.get_regions_request(req)

    regions_data_okay = helpers.load_bed("hg18_invalid_chr_okay")

    self.assertEqual(data, regions_data_okay)

  def test_add_with_out_of_bound_region(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg18_out_of_bounds")
    _format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, _format, None, self.admin_key)
    self.assertFailure(res)

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, _format, {"__trim_to_chromosome_size__": True}, self.admin_key)
    self.assertSuccess(res)

    res, q_exp = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None,self.admin_key)
    res, req = epidb.get_regions(q_exp, _format, self.admin_key)

    data = self.get_regions_request(req)

    regions_data_okay = helpers.load_bed("hg18_out_of_bounds_okay")

    self.assertEqual(data, regions_data_okay)


  def test_add_with_invalid_epigenetic_mark(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res = epidb.add_experiment("test_exp1", "hg19", "No Epigenetic ", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res)

  def test_shitty_deep_file(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    eid1 = self.insert_experiment(epidb, "deepshitty")
    res, qid1 = epidb.select_regions("deepshitty", "hg18", None, None, None,
        None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    res, req = epidb.get_experiments_by_query(qid1, self.admin_key)
    exps = self.get_regions_request(req)
    self.assertEqual(len(exps), 1)
    (res, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END", self.admin_key)
    exps = self.get_regions_request(req)
    self.assertEqual(exps, "chr1\t62125\t62154")

  def test_get_by_query(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
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
    self.assertEqual(exps, {eid1: "hg18_chr1_1"})

    (res, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END", self.admin_key)
    exps = self.get_regions_request(req)

    self.assertEqual(exps, 'chr1\t683240\t690390\nchr1\t697520\t697670\nchr1\t702900\t703050\nchr1\t714160\t714310\nchr1\t714540\t714690\nchr1\t715060\t715210\nchr1\t761180\t761330\nchr1\t762060\t762210\nchr1\t762420\t762570\nchr1\t762820\t762970\nchr1\t763020\t763170\nchr1\t839540\t839690\nchr1\t840080\t840230\nchr1\t840600\t840750\nchr1\t858880\t859030\nchr1\t859600\t859750\nchr1\t860240\t860390\nchr1\t861040\t861190\nchr1\t875400\t875550\nchr1\t875900\t876050\nchr1\t876400\t876650\nchr1\t877900\t878050\nchr1\t879180\t880330')


    (res, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END,@STRAND", self.admin_key)
    exps = self.get_regions_request(req)
    self.assertEqual(exps, 'chr1\t683240\t690390\t+\nchr1\t697520\t697670\t-\nchr1\t702900\t703050\t+\nchr1\t714160\t714310\t+\nchr1\t714540\t714690\t+\nchr1\t715060\t715210\t+\nchr1\t761180\t761330\t-\nchr1\t762060\t762210\t+\nchr1\t762420\t762570\t.\nchr1\t762820\t762970\t-\nchr1\t763020\t763170\t-\nchr1\t839540\t839690\t+\nchr1\t840080\t840230\t+\nchr1\t840600\t840750\t-\nchr1\t858880\t859030\t.\nchr1\t859600\t859750\t.\nchr1\t860240\t860390\t+\nchr1\t861040\t861190\t-\nchr1\t875400\t875550\t+\nchr1\t875900\t876050\t-\nchr1\t876400\t876650\t+\nchr1\t877900\t878050\t-\nchr1\t879180\t880330\t-')


    res, qid2 = epidb.select_regions(None, "hg19", "Methylation", None, None,
        None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    res, req = epidb.get_experiments_by_query(qid2, self.admin_key)
    self.assertSuccess(res, req)
    exps = self.get_regions_request(req)

    self.assertSuccess(res, exps)
    self.assertEqual(len(exps), 3)
    self.assertEqual({'e4': 'hg19_chr1_3', 'e3': 'hg19_chr1_2', 'e2': 'hg19_chr1_1'}, exps)
    self.assertTrue(eid2 in exps)
    self.assertTrue(eid3 in exps)
    self.assertTrue(eid4 in exps)

    (res, req) = epidb.get_regions(qid2, "CHROMOSOME,START,END", self.admin_key)
    x = self.get_regions_request(req)
    self.assertEqual(x, 'chr1\t567500\t567650\nchr1\t713000\t713070\nchr1\t713240\t713390\nchr1\t713280\t713430\nchr1\t713520\t713670\nchr1\t713520\t713670\nchr1\t713900\t714050\nchr1\t713920\t714070\nchr1\t714160\t714310\nchr1\t714200\t714350\nchr1\t714300\t714350\nchr1\t714460\t714590\nchr1\t714540\t714690\nchr1\t714540\t714690\nchr1\t715060\t715210\nchr1\t715080\t715230\nchr1\t719100\t719330\nchr1\t761180\t761330\nchr1\t762060\t762210\nchr1\t762060\t762210\nchr1\t762100\t762250\nchr1\t762420\t762570\nchr1\t762440\t762590\nchr1\t762460\t762500\nchr1\t762620\t762790\nchr1\t762820\t762970\nchr1\t762820\t762970\nchr1\t763020\t763170\nchr1\t839540\t839690\nchr1\t840000\t840150\nchr1\t840080\t840230\nchr1\t840100\t840310\nchr1\t840600\t840750\nchr1\t840640\t840790\nchr1\t840850\t840990\nchr1\t857740\t857890\nchr1\t858740\t858890\nchr1\t858880\t859030\nchr1\t859600\t859750\nchr1\t859640\t859790\nchr1\t859650\t859790\nchr1\t860220\t860370\nchr1\t860240\t860390\nchr1\t860600\t860750\nchr1\t861040\t861190\nchr1\t861040\t861190\nchr1\t875220\t875370\nchr1\t875400\t875550\nchr1\t875900\t876050\nchr1\t875920\t876070\nchr1\t876180\t876330\nchr1\t876420\t876570\nchr1\t877000\t877150')