import helpers
import time
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
    self.assertEquals(genes, [{'transcript_status': 'NOVEL', 'gene_name': 'CR381670.1', 'gene_type': 'miRNA', 'end': 9683272, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'gene_id': 'ENSG00000238411.1', 'start': 9683191, 'transcript_id': 'ENSG00000238411.1', 'score': 0.0, 'strand': '+', 'transcript_name': 'CR381670.1', '_id': 'gn52851', 'gene_status': 'NOVEL', 'transcript_type': 'miRNA', 'chromosome': 'chr21'}])

    (s, genes) = epidb.list_genes("", "chr21", 9683191, 9683272, gene_models[0][1], self.admin_key)
    self.assertEquals(genes, [{'transcript_status': 'NOVEL', 'gene_name': 'CR381670.1', 'gene_type': 'miRNA', 'end': 9683272, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'gene_id': 'ENSG00000238411.1', 'start': 9683191, 'transcript_id': 'ENSG00000238411.1', 'score': 0.0, 'strand': '+', 'transcript_name': 'CR381670.1', '_id': 'gn52851', 'gene_status': 'NOVEL', 'transcript_type': 'miRNA', 'chromosome': 'chr21'}])

    (s, genes) = epidb.list_genes("CR381670", "chr21", None, None, gene_models[0][1], self.admin_key)
    self.assertEquals(genes, [{'transcript_status': 'NOVEL', 'gene_name': 'CR381670.1', 'gene_type': 'miRNA', 'end': 9683272, 'source': 'ENSEMBL', 'frame': '.', 'level': '3', 'gene_id': 'ENSG00000238411.1', 'start': 9683191, 'transcript_id': 'ENSG00000238411.1', 'score': 0.0, 'strand': '+', 'transcript_name': 'CR381670.1', '_id': 'gn52851', 'gene_status': 'NOVEL', 'transcript_type': 'miRNA', 'chromosome': 'chr21'}])

    (s, genes) = epidb.list_genes("Pax", None, None, None, gene_models[0][1], self.admin_key)
    self.assertEquals(len(genes), 14)

    (s, genes) = epidb.list_genes(None, "chr10", None, None, gene_models[0][1], self.admin_key)
    self.assertEquals(2260, len(genes))

  def test_gene_expression(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = gzip.open("data/fpkm/51_Hf03_BlTN_Ct_mRNA_M_1.LXPv1.20150708_genes.fpkm_tracking.gz").read()
    (s, gene_expression) = epidb.add_gene_expression("s1", 0, data, "cufflinks", "ENCODE", None, self.admin_key)

    (s, gex) = epidb.list_gene_expressions(None, None, None, self.admin_key)
    self.assertEquals(gex, [['gx1', '']])
    (s, gex) = epidb.list_gene_expressions("s1", 0, None, self.admin_key)
    self.assertEquals(gex, [['gx1', '']])
    (s, gex) = epidb.list_gene_expressions(None, None, "ENCODE", self.admin_key)
    self.assertEquals(gex, [['gx1', '']])

    (s, gex) = epidb.list_gene_expressions(None, None, None, self.admin_key)
    self.assertEquals(gex, [['gx1', '']])
    (s, gex) = epidb.list_gene_expressions("s1", 0, None, self.admin_key)
    self.assertEquals(gex, [['gx1', '']])
    (s, gex) = epidb.list_gene_expressions(None, None, "ENCODE", self.admin_key)
    self.assertEquals(gex, [['gx1', '']])

    s, user = epidb.add_user("user", "email", "institution", self.admin_key)
    (user_id, user_key) = user
    self.assertSuccess(s)
    (s, ss) = epidb.modify_user_admin(user_id, "permission_level", "GET_DATA", self.admin_key)
    self.assertSuccess(s, ss)

    (s, gex) = epidb.list_gene_expressions(None, None, None, user_key)
    self.assertEquals(gex, [])
    (s, gex) = epidb.list_gene_expressions("s1", 0, None, user_key)
    self.assertEquals(gex, [])
    (s, gex) = epidb.list_gene_expressions(None, None, "ENCODE", user_key)
    self.assertEquals(gex, "107000:Project 'ENCODE' does not exist.")

    (s, info) = epidb.info(gene_expression, self.admin_key)

    self.assertEquals(info[0], {'format': 'TRACKING_ID,GENE_ID,GENE_SHORT_NAME,FPKM,FPKM_CONF_LO,FPKM_CONF_HI,FPKM_STATUS', 'sample_info': {'biosource_name': 'K562', 'karyotype': 'cancer', 'sex': 'F'}, 'content_format': 'cufflinks', 'total_genes': 57910, 'replica': 0, 'sample_id': 's1', '_id': 'gx1', 'extra_metadata': {}, 'columns': [{'name': 'TRACKING_ID', 'column_type': 'string'}, {'name': 'GENE_ID', 'column_type': 'string'}, {'name': 'GENE_SHORT_NAME', 'column_type': 'string'}, {'name': 'FPKM', 'column_type': 'double'}, {'name': 'FPKM_CONF_LO', 'column_type': 'double'}, {'name': 'FPKM_CONF_HI', 'column_type': 'double'}, {'name': 'FPKM_STATUS', 'column_type': 'string'}]})

    #data = gzip.open("data/grape2/SP8-TH91.gene_quantification.rsem_grape2_crg.GRCh38.20150622.results.txt.gz").read()
    #(s, gene_expression) = epidb.add_gene_expression("s1", 0, data, "grape2", "ENCODE", None, self.admin_key)
    #self.assertEquals(gene_expression, "131001:A Gene Expression with sample_id 's1' and replicate '0' already exists.")

#    data = gzip.open("data/grape2/SP8-TH91.gene_quantification.rsem_grape2_crg.GRCh38.20150622.results.txt.gz").read()
#    (s, gene_expression) = epidb.add_gene_expression("s1", 1, data, "grape2", "ENCODE", None, self.admin_key)
#    self.assertSuccess(s, gene_expression)

    data = gzip.open("data/gtf/gencode.v19.annotation.ONLY_GENES.gtf.gz").read()
    (s, ss) = epidb.add_gene_model("gencode v19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (status, gx_query) = epidb.select_gene_expressions("s1", 0, "OR4G11P", "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info("gx1", user_key)
    (status, r_id) = epidb.get_regions(gx_query, info[0]["format"], self.admin_key)
    self.assertSuccess(status, r_id)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "ENSG00000240361.1\tENSG00000240361.1\tOR4G11P\t0.0000\t0.0000\t0.0000\tOK")

    (status, gx_query) = epidb.select_gene_expressions("s1", 0, ['CCR1', 'CD164', 'CD1D', 'CD2', 'CD34', 'CD3G', 'CD44'], "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info("gx1", user_key)
    (status, r_id) = epidb.get_regions(gx_query, info[0]["format"], self.admin_key)
    self.assertSuccess(status, r_id)
    regions_a = self.get_regions_request(r_id)
    self.assertEquals(regions_a, "ENSG00000135535.10\tENSG00000135535.10\tCD164\t101.3820\t98.8947\t103.8680\tOK\nENSG00000026508.12\tENSG00000026508.12\tCD44\t193.4920\t189.4020\t197.5830\tOK\nENSG00000160654.5\tENSG00000160654.5\tCD3G\t53.0051\t51.4405\t54.5696\tOK\nENSG00000163823.3\tENSG00000163823.3\tCCR1\t0.0201\t0.0000\t0.0433\tOK\nENSG00000116824.4\tENSG00000116824.4\tCD2\t90.0146\t87.9630\t92.0661\tOK\nENSG00000158473.6\tENSG00000158473.6\tCD1D\t0.0241\t0.0000\t0.0519\tOK\nENSG00000174059.12\tENSG00000174059.12\tCD34\t0.0000\t0.0000\t0.0000\tOK")

    (status, gx_query) = epidb.select_gene_expressions("s1", 0, 'CCR1', "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info("gx1", user_key)
    (status, r_id) = epidb.get_regions(gx_query, info[0]["format"], self.admin_key)
    self.assertSuccess(status, r_id)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "ENSG00000163823.3\tENSG00000163823.3\tCCR1\t0.0201\t0.0000\t0.0433\tOK")

    q1 = gx_query

    (status, gx_query) = epidb.select_gene_expressions("s1", 0, 'CD164', "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info("gx1", user_key)
    (status, r_id) = epidb.get_regions(gx_query, info[0]["format"], self.admin_key)
    self.assertSuccess(status, r_id)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "ENSG00000135535.10\tENSG00000135535.10\tCD164\t101.3820\t98.8947\t103.8680\tOK")

    self.assertTrue(q1 != gx_query)

    (s, info) = epidb.info(ss, self.admin_key)

    self.assertEquals(info[0], {'total_genes': 57820, '_id': 'gs1', 'extra_metadata': {}, 'description': 'Test One Description', 'format': 'GTF'})

    (status, gene_info) = epidb.info("gn1", self.admin_key)
    self.assertEquals(gene_info[0], {'transcript_status': 'KNOWN', 'gene_name': 'DDX11L1', 'gene_type': 'pseudogene', 'end': 14412, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'gene_id': 'ENSG00000223972.4', 'start': 11869, 'transcript_id': 'ENSG00000223972.4', 'score': 0.0, 'strand': '+', 'havana_gene': 'OTTHUMG00000000961.2', 'transcript_name': 'DDX11L1', '_id': 'gn1', 'gene_status': 'KNOWN', 'transcript_type': 'pseudogene', 'chromosome': 'chr1'})


    (status, query) = epidb.select_gene_expressions("s1", [0, 2, 10, 122], None,  "ENCODE", "gencode v19", self.admin_key)
    query_one = query

    self.assertSuccess(status, query)
    (status, filtered) = epidb.filter_regions (query, "FPKM_STATUS", "!=", "OK", "string", self.admin_key)
    self.assertSuccess(status, filtered)
    (status, filtered_chr) = epidb.filter_regions (filtered,"CHROMOSOME", "==", "chr21", "string", self.admin_key)
    self.assertSuccess(status, filtered_chr)
    (status, r_id) = epidb.get_regions(filtered_chr, "GENE_ID,FPKM_STATUS,@SAMPLE_ID,@BIOSOURCE", self.admin_key)
    self.assertSuccess(status, r_id)

    regions = self.get_regions_request(r_id)

    self.assertEquals(regions, "ENSG00000240755.1\tLOWDATA\ts1\tK562\nENSG00000256386.1\tLOWDATA\ts1\tK562\nENSG00000198743.5\tLOWDATA\ts1\tK562\nENSG00000267937.1\tLOWDATA\ts1\tK562\nENSG00000238556.1\tLOWDATA\ts1\tK562\nENSG00000255902.1\tLOWDATA\ts1\tK562\nENSG00000266692.1\tLOWDATA\ts1\tK562")


    (status, query) = epidb.select_gene_expressions("s1", [0, 2, 10, 122], None,  "", "gencode v19", user_key)

    self.assertSuccess(status, query)
    (status, filtered) = epidb.filter_regions (query, "FPKM_STATUS", "!=", "OK", "string", user_key)
    self.assertSuccess(status, filtered)
    (status, filtered_chr) = epidb.filter_regions (filtered,"CHROMOSOME", "==", "chr21", "string", user_key)
    self.assertSuccess(status, filtered_chr)
    (status, req) = epidb.get_regions(filtered_chr, "GENE_ID,FPKM_STATUS,@SAMPLE_ID,@BIOSOURCE", user_key)
    self.assertSuccess(status, r_id)

    (s, ss) = epidb.info(req, user_key)
    while ss[0]["state"] != "done" :
      time.sleep(1)
      (s, ss) = epidb.info(req, user_key)

    s, regions = epidb.get_request_data(req, user_key)

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

