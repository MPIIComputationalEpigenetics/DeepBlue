import helpers

from deepblue_client import DeepBlueClient

class TestFacetingCommand(helpers.TestCase):

  def test_faceting(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    (s, v) = epidb.faceting_experiments("", "", "", "", "", "", "", self.admin_key)
    self.assertEqual(v, {'epigenetic_marks': [['em1', 'H3K4me3', 1], ['em2', 'Methylation', 6]], 'genomes': [['g1', 'hg18', 1], ['g2', 'hg19', 5]], 'biosources': [['bs1', 'K562', 6]], 'samples': [['s1', '', 6]], 'types': [['', 'peaks', 6]], 'projects': [['p1', 'ENCODE', 6]], 'techniques': [['t2', 'tech2', 1], ['t1', 'tech1', 5]]})


    (s, v) = epidb.faceting_experiments("hg19", "peaks", "", "", "", "", "", self.admin_key)
    self.assertEqual(v, {'epigenetic_marks': [['em1', 'H3K4me3', 1], ['em2', 'Methylation', 4]], 'genomes': [['g2', 'hg19', 5]], 'biosources': [['bs1', 'K562', 5]], 'samples': [['s1', '', 5]], 'types': [['', 'peaks',  5]], 'projects': [['p1', 'ENCODE', 5]], 'techniques': [['t2', 'tech2', 1], ['t1', 'tech1', 4]]})

    (s, v) = epidb.faceting_experiments("hg19", "peaks", "", "", "", "tech2", "", self.admin_key)
    self.assertEqual(v, {'epigenetic_marks': [['em1', 'H3K4me3', 1]], 'genomes': [['g2', 'hg19', 1]], 'biosources': [['bs1', 'K562', 1]], 'samples': [['s1', '', 1]], 'types': [['', 'peaks', 1]], 'projects': [['p1', 'ENCODE', 1]], 'techniques': [['t2', 'tech2', 1]]})

    (s, v) = epidb.faceting_experiments("hg18", "", "", "", "", "", "", self.admin_key)
    self.assertEqual(v, {'epigenetic_marks': [['em2', 'Methylation', 1]], 'genomes': [['g1', 'hg18', 1]], 'biosources': [['bs1', 'K562', 1]], 'samples': [['s1', '', 1]], 'projects': [['p1', 'ENCODE', 1]], 'types': [['', 'peaks', 1]], 'techniques': [['t1', 'tech1', 1]]})


  def test_collection_experiments_count(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    (s, v) = epidb.collection_experiments_count("epigenetic_marks", "hg19", "peaks", "", "", "", "", "", self.admin_key)


    (s, v) = epidb.collection_experiments_count("epigenetic_marks", "", "", "", "", "", "", "", self.admin_key)
    self.assertEqual(v, [['em1', 'H3K4me3', 1], ['em2', 'Methylation', 6]])

    (s, v) = epidb.collection_experiments_count("epigenetic_marks", "hg19", "peaks", "", "", "", "", "", self.admin_key)
    self.assertEqual(v, [['em1', 'H3K4me3', 1], ['em2', 'Methylation', 4]])

    (s, v) = epidb.collection_experiments_count("epigenetic_marks", "hg19", "peaks", "", "", "", "tech2", "", self.admin_key)
    self.assertEqual(v, [['em1', 'H3K4me3', 1]])

    (s, v) = epidb.collection_experiments_count("epigenetic_marks", "hg18", "", "", "", "", "", "", self.admin_key)
    self.assertEqual(v, [['em2', 'Methylation', 1]])
