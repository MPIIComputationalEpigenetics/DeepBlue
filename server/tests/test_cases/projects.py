import helpers

from deepblue_client import DeepBlueClient

class TestProjects(helpers.TestCase):

  def test_set_project_publictest_project(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    res = epidb.add_project("ENCODE", "The ENCODE Project: ENCyclopedia Of DNA Elements", self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_project("Other", "Some other project", self.admin_key)
    self.assertSuccess(res)

    res, projects = epidb.list_projects(self.admin_key)
    self.assertSuccess(res, projects)

    self.assertEqual(len(projects), 2)

    project_names = [x[1] for x in projects]

    self.assertTrue("ENCODE" in project_names)
    self.assertTrue("Other" in project_names)


  def test_set_project_public(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)
    s, user = epidb.add_user("user", "email", "institution", self.admin_key)
    (user_id, user_key) = user
    self.assertSuccess(s)

    s, info = epidb.info("me", user_key)
    self.assertSuccess(s, info)
    (s, uid) = epidb.modify_user_admin(user_id, "permission_level", "INCLUDE_EXPERIMENTS", self.admin_key)

    res = epidb.add_project("Other", "Some other project", user_key)
    self.assertSuccess(res)

    res = epidb.set_project_public("Other", True, user_key)
    self.assertSuccess(res)

    res = epidb.add_project("Other2", "Some other project2", user_key)
    self.assertSuccess(res)

    res = epidb.set_project_public("Other2", True, self.admin_key)
    self.assertSuccess(res)

  def test_set_project_public(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    res = epidb.add_project("Other", "Some other project", self.admin_key)
    self.assertSuccess(res)

    s, user = epidb.add_user("user", "email", "institution", self.admin_key)
    (user_id, user_key) = user
    self.assertSuccess(s)

    status, projects = epidb.list_projects(self.admin_key)
    self.assertEqual(projects, [['p1', 'ENCODE'], ['p2', 'Mouse ENCODE'], ['p3', 'Other']])

    status, projects = epidb.list_projects(user_key)
    self.assertEqual(projects, [])

    epidb.set_project_public("ENCODE", True, self.admin_key)

    status, projects = epidb.list_projects(user_key)
    self.assertEqual(projects, [["p1", "ENCODE"]])

    epidb.set_project_public("ENCODE", False, self.admin_key)

    status, projects = epidb.list_projects(user_key)
    self.assertEqual(projects, [])

    epidb.set_project_public("Mouse ENCODE", True, self.admin_key)
    epidb.set_project_public("ENCODE", True, self.admin_key)

    status, projects = epidb.list_projects(user_key)
    self.assertEqual(projects, [['p1', 'ENCODE'], ['p2', 'Mouse ENCODE']])

    epidb.set_project_public("Other", True, self.admin_key)

    status, projects = epidb.list_projects(user_key)
    self.assertEqual(projects, [['p1', 'ENCODE'], ['p2', 'Mouse ENCODE'], ['p3', 'Other']])


  def test_set_project_public(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    (s, user) = epidb.add_user("NAME", "EMAIL", "INSTITUTE", self.admin_key)

    s, tmp_user = epidb.modify_user_admin(user[0], "permission_level", "INCLUDE_EXPERIMENTS", self.admin_key)

    s, project = epidb.add_project("USER's PROJECT", "COOL", user[1])

    (s, user_two) = epidb.add_user("ANOTHER NAME", "ANOTHER EMAIL", "INSTITUTE", self.admin_key)

    (s, (user_add, project_add)) = epidb.add_user_to_project(user_two[0], project, True, self.admin_key)
    self.assertSuccess(s, (user, project))
    self.assertEqual(user_add, user_two[0])
    self.assertEqual(project, project_add)

  def test_set_project_public_no_permisson(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    (s, user) = epidb.add_user("NAME", "EMAIL", "INSTITUTE", self.admin_key)

    s, project = epidb.add_project("USER's PROJECT", "COOL", user[1])
    self.assertEqual(project, "100100:Insufficient permission. Permission INCLUDE_EXPERIMENTS is required.")

  def test_set_project_public_by_user(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    (s, user) = epidb.add_user("NAME", "EMAIL", "INSTITUTE", self.admin_key)
    (s, user_ass) = epidb.add_user("NAME ASS", "EMAIL ASS", "INSTITUTE", self.admin_key)

    s, tmp_user = epidb.modify_user_admin(user[0], "permission_level", "INCLUDE_EXPERIMENTS", self.admin_key)

    s, project = epidb.add_project("USER's PROJECT", "COOL", user[1])

    (s, user_two) = epidb.add_user("ANOTHER NAME", "ANOTHER EMAIL", "INSTITUTE", self.admin_key)

    (s, (user_add, project_add)) = epidb.add_user_to_project(user_two[0], project, True, user[1])
    self.assertSuccess(s, (user, project))
    self.assertEqual(user_add, user_two[0])
    self.assertEqual(project, project_add)

  def test_set_project_public_by_user_no_permission(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    (s, user) = epidb.add_user("NAME", "EMAIL", "INSTITUTE", self.admin_key)
    (s, user_ass) = epidb.add_user("NAME ASS", "EMAIL ASS", "INSTITUTE", self.admin_key)

    s, tmp_user = epidb.modify_user_admin(user[0], "permission_level", "INCLUDE_EXPERIMENTS", self.admin_key)
    s, tmp_user = epidb.modify_user_admin(user_ass[0], "permission_level", "INCLUDE_EXPERIMENTS", self.admin_key)

    s, project = epidb.add_project("USER's PROJECT", "COOL", user[1])
    (s, (user_add, project_add)) = epidb.add_user_to_project(user_ass[0], project, True, user[1])
    self.assertSuccess(s, (user, project))
    self.assertEqual(user_add, user_ass[0])
    self.assertEqual(project, project_add)

    (s, user_two) = epidb.add_user("ANOTHER NAME", "ANOTHER EMAIL", "INSTITUTE", self.admin_key)

    (s, status) = epidb.add_user_to_project(user_two[0], project, True, user_ass[1])
    self.assertEqual(status, "107100:You are not the project 'p3' owner and neither an administrator.")
