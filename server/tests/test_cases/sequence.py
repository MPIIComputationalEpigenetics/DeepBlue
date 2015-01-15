import helpers

from client import EpidbClient

class TestSequence(helpers.TestCase):

  def test_upload_sequence(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sequence = open("data/genomes/chromosomes/hg19.chr19").read().replace("\n", "")

    res = epidb.upload_chromosome("hg19", "chr19", sequence, self.admin_key)
    self.assertSuccess(res)

    self.insert_experiment(epidb, "hg19_big_1")

    res, qid = epidb.select_regions("hg19_big_1", "hg19", None, None, None, None, "chr19", 574556, 1000514, self.admin_key)
    self.assertSuccess(res, qid)

    fmt = "CHROMOSOME,START,END,@NAME,@LENGTH,@EPIGENETIC_MARK,@PROJECT,@BIOSOURCE,@SAMPLE_ID,@SEQUENCE"
    res, regions = epidb.get_regions(qid, fmt, self.admin_key)

    self.assertSuccess(res, regions)

  def test_tiling_regions(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sequence = open("data/genomes/chromosomes/hg19.chr19").read().replace("\n", "")

    res = epidb.upload_chromosome("hg19", "chr19", sequence, self.admin_key)
    self.assertSuccess(res)

    self.insert_experiment(epidb, "hg19_big_1")

    res, qid = epidb.tiling_regions(10000, "hg19", "chr19", self.admin_key)
    self.assertSuccess(res, qid)

    fmt = "CHROMOSOME,START,END,@NAME,@LENGTH,@EPIGENETIC_MARK,@PROJECT,@BIOSOURCE,@SAMPLE_ID,@SEQUENCE"
    res, regions = epidb.get_regions(qid, fmt, self.admin_key)

    self.assertSuccess(res, regions)

  def test_wrong_sequence_length(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sequence = open("data/genomes/chromosomes/hg19.chr19").read().replace("\n", "")[:-10]

    res = epidb.upload_chromosome("hg19", "chr19", sequence, self.admin_key)
    self.assertFailure(res)

  def test_double_upload(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sequence = open("data/genomes/chromosomes/hg19.chr19").read().replace("\n", "")

    res = epidb.upload_chromosome("hg19", "chr19", sequence, self.admin_key)
    self.assertSuccess(res)

    res = epidb.upload_chromosome("hg19", "chr19", sequence, self.admin_key)
    self.assertFailure(res)

  def test_multiple_sequences(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sequence = open("data/genomes/chromosomes/hg19.chr19").read().replace("\n", "")
    sequence = sequence + "\n" + sequence

    res = epidb.upload_chromosome("hg19", "chr19", sequence, self.admin_key)
    self.assertFailure(res)

  def test_nonexisting_genome(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sequence = "BLABLABLA"
    sequence = sequence + "\n" + sequence

    res = epidb.upload_chromosome("hg_nothing", "chr19", sequence, self.admin_key)
    self.assertFailure(res)

  def test_nonexisting_chromosome(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sequence = "BLABLABLA"
    sequence = sequence + "\n" + sequence

    res = epidb.upload_chromosome("hg19", "chrInvalid", sequence, self.admin_key)
    self.assertFailure(res)
    self.assertEquals(res[1], 'Chromosome chrInvalid not found.')

  def test_result(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    self.insert_experiment(epidb, "hg19_small")
    res, qid = epidb.select_regions("hg19_small", "hg19", None, None, None, None, "chrM", None, None, self.admin_key)
    self.assertSuccess(res, qid)
    fmt = "CHROMOSOME,START,END,@NAME,@SEQUENCE,@LENGTH,@EPIGENETIC_MARK,@PROJECT,@BIOSOURCE,@SAMPLE_ID"

    res, regions = epidb.get_regions(qid, fmt, self.admin_key)
    self.assertSuccess(res, regions)
    expected = """chrM\t0\t1\thg19_small\t\t1\tH3K4me3\tENCODE\tK562\ts1
chrM\t1\t2\thg19_small\t\t1\tH3K4me3\tENCODE\tK562\ts1
chrM\t2\t4\thg19_small\t\t2\tH3K4me3\tENCODE\tK562\ts1
chrM\t4\t8\thg19_small\t\t4\tH3K4me3\tENCODE\tK562\ts1
chrM\t8\t16\thg19_small\t\t8\tH3K4me3\tENCODE\tK562\ts1
chrM\t16\t32\thg19_small\t\t16\tH3K4me3\tENCODE\tK562\ts1"""

    self.assertEquals(regions, expected)

    sequence = open("data/genomes/chromosomes/chrM.fa").read()
    res = epidb.upload_chromosome("hg19", "chrM", sequence, self.admin_key)
    self.assertSuccess(res)
    res, regions = epidb.get_regions(qid, fmt, self.admin_key)

    expected = """chrM\t0\t1\thg19_small\tG\t1\tH3K4me3\tENCODE\tK562\ts1
chrM\t1\t2\thg19_small\tA\t1\tH3K4me3\tENCODE\tK562\ts1
chrM\t2\t4\thg19_small\tTC\t2\tH3K4me3\tENCODE\tK562\ts1
chrM\t4\t8\thg19_small\tACAG\t4\tH3K4me3\tENCODE\tK562\ts1
chrM\t8\t16\thg19_small\tGTCTATCA\t8\tH3K4me3\tENCODE\tK562\ts1
chrM\t16\t32\thg19_small\tCCCTATTAACCACTCA\t16\tH3K4me3\tENCODE\tK562\ts1"""

    self.assertEquals(regions, expected)

  def test_get_sequences(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sequence = open("data/genomes/chromosomes/chrM.fa").read()
    res = epidb.upload_chromosome("hg19", "chrM", sequence, self.admin_key)

    data = """chrM\t1\t30
chrM\t2\t31
chrM\t2340\t2377"""
    ann_name = "Interesting Regions at chrM"
    (s, aid) = epidb.add_annotation(ann_name, "hg19", None, data, None, None, self.admin_key)
    self.assertSuccess(s, aid)
    (s, sid) = epidb.select_annotations(ann_name, "hg19", None, None, None, self.admin_key)
    self.assertSuccess(s, sid)

    fmt = "CHROMOSOME,START,END,@NAME,@SEQUENCE,@LENGTH"
    (s, regions) = epidb.get_regions(sid, fmt, self.admin_key)

    expected = """chrM\t1\t30\tInteresting Regions at chrM\tATCACAGGTCTATCACCCTATTAACCACT\t29
chrM\t2\t31\tInteresting Regions at chrM\tTCACAGGTCTATCACCCTATTAACCACTC\t29
chrM\t2340\t2377\tInteresting Regions at chrM\tGCCTGCGTCAGATCAAAACACTGAACTGACAATTAAC\t37"""
    self.assertEqual(regions, expected)

  def test_long_sequence_content(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sequence = open("data/genomes/chromosomes/hg19.chr19").read()
    res = epidb.upload_chromosome("hg19", "chr19", sequence, self.admin_key)
    self.assertSuccess(res)

    join_seq = "".join(sequence.split("\n"))

    data = "chr19\t0\t59128983"
    ann_name = "Full chr19"
    (s, aid) = epidb.add_annotation(ann_name, "hg19", None, data, None, None, self.admin_key)
    (s, sid) = epidb.select_annotations(ann_name, "hg19", None, None, None, self.admin_key)

    fmt = "@SEQUENCE"
    (s, regions) = epidb.get_regions(sid, fmt, self.admin_key)

    self.assertEqual(len(regions), len(join_seq))
    self.assertEqual(regions, join_seq)
