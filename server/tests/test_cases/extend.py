import helpers

from deepblue_client import DeepBlueClient


class TestExtend(helpers.TestCase):

  def test_extend(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    (s, query_id) = epidb.tiling_regions(100000000, "hg18", "chr1", self.admin_key)
    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEqual(regions, "chr1\t0\t100000000\nchr1\t100000000\t200000000")

    (s, f_id) = epidb.extend(query_id, 10000000, "FORWARD", False, self.admin_key)
    (s, r_id) = epidb.get_regions(f_id, "CHROMOSOME,START,END", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEqual(regions, "chr1\t0\t110000000\nchr1\t100000000\t210000000")

    (s, f_id) = epidb.extend(query_id, 10000000, "BACKWARD", False, self.admin_key)
    (s, r_id) = epidb.get_regions(f_id, "CHROMOSOME,START,END", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEqual(regions, "chr1\t0\t100000000\nchr1\t90000000\t200000000")

    (s, f_id) = epidb.extend(query_id, 10000000, "BOTH", False, self.admin_key)
    (s, r_id) = epidb.get_regions(f_id, "CHROMOSOME,START,END", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEqual(regions, "chr1\t0\t110000000\nchr1\t90000000\t210000000")
