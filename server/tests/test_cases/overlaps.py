import helpers
from client import EpidbClient

class TestOverlaps(helpers.TestCase):

  def test_overlap_experiment_annotation(self):
    epidb = EpidbClient()
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
    epidb = EpidbClient()
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
