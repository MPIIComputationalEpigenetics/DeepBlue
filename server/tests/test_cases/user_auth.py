import helpers
import settings

from client import EpidbClient

class TestUserAuthCommand(helpers.TestCase):

    def test_success(self):
        epidb = EpidbClient()
        self.init_base(epidb)

        s = epidb.modify_user("password", "password123", self.admin_key)
        self.assertSuccess(s)
        s, key = epidb.user_auth(settings.EPIDB_TEST_ADMIN[1], "password123")
        self.assertSuccess(s)
        self.assertEquals(key, self.admin_key)

        s, user_info = epidb.add_user("user1", "email@example.com", "institution", self.admin_key)
        user_id, user_key = user_info
        self.assertSuccess(s)
        s = epidb.modify_user("password", "password567", user_key)
        self.assertSuccess(s)
        s, key = epidb.user_auth("email@example.com", "password567")
        self.assertSuccess(s)
        self.assertEquals(key, user_key)

    def test_wrong_password(self):
        epidb = EpidbClient()
        self.init_base(epidb)

        s, key = epidb.user_auth(settings.EPIDB_TEST_ADMIN[1], "wrong_password")
        self.assertFailure(s, key)

    def test_wrong_email(self):
        epidb = EpidbClient()
        self.init_base(epidb)

        s, key = epidb.user_auth("wrong@email.com", self.admin_key)
        self.assertFailure(s, key)