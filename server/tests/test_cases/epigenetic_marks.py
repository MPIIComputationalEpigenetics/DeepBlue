import helpers

from client import EpidbClient
import histones


class TestEpigeneticMarkCommands(helpers.TestCase):

  def test_epigenetic_mark(self):
    epidb = EpidbClient()
    self.init(epidb)

    self.assertSuccess(epidb.add_epigenetic_mark("Methylation", "DNA Methylation", {}, self.admin_key))
    self.assertSuccess(epidb.add_epigenetic_mark("OpenChromDnase", "Open Chromatin DNaseI", {}, self.admin_key))
    self.assertSuccess(epidb.add_epigenetic_mark("DNaseI", "DNaseI hypersensitive sites ", {}, self.admin_key))
    self.assertSuccess(epidb.add_epigenetic_mark("DNaseIUniform", "DNaseIUniform", {}, self.admin_key))

    res, marks = epidb.list_epigenetic_marks(self.admin_key)
    self.assertSuccess(res, marks)
    self.assertEqual(len(marks), 4)

    mark_names = [x[1] for x in marks]

    self.assertTrue("Methylation" in mark_names)
    self.assertTrue("OpenChromDnase" in mark_names)
    self.assertTrue("DNaseI" in mark_names)
    self.assertTrue("DNaseIUniform" in mark_names)


  def test_insert_histone_modifications(self):
    epidb = EpidbClient()
    self.init(epidb)

    for i in histones.histones_ptm.split("\n"):
      name, description =  i.split(None, 1)
      full_d = histones.d_ptm + description + ". " + histones.source
      res = epidb.add_epigenetic_mark(name, full_d, {"category": "histone post translation modification"}, self.admin_key)
      self.assertSuccess(res)

    for i in histones.histones_variants.split("\n"):
      name, description = i.split(None, 1)
      full_d = histones.d_hav + description + ". " + histones.source
      res = epidb.add_epigenetic_mark(name, full_d, {"category": "histone variant"}, self.admin_key)
      self.assertSuccess(res)

    res, marks = epidb.list_epigenetic_marks(self.admin_key)
    self.assertSuccess(res, marks)


  def test_normalize_histone_modifications(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.add_epigenetic_mark("H2A.Z", "H2A histone family, member Z", {}, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_epigenetic_mark("H2AZ", "H2A histone family, member Z", {}, self.admin_key)
    self.assertFailure(res)

    res = epidb.add_epigenetic_mark("H3K9ac", "H3K9ac", {}, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_epigenetic_mark("H3K09ac", "H3K9ac", {}, self.admin_key)
    self.assertFailure(res)

    res = epidb.add_epigenetic_mark("H3K009ac", "H3K009ac", {}, self.admin_key)
    self.assertFailure(res)

    res, marks = epidb.list_similar_epigenetic_marks("H3k09ac", self.admin_key)
    self.assertSuccess(res, marks)
    self.assertEqual(marks[0][1], "H3K9ac")
