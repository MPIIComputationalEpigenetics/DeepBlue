import helpers

from client import EpidbClient
from settings import EPIDB_TEST_ADMIN


class TestInitCommands(helpers.TestCase):

  def test_uninitialized_system(self):
    epidb = EpidbClient()

    # uninitialized system should fail
    res, msg = epidb.add_user("user1", "test@example.com", "test", "somekey")
    self.assertFailure(res, msg)


  def test_init_system(self):
    epidb = EpidbClient()

    res, admin_key = epidb.init_system(*EPIDB_TEST_ADMIN)
    self.assertSuccess(res, admin_key)

    # check if system is able to receive commands
    res, key = epidb.add_user("user1", "test@example.com", "test", admin_key)
    self.assertSuccess(res, key)
