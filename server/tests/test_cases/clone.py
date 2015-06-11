import time

import helpers

from client import EpidbClient

class TestClone(helpers.TestCase):
  maxDiff = None
  def test_clone_experiment(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    eid1 = self.insert_experiment(epidb, "hg18_chr1_1")

    (s, info_original) = epidb.info(eid1, self.admin_key)
    info_original[0]['upload_info']['upload_end'] = '0'
    info_original[0]['upload_info']['upload_start'] = '0'
    info_original[0]['upload_info']['client_address'] = '0'
    info_original[0]['upload_info']['total_size'] = '0'


    self.assertEqual(info_original[0], {'description': 'desc1', 'data_type': 'peaks', 'format': 'CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK', 'sample_info': {'biosource_name': 'K562', 'karyotype': 'cancer', 'sex': 'F'}, 'technique': 'tech1', 'upload_info': {'total_size': '0', 'content_format': 'bed', 'done': 'true', 'user': 'test_admin', 'upload_end': '0', 'upload_start': '0', 'client_address': '0'}, 'project': 'ENCODE', 'genome': 'hg18', 'sample_id': 's1', 'epigenetic_mark': 'Methylation', '_id': 'e1', 'type': 'experiment', 'columns': [{'name': 'CHROMOSOME', 'column_type': 'string'}, {'name': 'START', 'column_type': 'integer'}, {'name': 'END', 'column_type': 'integer'}, {'name': 'NAME', 'column_type': 'string'}, {'name': 'SCORE', 'column_type': 'double'}, {'name': 'STRAND', 'column_type': 'category', 'items': '+,-,.',}, {'name': 'SIGNAL_VALUE', 'column_type': 'double'}, {'name': 'P_VALUE', 'column_type': 'double'}, {'name': 'Q_VALUE', 'column_type': 'double'}, {'name': 'PEAK', 'column_type': 'integer'}], 'name': 'hg18_chr1_1'})

    (s, id_clone_plus) = epidb.clone_dataset(eid1, "new experiment clone", "", "", "", "", "getting only the default values", "", {"new data": "true", "cool": "a lot"}, self.admin_key)

    self.assertSuccess(s, id_clone_plus)
    (s, info_clone_plus) = epidb.info(id_clone_plus, self.admin_key)
    info_clone_plus[0]['upload_info']['upload_end'] = '0'

    self.assertEqual(info_clone_plus[0], {'description': 'getting only the default values', 'format': 'CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK', 'extra_metadata': {'new data': 'true', 'cool': 'a lot'}, 'sample_info': {'biosource_name': 'K562', 'karyotype': 'cancer', 'sex': 'F'}, 'technique': 'tech1', 'upload_info': {'upload_end': '0', 'done': 'true', 'user': 'test_admin', 'cloned_from': 'e1'}, 'project': 'ENCODE', 'genome': 'hg18', 'sample_id': 's1', 'epigenetic_mark': 'Methylation', '_id': 'e2', 'type': 'experiment', 'columns': [{'name': 'CHROMOSOME', 'column_type': 'string'}, {'name': 'START', 'column_type': 'integer'}, {'name': 'END', 'column_type': 'integer'}, {'name': 'NAME', 'column_type': 'string'}, {'name': 'SCORE', 'column_type': 'double'}, {'name': 'STRAND', 'column_type': 'category', 'items': '+,-,.',}, {'name': 'SIGNAL_VALUE', 'column_type': 'double'}, {'name': 'P_VALUE', 'column_type': 'double'}, {'name': 'Q_VALUE', 'column_type': 'double'}, {'name': 'PEAK', 'column_type': 'integer'}], 'name': 'new experiment clone'})

    (s, _id_clone_same) =  epidb.clone_dataset(eid1, "clone of new experiment", "", "", "", "", "", "CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK", {}, self.admin_key)
    self.assertSuccess(s, _id_clone_same)

    (s, info_clone) = epidb.info(_id_clone_same, self.admin_key)
    self.assertSuccess(s, info_clone)
    info_clone[0]['upload_info']['upload_end'] = '0'

    self.assertEqual(info_clone[0], {'description': 'desc1', 'format': 'CHROMOSOME,START,END,NAME,SCORE,STRAND,SIGNAL_VALUE,P_VALUE,Q_VALUE,PEAK', 'sample_info': {'biosource_name': 'K562', 'karyotype': 'cancer', 'sex': 'F'}, 'technique': 'tech1', 'upload_info': {'upload_end': '0', 'done': 'true', 'user': 'test_admin', 'cloned_from': 'e1'}, 'project': 'ENCODE', 'genome': 'hg18', 'sample_id': 's1', 'epigenetic_mark': 'Methylation', '_id': 'e3', 'type': 'experiment', 'columns': [{'name': 'CHROMOSOME', 'column_type': 'string'}, {'name': 'START', 'column_type': 'integer'}, {'name': 'END', 'column_type': 'integer'}, {'name': 'NAME', 'column_type': 'string'}, {'name': 'SCORE', 'column_type': 'double'}, {'name': 'STRAND', 'column_type': 'category', 'items': '+,-,.', 'name': 'STRAND'}, {'name': 'SIGNAL_VALUE', 'column_type': 'double'}, {'name': 'P_VALUE', 'column_type': 'double'}, {'name': 'Q_VALUE', 'column_type': 'double'}, {'name': 'PEAK', 'column_type': 'integer'}], 'name': 'clone of new experiment'})


  def test_clone_annotatation(self):
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

    (s, m) = epidb.create_column_type_simple("NICE_COLUMN", "",  "string", self.admin_key)
    self.assertSuccess(s,m)
    (s, msg) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "NICE_COLUMN,START,END,VALUE", {"new":"true"}, self.admin_key)
    self.assertEqual(msg, 'Column CHROMOSOME can not be renamed. Columns CHROMOSOME,START, and END are immutable.')


  def test_invalid_format_size(self):
    epidb = EpidbClient()
    self.init_base(epidb)
    aid1 = self.insert_annotation(epidb, "Cpg Islands All Fields")

    (s, m) = epidb.create_column_type_simple("CPG_ISLAND_NAME", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("CPG_NUM", "", "integer", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("GC_NUM", "", "integer", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("PER_CG", "", "double", self.admin_key)
    self.assertSuccess(s,m)

    (s, clone_id) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "CHROMOSOME,START,END,CPG_ISLAND_NAME,LENGTH,CPG_NUM,GC_NUM,PER_CPG,PER_CG,OBS_EXP", {"new":"true"}, self.admin_key)
    self.assertSuccess(s, clone_id)

    s, info = epidb.info(clone_id, self.admin_key)

    info[0]['upload_info']['upload_end'] = '0'
    self.assertEqual(info[0], {'description': 'CpG islands are associated ... (all fields)', 'format': 'CHROMOSOME,START,END,CPG_ISLAND_NAME,LENGTH,CPG_NUM,GC_NUM,PER_CPG,PER_CG,OBS_EXP', 'extra_metadata': {'new': 'true'}, 'upload_info': {'upload_end': '0', 'done': 'true', 'user': 'test_admin', 'cloned_from': 'a3'}, 'genome': 'hg19', '_id': 'a4', 'type': 'annotation', 'columns': [{'name': 'CHROMOSOME', 'column_type': 'string'}, {'name': 'START', 'column_type': 'integer'}, {'name': 'END', 'column_type': 'integer'}, {'name': 'CPG_ISLAND_NAME', 'column_type': 'string'}, {'name': 'LENGTH', 'column_type': 'integer'}, {'name': 'CPG_NUM', 'column_type': 'integer'}, {'name': 'GC_NUM', 'column_type': 'integer'}, {'name': 'PER_CPG', 'column_type': 'double'}, {'name': 'PER_CG', 'column_type': 'double'}, {'name': 'OBS_EXP', 'column_type': 'double'}], 'name': 'New CpG Islands'})

  def test_better_names(self):
    epidb = EpidbClient()
    self.init_base(epidb)
    aid1 = self.insert_annotation(epidb, "Cpg Islands All Fields")

    (s, m) = epidb.create_column_type_simple("CPG_ISLAND_NAME", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("CPG_NUM", "", "integer", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("GC_NUM", "", "integer", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("PER_CG", "", "double", self.admin_key)
    self.assertSuccess(s,m)

    (s, clone_id) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "CHROMOSOME,START,END,CPG_ISLAND_NAME,LENGTH,CPG_NUM,GC_NUM,PER_CPG,PER_CG,OBS_EXP", {"new":"true"}, self.admin_key)
    self.assertSuccess(s, clone_id)
    self.assertEqual(clone_id, "a4")

  def test_invalid_format_types(self):
    epidb = EpidbClient()
    self.init_base(epidb)
    aid1 = self.insert_annotation(epidb, "Cpg Islands All Fields")

    (s, m) = epidb.create_column_type_simple("CPG_ISLAND_NAME", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("CPG_NUM_S", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("GC_NUM_S", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("PER_CG_S", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("OBS_EXP_S", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("PER_CPG_S", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("LENGTH_S", "", "string", self.admin_key)
    self.assertSuccess(s,m)

    (s, clone_id) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "CHROMOSOME,START,END,CPG_ISLAND_NAME,LENGTH_S,CPG_NUM_S,GC_NUM_S,PER_CPG_S,PER_CG_S,OBS_EXP_S", {"new":"true"}, self.admin_key)
    self.assertFailure(s, clone_id)
    self.assertEqual(clone_id, "The column 'LENGTH_S' (type: string) is incompatible with the original column 'length' (type: integer)")

    # --

    (s, m) = epidb.create_column_type_simple("CPG_NUM", "", "integer", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("GC_NUM", "", "integer", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("OBS_EXP_INT", "", "integer", self.admin_key)
    self.assertSuccess(s,m)

    (s, m) = epidb.create_column_type_simple("PER_CG", "", "double", self.admin_key)
    self.assertSuccess(s,m)

    (s, clone_id) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "CHROMOSOME,START,END,CPG_ISLAND_NAME,LENGTH,CPG_NUM,GC_NUM,PER_CPG,PER_CG,OBS_EXP_INT", {"new":"true"}, self.admin_key)
    self.assertFailure(s, clone_id)
    self.assertEqual(clone_id, "The column 'OBS_EXP_INT' (type: integer) is incompatible with the original column 'obsExp' (type: double)")

    # --
    (s, m) = epidb.create_column_type_range("OBS_EXP_RANGE", "", -1.0, 1.0, self.admin_key)
    self.assertSuccess(s,m)

    (s, clone_id) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "CHROMOSOME,START,END,CPG_ISLAND_NAME,LENGTH,CPG_NUM,GC_NUM,PER_CPG,PER_CG,OBS_EXP_RANGE", {"new":"true"}, self.admin_key)
    self.assertFailure(s, clone_id)
    self.assertEqual(clone_id, "The column 'OBS_EXP_RANGE' (type: range) is incompatible with the original column 'obsExp' (type: double)")

    # --
    (s, m) = epidb.create_column_type_category("OBS_EXP_CATEGORY", "", ["+", "-"], self.admin_key)
    self.assertSuccess(s,m)

    (s, clone_id) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "CHROMOSOME,START,END,CPG_ISLAND_NAME,LENGTH,CPG_NUM,GC_NUM,PER_CPG,PER_CG,OBS_EXP_CATEGORY", {"new":"true"}, self.admin_key)
    self.assertFailure(s, clone_id)
    self.assertEqual(clone_id, "The column 'OBS_EXP_CATEGORY' (type: category) is incompatible with the original column 'obsExp' (type: double)")

    # --
    (s, clone_id) = epidb.clone_dataset(aid1, "New CpG Islands", "", "", "", "", "", "CHROMOSOME,START,END,CPG_ISLAND_NAME,LENGTH,CPG_NUM,GC_NUM,PER_CPG,PER_CG,OBS_EXP", {"new":"true"}, self.admin_key)
    self.assertSuccess(s, clone_id)

  def test_wig_clone(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    res, _id = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res, id)

    (s, m) = epidb.create_column_type_simple("METHYLATION_LEVEL", "", "double", self.admin_key)
    self.assertSuccess(s,m)

    (s, clone_id) = epidb.clone_dataset(_id, "New Wig File", "", "", "", "", "", "CHROMOSOME,START,END,METHYLATION_LEVEL", None, self.admin_key)
    self.assertSuccess(s, clone_id)

    (status, wig_data) = epidb.select_regions('test_exp1', "hg19", None, None, None, None, None, None, None, self.admin_key)
    (s, req) = epidb.get_regions(wig_data, "CHROMOSOME,START,END,VALUE", self.admin_key)
    rs = self.get_regions_request(req)

    (status, wig_data) = epidb.select_regions('New Wig File', "hg19", None, None, None, None, None, None, None, self.admin_key)
    (s, req2) = epidb.get_regions(wig_data, "CHROMOSOME,START,END,METHYLATION_LEVEL", self.admin_key)

    rs2 = self.get_regions_request(req2)
    self.assertEqual(rs.split("\n")[0].split("\t")[3], "8.1235")
    self.assertEqual(rs, rs2)


  def test_wig_clone_calculated(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]
    res, _id = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res, id)

    (s, m) = epidb.create_column_type_simple("METHYLATION_LEVEL", "", "double", self.admin_key)
    self.assertSuccess(s,m)

    res = epidb.create_column_type_calculated("METHYLATION_LEVEL_SQRT", "Square root of the methylation level", "return math.sqrt(value_of('VALUE'))", self.admin_key)
    self.assertSuccess(res)

    (s, clone_id) = epidb.clone_dataset(_id, "New Wig File", "", "", "", "", "", "CHROMOSOME,START,END,METHYLATION_LEVEL_SQRT", None, self.admin_key)
    self.assertFailure(s, clone_id)
    self.assertEqual(clone_id, "The column 'METHYLATION_LEVEL_SQRT' (type: calculated) is incompatible with the original column 'VALUE' (type: double)")

    (s, clone_id) = epidb.clone_dataset(_id, "New Wig File", "", "", "", "", "", "CHROMOSOME,START,END,METHYLATION_LEVEL", None, self.admin_key)
    self.assertSuccess(s, clone_id)

    (status, wig_data) = epidb.select_regions('New Wig File', "hg19", None, None, None, None, None, None, None, self.admin_key)
    (s, req) = epidb.get_regions(wig_data, "CHROMOSOME,START,END,METHYLATION_LEVEL,METHYLATION_LEVEL_SQRT", self.admin_key)
    rs = self.get_regions_request(req)

    self.assertEqual(rs.split("\n")[0].split("\t")[3], "8.1235")
    self.assertEqual(rs.split("\n")[0].split("\t")[4], "2.850168")
    self.assertEqual(rs.split("\n")[6].split("\t")[3], "30.0000")
    self.assertEqual(rs.split("\n")[6].split("\t")[4], "5.477226")

