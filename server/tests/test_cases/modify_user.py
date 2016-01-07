import helpers
import settings

from deepblue_client import DeepBlueClient

class TestModifyUserCommand(helpers.TestCase):

    def test_success(self):
        epidb = DeepBlueClient(address="localhost", port=31415)
        self.init_base(epidb)

        s = epidb.modify_user("password", "password123", self.admin_key)
        self.assertSuccess(s)
        s, key = epidb.user_auth(settings.EPIDB_TEST_ADMIN[1], "password123")
        self.assertSuccess(s)
        self.assertEquals(key, self.admin_key)

        s = epidb.modify_user("email", "new@email.com", self.admin_key)
        self.assertSuccess(s)
        s, key = epidb.user_auth("new@email.com", "password123")
        self.assertSuccess(s)
        self.assertEquals(key, self.admin_key)

        s = epidb.modify_user("institution", "new_institution", self.admin_key)
        self.assertSuccess(s)

    def test_nonexisting_key(self):
        epidb = DeepBlueClient(address="localhost", port=31415)
        self.init_base(epidb)

        s = epidb.modify_user("password", "password123", "nonExistingKey")
        self.assertFailure(s)