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
    