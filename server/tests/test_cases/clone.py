import time

import helpers

from client import EpidbClient

class TestClone(helpers.TestCase):
  maxDiff = None
  def __test_clone_experiment(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    eid1 = self.insert_experiment(epidb, "hg18_chr1_1")

    (s, info_original) = epidb.info(eid1, self.admin_key)
    info_original[0]['upload_info']['upload_end'] = '0'
    info_original[0]['upload_info']['upload_start'] = '0'
    info_original[0]['upload_info']['client_address'] = '0'
    info_original[0]['upload_info']['total_size'] = '0'

    self.assertEqual(info_original[0], {'format': 'CHROMOSOME,START,END,name:String:.,score:Integer:0,strand:String:.,signalValue:Double:-1,pValue:Double:-1,qValue:Double:-1,peak:Integer:-1', 'sample_info': {'sex': 'F', 'biosource_name': 'K562', 'karyotype': 'cancer'}, 'technique': 'tech1', 'upload_info': {'total_size': '0', 'content_format': 'bed', 'done': 'true', 'user': 'test_admin', 'upload_end': '0', 'upload_start': '0', 'client_address': '0', 'user': 'test_admin'}, 'name': 'hg18_chr1_1', 'project': 'ENCODE', 'genome': 'hg18', 'sample_id': 's1', 'epigenetic_mark': 'Methylation', '_id': 'e1', 'type': 'experiment', 'columns': [{'name': 'CHROMOSOME', 'column_type': 'string'}, {'name': 'START', 'column_type': 'integer'}, {'name': 'END', 'column_type': 'integer'}, {'default_value': '.', 'name': 'name', 'column_type': 'string'}, {'default_value': '0', 'name': 'score', 'column_type': 'integer'}, {'default_value': '.', 'name': 'strand', 'column_type': 'string'}, {'default_value': '-1', 'name': 'signalValue', 'column_type': 'double'}, {'default_value': '-1', 'name': 'pValue', 'column_type': 'double'}, {'default_value': '-1', 'name': 'qValue', 'column_type': 'double'}, {'default_value': '-1', 'name': 'peak', 'column_type': 'integer'}], 'description': 'desc1'})

    (s, id_clone_plus) = epidb.clone_dataset(eid1, "new experiment clone", "", "", "", "", "getting only the default values", "", {"new data": "true", "cool": "a lot"}, self.admin_key)

    self.assertSuccess(s, id_clone_plus)
    (s, info_clone_plus) = epidb.info(id_clone_plus, self.admin_key)
    info_clone_plus[0]['upload_info']['upload_end'] = '0'
    self.assertEqual(info_clone_plus[0], {'format': 'CHROMOSOME,START,END', 'extra_metadata': {'new data': 'true', 'cool': 'a lot'}, 'sample_info': {'sex': 'F', 'biosource_name': 'K562', 'karyotype': 'cancer'}, 'technique': 'tech1', 'upload_info': {'upload_end': '0', 'done': 'true', 'cloned_from': 'e1', 'user': 'test_admin'}, 'name': 'new experiment clone', 'project': 'ENCODE', 'genome': 'hg18', 'sample_id': 's1', 'epigenetic_mark': 'Methylation', '_id': 'e2', 'type': 'experiment', 'columns': [{'name': 'CHROMOSOME', 'column_type': 'string'}, {'name': 'START', 'column_type': 'integer'}, {'name': 'END', 'column_type': 'integer'}], 'description': 'getting only the default values'})

    (s, _id_clone_same) =  epidb.clone_dataset(eid1, "clone of new experiment", "", "", "", "", "", "CHROMOSOME,START,END,name:String:.,score:Integer:0,strand:String:.,signalValue:Double:-1,pValue:Double:-1,qValue:Double:-1,peak:Integer:-1", {}, self.admin_key)
    self.assertSuccess(s, _id_clone_same)

    (s, info_clone) = epidb.info(_id_clone_same, self.admin_key)
    self.assertSuccess(s, info_clone)
    info_clone[0]['upload_info']['upload_end'] = '0'
    self.assertEqual(info_clone[0], {'format': 'CHROMOSOME,START,END,name:String:.,score:Integer:0,strand:String:.,signalValue:Double:-1,pValue:Double:-1,qValue:Double:-1,peak:Integer:-1', 'sample_info': {'sex': 'F', 'biosource_name': 'K562', 'karyotype': 'cancer'}, 'technique': 'tech1', 'upload_info': {'upload_end': '0', 'done': 'true', 'cloned_from': 'e1', 'user': 'test_admin'}, 'name': 'clone of new experiment', 'project': 'ENCODE', 'genome': 'hg18', 'sample_id': 's1', 'epigenetic_mark': 'Methylation', '_id': 'e3', 'type': 'experiment', 'columns': [{'name': 'CHROMOSOME', 'column_type': 'string'}, {'name': 'START', 'column_type': 'integer'}, {'name': 'END', 'column_type': 'integer'}, {'default_value': '.', 'name': 'name', 'column_type': 'string'}, {'default_value': '0', 'name': 'score', 'column_type': 'integer'}, {'default_value': '.', 'name': 'strand', 'column_type': 'string'}, {'default_value': '-1', 'name': 'signalValue', 'column_type': 'double'}, {'default_value': '-1', 'name': 'pValue', 'column_type': 'double'}, {'default_value': '-1', 'name': 'qValue', 'column_type': 'double'}, {'default_value': '-1', 'name': 'peak', 'column_type': 'integer'}], 'description': 'desc1'})


  def __test_clone_annotatation(self):
    epidb = EpidbClient()
    self.init_base(epidb)
    aid1 = self.insert_annotation(epidb, "Cpg Islands")

    (s, id_clone_no_columns) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "", {"new":"true"}, self.admin_key)
    self.assertSuccess(s, id_clone_no_columns)
    (s, info) = epidb.info(id_clone_no_columns, self.admin_key)
    info[0]['upload_info']['upload_end'] = '0'

    self.assertEqual(info[0], {'format': 'CHROMOSOME,START,END', 'extra_metadata': {'new': 'true'}, 'upload_info': {'upload_end': '0', 'done': 'true', 'cloned_from': 'a3', 'user': 'test_admin'}, 'name': 'New CpG Islands', 'genome': 'hg19', '_id': 'a4', 'type': 'annotation', 'columns': [{'name': 'CHROMOSOME', 'column_type': 'string'}, {'name': 'START', 'column_type': 'integer'}, {'name': 'END', 'column_type': 'integer'}], 'description': 'CpG islands are associated ...'})


  def test_invalid_column(self):
    epidb = EpidbClient()
    self.init_base(epidb)
    aid1 = self.insert_annotation(epidb, "Cpg Islands")

    (s, msg) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "CHROMOSOME,START,END,INVALID_COLUMN", {"new":"true"}, self.admin_key)

    self.assertEqual(msg, "Error loading column type: 'INVALID_COLUMN'")

    (s, m) = epidb.create_column_type_simple("NICE_COLUMN", "", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, msg) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "NICE_COLUMN,START,END,VALUE", {"new":"true"}, self.admin_key)
    self.assertEqual(msg, "Column NICE_COLUMN not found in the original dataset")
