import helpers
import time
import gzip

from deepblue_client import DeepBlueClient


class TestGenes(helpers.TestCase):

  def test_gene_retrieve(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()
    (s, ss) = epidb.add_gene_model("Test One", "hg19", "Test One Description", data, "GTF", {}, self.admin_key)
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

    (s, query_id) = epidb.select_genes(None, "Test One", None, None, None, self.admin_key)
    (s, request_id) = epidb.count_regions(query_id, self.admin_key)

    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END,@GENE_ATTRIBUTE(gene_id),@GENE_ATTRIBUTE(gene_name),@NAME,@GENE_ID(Test One),@GENE_NAME(Test One)", self.admin_key)
    regions = self.get_regions_request(r_id)

    for line in regions.split("\n"):
        ls = line.split("\t")
        self.assertEquals(ls[3], ls[6])
        self.assertEquals(ls[4], ls[7])

  def test_genes_location(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = gzip.open("data/gtf/gencode.v19.annotation.ONLY_GENES.gtf.gz").read()
    (s, ss) = epidb.add_gene_model("Test One", "hg19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, query_id) = epidb.select_genes(None, "Test One", ["chr1"], 1000, 15000, self.admin_key)
    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END", self.admin_key)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "chr1\t11869\t14412\nchr1\t14363\t29806")

    (s, query_id) = epidb.select_genes(None, "Test One", ["chr1", "chr11", "chr21"], 10000, 2000000, self.admin_key)
    (s, r_id) = epidb.count_regions(query_id, self.admin_key)
    count = self.get_regions_request(r_id)
    self.assertEquals(count,  {'count': 269})

    status, gene_models = epidb.list_gene_models(self.admin_key)
    self.assertEquals(gene_models, [['gs1', 'Test One']])

    (s, genes) = epidb.list_genes("", "chr21", 9683191, 9683272, gene_models[0][1], self.admin_key)
    self.assertEquals(genes, [{'transcript_status': 'NOVEL', 'gene_name': 'CR381670.1', 'gene_type': 'miRNA', 'end': 9683272, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'gene_id': 'ENSG00000238411.1', 'start': 9683191, 'transcript_id': 'ENSG00000238411.1', 'score': 0.0, 'strand': '+', 'transcript_name': 'CR381670.1', '_id': 'gn52851', 'gene_status': 'NOVEL', 'transcript_type': 'miRNA', 'chromosome': 'chr21'}])

    (s, genes) = epidb.list_genes("", "chr21", 9683191, 9683272, gene_models[0][1], self.admin_key)
    self.assertEquals(genes, [{'transcript_status': 'NOVEL', 'gene_name': 'CR381670.1', 'gene_type': 'miRNA', 'end': 9683272, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'gene_id': 'ENSG00000238411.1', 'start': 9683191, 'transcript_id': 'ENSG00000238411.1', 'score': 0.0, 'strand': '+', 'transcript_name': 'CR381670.1', '_id': 'gn52851', 'gene_status': 'NOVEL', 'transcript_type': 'miRNA', 'chromosome': 'chr21'}])

    (s, genes) = epidb.list_genes("CR381670.1", "chr21", None, None, gene_models[0][1], self.admin_key)
    self.assertEquals(genes, [{'transcript_status': 'NOVEL', 'gene_name': 'CR381670.1', 'gene_type': 'miRNA', 'end': 9683272, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'gene_id': 'ENSG00000238411.1', 'start': 9683191, 'transcript_id': 'ENSG00000238411.1', 'score': 0.0, 'strand': '+', 'transcript_name': 'CR381670.1', '_id': 'gn52851', 'gene_status': 'NOVEL', 'transcript_type': 'miRNA', 'chromosome': 'chr21'}])

    (s, genes) = epidb.list_genes(None, "chr10", None, None, gene_models[0][1], self.admin_key)
    self.assertEquals(2260, len(genes))

  def test_select_genes(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()

    (s, ss) = epidb.add_gene_model("Test One", "hg19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, ss) = epidb.list_genes(["ENSG00000279457.3"], None, None, None, "Test One", self.admin_key)
    self.assertEquals(ss, [{'gene_name': 'FO538757.2', 'gene_type': 'protein_coding', 'end': 200322, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'tag': 'ncRNA_host', 'gene_id': 'ENSG00000279457.3', 'start': 184923, 'score': 0.0, 'strand': '-', '_id': 'gn20', 'gene_status': 'KNOWN', 'chromosome': 'chr1'}])

    (s, ss) = epidb.list_genes("ENSG00000279457", "chr1", None, None, "Test One", self.admin_key)
    self.assertEquals(ss, [])

    (s, ss) = epidb.list_genes(None, None, None, None, "Test One", self.admin_key)
    self.assertEquals(20, len(ss))

    (s, query_id) = epidb.select_genes(["RP11-34P13.7"], "Test One", None, None, None, self.admin_key)
    (s, req) = epidb.count_regions(query_id, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 1)

    (s, new_query_id) = epidb.select_genes(["RP11-34P13.7"], "Test One", None, None, None, self.admin_key)
    self.assertEquals(query_id, new_query_id)

    (s, new_query_id) = epidb.select_genes("RP11-34P13.7", "Test One", None, None, None, self.admin_key)
    self.assertEquals(query_id, new_query_id)

    (s, query_id) = epidb.select_genes(["RP11-34P13.234"], "Test One", None, None, None, self.admin_key)
    (s, req) = epidb.count_regions(query_id, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 0)

    (s, query_id) = epidb.select_genes(["RP11-34P13"], "Test One", None, None, None, self.admin_key)
    (s, req) = epidb.count_regions(query_id, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 0)

    status, gene_models = epidb.list_gene_models(self.admin_key)
    self.assertEquals(gene_models, [['gs1', 'Test One']])

  def test_gene_case_insensitive(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()

    (s, ss) = epidb.add_gene_model("Test One", "hg19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, query_id) = epidb.select_genes(["RP11-34P13.7"], "Test One", None, None, None, self.admin_key)
    (s, req) = epidb.count_regions(query_id, self.admin_key)
    count = self.count_request(req)
    self.assertEquals(count, 1)

    status, gene_models = epidb.list_gene_models(self.admin_key)
    self.assertEquals(gene_models, [['gs1', 'Test One']])
