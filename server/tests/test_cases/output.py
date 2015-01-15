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
      "length:Integer",
      "cpgNum:Integer",
      "gcNum:Integer",
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
    self.assertEquals("Unable to find the column 'chr:start:end:name:length:cpgNum:gcNum:perCpg:perGc:obsExp' in the dataset format or in the DeepBlue columns.", regions)

    res, regions = epidb.get_regions(qid_1, "CHROMOSOME,START,END,name,length,cpgNum,gcNum,perCpg,perGc,obsExp", self.admin_key)

    expected = """chr1\t28735\t29810\tCpG: 116\t1075\t116\t787\t21.6000\t73.2000\t0.8300
chr1\t135124\t135563\tCpG: 30\t439\t30\t295\t13.7000\t67.2000\t0.6400
chr1\t327790\t328229\tCpG: 29\t439\t29\t295\t13.2000\t67.2000\t0.6200
chr1\t437151\t438164\tCpG: 84\t1013\t84\t734\t16.6000\t72.5000\t0.6400
chr1\t449273\t450544\tCpG: 99\t1271\t99\t777\t15.6000\t61.1000\t0.8400
chr1\t533219\t534114\tCpG: 94\t895\t94\t570\t21.0000\t63.7000\t1.0400
chr1\t544738\t546649\tCpG: 171\t1911\t171\t1405\t17.9000\t73.5000\t0.6700
chr1\t713984\t714547\tCpG: 60\t563\t60\t385\t21.3000\t68.4000\t0.9200
chr1\t762416\t763445\tCpG: 115\t1029\t115\t673\t22.4000\t65.4000\t1.0700
chr1\t788863\t789211\tCpG: 28\t348\t28\t192\t16.1000\t55.2000\t1.0600"""

    self.assertSuccess(res, regions)
    self.assertEquals(expected, regions)