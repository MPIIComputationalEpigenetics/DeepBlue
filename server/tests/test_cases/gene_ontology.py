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

        print epidb.add_gene_ontology_term("GO:00001", "label 1", "description", "biological_process", self.admin_key)
        print epidb.add_gene_ontology_term("GO:00002", "label 2", "description 2", "biological_process", self.admin_key)
        print epidb.add_gene_ontology_term("GO:00003", "label 3", "description 3", "biological_process", self.admin_key)
        print epidb.add_gene_ontology_term("GO:00004", "label 4", "description 4", "biological_process", self.admin_key)

        print epidb.list_genes("", "", "chr1", 0, 1000000, "test one", self.admin_key)

        print epidb.annotate_gene("ENSG00000238009", "GO:00001", self.admin_key)

        print epidb.annotate_gene("ENSG00000239906", "GO:00002", self.admin_key)

        print epidb.annotate_gene("ENSG00000239945", "GO:00003", self.admin_key)

        print epidb.annotate_gene("ENSG00000238009", "GO:00001", self.admin_key)
        print epidb.annotate_gene("ENSG00000239945", "GO:00002", self.admin_key)
        print epidb.annotate_gene("ENSG00000239906", "GO:00003", self.admin_key)

        print epidb.annotate_gene("ENSG00000223972", "GO:00001", self.admin_key)
        print epidb.annotate_gene("ENSG00000223972", "GO:00004", self.admin_key)

        print epidb.list_genes("ENSG00000239906.1", None, "chr1", 0, 1000000, "Test One", self.admin_key)

        print epidb.list_genes("", "GO:00001", None, "chr1", 0, 1000000, "Test One", self.admin_key)
        print epidb.list_genes("", ["GO:00001", "GO:00002"], None, "chr1", 0, 1000000, "Test One", self.admin_key)


        (s, query_id) = epidb.select_genes(None, "", "Test One", None, None, None, self.admin_key)
        (s, r_id) = epidb.get_regions(query_id, "CHROMOSOME,START,END", self.admin_key)
        regions = self.get_regions_request(r_id)
        self.assertEquals("chr1\t11869\t14409", regions)