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

    res, marks = epidb.list_epigenetic_marks({}, self.admin_key)
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

    res, marks = epidb.list_epigenetic_marks({}, self.admin_key)
    self.assertSuccess(res, marks)

    hptm = epidb.list_epigenetic_marks({"category": "histone post translation modification"}, self.admin_key)
    self.assertEqual(hptm, ['okay', [['em1', 'H1K186me1'], ['em2', 'H1K25ac'], ['em3', 'H1K25me1'], ['em4', 'H1S172ph'], ['em5', 'H1S17ph'], ['em6', 'H1S186ph'], ['em7', 'H1S188ph'], ['em8', 'H1S26ph'], ['em9', 'H1T10ph'], ['em10', 'H1T137ph'], ['em11', 'H1T145ph'], ['em12', 'H1T154ph'], ['em13', 'H1T17ph'], ['em14', 'H1T30ph'], ['em15', 'H2AK119ub'], ['em16', 'H2AK121ub'], ['em17', 'H2AK125bio'], ['em18', 'H2AK127bio'], ['em19', 'H2AK129bio'], ['em20', 'H2AK13ar'], ['em21', 'H2AK13bio'], ['em22', 'H2AK5ac'], ['em23', 'H2AK9ac'], ['em24', 'H2AK9bio'], ['em25', 'H2AR3ci'], ['em26', 'H2AR3me2'], ['em27', 'H2AS137ph'], ['em28', 'H2AS139ph'], ['em29', 'H2AS1ph'], ['em30', 'H2AY142ph'], ['em31', 'H2BK120ac'], ['em32', 'H2BK120ub'], ['em33', 'H2BK12ac'], ['em34', 'H2BK15ac'], ['em35', 'H2BK16ac'], ['em36', 'H2BK20ac'], ['em37', 'H2BK30ar'], ['em38', 'H2BK46ac'], ['em39', 'H2BK5ac'], ['em40', 'H2BK5me1'], ['em41', 'H2BS14ph'], ['em42', 'H3K14ac'], ['em43', 'H3K18ac'], ['em44', 'H3K18bio'], ['em45', 'H3K23ac'], ['em46', 'H3K27ac'], ['em47', 'H3K27ar'], ['em48', 'H3K27me1'], ['em49', 'H3K27me2'], ['em50', 'H3K27me3'], ['em51', 'H3K36ac'], ['em52', 'H3K36me2'], ['em53', 'H3K36me3'], ['em54', 'H3K37ar'], ['em55', 'H3K4ac'], ['em56', 'H3K4me1'], ['em57', 'H3K4me2'], ['em58', 'H3K4me3'], ['em59', 'H3K56ac'], ['em60', 'H3K79me2'], ['em61', 'H3K9ac'], ['em62', 'H3K9bio'], ['em63', 'H3K9me1'], ['em64', 'H3K9me2'], ['em65', 'H3K9me3'], ['em66', 'H3R17ci'], ['em67', 'H3R17me1'], ['em68', 'H3R17me2'], ['em69', 'H3R26ci'], ['em70', 'H3R26me1'], ['em71', 'H3R2ci'], ['em72', 'H3R2me1'], ['em73', 'H3R2me2'], ['em74', 'H3R8ci'], ['em75', 'H3R8me2'], ['em76', 'H3S10ph'], ['em77', 'H3S28ph'], ['em78', 'H3S31ph'], ['em79', 'H3S6ph'], ['em80', 'H3T11ph'], ['em81', 'H3T3ph'], ['em82', 'H3T45ph'], ['em83', 'H3T6ph'], ['em84', 'H3Y41ph'], ['em85', 'H4K12ac'], ['em86', 'H4K12bio'], ['em87', 'H4K16ac'], ['em88', 'H4K16ar'], ['em89', 'H4K20me1'], ['em90', 'H4K20me2'], ['em91', 'H4K20me3'], ['em92', 'H4K5ac'], ['em93', 'H4K8ac'], ['em94', 'H4K8bio'], ['em95', 'H4K91ac'], ['em96', 'H4K91ub'], ['em97', 'H4R3ci'], ['em98', 'H4R3me1'], ['em99', 'H4R3me2'], ['em100', 'H4S1ph'], ['em101', 'H2AT120ph'], ['em102', 'H1T153ph'], ['em103', 'H3K36me1'], ['em104', 'H3K79me1'], ['em105', 'H3K79me3'], ['em106', 'H1S171ph']]])

    hv = epidb.list_epigenetic_marks({"category": "histone variant"}, self.admin_key)
    self.assertEqual(hv, ['okay', [['em107', 'H3.3'], ['em108', 'H3a'], ['em109', 'H3d'], ['em110', 'H2AC'], ['em111', 'H2Aa4'], ['em112', 'H4b'], ['em113', 'H2A.Z'], ['em114', 'H2ab'], ['em115', 'H2ah'], ['em116', 'H2aa'], ['em117', 'H2a'], ['em118', 'H2ba'], ['em119', 'H2bb'], ['em120', 'H2bf'], ['em121', 'H3.3C'], ['em122', 'H2bc'], ['em123', 'H2A.J'], ['em124', 'H2B']]])


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
