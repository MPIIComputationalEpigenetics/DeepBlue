import helpers
import data_info

from client import EpidbClient

class TestState(helpers.TestCase):

    def get_state(self, vocab, epidb):
        self.assertSuccess(epidb.get_state(vocab, self.admin_key))
        return epidb.get_state(vocab, self.admin_key)

    def test_state(self):
        epidb = EpidbClient()
        self.init(epidb)

        summaries = [[data_info.GENOMES, "genomes", self.insert_genome],
                     [data_info.ANNOTATIONS, "annotations", self.insert_annotation],
                     [data_info.TECHNIQUES, "techniques", self.insert_technique],
                     [data_info.BIOSOURCES, "biosources", self.insert_biosource],
                     [data_info.EPIGENETIC_MARKS, "epigenetic_marks", self.insert_epigenetic_mark],
                     [data_info.SAMPLES, "samples", self.insert_samples],
                     [data_info.PROJECTS, "projects", self.insert_project],
                     [data_info.EXPERIMENTS, "experiments", self.insert_experiment]]

        for summary in summaries:
            data = summary[0]
            vocab = summary[1]
            insert_function = summary[2]

            s, n1 = self.get_state(vocab, epidb)
            key, value = data.items()[0]
            insert_function(epidb, key)
            s, n2 = self.get_state(vocab, epidb)
            self.assertTrue(n1 + 1 == n2)
            key, value = data.items()[1]
            id = insert_function(epidb, key)
            s, n3 = self.get_state(vocab, epidb)
            self.assertTrue(n2 + 1 == n3)

            epidb.remove(id, self.admin_key)
            s, n4 = self.get_state(vocab, epidb)
            self.assertTrue(n3 + 1 == n4)

            insert_function(epidb, key)