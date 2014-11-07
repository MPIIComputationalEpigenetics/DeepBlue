import helpers

import data_info

from client import EpidbClient


class TestRemoveCommands(helpers.TestCase):

  def test_remove_genome(self):
    epidb = EpidbClient()
    self.init(epidb)

    genome_info = None
    with open("data/genomes/hg19", 'r') as f:
      genome_info = f.read().replace(",", "")

    res = epidb.add_genome("A1", "ANGELICA", genome_info, self.admin_key)
    self.assertSuccess(res)
    genome_id = res[1]

    res = epidb.add_genome("A1", "XUXA", genome_info, self.admin_key)
    self.assertFailure(res)

    (res, genomes) = epidb.list_genomes(self.admin_key)
    self.assertEqual(len(genomes), 1)
    self.assertEqual(genomes[0][0], genome_id)
    self.assertEqual(genomes[0][1], "A1")

    res = epidb.remove(genome_id, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_genome("A1", "XUXA", genome_info, self.admin_key)
    self.assertSuccess(res)

    status, found = epidb.search("XUXA", "", self.admin_key)
    self.assertEqual(len(found), 2)

    status, found = epidb.search("ANGELICA", "", self.admin_key);
    self.assertEqual(len(found), 0)

    status, genomes = epidb.list_genomes(self.admin_key)
    self.assertSuccess(status, genomes)

    for genome in genomes:
      res = epidb.remove(genome[0], self.admin_key)
      self.assertSuccess(res)
      res, anns = epidb.list_annotations(genome[1], self.admin_key)
      self.assertEqual(len(anns), 0)

  def test_remove_annotation(self):
    epidb = EpidbClient()
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

    annotation_id = res[1]
    res = epidb.remove(annotation_id, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
          file_data,
          "",
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)
    annotation_id = res[1]

    res, genomes = epidb.search("hg19", "genomes", self.admin_key)
    res, anns = epidb.remove(genomes[0][0], self.admin_key)
    self.assertFailure(res, anns)
    self.assertEqual(anns, "Some annotations are still using this genome. Use the list_annotations command for obtaining them.")

    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
          file_data,
          "",
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertFailure(res)

    res = epidb.remove(annotation_id, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
          file_data,
          "",
          {"url":"genome.ucsc.edu/cgi-bin/hgTables?db=hg19&hgta_group=regulation&hgta_track=cpgIslandExt&hgta_table=cpgIslandExt&hgta_doSchema=describe+table+schema"},
          self.admin_key)
    self.assertSuccess(res)

    res = epidb.remove(res[1], self.admin_key)
    self.assertSuccess(res)

    res, genomes = epidb.list_genomes(self.admin_key)

    for genome in genomes:
      res = epidb.remove(genome[0], self.admin_key)
      self.assertSuccess(res)

    res, anns = epidb.list_annotations("hg18", self.admin_key)
    self.assertSuccess(res, anns)
    self.assertEqual(len(anns), 0)

    res, anns = epidb.list_annotations("hg19", self.admin_key)
    self.assertSuccess(res, anns)
    self.assertEqual(len(anns), 0)

  def test_remove_project(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res, projects = epidb.list_projects(self.admin_key)

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              projects[0][1], "desc1", regions_data, format, None, self.admin_key)
    self.assertSuccess(res)

    res = epidb.remove(projects[0][0], self.admin_key )
    self.assertFailure(res)

