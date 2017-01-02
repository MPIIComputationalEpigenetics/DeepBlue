import helpers
import gzip

from deepblue_client import DeepBlueClient


class TestBinning(helpers.TestCase):

  def test_basic_binning(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    self.insert_experiment(epidb, "hg19_big_2", sample_id)

    res, qid = epidb.select_regions("hg19_big_2", "hg19", None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    status, req = epidb.binning(qid, "P_VALUE", 5, self.admin_key)
    self.assertSuccess(status, req)

    binning = self.get_regions_request(req)
    self.assertEquals(binning, {'binning': {'ranges': [2.0006, 37.8473, 73.6939, 109.5406, 145.3873, 181.234], 'counts': [63506, 2632, 3111, 6626, 1668]}})

    sample_id = self.sample_ids[0]
    self.insert_experiment(epidb, "hg19_big_1", sample_id)

    res, qid = epidb.select_regions("hg19_big_1", "hg19", None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    status, req = epidb.binning(qid, "P_VALUE", 5, self.admin_key)
    self.assertSuccess(status, req)

    binning = self.get_regions_request(req)
    self.assertEquals(binning, {'binning': {'ranges': [2.0017, 41.3178, 80.6338, 119.9499, 159.2659, 198.582], 'counts': [39349, 2824, 2510, 5325, 4299]}})


    status, req = epidb.binning(qid, "P_VALUE", 0, self.admin_key)
    self.assertFailure(status, req)

    status, req = epidb.binning(qid, "P_VALUE", -1, self.admin_key)
    self.assertFailure(status, req)

    status, req = epidb.binning(qid, "P_VALUE", 66666, self.admin_key)
    self.assertFailure(status, req)


    res, qid = epidb.select_regions("hg19_big_1", "hg19", None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    status, req = epidb.binning(qid, "QEERR", 5, self.admin_key)
    self.assertSuccess(status, req)

    msg = self.get_regions_request_error(req)
    status, msg = epidb.get_request_data(req, self.admin_key)
    self.assertEqual(msg, "hg19_big_1does not have the column 'QEERR'")


  def test_big_file(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    f = gzip.open("data/bedgraph/bigwig.bg.gz")
    data = f.read()
    (status, a1) = epidb.add_annotation("exp_wig", "hg19", "bla", data, "bedgraph", None, self.admin_key)

    (status, q1) = epidb.select_annotations("exp_wig", "hg19", None, None, None, self.admin_key)

    status, r1 = epidb.binning(q1, "VALUE", 5, self.admin_key)
    binning = self.get_regions_request(r1)
    self.assertEqual(binning, {'binning': {'ranges': [-1126.72, -726.6238, -326.5276, 73.5686, 473.6648, 873.761], 'counts': [8, 5, 3992582, 3489, 13]}})


    to_filter_low = binning["binning"]["ranges"][2]
    status, filtered = epidb.filter_regions (q1, "VALUE", ">", str(to_filter_low), "number", self.admin_key)

    to_filter_high = binning["binning"]["ranges"][4]
    status, filtered = epidb.filter_regions (q1, "VALUE", "<", str(to_filter_high), "number", self.admin_key)

    status, r_filtered = epidb.binning(filtered, "VALUE", 10, self.admin_key)
    binning = self.get_regions_request(r_filtered)
    self.assertEqual(binning,
      {'binning': {'counts': [4, 4, 1, 2, 2, 17, 1, 3932813, 772, 119], 'ranges': [-1126.72, -967.0013, -807.2826, -647.5638, -487.8452, -328.1265, -168.4077, -8.689, 151.0297, 310.7484, 470.4671] }})
#

