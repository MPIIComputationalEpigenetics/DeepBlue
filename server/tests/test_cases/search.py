import helpers

from client import EpidbClient
import data_info

class TestSearch(helpers.TestCase):

  def test_type_restricted(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, pid = epidb.add_project("ENCODE", "desc", self.admin_key)
    self.assertSuccess(res, pid)

    res, emid = epidb.add_epigenetic_mark("Methylation", "desc", self.admin_key)
    self.assertSuccess(res, emid)

    res, bsid = epidb.add_bio_source("K562", "desc", {}, self.admin_key)
    self.assertSuccess(res, bsid)

    res, ids = epidb.search("desc", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertTrue([pid, 'ENCODE', 'projects'] in ids)
    self.assertTrue([emid, 'Methylation', 'epigenetic_marks'] in ids)
    self.assertTrue([bsid, 'K562', 'bio_sources'] in ids)
    self.assertEqual(len(ids), 3)

    res, ids = epidb.search("desc", "projects", self.admin_key)
    self.assertSuccess(res, ids)
    self.assertTrue([pid, 'ENCODE', 'projects'] in ids)
    self.assertEqual(len(ids), 1)

    res, ids = epidb.search("desc", ["epigenetic_marks", "bio_sources"], self.admin_key)
    self.assertSuccess(res, ids)
    self.assertTrue([emid, 'Methylation', 'epigenetic_marks'] in ids)
    self.assertTrue([bsid, 'K562', 'bio_sources'] in ids)
    self.assertEqual(len(ids), 2)

  def test_genome(self):
    epidb = EpidbClient()
    self.init(epidb)

    gid = self.insert_genome(epidb, "hg19")

    res, ids = epidb.search("hg19", "genomes", self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], gid)

    res, ids = epidb.search("Human", None,  self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], gid)

    res, ids = epidb.search("human", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], gid)

    res, ids = epidb.search(gid, None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], gid)


  def test_epigenetic_mark(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, emid1 = epidb.add_epigenetic_mark("Methylation", "DNA Methylation", self.admin_key)
    self.assertSuccess(res, emid1)

    res, emid2 = epidb.add_epigenetic_mark("Methyl450", "DNA Methylation Infinium HumanMethylation450 BeadChip", self.admin_key)
    self.assertSuccess(res, emid2)

    res, emid3 = epidb.add_epigenetic_mark("OpenChromDnase", "Open Chromatin DNaseI", self.admin_key)
    self.assertSuccess(res, emid3)

    res, emid4 = epidb.add_epigenetic_mark("DNaseI", "DNaseI hypersensitive sites ", self.admin_key)
    self.assertSuccess(res, emid4)

    self.assertEqual(res, "okay")

    res, ids = epidb.search("Methylation", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], emid1)
    self.assertEqual(ids[1][0], emid2)

    res, ids = epidb.search("DNA", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], emid1)
    self.assertEqual(ids[1][0], emid2)

    res, ids = epidb.search("Dnasei", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], emid4)
    self.assertEqual(ids[1][0], emid3)

    res, ids = epidb.search(emid4, None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], emid4)


  def test_bio_source(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.add_bio_source("K562", "leukemia cell", {}, self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_bio_source("K562b", "leukemia cell", {}, self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_bio_source("HepG2", "hepatocellular carcinoma", {}, self.admin_key)
    self.assertSuccess(res)

    res, ids = epidb.search("bio_source", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(len(ids), 3)

    res, ids = epidb.search("bio_source -K562", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(len(ids), 2)

    res, ids = epidb.search("banana", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(len(ids), 0)


  def test_sample(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.add_bio_source("K562", "desc1", {}, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_sample_field("age", "string", "", self.admin_key)
    self.assertSuccess(res)
    res = epidb.add_sample_field("health", "string", "", self.admin_key)
    self.assertSuccess(res)

    res, sid = epidb.add_sample("K562", {"age":"55", "health":"deceased"}, self.admin_key)
    self.assertSuccess(res, sid)

    res = epidb.add_sample_field("karyotype", "string", "Sample Karyotype: cancer or normal", self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_sample_field("sex", "string", "Sex of the element: M or F", self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_sample("K562", {"karyotype":"cancer", "sex":"F"}, self.admin_key)
    self.assertSuccess(res)

    res, ids = epidb.search("deceased", None, self.admin_key)
    self.assertEqual(ids[0][0], sid)


  def test_project(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, pid1 = epidb.add_project("ENCODE", "The ENCODE Project: ENCyclopedia Of DNA Elements", self.admin_key)
    self.assertSuccess(res, pid1)

    res, pid2 = epidb.add_project("RoadMap Epigenomics", "Roadmap Epigenemics Project", self.admin_key)
    self.assertSuccess(res, pid2)

    res, ids = epidb.search("encode", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], pid1)

    res, ids = epidb.search("roadmap", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], pid2)

    res, ids = epidb.search("epigenomic encode", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], pid1)
    self.assertEqual(ids[1][0], pid2)


  def test_experiment(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    eid1 = self.insert_experiment(epidb, "hg19_chr1_1")
    eid2 = self.insert_experiment(epidb, "hg19_chr1_2")

    res, ids = epidb.search("hg19_chr1_1", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertTrue(len(ids), 1)
    self.assertEqual(ids[0][0], eid1)


  def test_experiment_metadata(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    regions_data = helpers.load_bed("hg19_chr1_1")
    format = data_info.EXPERIMENTS["hg19_chr1_1"]["format"]

    res, eid1 = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, {"source": "ncbi"}, self.admin_key)
    self.assertSuccess(res, eid1)

    res, eid2 = epidb.add_experiment("test_exp2", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", regions_data, format, {"source":"encode"}, self.admin_key)
    self.assertSuccess(res, eid2)

    res, ids = epidb.search("ncbi", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], eid1)

    # Should be the first because the "encode" name appears twice (project and extra metadata)
    res, ids = epidb.search("encode", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(ids[0][0], eid2)


  def test_exclude(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, bsid1 = epidb.add_bio_source("K562", "leukemia cell", {}, self.admin_key)
    self.assertSuccess(res, bsid1)
    res, bsid2 = epidb.add_bio_source("K562b", "leukemia cell", {}, self.admin_key)
    self.assertSuccess(res, bsid2)
    res, bsid3 = epidb.add_bio_source("HepG2", "hepatocellular carcinoma cell", {}, self.admin_key)
    self.assertSuccess(res, bsid3)

    res, ids = epidb.search("cell", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(len(ids), 3)

    res, ids = epidb.search("cell -leukemia", None, self.admin_key)
    self.assertSuccess(res, ids)
    self.assertEqual(len(ids), 1)
    self.assertTrue(ids[0][0], bsid3)

  def test_invalid_search(self):
    epidb = EpidbClient()
    self.init(epidb)
    s, e = epidb.search("hg19", "genome", self.admin_key)
    self.assertEqual("genome is not a valid type. The valid types are: 'annotations,bio_sources,column_types,epigenetic_marks,experiments,genomes,projects,samples,samples.fields,techniques,tilings'", e)

  def test_search_synonyms(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    res, bsid1 = epidb.add_bio_source("Bio Source A", "bio source A", {}, self.admin_key)
    self.assertSuccess(res, bsid1)

    res = epidb.set_bio_source_synonym("bio source a", "synonym name for bio source a", self.admin_key)
    self.assertSuccess(res)

    res = epidb.set_bio_source_synonym("bio source a", "bla bla blu blu", self.admin_key)
    self.assertSuccess(res)

    res = epidb.set_bio_source_synonym("synonym name for bio source a", "another synonym", self.admin_key)
    self.assertSuccess(res)

    (res, found) = epidb.search("another synonym", None, self.admin_key)
    self.assertEquals(found, [['bs2', 'Bio Source A', 'bio_sources'], ['t2', 'tech2', 'techniques']])

    (res, found) = epidb.search("bla bla blu blu", None, self.admin_key)
    self.assertEquals(found, [['bs2', 'Bio Source A', 'bio_sources']])

    (res, info) = epidb.info(bsid1, self.admin_key)
    self.assertSuccess(res, info)
    self.assertEquals(info, {'description': 'bio source A', 'synonyms': ['Bio Source A', 'synonym name for bio source a', 'bla bla blu blu', 'another synonym'], 'user': 'test_admin', '_id': 'bs2', 'type': 'bio_source', 'name': 'Bio Source A'})

  def test_search_embracing(self):
    epidb = EpidbClient()
    self.init(epidb)

    s = epidb.add_bio_source("Ana", "Ana", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Beatriz", "Beatriz", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Carolina", "Carolina", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Bianca Ana", "Bianca Ana", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Bruna Branca", "Bruna Branca", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Bianca Carolina", "Bianca Carolina", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Brunete Cinza Cerva", "Brunete Cinza Cerva", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_scope("Ana", "Carolina", self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_scope("Bianca Carolina", "Brunete Cinza Cerva", self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_scope("Beatriz", "Bianca Carolina", self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_scope("Beatriz", "Bruna branca", self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_synonym("Ana", "Zebra", self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_scope("Beatriz", "Bianca Ana", self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_scope("Ana", "Beatriz", self.admin_key)
    self.assertSuccess(s)

    (s, r1) = epidb.search("Ana Zebra Beatriz", None, self.admin_key)
    self.assertEquals([['bs2', 'Beatriz', 'bio_sources'], ['bs4', 'Bianca Ana', 'bio_sources'], ['bs1', 'Ana', 'bio_sources'], ['bs7', 'Brunete Cinza Cerva', 'bio_sources'], ['bs6', 'Bianca Carolina', 'bio_sources'], ['bs5', 'Bruna Branca', 'bio_sources'], ['bs3', 'Carolina', 'bio_sources']], r1)

    (s, r2) = epidb.search("Ana", None, self.admin_key)
    self.assertEquals([['bs1', 'Ana', 'bio_sources'], ['bs4', 'Bianca Ana', 'bio_sources'], ['bs2', 'Beatriz', 'bio_sources'], ['bs3', 'Carolina', 'bio_sources'], ['bs7', 'Brunete Cinza Cerva', 'bio_sources'], ['bs6', 'Bianca Carolina', 'bio_sources'], ['bs5', 'Bruna Branca', 'bio_sources']], r2)

    (s, r3) = epidb.search("Zebra", None, self.admin_key)
    self.assertEquals([['bs1', 'Ana', 'bio_sources'], ['bs2', 'Beatriz', 'bio_sources'], ['bs3', 'Carolina', 'bio_sources'], ['bs7', 'Brunete Cinza Cerva', 'bio_sources'], ['bs6', 'Bianca Carolina', 'bio_sources'], ['bs5', 'Bruna Branca', 'bio_sources'], ['bs4', 'Bianca Ana', 'bio_sources']], r3)


  def test_search_sample_related(self):
    epidb = EpidbClient()
    self.init(epidb)

    s = epidb.add_bio_source("Ana", "Ana", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Beatriz", "Beatriz", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Carolina", "Carolina", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_scope("Beatriz", "Carolina", self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_sample("Carolina", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_scope("Ana", "Beatriz", self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_synonym("Ana", "Zebra", self.admin_key)
    self.assertSuccess(s)

    (s, r1) = epidb.search("Zebra", "samples", self.admin_key)
    self.assertEquals([['s1', '', 'samples']], r1)

    (s, r2) = epidb.search("Beatriz", "", self.admin_key)
    self.assertEqual([['bs2', 'Beatriz', 'bio_sources'], ['s1', '', 'samples'], ['bs3', 'Carolina', 'bio_sources']], r2)

    (s, r3) = epidb.search("Zebra", [], self.admin_key)
    self.assertEqual([['bs1', 'Ana', 'bio_sources'], ['bs2', 'Beatriz', 'bio_sources'], ['s1', '', 'samples'], ['bs3', 'Carolina', 'bio_sources']], r3)


  def test_search_experiment_related(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    s = epidb.add_bio_source("Ana", "Ana", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Beatriz", "Beatriz", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.add_bio_source("Carolina", "Carolina", {}, self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_scope("Beatriz", "Carolina", self.admin_key)
    self.assertSuccess(s)

    (s, sid) = epidb.add_sample("Carolina", {}, self.admin_key)
    self.assertSuccess(s, sid)

    data = "chr1\t1\t100"
    (s, e) = epidb.add_experiment("las chicas", "hg19", "Methylation", sid, "tech1", "ENCODE", "interesting experiment", data, "CHROMOSOME,START,END", {}, self.admin_key)
    self.assertSuccess(s, e)

    s = epidb.set_bio_source_scope("Ana", "Beatriz", self.admin_key)
    self.assertSuccess(s)

    s = epidb.set_bio_source_synonym("Ana", "Zebra", self.admin_key)
    self.assertSuccess(s)

    (s, r1) = epidb.search("Zebra", "experiments", self.admin_key)
    self.assertEqual([['e1', 'las chicas', 'experiments']], r1)

    (s, r2) = epidb.search("Carolina", [], self.admin_key)
    self.assertEqual([['bs4', 'Carolina', 'bio_sources'], ['s2', '', 'samples'], ['e1', 'las chicas', 'experiments']], r2)

  def test_column_types_search(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_simple("name", "description", ".", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("string_column", "description", ".", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("integer_column", "description", "0", "integer", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("double_column", "description", "0.0", "double", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("score", "description", "X", 0.0, 1.0, self.admin_key)
    self.assertSuccess(res)
    strand = ["+", "-"]
    res = epidb.create_column_type_category("STRAND", "description", ".", strand, self.admin_key)
    self.assertSuccess(res)


    (s, ss) = epidb.search("string_column", "column_types", self.admin_key)
    (s, info) = epidb.info(ss[0][0], self.admin_key)
    self.assertEqual(info, {'type': 'string', 'ignore_if': '.', 'name': 'string_column'})

    (s, ss) = epidb.search("integer_column", "column_types", self.admin_key)
    (s, info) = epidb.info(ss[0][0], self.admin_key)
    self.assertEqual(info, {'type': 'integer', 'ignore_if': '0', 'name': 'integer_column'})

    (s, ss) = epidb.search("double_column", "column_types", self.admin_key)
    (s, info) = epidb.info(ss[0][0], self.admin_key)
    self.assertEqual(info, {'type': 'double', 'ignore_if': '0.0', 'name': 'double_column'})

    (s, ss) = epidb.search("score", "column_types", self.admin_key)
    (s, info) = epidb.info(ss[0][0], self.admin_key)
    self.assertEqual(info, {'ignore_if': 'X', 'max': '1', 'min': '0', 'name': 'score', 'type': 'range'})

    (s, ss) = epidb.search("STRAND", "column_types", self.admin_key)
    (s, info) = epidb.info(ss[0][0], self.admin_key)
    self.assertEqual(info, {'ignore_if': '.', 'name': 'STRAND', 'type': 'category', 'values': '+,-'})