import helpers
from deepblue_client import DeepBlueClient

class TestOverlaps(helpers.TestCase):

  def test_overlap_simple(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    # Test the intersect command
    data_one = "chr1\t3049996\t3050022\nchr1\t3050022\t3050040\nchr1\t3050040\t3050051\nchr1\t3050051\t3050126"
    region = "chr1\t3050022\t3050100"

    (s, q1) = epidb.input_regions("hg19", data_one, self.admin_key)
    self.assertSuccess(s, q1)

    (s, q_input) = epidb.input_regions("hg19", region, self.admin_key)
    self.assertSuccess(s, q_input)

    (s, q3) = epidb.intersection(q1, q_input, self.admin_key)
    self.assertSuccess(s, q3)

    (s, req) = epidb.get_regions(q3, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(s, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, 'chr1\t3050022\t3050040\nchr1\t3050040\t3050051\nchr1\t3050051\t3050126')

    # Test the select annotation
    sample_id = self.sample_ids[0]
    res = epidb.add_annotation("exp1", "hg19", "desc1", data_one, "CHROMOSOME,START,END", None, self.admin_key)
    self.assertSuccess(res)
    (s, q1) = epidb.select_annotations("exp1", "hg19", "chr1", 3050022, 3050100, self.admin_key)
    self.assertSuccess(s, q1)
    (s, req) = epidb.get_regions(q1, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(s, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, 'chr1\t3050022\t3050040\nchr1\t3050040\t3050051\nchr1\t3050051\t3050126')

    # Test the select experiment
    sample_id = self.sample_ids[0]
    res = epidb.add_experiment("exp1", "hg19", "Methylation", sample_id, "tech1",
                                 "ENCODE", "desc1", data_one, "CHROMOSOME,START,END", None, self.admin_key)
    self.assertSuccess(res)

    (s, q1) = epidb.select_experiments("exp1", "chr1", 3050022, 3050100, self.admin_key)
    self.assertSuccess(s, q1)
    (s, req) = epidb.get_regions(q1, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(s, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, 'chr1\t3050022\t3050040\nchr1\t3050040\t3050051\nchr1\t3050051\t3050126')

    (s, q1) = epidb.select_experiments("exp1", "chr1", None, None, self.admin_key)
    self.assertSuccess(s, q1)
    (s, q2) = epidb.aggregate(q1, q_input, "START", self.admin_key)
    self.assertSuccess(s, q2)
    (s, req) = epidb.get_regions(q2, "@AGG.MIN,@AGG.MAX,@AGG.COUNT", self.admin_key)
    self.assertSuccess(s, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, '3050022.0000\t3050051.0000\t3')

  def test_overlap_experiment_annotation(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
    self.insert_annotation(epidb, "Cpg Islands")

    # TODO: Is this test actually check anything?

    res, qid_1 = epidb.select_regions("hg19_chr1_1", "hg19", "Methylation", sample_id, "tech1",
                                 "ENCODE", "chr1", 760000, 860000, self.admin_key)
    self.assertSuccess(res, qid_1)
    res, req = epidb.count_regions(qid_1, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)

    res, qid_2 = epidb.select_annotations("Cpg Islands", "hg19", "chr1", None, None, self.admin_key)
    self.assertSuccess(res, qid_2)
    res, req = epidb.count_regions(qid_2, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)

    res, qid_3 = epidb.merge_queries(qid_1, qid_2, self.admin_key)
    self.assertSuccess(res, qid_3)
    res, req = epidb.count_regions(qid_3, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)


  def test_multiple_overlap(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
    self.insert_annotation(epidb, "Cpg Islands")

    res, qid_1 = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1)
    res, req = epidb.count_regions(qid_1, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEqual(count, 14)

    res, qid_2 = epidb.select_regions("hg19_chr1_1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertEqual(res, "okay")
    res, req = epidb.count_regions(qid_2, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEqual(count, 21)

    res, i_id = epidb.intersection(qid_1, qid_2, self.admin_key)
    self.assertEqual(res, "okay")
    res, req = epidb.count_regions(i_id, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEqual(count, 3)

    res, i_id = epidb.intersection(qid_2, qid_2, self.admin_key)
    self.assertSuccess(res, i_id)
    res, req = epidb.count_regions(i_id, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEqual(count, 21)

    res, i_id = epidb.intersection(qid_1, qid_1, self.admin_key)
    self.assertSuccess(res, i_id)
    res, req = epidb.count_regions(i_id, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEqual(count, 14)

    res, i_id = epidb.intersection(i_id, i_id, self.admin_key)
    self.assertSuccess(res, i_id)
    res, req = epidb.count_regions(i_id, self.admin_key)
    self.assertSuccess(res, req)
    count = self.count_request(req)
    self.assertEqual(count, 14)
