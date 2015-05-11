import helpers
import client
import re


class TestListRequests(helpers.TestCase):

    def test_result_ids(self):
        """Test the request-ID's returned by list_requests
        """
        epidb = client.EpidbClient()
        self.init_full(epidb)

        s, id = epidb.add_epigenetic_mark("DNA Methylation", "", self.admin_key)
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

        id_field = re.compile("_id: \"(r[0-9]*)\"")
        for id, status in requests_list:
            id_field_match = id_field.match(id)
            id = id_field_match.group(1)
            self.assertTrue(id in requests)
            requests.remove(id)
        self.assertTrue(len(requests) == 0)