import helpers

from deepblue_client import DeepBlueClient

class TestXML(helpers.TestCase):

  def test_types(self):
    epidb = DeepBlueClient(address="localhost", port=31415)

    in_vals = ("foo", 1, 123.2234, False, [1, 2, "test"], {"foo": "bar", "baz": 1})
    res, out_vals =  epidb.test_types(*in_vals)
    self.assertSuccess(res, out_vals)
    self.assertEqual(list(in_vals), out_vals)


  def test_int_for_string(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    genome_info = None
    with open("data/genomes/hg19", 'r') as f:
      genome_info = f.read().replace(",", "")

    res, msg = epidb.add_genome("hg19", 1, genome_info, self.admin_key)
    self.assertFailure(res, msg)
    self.assertTrue("[1:description]" in msg)