import helpers

from client import EpidbClient


class TestGetInfoCommand(helpers.TestCase):
  maxDiff = None
  def test_genome_info(self):
    epidb = EpidbClient()
    self.init(epidb)

    genome_info = None
    with open("data/genomes/hg19", 'r') as f:
      genome_info = f.read().replace(",", "")

    res, gid = epidb.add_genome("hg19", "Human genome 19", genome_info, self.admin_key)
    self.assertSuccess(res, gid)

    res, data = epidb.info(gid, self.admin_key)
    self.assertEqual(data[0]['description'], "Human genome 19")
    self.assertEqual(data[0]['name'], "hg19")
    self.assertEqual(data[0]['user'], "test_admin")
    self.assertEqual(data[0]['_id'], gid)


  def test_epigenetic_mark_info(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, emid = epidb.add_epigenetic_mark("Methylation", "DNA Methylation", self.admin_key)
    self.assertSuccess(res, emid)

    res, data = epidb.info(emid, self.admin_key)
    self.assertEqual(data[0]['name'], "Methylation")
    self.assertEqual(data[0]['description'], "DNA Methylation")
    self.assertEqual(data[0]['user'], "test_admin")
    self.assertEqual(data[0]['_id'], emid)


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
    self.assertEqual(data[0]['name'], "Cpg Islands")
    self.assertEqual(data[0]['description'], "CpG islands are associated ...")
    self.assertEqual(data[0]['genome'], "hg19")
    self.assertEqual(data[0]['extra_metadata'], {"url":"genome.ucsc.edu...", "meta":"data"})
    self.assertEqual(data[0]['upload_info']['user'], "test_admin")
    self.assertEqual(data[0]['format'], "CHROMOSOME,START,END")
    self.assertEqual(data[0]['_id'], aid)


  def test_experiment_info(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    format = ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "name:String",
      "score:Integer",
      "strand:String",
      "signalValue:Double",
      "pValue:Double",
      "qValue:Double",
      "peak:Integer"
    ])

    eid = None
    with open("data/bed/hg19_chr1_1.bed") as f:
      res, eid = epidb.add_experiment("exp1", "hg19", "Methylation", sample_id, "tech1",
                                      "ENCODE", "desc1", f.read(), format,
                                      {"foo":"bar", "extra":"123"}, self.admin_key)
      self.assertSuccess(res, eid)

    res, data = epidb.info(eid, self.admin_key)
    data[0]["upload_info"]["upload_start"] = '0'
    data[0]["upload_info"]["upload_end"] = '0'
    data[0]["upload_info"]["client_address"] = '0'
    data[0]["upload_info"]["total_size"] = '0'
    self.assertEqual(data[0], {'format': 'CHROMOSOME,START,END,name:String,score:Integer,strand:String,signalValue:Double,pValue:Double,qValue:Double,peak:Integer', 'extra_metadata': {'foo': 'bar', 'extra': '123'}, 'sample_info': {'karyotype': 'cancer', 'biosource_name': 'K562', 'karyotype': 'cancer', 'sex': 'F'}, 'technique': 'tech1', 'upload_info': {'total_size': '0', 'content_format': 'bed', 'done': 'true', 'user': 'test_admin', 'upload_end': '0', 'upload_start': '0', 'client_address': '0'}, 'name': 'exp1', 'project': 'ENCODE', 'genome': 'hg19', 'sample_id': 's1', 'epigenetic_mark': 'Methylation', '_id': 'e1', 'type': 'experiment', 'columns': [{'name': 'CHROMOSOME', 'column_type': 'string'}, {'name': 'START', 'column_type': 'integer'}, {'name': 'END', 'column_type': 'integer'}, {'name': 'name', 'column_type': 'string'}, {'name': 'score', 'column_type': 'integer'}, {'name': 'strand', 'column_type': 'string'}, {'name': 'signalValue', 'column_type': 'double'}, {'name': 'pValue', 'column_type': 'double'}, {'name': 'qValue', 'column_type': 'double'}, {'name': 'peak', 'column_type': 'integer'}], 'description': 'desc1', 'data_type': 'peaks'})
    self.assertEqual(res, 'okay')
    self.assertEqual(data[0]['sample_id'], sample_id)
    self.assertEqual(data[0]['description'], "desc1")

    self.assertEqual(data[0]['extra_metadata'], {"foo":"bar", "extra":"123"})
    self.assertEqual(data[0]['epigenetic_mark'], "Methylation")
    self.assertEqual(data[0]['genome'], "hg19")
    self.assertEqual(data[0]['name'], "exp1")
    self.assertEqual(data[0]['project'], "ENCODE")
    self.assertEqual(data[0]['technique'], "tech1")
    self.assertEqual(data[0]['upload_info']['user'], "test_admin")
    self.assertEqual(data[0]['format'], "CHROMOSOME,START,END,name:String,score:Integer,strand:String,signalValue:Double,pValue:Double,qValue:Double,peak:Integer")
    self.assertEqual(data[0]['_id'], eid)

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
    self.assertEqual(data[0]['_id'], qid)
    self.assertEqual(data[0]['type'], 'experiment_select')
    self.assertEqual(data[0]['args'], '{ "experiment_name" : [ "exp1" ], "epigenetic_mark" : [ "Methylation" ], "sample_id" : [ "s1" ], "project" : [ "ENCODE" ], "technique" : [ "tech1" ], "has_filter" : true, "start" : 713500, "end" : 850000, "chromosomes" : [ "chr1" ], "genomes" : [ "hg19" ] }')
    self.assertEqual(data[0]['user'], 'test_admin')

  def test_biosource_info(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, bsid = epidb.add_biosource("K562", "desc1", {}, self.admin_key)
    self.assertSuccess(res, bsid)

    res, data = epidb.info(bsid, self.admin_key)
    self.assertEqual(res, "okay")
    self.assertEqual(data[0]['description'], "desc1")
    self.assertEqual(data[0]['name'], "K562")
    self.assertEqual(data[0]['user'], "test_admin")
    self.assertEqual(data[0]['_id'], bsid)

  def test_sample_info(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, bsid = epidb.add_biosource("K562", "desc1", {}, self.admin_key)
    self.assertSuccess(res, bsid)

    res, sid = epidb.add_sample("K562", {"karyotype":"cancer","sex":"F"}, self.admin_key)
    self.assertSuccess(res, sid)

    res, data = epidb.info(sid, self.admin_key)
    self.assertEqual(data[0]['biosource_name'], "K562")
    self.assertEqual(data[0]['karyotype'], "cancer")
    self.assertEqual(data[0]['sex'], "F")
    self.assertEqual(data[0]['user'], "test_admin")
    self.assertEqual(data[0]['_id'], sid)
