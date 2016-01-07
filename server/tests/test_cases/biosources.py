import helpers

from deepblue_client import DeepBlueClient


class TestBioSourceCommands(helpers.TestCase):
  def test_biosource(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    self.assertSuccess(epidb.add_biosource("K562", "leukemia cell", {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("K562b", "leukemia cell", {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("HepG2", "hepatocellular carcinoma", {}, self.admin_key))

    res, biosources = epidb.list_biosources(None, self.admin_key)
    self.assertSuccess(res, biosources)

    biosources_names = [x[1] for x in biosources]

    self.assertEqual(len(biosources), 3)
    self.assertTrue("K562" in biosources_names)
    self.assertTrue("K562b" in biosources_names)
    self.assertTrue("HepG2" in biosources_names)


  def test_duplicated_biosource_with_synonym(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    s = epidb.add_biosource("Ana", "Ana", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_biosource_synonym("Ana", "Zebra", self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_biosource("Zebra", "Zebra", {}, self.admin_key)
    self.assertFailure(s)

  def test_biosource_scope(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    self.assertSuccess(epidb.add_biosource("GM12878", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("K562", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("Adult_CD4_naive", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("blood", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("mesoderm", None, {}, self.admin_key))

    res, biosources = epidb.list_biosources(None, self.admin_key)
    self.assertSuccess(res, biosources)

    biosources_names = [x[1] for x in biosources]

    self.assertEqual(len(biosources), 5)
    self.assertTrue("GM12878" in biosources_names)
    self.assertTrue("K562" in biosources_names)
    self.assertTrue("Adult_CD4_naive" in biosources_names)
    self.assertTrue("blood" in biosources_names)
    self.assertTrue("mesoderm" in biosources_names)

    self.assertSuccess(epidb.set_biosource_parent("blood", "GM12878", self.admin_key))
    self.assertSuccess(epidb.set_biosource_parent("blood", "K562", self.admin_key))
    self.assertSuccess(epidb.set_biosource_parent("blood", "Adult_CD4_naive", self.admin_key))
    self.assertSuccess(epidb.set_biosource_parent("mesoderm", "blood", self.admin_key))

    res = epidb.set_biosource_parent("GM12878", "mesoderm", self.admin_key)
    self.assertFailure(res)

    epidb.set_biosource_parent("avacado", "mesoderm", self.admin_key)
    self.assertFailure(res)

    res, scope = epidb.get_biosource_children("mesoderm", self.admin_key)
    self.assertSuccess(res, scope)
    scope_names = [x[1] for x in scope]
    self.assertEquals(scope_names, ['mesoderm', 'blood', 'GM12878', 'K562', 'Adult_CD4_naive'])

  def test_biosource_related(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    self.assertSuccess(epidb.add_biosource("A", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("A C", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("A D", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("A E", None, {}, self.admin_key))
    self.assertSuccess(epidb.add_biosource("C D", None, {}, self.admin_key))

    res = epidb.set_biosource_parent("A", "A C", self.admin_key)
    self.assertSuccess(res)
    res = epidb.set_biosource_parent("A", "A D", self.admin_key)
    self.assertSuccess(res)
    res = epidb.set_biosource_parent("A", "A E", self.admin_key)
    self.assertSuccess(res)
    res = epidb.set_biosource_parent("A", "C D", self.admin_key)
    self.assertSuccess(res)
    res = epidb.set_biosource_parent("A", "C D", self.admin_key)
    self.assertFailure(res)
    self.assertEqual(res[1], "104903:'A' and 'C D' are already connected.")

    res = epidb.set_biosource_parent("A C", "A D", self.admin_key)
    self.assertSuccess(res)
    res = epidb.set_biosource_parent("A D", "A E", self.admin_key)
    self.assertSuccess(res)

    res = epidb.set_biosource_parent("C D", "A", self.admin_key)
    self.assertFailure(res)
    self.assertEqual(res[1], "104903:'C D' and 'A' are already connected.")

  def test_check_embracing_bug(self):
	epidb = DeepBlueClient(address="localhost", port=31415)
	self.init(epidb)

	res = epidb.add_biosource("A", None, {}, self.admin_key)
	self.assertSuccess(res)

	res = epidb.set_biosource_synonym("A", "B", self.admin_key)
	self.assertSuccess(res)

	res = epidb.add_biosource("B", None, {}, self.admin_key)
	self.assertFailure(res)

	res = epidb.add_biosource("C", None, {}, self.admin_key)
	self.assertSuccess(res)

	res = epidb.set_biosource_parent("B", "C", self.admin_key)
	self.assertSuccess(res)

	res = epidb.get_biosource_related("A", self.admin_key)
	self.assertSuccess(res)
	self.assertEqual(res[1], [['bs1', 'A'], ['bs1', 'B'], ['bs2', 'C']])

	res = epidb.get_biosource_related("C", self.admin_key)
	self.assertSuccess(res)
	self.assertEqual(res[1], [['bs2', 'C']])
