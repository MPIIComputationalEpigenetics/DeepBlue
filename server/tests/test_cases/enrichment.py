import helpers

from deepblue_client import DeepBlueClient

import math

class TestEnrichment(helpers.TestCase):

    def test_enriochment_overlap(self):
      epidb = DeepBlueClient(address="localhost", port=31415)
      self.init_base(epidb)

      res = epidb.add_technique("ChIP-seq", "ChIP-sequencing", {}, self.admin_key)
      self.assertSuccess(res)

      sample_id = self.sample_ids[0]

      broad_peak_format = ",".join([
          "CHROMOSOME",
          "START",
          "END",
          "NAME",
          "SCORE",
          "STRAND",
          "SIGNAL_VALUE",
          "P_VALUE",
          "Q_VALUE",
      ])

      with open("data/wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed", 'r') as f:
          file_data = f.read()
          (res, exp) = epidb.add_experiment("wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed", "hg19", "H3k4me3", "s1", "ChIPseq",
                                            "ENCODE", "wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed from ENCODE",  file_data, broad_peak_format, None, self.admin_key)
          self.assertSuccess(res, exp)
          res, q_exp = epidb.select_experiments(
              "wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed", "chr1", None, None, self.admin_key)
          self.assertSuccess(res, q_exp)


      ## Testting with all overlaps

      res, q_tiling = epidb.tiling_regions(10000, "hg19", None, self.admin_key)
      res, r_id = epidb.enrich_regions_overlap(q_exp, q_tiling,
        {"H3k4me3": ["wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed"]}, "hg19", self.admin_key)

      result = self.get_regions_request(r_id)
      self.assertEquals(result['enrichment']['results'][0]['p_value_log'], float('Inf'))
      self.assertEquals(result, {'enrichment': {'count_query_regions': 3074, 'count_universe_regions': 313669, 'results': [{'c': 0, 'b': 30861, 'description': '', 'p_value_log': float('Inf'), 'experiment_size': 33270, 'database_name': 'H3k4me3', 'max_rank': 1, 'support_rank': 1, 'dataset': 'wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed', 'biosource': 'K562', 'odd_rank': 1, 'odds_ratio': float('Inf'), 'epigenetic_mark': 'H3k4me3', 'mean_rank': 1.0, 'log_rank': 1, 'support': 3066, 'd': 279742, 'msg': '', 'error': False}]}})


      ## Testting without any overlap
      _, q_input = epidb.input_regions("hg19", "chr1\t1\t2", self.admin_key)
      res, r_id = epidb.enrich_regions_overlap(q_input, q_tiling,
        {"H3k4me3": ["wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed"]}, "hg19", self.admin_key)

      result = self.get_regions_request(r_id)
      self.assertEquals(result['enrichment']['results'][0]['p_value_log'], 0.0)
      self.assertEquals({'enrichment': {'count_query_regions': 1, 'count_universe_regions': 313669, 'results': [{'c': 1, 'b': 33927, 'description': '', 'p_value_log': 0.0, 'experiment_size': 33270, 'database_name': 'H3k4me3', 'max_rank': 1, 'support_rank': 1, 'dataset': 'wgEncodeBroadHistoneH1hescH3k4me3StdPk.bed', 'biosource': 'K562', 'odd_rank': 1, 'odds_ratio': 0.0, 'epigenetic_mark': 'H3k4me3', 'mean_rank': 1.0, 'log_rank': 1, 'support': 0, 'd': 279741, 'error': False, 'msg': ''}]}}, result)
