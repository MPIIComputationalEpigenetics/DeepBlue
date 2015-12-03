import helpers

from client import EpidbClient

class TestFacetingCommand(helpers.TestCase):

  def test_faceting(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    import pprint
    (s, v) = epidb.faceting_experiments("", "", "", "", "", "", "", self.admin_key)

    self.assertEqual(v,  {'epigenetic_marks': [['em1', 'H3K4me3', 1], ['em2', 'Methylation', 5]], 'genomes': [['g1', 'hg18', 1], ['g2', 'hg19', 5]], 'biosources': [['bs1', 'K562', 6]], 'samples': [['s1', '', 6]], 'projects': [['p1', 'ENCODE', 6]], 'techniques': [['t2', 'tech2', 1], ['t1', 'tech1', 5]]})