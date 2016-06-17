import helpers

from deepblue_client import DeepBlueClient

class TestCoverageCommand(helpers.TestCase):

  def test_coverage(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    file_data = None
    with open("data/cpgIslandExtFull.txt", 'r') as f:
      file_data = f.read()

    cpg_island =  ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "LENGTH",
      "NUM_CPG",
      "NUM_GC",
      "PER_CPG",
      "PER_CG",
      "OBS_EXP"
      ])

    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
          file_data,
          cpg_island,
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)

    res, qid = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
    self.assertSuccess(res, qid)
    status, req = epidb.coverage(qid, "hg19", self.admin_key)
    coverage = self.get_regions_request(req)
    self.assertEqual(coverage,  {'coverages': {'chr8_gl000197_random': {'total': 3140, 'coverage': 8.4465, 'size': 37175}, 'chrUn_gl000229': {'total': 1962, 'coverage': 9.8529, 'size': 19913}, 'chr6_apd_hap1': {'total': 39175, 'coverage': 0.8475, 'size': 4622290}, 'chrUn_gl000223': {'total': 2949, 'coverage': 1.6342, 'size': 180455}, 'chrUn_gl000236': {'total': 1060, 'coverage': 2.5278, 'size': 41934}, 'chr13': {'total': 453125, 'coverage': 0.3934, 'size': 115169878}, 'chr12': {'total': 851919, 'coverage': 0.6365, 'size': 133851895}, 'chr11': {'total': 1036273, 'coverage': 0.7676, 'size': 135006516}, 'chr10': {'total': 966963, 'coverage': 0.7134, 'size': 135534747}, 'chr17': {'total': 1274581, 'coverage': 1.5698, 'size': 81195210}, 'chr16': {'total': 1081705, 'coverage': 1.1972, 'size': 90354753}, 'chr15': {'total': 665790, 'coverage': 0.6494, 'size': 102531392}, 'chr14': {'total': 622989, 'coverage': 0.5803, 'size': 107349540}, 'chr19': {'total': 1624580, 'coverage': 2.7475, 'size': 59128983}, 'chr18': {'total': 439689, 'coverage': 0.5631, 'size': 78077248}, 'chrUn_gl000228': {'total': 49978, 'coverage': 38.7066, 'size': 129120}, 'chrUn_gl000243': {'total': 296, 'coverage': 0.683, 'size': 43341}, 'chr9_gl000200_random': {'total': 223, 'coverage': 0.1192, 'size': 187035}, 'chr6_mann_hap4': {'total': 74514, 'coverage': 1.5911, 'size': 4683263}, 'chrUn_gl000237': {'total': 312, 'coverage': 0.6802, 'size': 45867}, 'chrUn_gl000213': {'total': 4465, 'coverage': 2.7186, 'size': 164239}, 'chrUn_gl000235': {'total': 406, 'coverage': 1.1777, 'size': 34474}, 'chrUn_gl000211': {'total': 1413, 'coverage': 0.8483, 'size': 166566}, 'chr17_gl000204_random': {'total': 2683, 'coverage': 3.2997, 'size': 81310}, 'chrUn_gl000212': {'total': 4931, 'coverage': 2.6389, 'size': 186858}, 'chrUn_gl000215': {'total': 5660, 'coverage': 3.2803, 'size': 172545}, 'chrUn_gl000214': {'total': 636, 'coverage': 0.4618, 'size': 137718}, 'chrUn_gl000217': {'total': 3354, 'coverage': 1.9483, 'size': 172149}, 'chrUn_gl000216': {'total': 1341, 'coverage': 0.7783, 'size': 172294}, 'chr6_dbb_hap3': {'total': 83010, 'coverage': 1.8005, 'size': 4610396}, 'chrUn_gl000218': {'total': 2063, 'coverage': 1.2802, 'size': 161147}, 'chr6_cox_hap2': {'total': 92751, 'coverage': 1.9342, 'size': 4795371}, 'chrUn_gl000219': {'total': 1778, 'coverage': 0.9922, 'size': 179198}, 'chr7': {'total': 1147864, 'coverage': 0.7213, 'size': 159138663}, 'chr1_gl000191_random': {'total': 1570, 'coverage': 1.4751, 'size': 106433}, 'chr4_ctg9_hap1': {'total': 617, 'coverage': 0.1045, 'size': 590426}, 'chrUn_gl000222': {'total': 4795, 'coverage': 2.5661, 'size': 186861}, 'chr17_ctg5_hap1': {'total': 30127, 'coverage': 1.7924, 'size': 1680828}, 'chr11_gl000202_random': {'total': 257, 'coverage': 0.6408, 'size': 40103}, 'chr6_qbl_hap6': {'total': 90147, 'coverage': 1.9546, 'size': 4611984}, 'chr22': {'total': 562123, 'coverage': 1.0957, 'size': 51304566}, 'chrY': {'total': 115810, 'coverage': 0.1951, 'size': 59373566}, 'chr20': {'total': 612174, 'coverage': 0.9713, 'size': 63025520}, 'chr21': {'total': 268220, 'coverage': 0.5573, 'size': 48129895}, 'chr6_mcf_hap5': {'total': 76009, 'coverage': 1.5726, 'size': 4833398}, 'chr17_gl000205_random': {'total': 301, 'coverage': 0.1724, 'size': 174588}, 'chrX': {'total': 732552, 'coverage': 0.4718, 'size': 155270560}, 'chr2': {'total': 1379397, 'coverage': 0.5672, 'size': 243199373}, 'chr4_gl000193_random': {'total': 602, 'coverage': 0.3172, 'size': 189789}, 'chr6': {'total': 942464, 'coverage': 0.5508, 'size': 171115067}, 'chr5': {'total': 966173, 'coverage': 0.534, 'size': 180915260}, 'chr4': {'total': 837951, 'coverage': 0.4384, 'size': 191154276}, 'chr3': {'total': 882783, 'coverage': 0.4458, 'size': 198022430}, 'chr1': {'total': 1881629, 'coverage': 0.7549, 'size': 249250621}, 'chr1_gl000192_random': {'total': 2945, 'coverage': 0.5379, 'size': 547496}, 'chr9_gl000199_random': {'total': 246, 'coverage': 0.1448, 'size': 169874}, 'chrUn_gl000224': {'total': 287, 'coverage': 0.1597, 'size': 179693}, 'chr4_gl000194_random': {'total': 1920, 'coverage': 1.0028, 'size': 191469}, 'chr9_gl000201_random': {'total': 1834, 'coverage': 5.0736, 'size': 36148}, 'chrUn_gl000220': {'total': 17252, 'coverage': 10.6624, 'size': 161802}, 'chrUn_gl000221': {'total': 303, 'coverage': 0.195, 'size': 155397}, 'chr9': {'total': 941315, 'coverage': 0.6666, 'size': 141213431}, 'chr8': {'total': 864319, 'coverage': 0.5905, 'size': 146364022}, 'chrUn_gl000225': {'total': 9599, 'coverage': 4.5456, 'size': 211173}, 'chr6_ssto_hap7': {'total': 70928, 'coverage': 1.4391, 'size': 4928567}, 'chrUn_gl000242': {'total': 257, 'coverage': 0.5905, 'size': 43523}, 'chrUn_gl000231': {'total': 1240, 'coverage': 4.5279, 'size': 27386}, 'chrUn_gl000240': {'total': 206, 'coverage': 0.4913, 'size': 41933}, 'chrUn_gl000241': {'total': 812, 'coverage': 1.9264, 'size': 42152}}})

    self.insert_experiment(epidb, "hg19_big_1")

    res, qid = epidb.select_experiments("hg19_big_1", None, None, None, self.admin_key)
    self.assertSuccess(res, qid)
    status, req = epidb.coverage(qid, "hg19", self.admin_key)
    coverage = self.get_regions_request(req)
    self.assertEqual(coverage, {'coverages': {'chrX': {'total': 1483495, 'coverage': 0.9554, 'size': 155270560}, 'chr13': {'total': 1132457, 'coverage': 0.9833, 'size': 115169878}, 'chr12': {'total': 2964677, 'coverage': 2.2149, 'size': 133851895}, 'chr11': {'total': 2899893, 'coverage': 2.148, 'size': 135006516}, 'chr10': {'total': 2420026, 'coverage': 1.7855, 'size': 135534747}, 'chr17': {'total': 3111151, 'coverage': 3.8317, 'size': 81195210}, 'chr16': {'total': 2134123, 'coverage': 2.3619, 'size': 90354753}, 'chr15': {'total': 1820040, 'coverage': 1.7751, 'size': 102531392}, 'chr14': {'total': 1692055, 'coverage': 1.5762, 'size': 107349540}, 'chr19': {'total': 3237248, 'coverage': 5.4749, 'size': 59128983}, 'chr18': {'total': 1012202, 'coverage': 1.2964, 'size': 78077248}, 'chr22': {'total': 1204818, 'coverage': 2.3484, 'size': 51304566}, 'chr20': {'total': 1436177, 'coverage': 2.2787, 'size': 63025520}, 'chr21': {'total': 589069, 'coverage': 1.2239, 'size': 48129895}, 'chr7': {'total': 2623790, 'coverage': 1.6487, 'size': 159138663}, 'chr6': {'total': 3309649, 'coverage': 1.9342, 'size': 171115067}, 'chr5': {'total': 2737174, 'coverage': 1.513, 'size': 180915260}, 'chr4': {'total': 2204447, 'coverage': 1.1532, 'size': 191154276}, 'chr3': {'total': 3096812, 'coverage': 1.5639, 'size': 198022430}, 'chr2': {'total': 3900776, 'coverage': 1.6039, 'size': 243199373}, 'chr1': {'total': 5292444, 'coverage': 2.1233, 'size': 249250621}, 'chr9': {'total': 2336737, 'coverage': 1.6548, 'size': 141213431}, 'chr8': {'total': 2050838, 'coverage': 1.4012, 'size': 146364022}}})