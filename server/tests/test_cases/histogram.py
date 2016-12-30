import helpers

from deepblue_client import DeepBlueClient


class TestHistogram(helpers.TestCase):

  def test_basic_histogram(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    """
    sample_id = self.sample_ids[0]
    self.insert_experiment(epidb, "hg19_big_2", sample_id)

    res, qid = epidb.select_regions("hg19_big_2", "hg19", None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    req, qid = epidb.histogram(qid, "P_VALUE", 5, self.admin_key)
    print req, qid

    #regions = self.get_regions_request(req)
    #print regions

    print
    print "2"

    sample_id = self.sample_ids[0]
    self.insert_experiment(epidb, "hg19_big_1", sample_id)

    res, qid = epidb.select_regions("hg19_big_1", "hg19", None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    req, qid = epidb.histogram(qid, "P_VALUE", 5, self.admin_key)
    print req, qid

    #regions = self.get_regions_request(req)
    #print regions

    print
    print "3"

    res, qid = epidb.select_regions("hg19_big_1", "hg19", None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    req, qid = epidb.histogram(qid, "QEERR", 5, self.admin_key)
    print req, qid
    """

    import gzip
    f = gzip.open("data/bedgraph/bigwig.bg.gz")
    data = f.read()
    (status, a1) = epidb.add_annotation("exp_wig", "hg19", "bla", data, "bedgraph", None, self.admin_key)
    print status, a1

    (status, q1) = epidb.select_annotations("exp_wig", "hg19", None, None, None, self.admin_key)
    print status, q1

    #status, r1  = epidb.histogram(q1, "P_VALUE", 5, self.admin_key)
    #print status, r1

    status, r1 = epidb.histogram(q1, "VALUE", 5, self.admin_key)
    print status, r1

