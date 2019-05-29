import helpers
import time
import gzip

from deepblue_client import DeepBlueClient


class TestExpressions(helpers.TestCase):

  def test_duplicated(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    (s, project) = epidb.add_project("DEEP", "Deutsche Epigenom", self.admin_key)
    self.assertSuccess(s, project)

    data = gzip.open("data/gtf/repetitive.gtf.gz").read()
    (s, ss) = epidb.add_gene_model("gencode v19", "hg19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    data = gzip.open("data/fpkm/duplicate_reverse_tracking.gz").read()
    (s, gene_expression) = epidb.add_expression("gene", "s2", 1, data, "cufflinks", "DEEP", None, self.admin_key)
    self.assertSuccess(s, gene_expression)

    (status, gx_query) = epidb.select_expressions("gene", "s2", 1, None, "DEEP", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)

    (status, r_id) = epidb.get_regions(gx_query, "@GENE_ID(gencode v19),@GENE_NAME(gencode v19),FPKM,CHROMOSOME,START,END,FPKM,@BIOSOURCE,@SAMPLE_ID,@STRAND", self.admin_key)
    self.assertSuccess(status, r_id)
    data = self.get_regions_request(r_id)

    self.assertEquals(data, 'ENSG00000240453.1\tRP11-206L10.10\t2.7720\tchr1\t745489\t753092\t2.7720\tBrain\ts2\t-\nENSG00000240453_REVERSE\tRP11-206L10_REVERSE\t9.1235\tchr1\t745489\t753092\t9.1235\tBrain\ts2\t+')

  def test_performance(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    (s, project) = epidb.add_project("DEEP", "Deutsche Epigenom", self.admin_key)
    self.assertSuccess(s, project)

    data = gzip.open("data/gtf/gencode.v19.annotation.ONLY_GENES.gtf.gz").read()
    (s, ss) = epidb.add_gene_model("gencode v19", "hg19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    data = gzip.open("data/fpkm/51_Hf03_BlTN_Ct_mRNA_M_1.LXPv1.20150708_genes.fpkm_tracking.gz").read()
    (s, gene_expression) = epidb.add_expression("gene", "s2", 1, data, "cufflinks", "DEEP", None, self.admin_key)
    self.assertSuccess(s, gene_expression)

    (status, gx_query) = epidb.select_expressions("gene", "s2", 1, None, "DEEP", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)

    (status, r_id) = epidb.get_regions(gx_query, "@GENE_ID(gencode v19),@GENE_NAME(gencode v19),FPKM,CHROMOSOME,START,END,FPKM,@BIOSOURCE,@SAMPLE_ID,@STRAND", self.admin_key)
    self.assertSuccess(status, r_id)
    data = self.get_regions_request(r_id)

  def test_gene_retrieve(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()
    (s, ss) = epidb.add_gene_model("Test One", "hg19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, query_id) = epidb.select_genes(["ENSG00000223972.5", "ENSG00000223972.5", "DDX11L1"], "", "Test One", None, None, None, self.admin_key)
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

    (s, query_id) = epidb.select_genes(None, "", "Test One", None, None, None, self.admin_key)
    (s, request_id) = epidb.count_regions(query_id, self.admin_key)

    (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END,@GENE_ATTRIBUTE(gene_id),@GENE_ATTRIBUTE(gene_name),@NAME,@GENE_ID(Test One),@GENE_NAME(Test One)", self.admin_key)
    regions = self.get_regions_request(r_id)

    for line in regions.split("\n"):
        ls = line.split("\t")
        self.assertEquals(ls[3], ls[6])
        self.assertEquals(ls[4], ls[7])

  def test_gene_expression(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    (s, project) = epidb.add_project("DEEP", "Deutsche Epigenom", self.admin_key)
    self.assertSuccess(s, project)

    data = gzip.open("data/fpkm/small_1.fpkm_tracking.gz").read()
    (s, gene_expression) = epidb.add_expression("gene", "s2", 1, data, "cufflinks", "DEEP", None, self.admin_key)
    self.assertSuccess(s, gene_expression)

    data = gzip.open("data/fpkm/small_2.fpkm_tracking.gz").read()
    (s, gene_expression) = epidb.add_expression("gene", "s2", 2, data, "cufflinks", "DEEP", None, self.admin_key)
    self.assertSuccess(s, gene_expression)

    data = gzip.open("data/fpkm/small_3.fpkm_tracking.gz").read()
    (s, gene_expression) = epidb.add_expression("gene", "s2", 44, data, "cufflinks", "DEEP", None, self.admin_key)
    self.assertSuccess(s, gene_expression)

    (s, gex) = epidb.list_expressions("gene", "s1", None, None, self.admin_key)
    self.assertEquals(gex, [])

    (s, gex) = epidb.list_expressions("gene", "s2", None, None, self.admin_key)
    self.assertEquals(gex, [['gx1', ''], ['gx2', ''], ['gx3', '']])

    (s, gex) = epidb.list_expressions("gene", "s2", [1, 2], None, self.admin_key)
    self.assertEquals(gex, [['gx1', ''],['gx2', '']])

    (s, gex) = epidb.list_expressions("gene", "s2", 44, None, self.admin_key)
    self.assertEquals(gex, [['gx3', '']])

    (s, gex) = epidb.list_expressions("gene", None, 1, "DEEP", self.admin_key)
    self.assertEquals(gex, [['gx1', '']])

    data = gzip.open("data/fpkm/51_Hf03_BlTN_Ct_mRNA_M_1.LXPv1.20150708_genes.fpkm_tracking.gz").read()
    (s, gene_expression) = epidb.add_expression("gene", "s1", 1, data, "cufflinks", "ENCODE", None, self.admin_key)
    self.assertSuccess(s, gene_expression)

    (s, gex) = epidb.list_expressions("gene", None, None, None, self.admin_key)
    self.assertEquals(gex, [['gx1', ''], ['gx2', ''], ['gx3', ''], ['gx4', '']])
    (s, gex) = epidb.list_expressions("gene", "s1", 1, None, self.admin_key)
    self.assertEquals(gex, [['gx4', '']])
    (s, gex) = epidb.list_expressions("gene", None, None, "ENCODE", self.admin_key)
    self.assertEquals(gex, [['gx4', '']])

    (s, gex) = epidb.list_expressions("gene", ["s1", "s2"], 1, None, self.admin_key)
    self.assertEquals(gex, [['gx1', ''], ['gx4', '']])

    (s, gex) = epidb.list_expressions("gene", ["s1", "s2"], 2, None, self.admin_key)
    self.assertEquals(gex, [['gx2', '']])

    (s, gex) = epidb.list_expressions("gene", None, 1, "ENCODE", self.admin_key)
    self.assertEquals(gex, [['gx4', '']])

    s, user = epidb.add_user("user", "email", "institution", self.admin_key)
    (user_id, user_key) = user
    self.assertSuccess(s)
    (s, ss) = epidb.modify_user_admin(user_id, "permission_level", "GET_DATA", self.admin_key)
    self.assertSuccess(s, ss)

    (s, gex) = epidb.list_expressions("gene", None, None, None, user_key)
    self.assertEquals(gex, [])
    (s, gex) = epidb.list_expressions("gene", "s1", 1, None, user_key)
    self.assertEquals(gex, [])
    (s, gex) = epidb.list_expressions("gene", None, None, "ENCODE", user_key)
    self.assertEquals(gex, "107000:Project 'ENCODE' does not exist.")

    (s, info) = epidb.info(gene_expression, self.admin_key)

    self.assertEquals(info[0], {'format': 'TRACKING_ID,GENE_ID,GENE_SHORT_NAME,FPKM,FPKM_CONF_LO,FPKM_CONF_HI,FPKM_STATUS', 'sample_info': {'biosource_name': 'K562', 'karyotype': 'cancer', 'sex': 'F'}, 'content_format': 'cufflinks', 'total_genes': 57910, 'replica': 1, 'sample_id': 's1', '_id': 'gx4', 'extra_metadata': {}, 'columns': [{'name': 'TRACKING_ID', 'column_type': 'string'}, {'name': 'GENE_ID', 'column_type': 'string'}, {'name': 'GENE_SHORT_NAME', 'column_type': 'string'}, {'name': 'FPKM', 'column_type': 'double'}, {'name': 'FPKM_CONF_LO', 'column_type': 'double'}, {'name': 'FPKM_CONF_HI', 'column_type': 'double'}, {'name': 'FPKM_STATUS', 'column_type': 'string'}]})

    data = gzip.open("data/grape2/SP8-TH91.gene_quantification.rsem_grape2_crg.GRCh38.20150622.results.txt.gz").read()
    (s, gene_expression) = epidb.add_expression("gene", "s1", 1, data, "grape2", "ENCODE", None, self.admin_key)
    self.assertEquals(gene_expression, "131001:A Expression of the type 'gene' with sample_id 's1' and replica '1' already exists.")

    (s, gene_expression) = epidb.add_expression("gene", "s1", 2, data, "grape2", "ENCODE", None, self.admin_key)
    self.assertSuccess(s, gene_expression)

    data = gzip.open("data/gtf/gencode.v19.annotation.ONLY_GENES.gtf.gz").read()
    (s, ss) = epidb.add_gene_model("gencode v19", "hg19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (status, gx_query) = epidb.select_expressions("gene", "s1", 2, "ENSG00000000003.13", "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info(gx_query, user_key)
    (status, r_id) = epidb.get_regions(gx_query, "CHROMOSOME,START,END,@STRAND,GENE_ID,TRANSCRIPT_IDS,LENGTH,EFFECTIVE_LENGTH,EXPECTED_COUNT,TPM,FPKM,POSTERIOR_MEAN_COUNT,POSTERIOR_STANDARD_DEVIATION_OF_COUNT,PME_TPM,PME_FPKM,TPM_CI_LOWER_BOUND,TPM_CI_UPPER_BOUND,FPKM_CI_LOWER_BOUND,FPKM_CI_UPPER_BOUND", self.admin_key)
    self.assertSuccess(status, r_id)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "chrX\t99883667\t99894988\t-\tENSG00000000003.13\tENSG00000000003.13\t2025\t1855.4301\t161.0000\t1.0000\t2.1300\t161.0000\t0.0000\t1.0500\t2.2700\t0.8742\t1.2451\t1.8882\t2.6879")

    (status, gx_query) = epidb.select_expressions("gene", "s1", 2, "ENSG00000000003.13", "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info(gx_query, user_key)
    (status, r_id) = epidb.get_regions(gx_query, "GENE_ID,TRANSCRIPT_IDS,LENGTH,EFFECTIVE_LENGTH,EXPECTED_COUNT,TPM,FPKM,POSTERIOR_MEAN_COUNT,POSTERIOR_STANDARD_DEVIATION_OF_COUNT,PME_TPM,PME_FPKM,TPM_CI_LOWER_BOUND,TPM_CI_UPPER_BOUND,FPKM_CI_LOWER_BOUND,FPKM_CI_UPPER_BOUND", self.admin_key)
    self.assertSuccess(status, r_id)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "ENSG00000000003.13\tENSG00000000003.13\t2025\t1855.4301\t161.0000\t1.0000\t2.1300\t161.0000\t0.0000\t1.0500\t2.2700\t0.8742\t1.2451\t1.8882\t2.6879")

    (status, gx_query) = epidb.select_expressions("gene", "s1", 1, "OR4G11P", "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info("gx1", user_key)
    (status, r_id) = epidb.get_regions(gx_query, info[0]["format"], self.admin_key)
    self.assertSuccess(status, r_id)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "ENSG00000240361.1\tENSG00000240361.1\tOR4G11P\t0.0000\t0.0000\t0.0000\tOK")

    (status, gx_query) = epidb.select_expressions("gene", "s1", 1, ['CCR1', 'CD164', 'CD1D', 'CD2', 'CD34', 'CD3G', 'CD44'], "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info("gx1", user_key)
    (status, r_id) = epidb.get_regions(gx_query, info[0]["format"], self.admin_key)
    self.assertSuccess(status, r_id)
    regions_a = self.get_regions_request(r_id)

    excepted = "ENSG00000135535.10\tENSG00000135535.10\tCD164\t101.3820\t98.8947\t103.8680\tOK\nENSG00000026508.12\tENSG00000026508.12\tCD44\t193.4920\t189.4020\t197.5830\tOK\nENSG00000160654.5\tENSG00000160654.5\tCD3G\t53.0051\t51.4405\t54.5696\tOK\nENSG00000163823.3\tENSG00000163823.3\tCCR1\t0.0201\t0.0000\t0.0433\tOK\nENSG00000116824.4\tENSG00000116824.4\tCD2\t90.0146\t87.9630\t92.0661\tOK\nENSG00000158473.6\tENSG00000158473.6\tCD1D\t0.0241\t0.0000\t0.0519\tOK\nENSG00000174059.12\tENSG00000174059.12\tCD34\t0.0000\t0.0000\t0.0000\tOK"

    lexp = excepted.split("\n")

    lresult = regions_a.split("\n")

    self.assertEquals(len(lresult), len(lexp))
    for l in lresult:
      self.assertTrue(l in lexp)

    (status, gx_query) = epidb.select_expressions("gene", "s1", 1, 'CCR1', "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info("gx1", user_key)
    (status, r_id) = epidb.get_regions(gx_query, info[0]["format"], self.admin_key)
    self.assertSuccess(status, r_id)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "ENSG00000163823.3\tENSG00000163823.3\tCCR1\t0.0201\t0.0000\t0.0433\tOK")

    q1 = gx_query

    (status, gx_query) = epidb.select_expressions("gene", "s1", 1, 'CD164', "ENCODE", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)
    status, info = epidb.info("gx1", user_key)
    (status, r_id) = epidb.get_regions(gx_query, info[0]["format"], self.admin_key)
    self.assertSuccess(status, r_id)
    regions = self.get_regions_request(r_id)
    self.assertEquals(regions, "ENSG00000135535.10\tENSG00000135535.10\tCD164\t101.3820\t98.8947\t103.8680\tOK")

    self.assertTrue(q1 != gx_query)

    (s, info) = epidb.info(ss, self.admin_key)

    self.assertEquals(info[0], {'total_genes': 57820, '_id': 'gs1', 'genome': 'hg19', 'description': 'Test One Description', 'format': 'GTF', 'name': 'gencode v19'})

    (status, gene_info) = epidb.info("gn1", self.admin_key)
    self.assertEquals(gene_info[0], {'transcript_status': 'KNOWN', 'gene_name': 'DDX11L1', 'gene_type': 'pseudogene', 'end': 14412, 'source': 'HAVANA', 'frame': '.', 'level': '2', 'gene_id': 'ENSG00000223972.4', 'start': 11869, 'transcript_id': 'ENSG00000223972.4', 'score': 0.0, 'strand': '+', 'havana_gene': 'OTTHUMG00000000961.2', 'transcript_name': 'DDX11L1', '_id': 'gn1', 'gene_status': 'KNOWN', 'transcript_type': 'pseudogene', 'chromosome': 'chr1'})


    (status, query) = epidb.select_expressions("gene", "s1", [1, 5, 10, 122], None,  "ENCODE", "gencode v19", self.admin_key)
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


    (status, query) = epidb.select_expressions("gene", "s1", [1, 5, 10, 122], None,  "", "gencode v19", user_key)

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


  def test_salmon_include_retrieve(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    data = gzip.open("data/gtf/gencode.v19.annotation.ONLY_GENES.gtf.gz").read()
    (s, ss) = epidb.add_gene_model("gencode v19", "hg19", "Test One Description", data, "GTF", {}, self.admin_key)
    self.assertSuccess(s, ss)

    (s, project) = epidb.add_project("BLUEPRINT", "BLUEPRINT", self.admin_key)
    self.assertSuccess(s, project)

    data = gzip.open("data/tpm/test.genes.sf.gz").read()
    (s, gene_expression) = epidb.add_expression("gene", "s2", 1, data, "salmon", "BLUEPRINT", None, self.admin_key)
    self.assertSuccess(s, gene_expression)

    (status, gx_query) = epidb.select_expressions("gene", "s2", 1, None, "BLUEPRINT", "gencode v19", self.admin_key)
    self.assertSuccess(status, gx_query)

    (status, r_id) = epidb.get_regions(gx_query, "GENE_ID,LENGTH,EFFECTIVE_LENGTH,TPM,NUM_READS", self.admin_key)
    self.assertSuccess(status, r_id)
    data = self.get_regions_request(r_id)

    self.assertEquals(data, 'ENSG00000198712.1\t684\t508.2220\t238.0270\t10249.0000\nENSG00000210164.1\t68\t20.0000\t9.4425\t16.0000\nENSG00000212907.2\t297\t134.0000\t178.6330\t2028.0000\nENSG00000210176.1\t69\t20.0000\t151.6710\t257.0000\nENSG00000198786.2\t1812\t1477.8101\t33.5372\t4198.9902\nENSG00000198695.2\t525\t330.2690\t14.8310\t414.9930\nENSG00000210194.1\t69\t20.0000\t37.7701\t64.0000\nENSG00000198727.2\t1141\t867.7130\t90.2670\t6636.0000\nENSG00000210196.2\t68\t20.0000\t112.7200\t191.0000')