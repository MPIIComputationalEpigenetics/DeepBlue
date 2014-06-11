import helpers

from client import EpidbClient


class TestTechniques(helpers.TestCase):

  def test_add_techniques(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.add_technique("ChIP-seq", "ChIP-sequencing", {}, self.admin_key)
    self.assertSuccess(res)

    res, info = epidb.info(res[1], self.admin_key)
    self.assertSuccess(res, info)
    self.assertEquals(info["name"], 'ChIP-seq')
    self.assertEquals(info["description"], "ChIP-sequencing")


  def test_list_techniques(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.add_technique("illumina 450k", "illumina system", {"price": "low"}, self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_technique("RRBS", "Reduced representation bisulfite sequencing", {}, self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_technique("ChIP-seq", "ChIP-sequencing", {}, self.admin_key)
    self.assertSuccess(res)

    res, techniques = epidb.list_techniques(self.admin_key)
    self.assertSuccess(res, techniques)

    self.assertEqual(len(techniques), 3)

    technique_names = [x[1] for x in techniques]

    self.assertTrue("illumina 450k" in technique_names)
    self.assertTrue("RRBS" in technique_names)
    self.assertTrue("ChIP-seq" in technique_names)


  def test_list_similar_techniques(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.add_technique("ChIP-seq", "ChIP-sequencing", {}, self.admin_key)
    self.assertSuccess(res)

    res, similar_techniques = epidb.list_similar_techniques("chipseq", self.admin_key)
    self.assertSuccess(res, similar_techniques)
    self.assertEquals(similar_techniques[0][1], 'ChIP-seq')
    