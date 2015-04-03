import helpers
import data_info

from client import EpidbClient


class TestState(helpers.TestCase):


    def test_state_experiments(self):
        """Tests state after insert and remove for experiments."""
        epidb = EpidbClient()
        self.init_base(epidb)

        name = "experiments"
        data = self._get_data_names()[name]["data"]
        insert = self._get_data_names()[name]["insert"]

        ids = self.insert_all(epidb, data, name, insert)

        self.remove(epidb, "experiments", ids)

    def insert_random(self, epidb, data, vocab, insert):
        """Inserts data adding numbers to the name, checks state
        :param epidb: DeepBlue instance
        :param data: Data to be inserted
        :param vocab: Data-type
        :param insert: Function which inserts the data
        :return: DeepBlue IDs of inserted data
        """
        ids = []
        state_old = self.get_state(vocab, epidb)
        key, value = data.items()[0]

        for i in range(0, 7):
            new_key = key + str(i)
            data[new_key] = value
            ids.append(insert(epidb, new_key))
            state_new = self.get_state(vocab, epidb)
            self.assertTrue(state_old + 1 == state_new)
            state_old = state_new

        return ids

    def insert_all(self, epidb, data, name, insert):
        """Insert all data from data_info, checks state
        :param epidb: DeepBlue instance
        :param data: Data to be inserted
        :param name: Data-type
        :param insert: Function which inserts the data
        :return: DeepBlue IDs of inserted data
        """
        ids = []
        state_old = self.get_state(name, epidb)
        for item in data.items():
            key, value = item
            ids.append(insert(epidb, key))
            state_new = self.get_state(name, epidb)
            self.assertTrue(state_old + 1 == state_new)
            state_old = state_new
        return ids

    def remove(self, epidb, name, ids):
        """

        :param epidb: DeepBlue instance
        :param name: Data-type
        :param ids: DeepBlue IDs to be removed
        """

        state_old = self.get_state(name, epidb)
        for _id in ids:
            epidb.remove(_id, self.admin_key)
            state_new = self.get_state(name, epidb)
            self.assertTrue(state_old + 1 == state_new)
            state_old = state_new

    def get_states(self, epidb, exceptions):
        """States from all data-types except some
        :param epidb: DeepBlue instance
        :param exceptions: data-types not to be included
        :return: List of states
        """
        states = []
        for key in self._get_data_names().keys():
            if key not in exceptions:
                states.append(self.get_state(key, epidb))
        return states

    def get_state(self, name, epidb):
        """Calls get_state, asserts success
        :param name: Data-type
        :param epidb: DeepBlue instance
        :return: state
        """
        self.assertSuccess(epidb.get_state(name, self.admin_key))
        return epidb.get_state(name, self.admin_key)[1]

    def _get_data_names(self):
        return {
            "genomes": {
                "data": data_info.GENOMES,
                "insert": self.insert_genome
            },
            "annotations": {
                "data": data_info.ANNOTATIONS,
                "insert": self.insert_annotation
            },
            "techniques": {
                "data": data_info.TECHNIQUES,
                "insert": self.insert_technique
            },
            "epigenetic_marks": {
                "data": data_info.EPIGENETIC_MARKS,
                "insert": self.insert_epigenetic_mark
            },
            "projects": {
                "data": data_info.PROJECTS,
                "insert": self.insert_project
            },
            "biosources": {
                "data": data_info.BIOSOURCES,
                "insert": self.insert_biosource
            },
            "samples": {
                "data": data_info.SAMPLES,
                "insert": self.insert_samples
            },
            "experiments": {
                "data": data_info.EXPERIMENTS,
                "insert": self.insert_experiment
            }}