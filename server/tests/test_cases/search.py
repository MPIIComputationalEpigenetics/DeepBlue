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
    self.assertEqual("genome is not a valid type. The valid types are: 'annotations,bio_sources,epigenetic_marks,experiments,genomes,projects,samples,samples.fields,techniques,tilings'", e)
  
  def test_search_synonyms(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, bsid1 = epidb.add_bio_source("Bio Source A", "bio source A", {}, self.admin_key)
    self.assertSuccess(res, bsid1)
    
    res = epidb.set_bio_source_synonym("bio source a", "synonym name for bio source a", self.admin_key)
    self.assertSuccess(res)

    res = epidb.set_bio_source_synonym("synonym name for bio source a", "another synonym", self.admin_key)
    self.assertSuccess(res)

    (res, found) = epidb.search("another synonym", None, self.admin_key)
    self.assertEquals(found, [['bs1', 'Bio Source A', 'bio_sources']])
