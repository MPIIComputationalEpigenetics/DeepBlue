import helpers

from client import EpidbClient


class TestSamples(helpers.TestCase):

  def test_find_sample(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.add_biosource("K562", "desc1", {}, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_sample_field("age", "string", "", self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_sample_field("health", "string", "", self.admin_key)
    self.assertSuccess(res)

    res, sid = epidb.add_sample("K562", {"age":"55","health":"deceased"}, self.admin_key)
    self.assertSuccess(res, sid)

    res, found_id = epidb.list_samples("K562", {"age":"55", "health":"deceased"}, self.admin_key)
    self.assertSuccess(res, found_id)
    self.assertEqual(sid, found_id[0][0])

  def test_find_samples(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.add_biosource("K562", "desc1", {}, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_biosource("KKKK", "desc1", {}, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_sample_field("age", "string", "", self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_sample_field("health", "string", "", self.admin_key)
    self.assertSuccess(res)

    res, id1 = epidb.add_sample("K562", {"age":"55","health":"deceased"}, self.admin_key)
    self.assertSuccess(res, id1)

    res, id2 = epidb.add_sample("K562", {"age":"55","health":"healthy"}, self.admin_key)
    self.assertSuccess(res, id2)

    res, id3 = epidb.add_sample("K562", {"age":"15"}, self.admin_key)
    self.assertSuccess(res, id3)

    res, id4 = epidb.add_sample("KKKK", {"age":"55","health":"deceased"}, self.admin_key)
    self.assertSuccess(res, id4)

    res, samples = epidb.list_samples("K562", {}, self.admin_key)
    self.assertSuccess(res, samples)

    found_ids = [(y[0]) for y in samples]

    self.assertTrue(id1 in found_ids)
    self.assertTrue(id2 in found_ids)
    self.assertTrue(id3 in found_ids)
    self.assertFalse(id4 in found_ids)

  def test_list_samples(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.add_biosource("K562", "desc1", {}, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_biosource("KKKK", "desc1", {}, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_sample_field("age", "string", "", self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_sample_field("health", "string", "", self.admin_key)
    self.assertSuccess(res)

    res, id1 = epidb.add_sample("K562", {"age":"55","health":"deceased"}, self.admin_key)
    self.assertSuccess(res, id1)

    res, id2 = epidb.add_sample("K562", {"age":"55","health":"healthy"}, self.admin_key)
    self.assertSuccess(res, id2)

    res, id3 = epidb.add_sample("K562", {"age":"15"}, self.admin_key)
    self.assertSuccess(res, id3)

    res, id4 = epidb.add_sample("KKKK", {"age":"55","health":"deceased"}, self.admin_key)
    self.assertSuccess(res, id4)

    (res, k562_samples) = epidb.list_samples("K562", {}, self.admin_key)
    self.assertSuccess(res, k562_samples)
    self.assertEqual(3, len(k562_samples))

    self.assertEqual(['s1', {'age': '55', '_id': 's1', 'health': 'deceased', 'user': 'test_admin', 'biosource_name': 'K562'}], k562_samples[0])

  def test_multiple_biosources_samples(self):
    epidb = EpidbClient()
    self.init(epidb)

    (res, k562_id) = epidb.add_biosource("K562", "desc1", {}, self.admin_key)
    (res, colon_id) = epidb.add_biosource("Colon", "desc1", {}, self.admin_key)
    (res, colon_md_id) = epidb.add_biosource("Colon_MD", "desc1", {}, self.admin_key)
    (res, intestine_id) = epidb.add_biosource("Intestine", "desc1", {}, self.admin_key)

    res = epidb.add_sample_field("age", "string", "", self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_sample_field("health", "string", "", self.admin_key)
    self.assertSuccess(res)

    res, id1 = epidb.add_sample("K562", {"age":"55","health":"deceased"}, self.admin_key)
    self.assertSuccess(res, id1)

    res, id2 = epidb.add_sample("K562", {"age":"55","health":"healthy"}, self.admin_key)
    self.assertSuccess(res, id2)

    res, id3 = epidb.add_sample("K562", {"age":"15"}, self.admin_key)
    self.assertSuccess(res, id3)

    res, id4 = epidb.add_sample("Colon", {"age":"55","health":"deceased"}, self.admin_key)
    self.assertSuccess(res, id4)

    res, id5 = epidb.add_sample("Colon_MD", {"age":"55","health":"deceased"}, self.admin_key)
    self.assertSuccess(res, id4)

    res, id6 = epidb.add_sample("Intestine", {"age":"55","health":"deceased"}, self.admin_key)
    self.assertSuccess(res, id4)

    (res, multiple_samples) = epidb.list_samples(["K562", "Colon", "Colon_MD", "Intestine"], {}, self.admin_key)

    returned_sources = [(y[1]["biosource_name"]) for y in multiple_samples]

    self.assertTrue("K562" in returned_sources)
    self.assertTrue("Colon" in returned_sources)
    self.assertTrue("Colon_MD" in returned_sources)
    self.assertTrue("Intestine" in returned_sources)
    self.assertFalse("Intestine2" in returned_sources)
    self.assertFalse("K562a" in returned_sources)
