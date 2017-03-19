import helpers
import time
import gzip

from deepblue_client import DeepBlueClient


class TestGenes(helpers.TestCase):

    def test_gene_retrieve(self):
        epidb = DeepBlueClient(address="localhost", port=31415)
        self.init_base(epidb)

        data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()
        (s, ss) = epidb.add_gene_model("Test One", "hg19",
                                       "Test One Description", data, "GTF", {}, self.admin_key)
        self.assertSuccess(s, ss)

        (s, ss) = epidb.add_gene_ontology_term("GO:00001", "label 1", "description", "biological_process", self.admin_key)
        self.assertSuccess(s, ss)
        (s, ss) = epidb.add_gene_ontology_term("GO:00002", "label 2", "description 2", "biological_process", self.admin_key)
        self.assertSuccess(s, ss)
        (s, ss) = epidb.add_gene_ontology_term("GO:00003", "label 3", "description 3", "biological_process", self.admin_key)
        self.assertSuccess(s, ss)
        (s, ss) = epidb.add_gene_ontology_term("GO:00004", "label 4", "description 4", "biological_process", self.admin_key)
        self.assertSuccess(s, ss)

        (s, ss) = epidb.list_genes("", "", "chr1", 0, 1000000, "test one", self.admin_key)
        self.assertSuccess(s, ss)

        (s, ss) = epidb.annotate_gene("ENSG00000238009", "GO:00001", self.admin_key)
        self.assertSuccess(s, ss)

        (s, ss) = epidb.annotate_gene("ENSG00000239906", "GO:00002", self.admin_key)
        self.assertSuccess(s, ss)

        (s, ss) = epidb.annotate_gene("ENSG00000239945", "GO:00003", self.admin_key)
        self.assertSuccess(s, ss)

        (s, ss) = epidb.annotate_gene("ENSG00000238009", "GO:00001", self.admin_key)
        self.assertSuccess(s, ss)
        (s, ss) = epidb.annotate_gene("ENSG00000239945", "GO:00002", self.admin_key)
        self.assertSuccess(s, ss)
        (s, ss) = epidb.annotate_gene("ENSG00000239906", "GO:00003", self.admin_key)
        self.assertSuccess(s, ss)

        (s, ss) = epidb.annotate_gene("ENSG00000223972", "GO:00001", self.admin_key)
        self.assertSuccess(s, ss)
        (s, ss) = epidb.annotate_gene("ENSG00000223972", "GO:00004", self.admin_key)
        self.assertSuccess(s, ss)

        (s, ss) = epidb.list_genes("ENSG00000239906.1", None, "chr1", 0, 1000000, "Test One", self.admin_key)
        self.assertSuccess(s, ss)

        (s, ss) = epidb.list_genes("", "GO:00001", "chr1", 0, 1000000, "Test One", self.admin_key)
        self.assertSuccess(s, ss)
        (s, ss) = epidb.list_genes("", ["GO:00001", "GO:00002"], "chr1", 0, 1000000, "Test One", self.admin_key)
        self.assertSuccess(s, ss)


        (s, query_id) = epidb.select_genes(None, "", "Test One", None, None, None, self.admin_key)
        (s, r_id) = epidb.get_regions(query_id, "@GENE_ID(Test One),@GENE_NAME(Test One),@GO_IDS(Test One),@GO_LABELS(Test One)", self.admin_key)
        regions = self.get_regions_request(r_id)
        self.assertEquals("ENSG00000223972.5\tDDX11L1\tGO:00001,GO:00004\tlabel 1,label 4\nENSG00000227232.5\tWASH7P\t\t\nENSG00000278267.1\tMIR6859-1\t\t\nENSG00000243485.3\tRP11-34P13.3\t\t\nENSG00000274890.1\tMIR1302-2\t\t\nENSG00000237613.2\tFAM138A\t\t\nENSG00000268020.3\tOR4G4P\t\t\nENSG00000240361.1\tOR4G11P\t\t\nENSG00000186092.4\tOR4F5\t\t\nENSG00000238009.6\tRP11-34P13.7\tGO:00001\tlabel 1\nENSG00000239945.1\tRP11-34P13.8\tGO:00003,GO:00002\tlabel 3,label 2\nENSG00000233750.3\tCICP27\t\t\nENSG00000268903.1\tRP11-34P13.15\t\t\nENSG00000269981.1\tRP11-34P13.16\t\t\nENSG00000239906.1\tRP11-34P13.14\tGO:00002,GO:00003\tlabel 2,label 3\nENSG00000241860.6\tRP11-34P13.13\t\t\nENSG00000222623.1\tRNU6-1100P\t\t\nENSG00000241599.1\tRP11-34P13.9\t\t\nENSG00000279928.1\tFO538757.3\t\t\nENSG00000279457.3\tFO538757.2\t\t", regions)