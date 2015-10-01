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
    status, info = epidb.info(_id[0], self.admin_key)
    status, info2 = epidb.info(_id2[0], self.admin_key)
    self.assertEquals(info, info2)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info[0]["name"], info[0]["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, req = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, req)
    c = self.count_request(req)
    self.assertEquals(c, 185269)

    status, _id = epidb.find_pattern("TATA", "hg19_only_chr19", False, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id[0], self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info[0]["name"], info[0]["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, req = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, req)
    c = self.count_request(req)
    self.assertEquals(c, 159464)

    status, _id = epidb.find_pattern("TATA+", "hg19_only_chr19", True, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id[0], self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info[0]["name"], info[0]["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, req = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, req)
    c = self.count_request(req)
    self.assertEquals(c, 185269)

    status, _id = epidb.find_pattern("TATA+", "hg19_only_chr19", False, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id[0], self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info[0]["name"], info[0]["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, req = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, req)
    c = self.count_request(req)
    self.assertEquals(c, 159464)

    status, _id = epidb.find_pattern("TATA*", "hg19_only_chr19", True, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id[0], self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info[0]["name"], info[0]["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, req = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, req)
    c = self.count_request(req)
    self.assertEquals(c, 698653)

    status, _id = epidb.find_pattern("TATA*", "hg19_only_chr19", False, self.admin_key)
    self.assertSuccess(status, _id)
    status, info = epidb.info(_id[0], self.admin_key)
    self.assertSuccess(status, info)
    status, _id = epidb.select_annotations(info[0]["name"], info[0]["genome"], "chr19", None, None, self.admin_key)
    self.assertSuccess(status, _id)
    status, req = epidb.count_regions(_id, self.admin_key)
    self.assertSuccess(status, req)
    c = self.count_request(req)
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
    res, qid = epidb.select_annotations("Chromosomes size for hg19_only_chr19", "hg19_only_chr19", None, None, None, self.admin_key)
    fmt = "CHROMOSOME,START,END,@NAME,@LENGTH,@COUNT.NON-OVERLAP(TATA),@COUNT.NON-OVERLAP(GAGA),@COUNT.NON-OVERLAP((TATA|GAGA))"
    res, req = epidb.get_regions(qid, fmt, self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    expected = "chr19\t0\t59128983\tChromosomes size for hg19_only_chr19\t59128983\t159464\t336889\t496353"
    self.assertEquals(expected, regions)

  def test_pattern_duplicate(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    genome_info = "chrM 16571"
    res = epidb.add_genome("hg19_only_chrM", "hg19 with only the chr19", genome_info, self.admin_key)
    self.assertSuccess(res)

    sequence = open("data/genomes/chromosomes/chrM.fa").read()
    res = epidb.upload_chromosome("hg19_only_chrM", "chrM", sequence, self.admin_key)
    self.assertSuccess(res)

    status, ann = epidb.find_pattern("GAGA", "hg19_only_chrM", False, self.admin_key)
    self.assertSuccess(status, ann)

    status, ann2 = epidb.find_pattern("GAGA", "hg19_only_chrM", False, self.admin_key)
    self.assertSuccess(status, ann2)

    self.assertEquals(ann, ann2)

    (s, q) = epidb.select_annotations(ann2[1], "hg19_only_chrM", None, None, None, self.admin_key)
    (s, r) = epidb.count_regions(q, self.admin_key)
    c = self.count_request(r)
    self.assertEquals(c, 23)

    (s, r) = epidb.get_regions(q, "CHROMOSOME,START,END", self.admin_key)
    rgs = self.get_regions_request(r)
    self.assertEquals(rgs, 'chrM\t91\t95\nchrM\t1439\t1443\nchrM\t1553\t1557\nchrM\t1643\t1647\nchrM\t1885\t1889\nchrM\t2144\t2148\nchrM\t3137\t3141\nchrM\t3880\t3884\nchrM\t5758\t5762\nchrM\t6580\t6584\nchrM\t7321\t7325\nchrM\t8340\t8344\nchrM\t8517\t8521\nchrM\t9394\t9398\nchrM\t11889\t11893\nchrM\t12054\t12058\nchrM\t12207\t12211\nchrM\t12767\t12771\nchrM\t12923\t12927\nchrM\t14601\t14605\nchrM\t14958\t14962\nchrM\t15928\t15932\nchrM\t15959\t15963')




