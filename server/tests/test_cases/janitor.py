import helpers
import time


from deepblue_client import DeepBlueClient

class TestJanitor(helpers.TestCase):\

  def test_janitor(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base()

    time.sleep(2)

    res, qid = epidb.tiling_regions(1000000, "hg19", ["chr15", "chrX", "chr3"], self.admin_key)
    self.assertSuccess(res, qid)

    res, req = epidb.get_regions(qid, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    chr3_tiles = 198022430 / 1000000
    chr15_tiles = 102531392 / 1000000
    chrX_tiles = 155270560 / 1000000

    self.assertEqual(len(regions.strip().split('\n')), chr3_tiles + chr15_tiles + chrX_tiles)

    print epidb.modify_user_admin(None, "old_request_age_in_sec", "1", self.admin_key)

    time.sleep(1)

    print epidb.info(req, self.admin_key)

    s, m = epidb.get_request_data(req, self.admin_key)
    self.assertFailure(s, m)
    self.assertEqual(m, 'Request ID r1 was cleared. We are going to reprocess this request. Please, check its status.')

