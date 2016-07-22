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

    (s, query_id) = epidb.select_genes(".*", "Test One", None, None, None, self.admin_key)
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
    (s, ss) = epidb.add_gene_model("Test One", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    status, gene_models = epidb.list_gene_models(self.admin_key)
    self.assertEquals(gene_models, [['gs1', 'Test One']])

    (s, genes) = epidb.list_genes("", "chr21", 9683191, 9683272, gene_models[0][1], self.admin_key)
    self.assertEquals(genes, [{'transcript_status': 'NOVEL', 'gene_name': 'CR381670.1', 'gene_type': 'miRNA', 'end': 9683272, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'feature': 'gene', 'gene_id': 'ENSG00000238411.1', 'start': 9683191, 'score': 0.0, 'strand': '+', 'transcript_id': 'ENSG00000238411.1', 'transcript_name': 'CR381670.1', 'gene_status': 'NOVEL', 'transcript_type': 'miRNA', 'chromosome': 'chr21'}])

    (s, genes) = epidb.list_genes("", "chr21", 9683191, 9683272, gene_models[0][1], self.admin_key)
    self.assertEquals(genes, [{'transcript_status': 'NOVEL', 'gene_name': 'CR381670.1', 'gene_type': 'miRNA', 'end': 9683272, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'feature': 'gene', 'gene_id': 'ENSG00000238411.1', 'start': 9683191, 'score': 0.0, 'strand': '+', 'transcript_id': 'ENSG00000238411.1', 'transcript_name': 'CR381670.1', 'gene_status': 'NOVEL', 'transcript_type': 'miRNA', 'chromosome': 'chr21'}])

    (s, genes) = epidb.list_genes("CR381670", "chr21", None, None, gene_models[0][1], self.admin_key)
    self.assertEquals(genes, [{'transcript_status': 'NOVEL', 'gene_name': 'CR381670.1', 'gene_type': 'miRNA', 'end': 9683272, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'feature': 'gene', 'gene_id': 'ENSG00000238411.1', 'start': 9683191, 'score': 0.0, 'strand': '+', 'transcript_id': 'ENSG00000238411.1', 'transcript_name': 'CR381670.1', 'gene_status': 'NOVEL', 'transcript_type': 'miRNA', 'chromosome': 'chr21'}])

    (s, genes) = epidb.list_genes("Pax", None, None, None, gene_models[0][1], self.admin_key)
    self.assertEquals(len(genes), 14)

    (s, genes) = epidb.list_genes(None, "chr10", None, None, gene_models[0][1], self.admin_key)
    self.assertEquals(2260, len(genes))

  def test_gene_expression(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = gzip.open("data/fpkm/51_Hf03_BlTN_Ct_mRNA_M_1.LXPv1.20150708_genes.fpkm_tracking.gz").read()
    (s, gene_expression) = epidb.add_gene_expression("s1", 0, data, "cufflinks", None, self.admin_key)

    data = gzip.open("data/gtf/gencode.v19.annotation.ONLY_GENES.gtf.gz").read()
    (s, ss) = epidb.add_gene_model("gencode v19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (status, query) = epidb.select_gene_expressions("s1", [0, 2, 10, 122], None, "gencode v19", self.admin_key)
    self.assertSuccess(status, query)
    (status, filtered) = epidb.filter_regions (query, "FPKM_STATUS", "!=", "OK", "string", self.admin_key)
    self.assertSuccess(status, filtered)
    (status, filtered_chr) = epidb.filter_regions (filtered,"CHROMOSOME", "==", "chr21", "string", self.admin_key)
    self.assertSuccess(status, filtered_chr)
    (status, r_id) = epidb.get_regions(filtered_chr, "GENE_ID,FPKM_STATUS,@SAMPLE_ID,@BIOSOURCE", self.admin_key)
    self.assertSuccess(status, r_id)

    regions = self.get_regions_request(r_id)

    self.assertEquals(regions, "ENSG00000240755.1\tLOWDATA\ts1\tK562\nENSG00000256386.1\tLOWDATA\ts1\tK562\nENSG00000198743.5\tLOWDATA\ts1\tK562\nENSG00000267937.1\tLOWDATA\ts1\tK562\nENSG00000238556.1\tLOWDATA\ts1\tK562\nENSG00000255902.1\tLOWDATA\ts1\tK562\nENSG00000266692.1\tLOWDATA\ts1\tK562")

  def test_gene_re(self):
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

  def test_gene_case_insensitive(self):
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

  def test_gene_chr1_retrieve(self):
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

