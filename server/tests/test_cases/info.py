import helpers

from client import EpidbClient


class TestGetInfoCommand(helpers.TestCase):

  def test_genome_info(self):
    epidb = EpidbClient()
    self.init(epidb)

    genome_info = None
    with open("data/genomes/hg19", 'r') as f:
      genome_info = f.read().replace(",", "")

    res, gid = epidb.add_genome("hg19", "Human genome 19", genome_info, self.admin_key)
    self.assertSuccess(res, gid)

    res, data = epidb.info(gid, self.admin_key)
    self.assertEqual(data['description'], "Human genome 19")
    self.assertEqual(data['name'], "hg19")
    self.assertEqual(data['user'], "test_admin")
    self.assertEqual(data['_id'], gid)


  def test_epigenetic_mark_info(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, emid = epidb.add_epigenetic_mark("Methylation", "DNA Methylation", self.admin_key)
    self.assertSuccess(res, emid)

    res, data = epidb.info(emid, self.admin_key)
    self.assertEqual(data['name'], "Methylation")
    self.assertEqual(data['description'], "DNA Methylation")
    self.assertEqual(data['user'], "test_admin")
    self.assertEqual(data['_id'], emid)


  def test_annotation_info(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    aid = None
    with open("data/cpgIslandExt.txt") as f:
      res, aid = epidb.add_annotation("Cpg Islands", "hg19", "CpG islands are associated ...",
                                      f.read(), "", {"url":"genome.ucsc.edu...", "meta":"data"},
                                      self.admin_key)
      self.assertSuccess(res, aid)

    res, data = epidb.info(aid, self.admin_key)
    self.assertEqual(res, 'okay')
    self.assertEqual(data['name'], "Cpg Islands")
    self.assertEqual(data['description'], "CpG islands are associated ...")
    self.assertEqual(data['genome'], "hg19")
    self.assertEqual(data['extra_metadata'], {"url":"genome.ucsc.edu...", "meta":"data"})
    self.assertEqual(data['upload_info']['user'], "test_admin")
    self.assertEqual(data['format'], "CHROMOSOME,START,END")
    self.assertEqual(data['_id'], aid)


  def test_experiment_info(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    format = ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "name:String:.",
      "score:Integer:0",
      "strand:String:.",
      "signalValue:Double:-1",
      "pValue:Double:-1",
      "qValue:Double:-1",
      "peak:Integer:-1"
    ])

    eid = None
    with open("data/bed/hg19_chr1_1.bed") as f:
      res, eid = epidb.add_experiment("exp1", "hg19", "Methylation", sample_id, "tech1",
                                      "ENCODE", "desc1", f.read(), format,
                                      {"foo":"bar", "extra":"123"}, self.admin_key)
      self.assertSuccess(res, eid)

    res, data = epidb.info(eid, self.admin_key)
    self.assertEqual(res, 'okay')
    self.assertEqual(data['sample_id'], sample_id)
    self.assertEqual(data['description'], "desc1")
    self.assertEqual(data['extra_metadata'], {"foo":"bar", "extra":"123"})
    self.assertEqual(data['epigenetic_mark'], "Methylation")
    self.assertEqual(data['genome'], "hg19")
    self.assertEqual(data['name'], "exp1")
    self.assertEqual(data['project'], "ENCODE")
    self.assertEqual(data['technique'], "tech1")
    self.assertEqual(data['user'], "test_admin")
    self.assertEqual(data['format'], "CHROMOSOME,START,END,name:String:.,score:Integer:0,strand:String:.,signalValue:Double:-1,pValue:Double:-1,qValue:Double:-1,peak:Integer:-1")
    self.assertEqual(data['_id'], eid)

  def test_query_info(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    self.insert_experiment(epidb, "hg19_chr1_1", sample_id)

    res, qid = epidb.select_regions("exp1", "hg19", "Methylation", sample_id, "tech1",
                                    "ENCODE", "chr1", 713500, 850000, self.admin_key)
    self.assertSuccess(res, qid)

    res, data = epidb.info(qid, self.admin_key)
    self.assertSuccess(res, data)
    self.assertEqual(data['_id'], qid)
    self.assertEqual(data['type'], 'experiment_select')
    #self.assertEqual(data['args'], '{ "experiment_name" : [ "exp1" ], "epigenetic_mark" : [ "Methylation" ], "sample_id" : [ "s1" ], "project" : [ "ENCODE" ], "technique" : [ "tech1" ], "has_filter" : true, "start" : { "$numberLong" : "713500" }, "end" : { "$numberLong" : "850000" }, "chromosomes" : [ "chr1" ], "genomes" : [ "hg19" ] }')
    self.assertEqual(data['args'], '{ "experiment_name" : [ "exp1" ], "epigenetic_mark" : [ "Methylation" ], "sample_id" : [ "s1" ], "project" : [ "ENCODE" ], "technique" : [ "tech1" ], "has_filter" : true, "start" : 713500, "end" : 850000, "chromosomes" : [ "chr1" ], "genomes" : [ "hg19" ] }')
    self.assertEqual(data['user'], 'test_admin')

  def test_biosource_info(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, bsid = epidb.add_biosource("K562", "desc1", {}, self.admin_key)
    self.assertSuccess(res, bsid)

    res, data = epidb.info(bsid, self.admin_key)
    self.assertEqual(res, "okay")
    self.assertEqual(data['description'], "desc1")
    self.assertEqual(data['name'], "K562")
    self.assertEqual(data['user'], "test_admin")
    self.assertEqual(data['_id'], bsid)

  def test_sample_info(self):
    epidb = EpidbClient()
    self.init(epidb)

    (status, sf_id_1) = epidb.add_sample_field("karyotype", "string", "Sample Karyotype: cancer or normal", self.admin_key)
    self.assertSuccess(status, sf_id_1)

    (status, sf_id_2) = epidb.add_sample_field("sex", "string", "Sex of the element: M or F", self.admin_key)
    self.assertSuccess(status, sf_id_2)

    res, bsid = epidb.add_biosource("K562", "desc1", {}, self.admin_key)
    self.assertSuccess(res, bsid)

    res, sid = epidb.add_sample("K562", {"karyotype":"cancer","sex":"F"}, self.admin_key)
    self.assertSuccess(res, sid)

    res, data = epidb.info(sid, self.admin_key)
    self.assertEqual(data['biosource_name'], "K562")
    self.assertEqual(data['karyotype'], "cancer")
    self.assertEqual(data['sex'], "F")
    self.assertEqual(data['user'], "test_admin")
    self.assertEqual(data['_id'], sid)


    (status, infos) = epidb.info([sf_id_1, sf_id_2, bsid, sid], self.admin_key)
    self.assertEqual(len(infos), 4)
