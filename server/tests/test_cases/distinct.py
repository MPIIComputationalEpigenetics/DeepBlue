import helpers
import gzip

from deepblue_client import DeepBlueClient


class TestDistinct(helpers.TestCase):

  def test_basic_distinct(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = None
    with open("data/wgEncodeBroadHmmGm12878HMM.bed", 'r') as f:
      data = f.read()

    sample_id = self.sample_ids[0]
    fmt = "CHROMOSOME,START,END,NAME,SCORE,STRAND,THICK_START,THICK_END,ITEM_RGB"
    ## It is a Chromatin State Segmentation data, but just feeling these metadata...
    (res, a_1) = epidb.add_experiment("test", "hg19", "H3K4me3", sample_id, "tech1", "ENCODE", "wgEncodeBroadHistoneH1hescH3k27me3StdPk.bed from ENCODE",  data, fmt, None, self.admin_key)

    res, qid = epidb.select_regions("test", "hg19", None, None, None,
                                 None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    status, req = epidb.distinct_column_values(qid, "NAME", self.admin_key)
    self.assertSuccess(status, req)

    distinct = self.get_regions_request(req)
    self.assertEqual(distinct, {'distinct': {'13_Heterochrom/lo': 75112, '2_Weak_Promoter': 35065, '7_Weak_Enhancer': 109468, '15_Repetitive/CNV': 6128, '12_Repressed': 25483, '4_Strong_Enhancer': 25486, '5_Strong_Enhancer': 38604, '11_Weak_Txn': 82312, '3_Poised_Promoter': 5263, '8_Insulator': 33265, '14_Repetitive/CNV': 8028, '1_Active_Promoter': 15278, '6_Weak_Enhancer': 69111, '10_Txn_Elongation': 26509, '9_Txn_Transition': 16227}})