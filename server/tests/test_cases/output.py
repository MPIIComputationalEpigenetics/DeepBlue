import helpers

from client import EpidbClient


class TestOutput(helpers.TestCase):

  def test_output_format(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    file_data = open("data/cpgIslandAllFields.txt").read()

    cpg_island =  ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "name:String",
      "length:Integer:0",
      "cpgNum:Integer:0",
      "gcNum:Integer:0",
      "perCpg:Double",
      "perGc:Double",
      "obsExp:Double"
    ])

    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands", file_data, cpg_island, None, self.admin_key)
    self.assertSuccess(res)

    res, qid_1 = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
    self.assertSuccess(res, qid_1)
    self.assertEqual(qid_1, 'q1')

    res, count = epidb.count_regions(qid_1, self.admin_key)
    self.assertSuccess(res, count)
    self.assertEqual(10, count)

    res, regions = epidb.get_regions(qid_1, "chr:start:end:name:length:cpgNum:gcNum:perCpg:perGc:obsExp", self.admin_key)
    self.assertEquals("Wrong output format: 'chr:start:end:name:length:cpgNum:gcNum:perCpg:perGc:obsExp'. The format is \"<field:default_value>,<field:default_value>,..\"", regions)

    res, regions = epidb.get_regions(qid_1, "CHROMOSOME,START,END,name,length,cpgNum,gcNum,perCpg,perGc,obsExp", self.admin_key)

    expected = 'chr1\t28735\t29810\tCpG: 116\t1075\t116\t787\t21.6\t73.2\t0.83\nchr1\t135124\t135563\tCpG: 30\t439\t30\t295\t13.7\t67.2\t0.64\nchr1\t327790\t328229\tCpG: 29\t439\t29\t295\t13.2\t67.2\t0.62\nchr1\t437151\t438164\tCpG: 84\t1013\t84\t734\t16.6\t72.5\t0.64\nchr1\t449273\t450544\tCpG: 99\t1271\t99\t777\t15.6\t61.1\t0.84\nchr1\t533219\t534114\tCpG: 94\t895\t94\t570\t21\t63.7\t1.04\nchr1\t544738\t546649\tCpG: 171\t1911\t171\t1405\t17.9\t73.5\t0.67\nchr1\t713984\t714547\tCpG: 60\t563\t60\t385\t21.3\t68.4\t0.92\nchr1\t762416\t763445\tCpG: 115\t1029\t115\t673\t22.4\t65.4\t1.07\nchr1\t788863\t789211\tCpG: 28\t348\t28\t192\t16.1\t55.2\t1.06'

    self.assertSuccess(res, regions)
    self.assertEquals(expected, regions)