import helpers

from client import EpidbClient

class TestProjects(helpers.TestCase):

  def test_project(self):
    epidb = EpidbClient()
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
    epidb = EpidbClient()
    self.init_base(epidb)
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
    epidb = EpidbClient()
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
