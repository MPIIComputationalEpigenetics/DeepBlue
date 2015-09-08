import helpers

from client import EpidbClient


class TestAnnotationCommands(helpers.TestCase):

  def test_annotation(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    data = open("data/gtf/gencode.v23.basic.annotation_head.gtf").read()
    print data

    print epidb.add_gene_set("Test One", "Test One Description", data, "GTF", {}, self.admin_key)