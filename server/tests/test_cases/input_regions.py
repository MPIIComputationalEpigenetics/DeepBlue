import helpers

from deepblue_client import DeepBlueClient

class TestInputRegions(helpers.TestCase):
    def test_misc_formats(self):
        epidb = DeepBlueClient(address="localhost", port=31415)
        self.init_base(epidb)

        files = ["chr_s_e_name.bed", "chr_s_e_score.bed", "bed10.bed"]

        print epidb.list_genomes(self.admin_key)

        for f in files:
          print f
          content = open("data/bed/" + f).read()
          res, q1 = epidb.input_regions("hg19", content, self.admin_key)
          print epidb.info(q1, self.admin_key)
          fmt = epidb.info(q1, self.admin_key)[1][0]['args']['format']
          self.assertSuccess(res, q1)

          print fmt
          r, r1 = epidb.get_regions(q1, fmt, self.admin_key)
          self.assertSuccess(r, r1)

          for l in self.get_regions_request(r1).split("\n"):
            print l
            print len(l.split("\t"))
