import helpers
import data_info

from client import EpidbClient


class TestPermissions(helpers.TestCase):

    def get_new_user(self, epidb):
        s, user = epidb.add_user("user", "email", "institution", self.admin_key)
        self.assertSuccess(s)
        return user

    def modify_user_permission(self, epidb, user_id, permission):
        s = epidb.modify_user_admin(user_id, "permission_level", permission, self.admin_key)
        self.assertSuccess(s)

    def test_admin(self):
        epidb = EpidbClient()
        self.init_base(epidb)

        s = epidb.add_user("user", "email", "institution", self.admin_key)
        self.assertSuccess(s)

        self.modify_user_permission(epidb, "u1", "INCLUDE_COLLECTION_TERMS")

        s = epidb.add_user("user2", "email2", "institution", self.admin_key)
        self.assertFailure(s)

    def test_include_collection_terms(self):
        epidb = EpidbClient()
        self.init_base(epidb)

        user_id, user_key = self.get_new_user(epidb)

        s = epidb.add_biosource("b", "D", {}, user_key)
        self.assertFailure(s)

        self.modify_user_permission(epidb, user_id, "INCLUDE_COLLECTION_TERMS")

        s = epidb.add_biosource("b", "D", {}, user_key)
        self.assertSuccess(s)

    def test_include_annotations(self):
        epidb = EpidbClient()
        self.init_base(epidb)

        user_id, user_key = self.get_new_user(epidb)

        ann = helpers.data.ANNOTATIONS["Cpg Islands"]
        with open(ann["data_file"], 'r') as f:
            annotation_data = f.read()

        s = epidb.add_annotation("Cpg Islands", ann["genome"], ann["description"],
                                    annotation_data, ann["format"], ann["metadata"],
                                    user_key)
        self.assertFailure(s)

        self.modify_user_permission(epidb, user_id, "INCLUDE_ANNOTATIONS")

        s = epidb.add_annotation("Cpg Islands", ann["genome"], ann["description"],
                                 annotation_data, ann["format"], ann["metadata"],
                                 user_key)
        self.assertSuccess(s)

    def test_get_data(self):
        epidb = EpidbClient()
        self.init_base(epidb)

        user_id, user_key = self.get_new_user(epidb)

        s = epidb.list_requests("running", user_key)
        self.assertFailure(s)

        self.modify_user_permission(epidb, user_id, "GET_DATA")

        s = epidb.list_requests("running", user_key)
        self.assertSuccess(s)

    def test_list_collections(self):
        epidb = EpidbClient()
        self.init_base(epidb)

        user_id, user_key = self.get_new_user(epidb)

        s = epidb.info("me", user_key)
        self.assertSuccess(s)

        self.modify_user_permission(epidb, user_id, "NONE")

        s = epidb.info("me", user_key)
        self.assertSuccess(s)

        s = epidb.info("e1", user_key)
        self.assertFailure(s)

        s = epidb.info("p1", user_key)
        self.assertFailure(s)

        s = epidb.info("bs1", user_key)
        self.assertFailure(s)

    def test_change_extra_metadata(self):
        epidb = EpidbClient()
        self.init_base(epidb)

        user_id, user_key = self.get_new_user(epidb)
        self.modify_user_permission(epidb, user_id, "INCLUDE_COLLECTION_TERMS")

        s, id = epidb.add_biosource("lsdjf", "sdf", {"a": "b", "c": "d"}, user_key)

        s = epidb.change_extra_metadata(id, "a", "f", user_key)
        self.assertSuccess(s)

        s = epidb.change_extra_metadata(id, "c", "g", self.admin_key)
        self.assertSuccess(s)

        s, user = epidb.add_user("user2", "email2", "institution2", self.admin_key)
        self.assertSuccess(s)
        user_id2, user_key2 = user
        self.modify_user_permission(epidb, user_id2, "INCLUDE_COLLECTION_TERMS")

        s = epidb.change_extra_metadata(id, "c", "g", user_key2)
        self.assertFailure(s)