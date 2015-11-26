import helpers

from client import EpidbClient


class TestListCommandsCommand(helpers.TestCase):

  def test_commands(self):
    epidb = EpidbClient()
    # explicitly no initialization. The command should always work.

    res, cmds = epidb.commands()
    self.assertSuccess(res, cmds)
    self.assertEqual(len(cmds), 71)
    self.assertEqual(cmds["add_experiment"]["parameters"][0], ['name', 'string', False, 'experiment name'])
    self.assertEqual(cmds["add_experiment"]["parameters"][1], ['genome', 'string', False, 'the target genome'])
    self.assertEqual(cmds["add_experiment"]["parameters"][2], ['epigenetic_mark', 'string', False, 'epigenetic mark of the experiment'])
    self.assertEqual(cmds["add_experiment"]["parameters"][3], ['sample', 'string', False, 'id of the used sample'])
    self.assertEqual(cmds["add_experiment"]["parameters"][4], ['technique', 'string', False, 'technique used by this experiment'])
    self.assertEqual(cmds["add_experiment"]["parameters"][5], ['project', 'string', False, 'the project name'])
    self.assertEqual(cmds["add_experiment"]["parameters"][6], ['description', 'string', False, 'description of the experiment'])
    self.assertEqual(cmds["add_experiment"]["parameters"][7], ['data', 'string', False, 'the BED formated data'])
    self.assertEqual(cmds["add_experiment"]["parameters"][8], ['format', 'string', False, 'format of the provided data'])
    self.assertEqual(cmds["add_experiment"]["parameters"][9], ['extra_metadata', 'struct', False, 'additional metadata'])
    self.assertEqual(cmds["add_experiment"]["parameters"][10], ['user_key', 'string', False, 'users token key'])
