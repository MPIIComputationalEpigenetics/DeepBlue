import helpers

from deepblue_client import DeepBlueClient


class TestAnnotationCommands(helpers.TestCase):

  def test_annotation(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    file_data = None
    with open("data/cpgIslandExt.txt", 'r') as f:
      file_data = f.read()

    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
          file_data,
          "",
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)
    print res

    res, annotations = epidb.list_annotations("hg19", self.admin_key)
    print res
    print annotations
    self.assertSuccess(res, annotations)
    self.assertEqual(len(annotations), 2)

    self.assertEqual(annotations[0][1], "Chromosomes size for hg19")
    self.assertEqual(annotations[1][1], "Cpg Islands")

    size = len(file_data.split("\n"))

    res, qid = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, req = epidb.count_regions(qid, self.admin_key)
    self.assertSuccess(res, req)

    count = self.count_request(req)

    self.assertEqual(size, count)

    res, req = epidb.get_regions(qid, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)
    self.assertEqual(regions, file_data)

  def _test_invalid_annotation(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    res, qid = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
    self.assertFailure(res, qid)
    self.assertEqual(qid, "102000:Unable to find the annotation 'Cpg Islands' in the genome hg19.")

  def _test_missing_annotation_name(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    res, qid = epidb.select_annotations([], "hg19", None, None, None, self.admin_key)
    self.assertFailure(res, qid)

  def _test_list_annotations(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)

    genome_info = None
    with open("data/genomes/hg19", 'r') as f:
      genome_info = f.read().replace(",", "")

    file_data = None
    with open("data/cpgIslandExt.txt", 'r') as f:
      file_data = f.read()

    res = epidb.add_genome("hg19", "Human genome 19", genome_info, self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
          file_data,
          "",
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_genome("hg19a", "Human genome 19a", genome_info, self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_annotation("Cpg Islands", "hg19a", "CpG islands are associated ...",
          file_data,
          "",
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_genome("hg19b", "Human genome 19b", genome_info, self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_annotation("Cpg Islands", "hg19b", "CpG islands are associated ...",
          file_data,
          "",
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)

    expected = ['okay', [['a1', 'Chromosomes size for hg19'], ['a2', 'Cpg Islands'], ['a3', 'Chromosomes size for hg19a'], ['a4', 'Cpg Islands'], ['a5', 'Chromosomes size for hg19b'], ['a6', 'Cpg Islands']]]
    result = epidb.list_annotations("", self.admin_key)

    self.assertEqual(expected, result)

  def _test_annotation_full_cpg_islands(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    cpg_island =  ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "LENGTH",
      "NUM_CPG",
      "NUM_GC",
      "PER_CPG",
      "PER_CG",
      "OBS_EXP"
      ])

    with open("data/cpgIslandExtFull.txt", 'r') as f:
      file_data = f.read()
      regions_count = len(file_data.split("\n"))
      (res, a_1) = epidb.add_annotation("Cpg Islands", "hg19", "Complete CpG islands", file_data, cpg_island, None, self.admin_key)
      self.assertSuccess(res, a_1)
      res, qid_1 = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
      self.assertSuccess(res, qid_1)

      (s, req) = epidb.count_regions(qid_1, self.admin_key)
      count = self.count_request(req)

      self.assertEqual(regions_count, count)


  def _test_annotation_shuffle(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    cpg_island =  ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "LENGTH",
      "NUM_CPG",
      "NUM_GC",
      "PER_CPG",
      "PER_CG",
      "OBS_EXP"
      ])

    file_data = None
    with open("data/cpgIslandExtFull.bed", 'r') as f:
      file_data = f.read()

    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
          file_data,
          cpg_island,
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)

    res, qid = epidb.select_annotations("Cpg Islands", "hg19", None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, req = epidb.get_regions(qid, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions = self.get_regions_request(req)

    # ---

    file_data = None
    with open("data/cpgIslandExtShuffle.bed", 'r') as f:
      file_data = f.read()

    res = epidb.add_annotation("Cpg Islands Shuffle", "hg19", "CpG islands are associated ...",
          file_data,
          cpg_island,
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)

    res, qid_shuffle = epidb.select_annotations("Cpg Islands Shuffle", "hg19", None, None, None, self.admin_key)
    self.assertSuccess(res, qid)

    res, req_shuffle = epidb.get_regions(qid_shuffle, "CHROMOSOME,START,END", self.admin_key)
    self.assertSuccess(res, req)
    regions_shuffle = self.get_regions_request(req_shuffle)
    self.assertEqual(regions_shuffle, regions)

    self.assertEqual(regions, regions_shuffle)

  def _test_annotation_signal_bedgraph(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    files = ["test1"]

    for filename in files:
      wig_data = helpers.load_bedgraph(filename)
      res = epidb.add_annotation(filename, "hg19", "Test data", wig_data, "bedgraph", None, self.admin_key)
      self.assertSuccess(res)

    (s, q) = epidb.select_annotations(files, "hg19", None, None, None, self.admin_key)

    (s, req) = epidb.count_regions(q, self.admin_key)
    self.assertSuccess(s, req)
    count = self.count_request(req)

    self.assertEqual(1000, count)

    (s, q_filtered_down) = epidb.filter_regions(q, "VALUE", ">", "0.75", "number", self.admin_key)
    (s, q_filtered_up) = epidb.filter_regions(q_filtered_down, "VALUE", "<", "0.8", "number", self.admin_key)
    (s, q_chr_x) = epidb.filter_regions(q_filtered_up, "CHROMOSOME", "!=", "chrX", "string", self.admin_key)
    (s, q_chr_7) = epidb.filter_regions(q_chr_x, "CHROMOSOME", "!=", "chr7", "string", self.admin_key)

    (s, req) = epidb.get_regions(q_chr_7, "CHROMOSOME,START,END,VALUE,@NAME,@EPIGENETIC_MARK", self.admin_key)
    regions = self.get_regions_request(req)

    self.assertEqual(regions, 'chr1\t104372258\t104372293\t0.7767\ttest1\t\nchr10\t126498141\t126498176\t0.7695\ttest1\t\nchr11\t66110277\t66110312\t0.7613\ttest1\t\nchr15\t38653026\t38653061\t0.7720\ttest1\t\nchr15\t87725326\t87725361\t0.7727\ttest1\t\nchr16\t2119419\t2119454\t0.7696\ttest1\t\nchr16\t63360719\t63360754\t0.7740\ttest1\t\nchr19\t46369215\t46369250\t0.7727\ttest1\t\nchr8\t21923667\t21923702\t0.7930\ttest1\t')


  def _test_annotation_signal_wig(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    files = ["scores1", "scores2", "scores3", "scores4", "scores5",
             "scores6", "scores7", "yeast_pol2", "yeast_rap1"]

    for filename in files:
      wig_data = helpers.load_wig(filename)
      res = epidb.add_annotation(filename, "hg19", "Test data", wig_data, "wig", None, self.admin_key)
      self.assertSuccess(res)

    (s, r) = epidb.select_annotations(files, "hg19", None, None, None, self.admin_key)

    (s, req) = epidb.count_regions(r, self.admin_key)
    count = self.count_request(req)
    self.assertEqual(5667, count)

  def _test_list_annotations2(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init(epidb)


    res = epidb.add_genome("GRCh38", "GRCh38", "chr1 1000000\nchr2 2000000", self.admin_key)
    self.assertSuccess(res)


    status, anns = epidb.list_annotations("GRCh38", self.admin_key)
    self.assertEqual(anns, [['a1', 'Chromosomes size for GRCh38']])
