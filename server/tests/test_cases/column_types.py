import helpers

from client import EpidbClient


class TestColumnTypes(helpers.TestCase):

  def test_duplicate_column_type(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_simple("name2", "description", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("name2", "description", "integer", self.admin_key)
    self.assertFailure(res)


  def test_basic_column_types(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_simple("string_column", "description", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("integer_column", "description", "integer", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("double_column", "description", "double", self.admin_key)
    self.assertSuccess(res)


  def test_column_complex_types(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_range("score", "description", 0.0, 1.0, self.admin_key)
    self.assertSuccess(res)
    strand = ["+", "-"]
    res = epidb.create_column_type_category("STRAND", "description", strand, self.admin_key)
    self.assertSuccess(res)


  def test_list_column_types(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_simple("name", "description", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("string_column", "description", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("integer_column", "description", "integer", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("double_column", "description", "double", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("score", "description", 0.0, 1.0, self.admin_key)
    self.assertSuccess(res)
    strand = ["+", "-"]
    res = epidb.create_column_type_category("STRAND", "description", strand, self.admin_key)
    self.assertSuccess(res)

    res, column_types = epidb.list_column_types(self.admin_key)
    self.assertSuccess(res, column_types)

    self.assertEqual(column_types[0][1], "column type name: 'CHROMOSOME' type: 'string'")
    self.assertEqual(column_types[1][1], "column type name: 'START' type: 'integer'")
    self.assertEqual(column_types[2][1], "column type name: 'END' type: 'integer'")
    self.assertEqual(column_types[3][1], "column type name: 'VALUE' type: 'double'")
    self.assertEqual(column_types[4][1], "column type name: 'name' type: 'string'")
    self.assertEqual(column_types[5][1], "column type name: 'string_column' type: 'string'")
    self.assertEqual(column_types[6][1], "column type name: 'integer_column' type: 'integer'")
    self.assertEqual(column_types[7][1], "column type name: 'double_column' type: 'double'")
    self.assertEqual(column_types[8][1], "column type name: 'score' type: 'range' : 0.0000,1.0000")
    self.assertEqual(column_types[9][1], "column type name: 'STRAND' type: 'category' values: +,-")


  def test_no_ignore_if(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_simple("name", "description", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("string_column", "description", "string", self.admin_key)
    self.assertSuccess(res)

    res, column_types = epidb.list_column_types(self.admin_key)
    self.assertSuccess(res, column_types)

    self.assertEqual(column_types[0][1], "column type name: 'CHROMOSOME' type: 'string'")
    self.assertEqual(column_types[1][1], "column type name: 'START' type: 'integer'")
    self.assertEqual(column_types[2][1], "column type name: 'END' type: 'integer'")
    self.assertEqual(column_types[3][1], "column type name: 'VALUE' type: 'double'")
    self.assertEqual(column_types[4][1], "column type name: 'name' type: 'string'")
    self.assertEqual(column_types[5][1], "column type name: 'string_column' type: 'string'")


  def test_insert_experiment_fail(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    format = ",".join([
      "NAME",
      "SCORE",
      "STRAND",
      "SIGNAL_VALUE",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
                               "ENCODE", "desc1", "", format, None, self.admin_key)
    self.assertFailure(res)
    self.assertEqual(res[1], "120002:The CHROMOSOME is missing in the format. Please, inform the CHROMOSOME column in the Format.")


  def test_range_fail(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    format = ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND",
      "SIGNAL_VALUE",
      "P_VALUE_RANGE",
      "Q_VALUE",
      "PEAK"
    ])

    res = epidb.create_column_type_range("P_VALUE_RANGE", "p-value", 0.0, 10.0, self.admin_key)
    self.assertSuccess(res)

    regions_data = helpers.load_bed("hg19_chr1_1")
    res, msg = epidb.add_experiment("test_exp_fail", "hg19", "Methylation", sample_id, "tech1", "ENCODE", "desc1",
                                    regions_data, format, None, self.admin_key)
    self.assertFailure(res, msg)
    self.assertTrue("P_VALUE_RANGE" in msg)
    # self.assertEqual(msg, "Invalid value '69.6' for column P_VALUE")

  def test_category_fail(self):
    epidb = EpidbClient()
    self.init_base(epidb)

    sample_id = self.sample_ids[0]

    format = ",".join([
      "CHROMOSOME",
      "START",
      "END",
      "NAME",
      "SCORE",
      "STRAND_X",
      "SIGNAL_VALUE",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])

    res = epidb.create_column_type_category("STRAND_X", "strand of the region", ["X", "-"], self.admin_key)
    self.assertSuccess(res)

    regions_data = helpers.load_bed("hg19_chr1_1")
    res, msg = epidb.add_experiment("test_exp_fail2", "hg19", "Methylation", sample_id, "tech1", "ENCODE",
                                    "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res, msg)
    self.assertTrue("STRAND_X" in msg)

  def test_remove_column(self):
    epidb = EpidbClient()
    self.init(epidb)

    res, u1 = epidb.add_user("user1", "test1@example.com", "test", self.admin_key)
    self.assertSuccess(res, u1)
    user_key = u1[1]

    s, user_tmp = epidb.modify_user_admin(u1[0], "permission_level", "INCLUDE_COLLECTION_TERMS", self.admin_key)
    self.assertSuccess(s)

    res, c = epidb.create_column_type_simple("GENE_ID_ENTREZ", "rebimboca da parafuseta", "string", user_key)
    self.assertSuccess(res, c)

    res = epidb.remove(c, user_key)
    self.assertSuccess(res)

    res, s = epidb.search("GENE_ID_ENTREZ", None, user_key)
    self.assertSuccess(res, s)
    self.assertEqual(len(s), 0)

    res, s = epidb.search("rebimboca", None, user_key)
    self.assertSuccess(res, s)
    self.assertEqual(len(s), 0)

    res, s = epidb.search("parafuseta", None, user_key)
    self.assertSuccess(res, s)
    self.assertEqual(len(s), 0)

