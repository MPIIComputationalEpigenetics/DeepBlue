import helpers
import gzip

from deepblue_client import DeepBlueClient


class TestGenes(helpers.TestCase):

  def test_gene_retrieve(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()

    (s, ss) = epidb.add_gene_model("Test One", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, query_id) = epidb.select_genes(["ENSG00000223972.5", "ENSG00000223972.5", "DDX11L1"], "Test One", None, None, None, self.admin_key)
    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEquals("chr1\t11869\t14409", regions)

    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END,@GENE_ATTRIBUTE(gene_id),@GENE_ATTRIBUTE(gene_name),@NAME", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEquals("chr1\t11869\t14409\tENSG00000223972.5\tDDX11L1\tTest One", regions)


    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END,@GENE_ATTRIBUTE(gene_id),@GENE_ATTRIBUTE(gene_name),@NAME,@GENE_ATTRIBUTE(noooo)", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEquals("chr1\t11869\t14409\tENSG00000223972.5\tDDX11L1\tTest One\t", regions)

    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,SOURCE,FEATURE,START,END,GTF_SCORE,STRAND,FRAME,GTF_ATTRIBUTES,@GENE_ATTRIBUTE(gene_name)", self.admin_key)

    regions = self.get_regions_request(r_id)
    self.assertEquals('chr1\tHAVANA\tgene\t11869\t14409\t.\t+\t.\tgene_id "ENSG00000223972.5"; gene_name "DDX11L1"; gene_status "KNOWN"; gene_type "transcribed_unprocessed_pseudogene"; havana_gene "OTTHUMG00000000961.2"; level "2"\tDDX11L1', regions)

    status, gene_models = epidb.list_gene_models(self.admin_key)
    self.assertEquals(gene_models, [['gs1', 'Test One']])

    (s, query_id) = epidb.select_genes(".*", "Test One", None, None, None, self.admin_key)
    print s, query_id
    (s, request_id) = epidb.count_regions(query_id, self.admin_key)
    print 'count genes:'
    print self.count_request(request_id)

    print gene_models[0][1]
    print epidb.list_genes(gene_models[0][1], self.admin_key)

  def _test_gene_re(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()

    (s, ss) = epidb.add_gene_model("Test One", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, query_id) = epidb.select_genes(["RP11-34P13.7"], "Test One", None, None, None, self.admin_key)
    (s, req) = epidb.count_regions(query_id, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 1)

    (s, query_id) = epidb.select_genes(["RP11-34P13.234"], "Test One", None, None, None, self.admin_key)
    (s, req) = epidb.count_regions(query_id, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 0)

    (s, query_id) = epidb.select_genes(["RP11-34P13"], "Test One", None, None, None, self.admin_key)
    (s, req) = epidb.count_regions(query_id, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 8)

    (s, query_id) = epidb.select_genes(["RP1?"], "Test One", None, None, None, self.admin_key)
    (s, req) = epidb.count_regions(query_id, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 8)

    status, gene_models = epidb.list_gene_models(self.admin_key)
    self.assertEquals(gene_models, [['gs1', 'Test One']])

  def _test_gene_case_insensitive(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()

    (s, ss) = epidb.add_gene_model("Test One", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, query_id) = epidb.select_genes(["Rp11-34p13.7"], "Test One", None, None, None, self.admin_key)
    (s, req) = epidb.count_regions(query_id, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 1)

    status, gene_models = epidb.list_gene_models(self.admin_key)
    self.assertEquals(gene_models, [['gs1', 'Test One']])

  def _test_gene_chr1_retrieve(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = gzip.open("data/gtf/gencode.v19.annotation.ONLY_GENES.gtf.gz").read()

    (s, ss) = epidb.add_gene_model("Test One", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, query_id) = epidb.select_genes(".*", "Test One", ["chr1"], 1000, 15000, self.admin_key)
    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "chr1\t11869\t14412\nchr1\t14363\t29806")

    (s, query_id) = epidb.select_genes(".*", "Test One", ["chr1", "chr11", "chr21"], 10000, 2000000, self.admin_key)
    (s, r_id) = epidb.count_regions(query_id, self.admin_key)
    count = self.get_regions_request(r_id)
    self.assertEquals(count,  {'count': 269})

