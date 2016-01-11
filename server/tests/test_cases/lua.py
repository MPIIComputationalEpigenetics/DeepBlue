import time

import helpers

from deepblue_client import DeepBlueClient
import data_info


class TestExperiments(helpers.TestCase):

  def test_calculated_string(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    sample_id = self.sample_ids[0]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res)

    res, qid1 = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    res = epidb.create_column_type_calculated("calculated", "description", "return value_of('CHROMOSOME') .. ' - ' .. value_of('START') .. ' - ' .. value_of('END') .. ' - ' .. value_of('VALUE')", self.admin_key)
    self.assertSuccess(res)


    (s, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END,VALUE, calculated", self.admin_key)
    self.assertSuccess(s, req)
    regions = self.get_regions_request(req)
    r =regions.split("\n")[0].split("\t")[4]
    self.assertEqual(r, 'chr1 - 0 - 10 - 8.1234569549561')


  def test_calculated_math(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    sample_id = self.sample_ids[0]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res)

    res, qid1 = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    res = epidb.create_column_type_calculated("calculated", "description", "return value_of('START') - value_of('END') * value_of('VALUE')", self.admin_key)
    self.assertSuccess(res)


    (s, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END,VALUE, calculated", self.admin_key)
    regions = self.get_regions_request(req)

    r =regions.split("\n")[0].split("\t")[4]
    self.assertEqual(r, '-81.234570')


  def test_calculated_metafield(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    sample_id = self.sample_ids[0]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res)

    res, qid1 = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    res = epidb.create_column_type_calculated("calculated", "description", "return 'EM and Name: - '.. value_of('@EPIGENETIC_MARK') .. ' - ' .. value_of('@NAME')", self.admin_key)
    self.assertSuccess(res)


    (s, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END,VALUE, calculated", self.admin_key)
    self.assertSuccess(s, req)
    regions = self.get_regions_request(req)
    r =regions.split("\n")[0].split("\t")[4]

    self.assertEqual(r, 'EM and Name: - Methylation - test_exp1')

  def test_wrong_column_creation(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    sample_id = self.sample_ids[0]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res)

    res, qid1 = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    # It is missing a '  before the EM
    res = epidb.create_column_type_calculated("calculated", "description", "return EM and Name: - '.. value_of('@EPIGENETIC_MARK') .. ' - ' .. value_of('@NAME')", self.admin_key)
    self.assertFailure(res)
    self.assertEqual(res[1], '[string "function row_value()..."]:2: \'<name>\' expected near \'-\'')


  def test_calculated_get_region(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    sample_id = self.sample_ids[0]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res)

    res, qid1 = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    (s, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END,VALUE,@CALCULATED(return math.log(value_of('VALUE'))),@CALCULATED(em = value_of('@EPIGENETIC_MARK') if em == 'Methylation' then return 'it is methylation!' else return 'it is not methylation' end)", self.admin_key)

    self.assertSuccess(s, req)

    regions_1 = self.get_regions_request(req)

    r0 = regions_1.split('\n')[0].split('\t')[3]
    r1 = regions_1.split('\n')[0].split('\t')[4]
    r2 = regions_1.split('\n')[0].split('\t')[5]

    self.assertEqual(r0, '8.1235')
    self.assertEqual(r1, '2.094756')
    self.assertEqual(r2, 'it is methylation!')

  def test_error_calculated_get_region(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    sample_id = self.sample_ids[0]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res)

    res, qid1 = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    # missing column definition
    (s, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END,VALUE,@CALCULATED(return log(value_of('VALUE'))), calculated,@CALCULATED(em = value_of('@EPIGENETIC_MARK') if em == 'Methylation' then return 'it is methylation!' else return 'it is not methylation' end)", self.admin_key)
    self.assertSuccess(s, req)

    msg = self.get_regions_request_error(req)
    self.assertEqual(msg, "123000:Column name 'calculated' does not exist.")

    # missing math. before log
    (s, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END,VALUE,@CALCULATED(return log(value_of('VALUE'))), @CALCULATED(em = value_of('@EPIGENETIC_MARK') if em == 'Methylation' then return 'it is methylation!' else return 'it is not methylation' end)", self.admin_key)
    self.assertSuccess(s, req)

    msg = self.get_regions_request_error(req)
    self.assertEqual(msg, '[string "function row_value()..."]:2: attempt to call global \'log\' (a nil value)')


  def test_error_maximum_number_of_instructions(self):
    epidb = DeepBlueClient(address="localhost", port=31415)
    self.init_base(epidb)
    sample_id = self.sample_ids[0]

    # adding two experiments with the same data should work
    res = epidb.add_experiment("test_exp1", "hg19", "Methylation", sample_id, "tech1",
              "ENCODE", "desc1", "", "wig", {"__local_file__": "../tests/data/wig/scores1.wig"}, self.admin_key)
    self.assertSuccess(res)

    res, qid1 = epidb.select_regions("test_exp1", "hg19", None, None, None, None, None, None, None, self.admin_key)
    self.assertSuccess(res, qid1)

    # missing math. before log
    (s, req) = epidb.get_regions(qid1, "CHROMOSOME,START,END,VALUE,@CALCULATED(while 1 do math.log(value_of('VALUE')) end return 'never')", self.admin_key)
    self.assertSuccess(s, req)

    msg = self.get_regions_request_error(req)
    self.assertEqual(msg, 'The maximum number of instructions has been reached')
