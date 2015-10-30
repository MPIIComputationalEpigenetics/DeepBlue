import helpers

from client import EpidbClient


class TestFlank(helpers.TestCase):

  def test_flank_genes(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()

    (s, ss) = epidb.add_gene_set("Test One", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, query_id) = epidb.select_genes(["ENSG00000223972.5", "ENSG00000223972.5", "DDX11L1", "RP11-34P13.7"], "Test One", self.admin_key)
    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END,STRAND", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEqual(regions, "chr1\t11869\t14409\t+\nchr1\t89295\t133723\t-")

    (s, f_id) = epidb.flank(query_id, -2500, 2000, False, self.admin_key)
    (s, r_id) = epidb.get_regions(f_id, "CHROMOSOME,START,END,STRAND", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEqual(regions, "chr1\t9369\t11369\t+\nchr1\t86795\t88795\t-")

    (s, f_id) = epidb.flank(query_id, -2500, 2000, True, self.admin_key)
    (s, r_id) = epidb.get_regions(f_id, "CHROMOSOME,START,END,STRAND", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEqual(regions, 'chr1\t9369\t11369\t+\nchr1\t136223\t138223\t-')

    (s, f_id) = epidb.flank(query_id, 2500, 2000, False, self.admin_key)
    (s, r_id) = epidb.get_regions(f_id, "CHROMOSOME,START,END,STRAND", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEqual(regions, "chr1\t16909\t18909\t+\nchr1\t136223\t138223\t-")

    (s, f_id) = epidb.flank(query_id, 2500, 2000, True, self.admin_key)
    (s, r_id) = epidb.get_regions(f_id, "CHROMOSOME,START,END,STRAND", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEqual(regions, 'chr1\t16909\t18909\t+\nchr1\t84795\t86795\t-')