import helpers

from client import EpidbClient


class TestBioSourceCommands(helpers.TestCase):

  def test_bio_source(self):
    epidb = EpidbClient()
    self.init(epidb)

    self.assertSuccess(epidb.add_bio_source("K562", "leukemia cell", {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("K562b", "leukemia cell", {}, self.admin_key))
    self.assertSuccess(epidb.add_bio_source("HepG2", "hepatocellular carcinoma", {}, self.admin_key))

    res, bio_sources = epidb.list_bio_sources(self.admin_key)
    self.assertSuccess(res, bio_sources)

    bio_sources_names = [x[1] for x in bio_sources]

    self.assertEqual(len(bio_sources), 3)
    self.assertTrue("K562" in bio_sources_names)
    self.assertTrue("K562b" in bio_sources_names)
    self.assertTrue("HepG2" in bio_sources_names)


  def test_duplicated_bio_source_with_synonym(self):
    epidb = EpidbClient()
    self.init(epidb)

    s = epidb.add_bio_source("Ana", "Ana", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_synonym("Ana", "Zebra", self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Zebra", "Zebra", {}, self.admin_key)
    self.assertFailure(s)
