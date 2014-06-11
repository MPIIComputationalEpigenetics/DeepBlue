import helpers

from client import EpidbClient


class TestListSimilarCommands(helpers.TestCase):

  def test_list_similar_experiments(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)
    self.insert_experiment(epidb, "hg19_chr1_2", sample_id)
    self.insert_experiment(epidb, "hg19_chr1_3", sample_id)

    res, exps = epidb.list_similar_experiments("hg19_chr1_1", "hg19", self.admin_key)
    self.assertSuccess(res, exps)

    exp_names = [x[1] for x in exps]

    self.assertEqual(exp_names, ['hg19_chr1_1', 'hg19_chr1_2', 'hg19_chr1_3'])
