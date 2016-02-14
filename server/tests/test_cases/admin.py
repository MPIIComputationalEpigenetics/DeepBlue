import helpers

from deepblue_client import DeepBlueClient
from settings import EPIDB_TEST_ADMIN


class TestAdminCommands(helpers.TestCase):

  def test_users(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    res, user = epidb.add_user("user1", "test1@example.com", "test", self.admin_key)
    self.assertSuccess(res, user)

    res, users = epidb.list_users(self.admin_key)
    self.assertSuccess(res, users)

    user_names = epidb.extract_names(users)[1]
    self.assertEqual(len(users), 2) # the admin and the newly created user
    self.assertTrue(EPIDB_TEST_ADMIN[0] in user_names)
    self.assertTrue("user1" in user_names)


  def test_anonymous(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    res, user = epidb.add_user("anonymous", "test1@example.com", "test", self.admin_key)
    self.assertSuccess(res, user)

    res, msg = epidb.modify_user("password", "123456", user[1])
    self.assertFailure(res, msg)
    self.assertEquals(msg,  'It is not allowed to change the attributes of the anonymous user')

  def test_echo(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)
    self.assertEquals(epidb.echo(self.admin_key), ['okay', 'DeepBlue (1.6.7) says hi to test_admin'])
    self.assertEquals(epidb.echo("invalid"), ['okay', 'DeepBlue (1.6.7) says hi to a Stranger'])

  def test_unequal_keys(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)
    # Note: this test has not 100% hit chance since the problem only
    # occured when two keys were generated within one second.
    # Though 3 users give a high chance for this to happen.

    res, u1 = epidb.add_user("user1", "test1@example.com", "test", self.admin_key)
    self.assertSuccess(res, u1)
    res, u2 = epidb.add_user("user2", "test2@example.com", "test", self.admin_key)
    self.assertSuccess(res, u2)
    res, u3 = epidb.add_user("user3", "test3@example.com", "test", self.admin_key)
    self.assertSuccess(res, u3)

    self.assertTrue(u1[1] != u2[1]) # ensure different user keys
    self.assertTrue(u1[1] != u3[1])
    self.assertTrue(u2[1] != u3[1])

    res, users = epidb.list_users(self.admin_key)
    self.assertSuccess(res, users)

    user_names = epidb.extract_names(users)[1]

    self.assertEqual(len(users), 4)
    self.assertTrue(EPIDB_TEST_ADMIN[0] in user_names)
    self.assertTrue("user1" in user_names)
    self.assertTrue("user2" in user_names)
    self.assertTrue("user3" in user_names)


  def test_add_user_by_non_admin(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    res, u1 = epidb.add_user("user1", "test1@example.com", "test", self.admin_key)
    self.assertSuccess(res, u1)

    res, msg = epidb.add_user("user2", "test2@example.com", "test", u1[1])
    self.assertFailure(res, msg)
