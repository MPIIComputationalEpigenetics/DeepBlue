import helpers

from deepblue_client import DeepBlueClient

class TestChanges(helpers.TestCase):

  def test_change_extra_metadata(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = "chr1\t1\t100"
    format = ""

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, {"NAME":"FELIPE", "LAST_NAME": "ALBRECHT"}, self.admin_key)
    self.assertSuccess(res)
    _id = res[1]

    res = epidb.change_extra_metadata(_id, "NAME", "JOSE", self.admin_key)
    self.assertSuccess(res)
    res = epidb.change_extra_metadata(_id, "LAST_NAME", "FERNANDES", self.admin_key)
    self.assertSuccess(res)

    status, info = epidb.info(_id, self.admin_key)
    self.assertSuccess(status, info)
    self.assertEqual({"NAME":"JOSE", "LAST_NAME":"FERNANDES"}, info[0]["extra_metadata"])

    (status, ss) = epidb.search("JOSE", "", self.admin_key)
    self.assertEqual(1, len(ss))
    (status, ss) = epidb.search("FELIPE", "", self.admin_key)
    self.assertEqual(0, len(ss))


    res = epidb.change_extra_metadata(sample_id, "source", "ENCODE", self.admin_key)
    self.assertSuccess(res)

    s, info = epidb.info(sample_id, self.admin_key)
    self.assertEqual(info[0]["source"], "ENCODE")

