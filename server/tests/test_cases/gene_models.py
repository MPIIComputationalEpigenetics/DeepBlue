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
    (s, request_id) = epidb.count_regions(query_id, self.admin_key)

    (s, genes) = epidb.list_genes(gene_models[0][1], self.admin_key)
    self.assertSuccess(s, genes)
    self.assertEquals(genes, [{'gene_name': 'DDX11L1', 'gene_status': 'KNOWN', 'end': 14409, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000223972.5', 'start': 11869, 'score': 0.0, 'strand': '+', 'havana_gene': 'OTTHUMG00000000961.2', 'gene_type': 'transcribed_unprocessed_pseudogene', 'chromosome': 'chr1'}, {'gene_name': 'WASH7P', 'gene_status': 'KNOWN', 'end': 29570, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000227232.5', 'start': 14404, 'score': 0.0, 'strand': '-', 'havana_gene': 'OTTHUMG00000000958.1', 'gene_type': 'unprocessed_pseudogene', 'chromosome': 'chr1'}, {'gene_name': 'MIR6859-1', 'gene_status': 'KNOWN', 'end': 17436, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'feature': 'gene', 'gene_id': 'ENSG00000278267.1', 'start': 17369, 'score': 0.0, 'strand': '-', 'gene_type': 'miRNA', 'chromosome': 'chr1'}, {'gene_name': 'RP11-34P13.3', 'gene_status': 'KNOWN', 'end': 31109, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'tag': 'ncRNA_host', 'feature': 'gene', 'gene_id': 'ENSG00000243485.3', 'start': 29554, 'score': 0.0, 'strand': '+', 'havana_gene': 'OTTHUMG00000000959.2', 'gene_type': 'lincRNA', 'chromosome': 'chr1'}, {'gene_name': 'MIR1302-2', 'gene_status': 'KNOWN', 'end': 30503, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'feature': 'gene', 'gene_id': 'ENSG00000274890.1', 'start': 30366, 'score': 0.0, 'strand': '+', 'gene_type': 'miRNA', 'chromosome': 'chr1'}, {'gene_name': 'FAM138A', 'gene_status': 'KNOWN', 'end': 36081, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000237613.2', 'start': 34554, 'score': 0.0, 'strand': '-', 'havana_gene': 'OTTHUMG00000000960.1', 'gene_type': 'lincRNA', 'chromosome': 'chr1'}, {'gene_name': 'OR4G4P', 'gene_status': 'KNOWN', 'end': 53312, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000268020.3', 'start': 52473, 'score': 0.0, 'strand': '+', 'havana_gene': 'OTTHUMG00000185779.1', 'gene_type': 'unprocessed_pseudogene', 'chromosome': 'chr1'}, {'gene_name': 'OR4G11P', 'gene_status': 'KNOWN', 'end': 63887, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000240361.1', 'start': 62948, 'score': 0.0, 'strand': '+', 'havana_gene': 'OTTHUMG00000001095.2', 'gene_type': 'unprocessed_pseudogene', 'chromosome': 'chr1'}, {'gene_name': 'OR4F5', 'gene_status': 'KNOWN', 'end': 70008, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000186092.4', 'start': 69091, 'score': 0.0, 'strand': '+', 'havana_gene': 'OTTHUMG00000001094.1', 'gene_type': 'protein_coding', 'chromosome': 'chr1'}, {'gene_name': 'RP11-34P13.7', 'gene_status': 'KNOWN', 'end': 133723, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000238009.6', 'start': 89295, 'score': 0.0, 'strand': '-', 'havana_gene': 'OTTHUMG00000001096.2', 'gene_type': 'lincRNA', 'chromosome': 'chr1'}, {'gene_name': 'RP11-34P13.8', 'gene_status': 'KNOWN', 'end': 91105, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000239945.1', 'start': 89551, 'score': 0.0, 'strand': '-', 'havana_gene': 'OTTHUMG00000001097.2', 'gene_type': 'lincRNA', 'chromosome': 'chr1'}, {'gene_name': 'CICP27', 'gene_status': 'KNOWN', 'end': 134836, 'source': 'HAVANA', 'frame': '.', 'level': '1', 'tag': 'pseudo_consens', 'feature': 'gene', 'gene_id': 'ENSG00000233750.3', 'start': 131025, 'score': 0.0, 'strand': '+', 'havana_gene': 'OTTHUMG00000001257.3', 'gene_type': 'processed_pseudogene', 'chromosome': 'chr1'}, {'gene_name': 'RP11-34P13.15', 'gene_status': 'KNOWN', 'end': 135895, 'source': 'HAVANA', 'frame': '.', 'level': '1', 'tag': 'pseudo_consens', 'feature': 'gene', 'gene_id': 'ENSG00000268903.1', 'start': 135141, 'score': 0.0, 'strand': '-', 'havana_gene': 'OTTHUMG00000182518.2', 'gene_type': 'processed_pseudogene', 'chromosome': 'chr1'}, {'gene_name': 'RP11-34P13.16', 'gene_status': 'KNOWN', 'end': 137965, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000269981.1', 'start': 137682, 'score': 0.0, 'strand': '-', 'havana_gene': 'OTTHUMG00000182738.2', 'gene_type': 'processed_pseudogene', 'chromosome': 'chr1'}, {'gene_name': 'RP11-34P13.14', 'gene_status': 'KNOWN', 'end': 140339, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000239906.1', 'start': 139790, 'score': 0.0, 'strand': '-', 'havana_gene': 'OTTHUMG00000002481.1', 'gene_type': 'antisense', 'chromosome': 'chr1'}, {'gene_name': 'RP11-34P13.13', 'gene_status': 'KNOWN', 'end': 173862, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'tag': 'ncRNA_host', 'feature': 'gene', 'gene_id': 'ENSG00000241860.6', 'start': 141474, 'score': 0.0, 'strand': '-', 'havana_gene': 'OTTHUMG00000002480.3', 'gene_type': 'processed_transcript', 'chromosome': 'chr1'}, {'gene_name': 'RNU6-1100P', 'gene_status': 'KNOWN', 'end': 157887, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'feature': 'gene', 'gene_id': 'ENSG00000222623.1', 'start': 157784, 'score': 0.0, 'strand': '-', 'gene_type': 'snRNA', 'chromosome': 'chr1'}, {'gene_name': 'RP11-34P13.9', 'gene_status': 'KNOWN', 'end': 161525, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'feature': 'gene', 'gene_id': 'ENSG00000241599.1', 'start': 160446, 'score': 0.0, 'strand': '+', 'havana_gene': 'OTTHUMG00000002525.1', 'gene_type': 'lincRNA', 'chromosome': 'chr1'}, {'gene_name': 'FO538757.3', 'gene_status': 'KNOWN', 'end': 184158, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'feature': 'gene', 'gene_id': 'ENSG00000279928.1', 'start': 182393, 'score': 0.0, 'strand': '+', 'gene_type': 'protein_coding', 'chromosome': 'chr1'}, {'gene_name': 'FO538757.2', 'gene_status': 'KNOWN', 'end': 200322, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'tag': 'ncRNA_host', 'feature': 'gene', 'gene_id': 'ENSG00000279457.3', 'start': 184923, 'score': 0.0, 'strand': '-', 'gene_type': 'protein_coding', 'chromosome': 'chr1'}])

    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END,@GENE_ATTRIBUTE(gene_id),@GENE_ATTRIBUTE(gene_name),@NAME,@GENE_ID(Test One),@GENE_NAME(Test One)", self.admin_key)
    regions = self.get_regions_request(r_id)

    for line in regions.split("\n"):
        print line
        ls = line.split("\t")
        self.assertEquals(ls[3], ls[6])
        self.assertEquals(ls[4], ls[7])

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

