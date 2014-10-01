import helpers

from client import EpidbClient


class TestColumnTypes(helpers.TestCase):

  def test_duplicate_column_type(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_simple("name", "description", ".", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("name", "description", ".", "integer", self.admin_key)
    self.assertFailure(res)


  def test_basic_column_types(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_simple("string_column", "description", ".", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("integer_column", "description", ".", "integer", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("double_column", "description", ".", "double", self.admin_key)
    self.assertSuccess(res)


  def test_column_complex_types(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_range("score", "description", ".", 0.0, 1.0, self.admin_key)
    self.assertSuccess(res)
    strand = ["+", "-"]
    res = epidb.create_column_type_category("STRAND", "description", ".", strand, self.admin_key)
    self.assertSuccess(res)


  def test_list_column_types(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_simple("name", "description", ".", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("string_column", "description", ".", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("integer_column", "description", ".", "integer", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("double_column", "description", ".", "double", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("score", "description", ".", 0.0, 1.0, self.admin_key)
    self.assertSuccess(res)
    strand = ["+", "-"]
    res = epidb.create_column_type_category("STRAND", "description", ".", strand, self.admin_key)
    self.assertSuccess(res)

    res, column_types = epidb.list_column_types(self.admin_key)
    self.assertSuccess(res, column_types)

    self.assertEqual(column_types[0][1], "column type name: 'CHROMOSOME' default: '' type: 'string'")
    self.assertEqual(column_types[1][1], "column type name: 'START' default: '' type: 'integer'")
    self.assertEqual(column_types[2][1], "column type name: 'END' default: '' type: 'integer'")
    self.assertEqual(column_types[3][1], "column type name: 'VALUE' default: '' type: 'double'")
    self.assertEqual(column_types[4][1], "column type name: 'name' default: '.' type: 'string'")
    self.assertEqual(column_types[5][1], "column type name: 'string_column' default: '.' type: 'string'")
    self.assertEqual(column_types[6][1], "column type name: 'integer_column' default: '.' type: 'integer'")
    self.assertEqual(column_types[7][1], "column type name: 'double_column' default: '.' type: 'double'")
    self.assertEqual(column_types[8][1], "column type name: 'score' default: '.' type: 'range' : 0.0000,1.0000")
    self.assertEqual(column_types[9][1], "column type name: 'STRAND' default: '.' type: 'category' values: +,-")


  def test_no_ignore_if(self):
    epidb = EpidbClient()
    self.init(epidb)

    res = epidb.create_column_type_simple("name", "description", None, "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("string_column", "description", "", "string", self.admin_key)
    self.assertSuccess(res)

    res, column_types = epidb.list_column_types(self.admin_key)
    self.assertSuccess(res, column_types)

    self.assertEqual(column_types[0][1], "column type name: 'CHROMOSOME' default: '' type: 'string'")
    self.assertEqual(column_types[1][1], "column type name: 'START' default: '' type: 'integer'")
    self.assertEqual(column_types[2][1], "column type name: 'END' default: '' type: 'integer'")
    self.assertEqual(column_types[3][1], "column type name: 'VALUE' default: '' type: 'double'")
    self.assertEqual(column_types[4][1], "column type name: 'name' default: '' type: 'string'")
    self.assertEqual(column_types[5][1], "column type name: 'string_column' default: '' type: 'string'")


  def test_insert_experiment(self):
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

    res = epidb.create_column_type_simple("NAME", "name of the region", ".", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("SCORE", "score of the region", "0", "integer", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_category("STRAND", "strand of the region", ".", ["+", "-"], self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("SIGNAL_VALUE", "signal value", ".", 0.0, 100.0, self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("P_VALUE", "p-value", "-1", 0.0, 200.0, self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("Q_VALUE", "q-value", "-1", 0.0, 100.0, self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("PEAK", "peak", "-1", 0.0, 100.0, self.admin_key)
    self.assertSuccess(res)

    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
                               "ENCODE", "desc1", "", format, None, self.admin_key)
    self.assertSuccess(res)


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
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])

    res = epidb.create_column_type_simple("NAME", "name of the region", ".", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("SCORE", "score of the region", "0", "integer", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_category("STRAND", "strand of the region", ".", ["+", "-"], self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("SIGNAL_VALUE", "signal value", ".", 0.0, 100.0, self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("P_VALUE", "p-value", "-1", 0.0, 10.0, self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("Q_VALUE", "q-value", "-1", 0.0, 100.0, self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("PEAK", "peak", "-1", 0.0, 100.0, self.admin_key)
    self.assertSuccess(res)

    regions_data = helpers.load_bed("hg19_chr1_1")
    res, msg = epidb.add_experiment("test_exp_fail", "hg19", "Methylation", sample_id, "tech1", "ENCODE", "desc1",
                                    regions_data, format, None, self.admin_key)
    self.assertFailure(res, msg)
    self.assertTrue("p_value" in msg.lower())
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
      "STRAND",
      "SIGNAL_VALUE",
      "P_VALUE",
      "Q_VALUE",
      "PEAK"
    ])

    res = epidb.create_column_type_simple("NAME", "name of the region", ".", "string", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_simple("SCORE", "score of the region", "0", "integer", self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_category("STRAND", "strand of the region", ".", ["X", "-"], self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("SIGNAL_VALUE", "signal value", ".", 0.0, 100.0, self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("P_VALUE", "p-value", "-1", 0.0, 200.0, self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("Q_VALUE", "q-value", "-1", 0.0, 100.0, self.admin_key)
    self.assertSuccess(res)
    res = epidb.create_column_type_range("PEAK", "peak", "-1", 0.0, 100.0, self.admin_key)
    self.assertSuccess(res)

    regions_data = helpers.load_bed("hg19_chr1_1")
    res, msg = epidb.add_experiment("test_exp_fail2", "hg19", "Methylation", sample_id, "tech1", "ENCODE",
                                    "desc1", regions_data, format, None, self.admin_key)
    self.assertFailure(res, msg)
    self.assertTrue("strand" in msg.lower())
    # self.assertEqual(res[1], "Invalid value '+' for column STRAND")
