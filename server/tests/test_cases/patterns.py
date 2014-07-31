import helpers

from client import EpidbClient

class TestPattern(helpers.TestCase):

  def test_pattern_chromosome(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    genome_info = "chr19 59128983"
    res = epidb.add_genome("hg19_only_chr19", "hg19 with only the chr19", genome_info, self.admin_key)
    self.assertSuccess(res)

    sequence = open("data/genomes/chromosomes/hg19.chr19").read().replace("\n", "")
    res = epidb.upload_chromosome("hg19_only_chr19", "chr19", sequence, self.admin_key)
    self.assertSuccess(res)

    status, _id = epidb.find_pattern("TATA", "hg19_only_chr19", True, self.admin_key)
    self.assertSuccess(status, _id)
    status, _id2 = epidb.find_pattern("TATA", "hg19_only_chr19", True, self.admin_key)
    self.assertSuccess(status, _id2)
    self.assertEquals(_id, _id2)
    status, info = epidb.info(_id, self.admin_key)
    status, info2 = epidb.info(_id2, self.admin_key)
    self.assertEquals(info, info2)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info["name"], info["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, c = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, c)
    self.assertEquals(c, 185269)

    status, _id = epidb.find_pattern("TATA", "hg19_only_chr19", False, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id, self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info["name"], info["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, c = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, c)
    self.assertEquals(c, 159464)

    status, _id = epidb.find_pattern("TATA+", "hg19_only_chr19", True, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id, self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info["name"], info["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, c = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, c)
    self.assertEquals(c, 185269)

    status, _id = epidb.find_pattern("TATA+", "hg19_only_chr19", False, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id, self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info["name"], info["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, c = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, c)
    self.assertEquals(c, 159464)

    status, _id = epidb.find_pattern("TATA*", "hg19_only_chr19", True, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id, self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info["name"], info["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, c = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, c)
    self.assertEquals(c, 698653)

    status, _id = epidb.find_pattern("TATA*", "hg19_only_chr19", False, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id, self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info["name"], info["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, c = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, c)
    self.assertEquals(c, 638969)

  def test_upload_sequence(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    genome_info = "chr19 59128983"
    res = epidb.add_genome("hg19_only_chr19", "hg19 with only the chr19", genome_info, self.admin_key)
    self.assertSuccess(res)

    sequence = open("data/genomes/chromosomes/hg19.chr19").read().replace("\n", "")
    res = epidb.upload_chromosome("hg19_only_chr19", "chr19", sequence, self.admin_key)
    self.assertSuccess(res)

    res = epidb.find_pattern("GAGA", "hg19_only_chr19", False, self.admin_key)
    self.assertSuccess(res)
    res = epidb.find_pattern("TATA", "hg19_only_chr19", False, self.admin_key)
    self.assertSuccess(res)
    res = epidb.find_pattern("(TATA|GAGA)", "hg19_only_chr19", False, self.admin_key)
    self.assertSuccess(res)

    d = "chr19\t1\t59128983"
    res, qid = epidb.select_annotations("hg19_only_chr19", "hg19_only_chr19", None, None, None, self.admin_key)
    fmt = "CHROMOSOME,START,END,@NAME:none,@LENGTH,@COUNT.NON-OVERLAP(TATA),@COUNT.NON-OVERLAP(GAGA),@COUNT.NON-OVERLAP((TATA|GAGA))"
    res, regions = epidb.get_regions(qid, fmt, self.admin_key)
    self.assertSuccess(res, regions)

    expected = "chr19\t0\t59128983\thg19_only_chr19\t59128983\t159464\t336889\t496353"
    self.assertEquals(expected, regions)