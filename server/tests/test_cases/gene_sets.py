import helpers

from client import EpidbClient


class TestAnnotationCommands(helpers.TestCase):

  def test_annotation(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()

    print "inserting..."
    print epidb.add_gene_set("Test One", "Test One Description", data, "GTF", {}, self.admin_key)


    (s, query_id) = epidb.select_genes(["ENSG00000223972.5", "ENSG00000223972.5", "DDX11L1"], "Test One", self.admin_key)

    print epidb.get_regions(query_id, "CHROMOSOME,START,END", self.admin_key)

    import time
    time.sleep(0.1)
    print epidb.get_request_data("r1", self.admin_key)