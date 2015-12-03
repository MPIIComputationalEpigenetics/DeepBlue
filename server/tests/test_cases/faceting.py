import helpers

from client import EpidbClient

class TestFacetingCommand(helpers.TestCase):

  def test_faceting(self):
    epidb = EpidbClient()
    self.init_full(epidb)

    print epidb.faceting_experiments("", "", "", "", "", "", "", self.admin_key)
