import time

import helpers

from deepblue_client import DeepBlueClient
import data_info

class TestExperimentSets(helpers.TestCase):

  def test_experiment_sets(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_full(epidb)

    epidb.list_experiments

    (s, ees) = epidb.list_experiments("", "", "", "", "", "", "", self.admin_key)
    (s, names) = epidb.extract_names(ees)

    print names

    print

    (s, es1) = epidb.create_experiments_set("Test 1", "set test one", True, names, self.admin_key)

    print s
    print es1

    (s, info_es1) = epidb.info(es1, self.admin_key)

    print info_es1