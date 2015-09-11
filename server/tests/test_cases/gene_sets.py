import helpers

from client import EpidbClient


class TestGenes(helpers.TestCase):

  def test_gene_retrieve(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()

    (s, ss) = epidb.add_gene_set("Test One", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, query_id) = epidb.select_genes(["ENSG00000223972.5", "ENSG00000223972.5", "DDX11L1"], "Test One", self.admin_key)
    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEquals("chr1\t11869\t14409", regions)

    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END,@GENE_ATTRIBUTE(gene_id),@GENE_ATTRIBUTE(gene_name),@NAME,@GENE_ATTRIBUTE(noooo)", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEquals("chr1\t11869\t14409\tENSG00000223972.5\tDDX11L1\tTest One\t", regions)
