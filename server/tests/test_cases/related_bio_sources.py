import helpers

from client import EpidbClient


class TestBioSourceRelated(helpers.TestCase):

  def test_bio_source(self):
    epidb = EpidbClient()
    self.init(epidb)

    self.assertSuccess(epidb.add_bio_source("GM12878", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("K562", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("Adult_CD4_naive", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("blood", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("mesoderm", None, {}, self.admin_key))

    res, bio_sources = epidb.list_bio_sources(self.admin_key)
    self.assertSuccess(res, bio_sources)

    bio_sources_names = [x[1] for x in bio_sources]

    self.assertEqual(len(bio_sources), 5)
    self.assertTrue("GM12878" in bio_sources_names)
    self.assertTrue("K562" in bio_sources_names)
    self.assertTrue("Adult_CD4_naive" in bio_sources_names)
    self.assertTrue("blood" in bio_sources_names)
    self.assertTrue("mesoderm" in bio_sources_names)

    self.assertSuccess(epidb.set_bio_source_scope("blood", "GM12878", self.admin_key))
    self.assertSuccess(epidb.set_bio_source_scope("blood", "K562", self.admin_key))
    self.assertSuccess(epidb.set_bio_source_scope("blood", "Adult_CD4_naive", self.admin_key))
    self.assertSuccess(epidb.set_bio_source_scope("mesoderm", "blood", self.admin_key))

    res = epidb.set_bio_source_scope("GM12878", "mesoderm", self.admin_key)
    self.assertFailure(res)

    epidb.set_bio_source_scope("avacado", "mesoderm", self.admin_key)
    self.assertFailure(res)

    res, scope = epidb.get_bio_source_scope("mesoderm", self.admin_key)
    self.assertSuccess(res, scope)
    scope_names = [x[1] for x in scope]
    self.assertEquals(scope_names, ['mesoderm', 'blood', 'GM12878', 'K562', 'Adult_CD4_naive'])

  def test_bio_source_related(self):
    epidb = EpidbClient()
    self.init(epidb)

    self.assertSuccess(epidb.add_bio_source("A", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("A C", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("A D", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("A E", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("C D", None, {}, self.admin_key))

    res = epidb.set_bio_source_scope("A", "A C", self.admin_key)
    self.assertSuccess(res)
    res = epidb.set_bio_source_scope("A", "A D", self.admin_key)
    self.assertSuccess(res)
    res = epidb.set_bio_source_scope("A", "A E", self.admin_key)
    self.assertSuccess(res)
    res = epidb.set_bio_source_scope("A", "C D", self.admin_key)
    self.assertSuccess(res)
    res = epidb.set_bio_source_scope("A", "C D", self.admin_key)
    self.assertFailure(res)
    self.assertEqual(res[1], "104901:'A' is already more embracing than 'C D'.")

