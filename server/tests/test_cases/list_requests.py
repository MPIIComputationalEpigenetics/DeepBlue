import helpers
import re

from deepblue_client import DeepBlueClient


class TestListRequests(helpers.TestCase):

    def test_result_ids_reuse(self):
        """
        Test the request-ID's returned by list_requests
        """
        epidb = DeepBlueClient(address="localhost", port=31415)
        self.init_full(epidb)

        s, id = epidb.add_epigenetic_mark("DNA Methylation", "", {}, self.admin_key)
        self.assertSuccess(s, id)
        s, query_id = epidb.select_regions(None, "hg19", "DNA Methylation", None, None, None, "chr1", None, None, self.admin_key)
        self.assertSuccess(s, query_id)

        requests = []
        for i in xrange(0, 10):
            s, request_id = epidb.get_regions(query_id, "CHROMOSOME,START,END", self.admin_key)
            self.assertSuccess(s, request_id)
            requests.append(request_id)

        s, requests_list = epidb.list_requests(None, self.admin_key)
        self.assertSuccess(s, requests_list)

        for _id, status in requests_list:
            self.assertTrue(_id in requests)

        requests_ids = [r[0] for r in requests_list]

        self.assertEquals(len(requests_ids), 1)


    def test_result_ids_new(self):
        """
        Test the request-ID's returned by list_requests
        """
        epidb = DeepBlueClient(address="localhost", port=31415)
        self.init_full(epidb)

        s, id = epidb.add_epigenetic_mark("DNA Methylation", "", {}, self.admin_key)
        self.assertSuccess(s, id)

        requests = []
        for i in xrange(0, 10):
            s, query_id = epidb.select_regions(None, "hg19", "DNA Methylation", None, None, None, "chr"+str(i), None, None, self.admin_key)
            self.assertSuccess(s, query_id)
            s, request_id = epidb.get_regions(query_id, "CHROMOSOME,START,END", self.admin_key)
            self.assertSuccess(s, request_id)
            requests.append(request_id)

        s, requests_list = epidb.list_requests(None, self.admin_key)
        self.assertSuccess(s, requests_list)

        for _id, status in requests_list:
            self.assertTrue(_id in requests)

        requests_ids = [r[0] for r in requests_list]

        self.assertEquals(requests, requests_ids)

